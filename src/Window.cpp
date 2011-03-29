#include <windows.h>

#include <gl\gl.h>

#include "Settings.h"
#include "Main.h"
#include "Error.h"
#include "Window.h"




#define Z_BUFFER_BITS 32

#define USE_PASSWORD_THREAD false






HWND CWindow::hwnd=NULL;
HDC CWindow::hDC=NULL;
HGLRC CWindow::hRC=NULL;
int CWindow::winwidth=0;
int CWindow::winheight=0;








CWindow::CWindow()
{
}





CWindow::~CWindow()
{
}





bool CWindow::ChangeVideoMode(int width, int height)
{
	DEVMODE dmScreenSettings;
	ZeroMemory(&dmScreenSettings,sizeof(dmScreenSettings));
	dmScreenSettings.dmSize=sizeof(dmScreenSettings);
	dmScreenSettings.dmPelsWidth	= (DWORD)width;
	dmScreenSettings.dmPelsHeight	= (DWORD)height;
	dmScreenSettings.dmFields		= DM_PELSWIDTH | DM_PELSHEIGHT;
	if (ChangeDisplaySettings(&dmScreenSettings,CDS_FULLSCREEN)!=DISP_CHANGE_SUCCESSFUL)
		return false;
	return true;
}





bool CWindow::RestoreVideoMode()
{
	if (ChangeDisplaySettings(NULL,0)!=DISP_CHANGE_SUCCESSFUL)
		return false;
	return true;
}





static BOOL VerifyPassword(HWND hwnd)
{
	OSVERSIONINFO osv; osv.dwOSVersionInfoSize=sizeof(osv); GetVersionEx(&osv);
	if (osv.dwPlatformId==VER_PLATFORM_WIN32_NT) return TRUE;
	HINSTANCE hpwdcpl=::LoadLibrary("PASSWORD.CPL");
	if (hpwdcpl==NULL)
		return TRUE;
	typedef BOOL (WINAPI *VERIFYSCREENSAVEPWD)(HWND hwnd);
	VERIFYSCREENSAVEPWD VerifyScreenSavePwd;
	VerifyScreenSavePwd=(VERIFYSCREENSAVEPWD)GetProcAddress(hpwdcpl,"VerifyScreenSavePwd");
	if (VerifyScreenSavePwd==NULL)
	{
		FreeLibrary(hpwdcpl);
		return TRUE;
	}
	BOOL bres=VerifyScreenSavePwd(hwnd);
	FreeLibrary(hpwdcpl);
	return bres;
}





DWORD WINAPI CWindow::PasswordThread(LPVOID lpParameter)
{
	BOOL CanClose=TRUE;
	HWND hWnd=(HWND)lpParameter;
	if ((GetTickCount()-CSettings::InitTime)>(1000*CSettings::PasswordDelay))
	{
		CSettings::StartDialog();
		CanClose=VerifyPassword(hWnd);
		CSettings::EndDialog();
	}
	if (CanClose)
	{
		CSettings::PasswordOK=TRUE;
		CSettings::CloseSaverWindow();
	}
	return 0;
}





LRESULT CALLBACK CWindow::WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_CREATE:
		if (ScrMode==smSaver && !DEBUG)
			SystemParametersInfo(SPI_SETSCREENSAVERRUNNING,1,NULL,0);
		CSettings::hwnd=hWnd;
		GetCursorPos(&(CSettings::InitCursorPos));
		CSettings::InitTime=GetTickCount();
		break;
	case WM_ACTIVATE:
	case WM_ACTIVATEAPP:
	case WM_NCACTIVATE:
		if (ScrMode==smSaver && !CSettings::IsDialogActive && LOWORD(wParam)==WA_INACTIVE && !DEBUG)
			CSettings::CloseSaverWindow();
		break;
	case WM_SETCURSOR:
		if (ScrMode==smSaver && !CSettings::IsDialogActive && !DEBUG)
			SetCursor(NULL);
		else
			SetCursor(LoadCursor(NULL,IDC_ARROW));
		break;
	case WM_LBUTTONDOWN:
	case WM_MBUTTONDOWN:
	case WM_RBUTTONDOWN:
	case WM_KEYDOWN:
		if (ScrMode==smSaver && !CSettings::IsDialogActive && !DEBUG)
			CSettings::CloseSaverWindow();
		break;
	case WM_MOUSEMOVE:
		if (ScrMode==smSaver && !CSettings::IsDialogActive && !DEBUG)
		{
			POINT pt; GetCursorPos(&pt);
			int dx=pt.x-CSettings::InitCursorPos.x; if (dx<0) dx=-dx;
			int dy=pt.y-CSettings::InitCursorPos.y; if (dy<0) dy=-dy;
			if (dx>(int)CSettings::MouseThreshold || dy>(int)CSettings::MouseThreshold)
				CSettings::CloseSaverWindow();
		}
		break;
	case WM_SYSCOMMAND:
		if (ScrMode==smSaver)
		{
			if (wParam==SC_SCREENSAVE) return FALSE;
			if (wParam==SC_CLOSE && !DEBUG) return FALSE;
		}
		break;
	case WM_CLOSE:
		if (ScrMode==smSaver)
		{
			if ((CSettings::ReallyClose || DEBUG) && !CSettings::IsDialogActive)
			{
				CSettings::ReallyClose=FALSE;
				if (CSettings::PasswordOK)
				{
					DestroyWindow(hWnd);
					CSettings::hwnd=NULL;
				}
				else
				{
					if (USE_PASSWORD_THREAD)
					{
						DWORD thID;
						CreateThread(NULL,0,PasswordThread,(LPVOID)hWnd,0,&thID);
					}
					else
					{
						PasswordThread((LPVOID)hWnd);
					}
				}
			}
			return FALSE;
		}
		break;
	case WM_DESTROY:
		if (ScrMode==smSaver && !DEBUG)
			SystemParametersInfo(SPI_SETSCREENSAVERRUNNING,0,NULL,0);
		PostQuitMessage(0);
		break;
	}
	return DefWindowProc(hWnd,uMsg,wParam,lParam);
}





bool CWindow::Create(HWND hParent)
{
	if (hwnd!=NULL)
	{
		CError::LogError(ERROR_CODE,"Window already exists, can not create new.");
		return false;
	}
	WNDCLASS wc;
	ZeroMemory(&wc,sizeof(wc));
	wc.style				= CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wc.lpfnWndProc			= WindowProc;
	wc.cbClsExtra			= 0;
	wc.cbWndExtra			= 0;
	wc.hInstance			= GetModuleHandle(NULL);
	wc.hIcon				= NULL;
	wc.hCursor				= NULL;
	wc.hbrBackground		= (HBRUSH)GetStockObject(BLACK_BRUSH);
	wc.lpszMenuName			= NULL;
	wc.lpszClassName		= APPNAME " window class";
	if (!RegisterClass(&wc))
	{
		CError::LogError(ERROR_CODE,"Unable to register window class.");
		return false;
	}
	DWORD dwStyle=0;
	DWORD dwExStyle=0;
	int width, height;
	int wadd, hadd;
	RECT parent_rect;
	if (ScrMode==smPreview)
	{
		GetWindowRect(hParent,&parent_rect);
		width=parent_rect.right-parent_rect.left; height=parent_rect.bottom-parent_rect.top;
		dwStyle=WS_CHILD;
		dwExStyle=0;
	}
	else
	{
		switch (CSettings::VideoMode)
		{
		default:
		case 0:
			width=640;
			height=480;
			break;
		case 1:
			width=800;
			height=600;
			break;
		case 2:
			width=1024;
			height=768;
			break;
		}
		if (CSettings::DefaultRes)
		{
			HWND hwDesktop=GetDesktopWindow();
			GetWindowRect(hwDesktop,&parent_rect);
			width=parent_rect.right-parent_rect.left;
			height=parent_rect.bottom-parent_rect.top;
		}
		if (DEBUG)
		{
			width/=2; height/=2;
			dwStyle=WS_POPUPWINDOW|WS_CAPTION|WS_MINIMIZEBOX;
			dwExStyle=0;
		}
		else
		{
			dwExStyle=WS_EX_TOPMOST;
			dwStyle=WS_POPUP;
			if (!CSettings::DefaultRes)
			{
				if (!ChangeVideoMode(width,height))
				{
					CError::LogError(ERROR_CODE,"Unable to switch to selected video mode (resolution and/or color depth).");
					return false;
				}
			}
		}
	}
	{
		RECT win_rect;
		win_rect.top=win_rect.left=0;
		win_rect.right=width; win_rect.bottom=height;
		AdjustWindowRectEx(&win_rect,dwStyle,FALSE,dwExStyle);
		wadd= (win_rect.right-win_rect.left) - width;
		hadd= (win_rect.bottom-win_rect.top) - height;
	}
	if (!(hwnd=CreateWindowEx(	dwExStyle,			// Extended Style For The Window
								wc.lpszClassName,	// Class Name
								APPNAME,			// Window Title
								dwStyle |			// Defined Window Style
								WS_CLIPSIBLINGS |	// Required Window Style
								WS_CLIPCHILDREN,	// Required Window Style
								0, 0,				// Window Position
								width+wadd,			// Selected Width
								height+hadd,		// Selected Height
								hParent,			// Parent Window
								NULL,				// No Menu
								wc.hInstance,		// Instance
								NULL )))			// Dont Pass Anything To WM_CREATE
	{
		CError::LogError(ERROR_CODE,"Window creation error.");
		return false;								// Return FALSE
	}
	if (!(hDC=GetDC(hwnd)))
	{
		CError::LogError(ERROR_CODE,"Unable to retrieve window device context.");
		return false;
	}
	GLuint PixelFormat;
	PIXELFORMATDESCRIPTOR pfd=				// pfd Tells Windows How We Want Things To Be
	{
		sizeof(PIXELFORMATDESCRIPTOR),				// Size Of This Pixel Format Descriptor
		1,											// Version Number
		PFD_DRAW_TO_WINDOW |						// Format Must Support Window
		PFD_SUPPORT_OPENGL |						// Format Must Support OpenGL
		PFD_DOUBLEBUFFER,							// Must Support Double Buffering
		PFD_TYPE_RGBA,								// Request An RGBA Format
		0,			//bpp							// Select Our Color Depth
		0, 0, 0, 0, 0, 0,							// Color Bits Ignored
		0,											// No Alpha Buffer
		0,											// Shift Bit Ignored
		0,											// No Accumulation Buffer
		0, 0, 0, 0,									// Accumulation Bits Ignored
		Z_BUFFER_BITS,								// 16Bit Z-Buffer (Depth Buffer)  
		0,											// No Stencil Buffer
		0,											// No Auxiliary Buffer
		PFD_MAIN_PLANE,								// Main Drawing Layer
		0,											// Reserved
		0, 0, 0										// Layer Masks Ignored
	};
	pfd.cColorBits=(unsigned char)0;
	if (!(PixelFormat=ChoosePixelFormat(hDC,&pfd)))
	{
		CError::LogError(ERROR_CODE,"No suitable OpenGL pixel format found.");
		return false;
	}
	if (!SetPixelFormat(hDC,PixelFormat,&pfd))
	{
		CError::LogError(ERROR_CODE,"Unable to set an OpenGL pixel format to the window DC.");
		return false;
	}
	if (!(hRC=wglCreateContext(hDC)))
	{
		CError::LogError(ERROR_CODE,"Unable to create an OpenGL rendering context.");
		return false;
	}
	if (!wglMakeCurrent(hDC,hRC))
	{
		CError::LogError(ERROR_CODE,"Unable to select the OpenGL rendering context into the window DC.");
		return false;
	}
	glViewport(0,0,width,height);
	ShowWindow(hwnd, SW_SHOWNORMAL);
	winwidth=width;
	winheight=height;
	return true;
}





void CWindow::Destroy()
{
	if (hRC)
	{
		wglMakeCurrent(NULL,NULL);
		wglDeleteContext(hRC);
	}
	if (hwnd)
	{
		if (hDC)
		{
			ReleaseDC(hwnd,hDC);
		}
		DestroyWindow(hwnd);
		MessagePump();
	}
	hRC=NULL;
	hDC=NULL;
	hwnd=NULL;

	if (ScrMode==smSaver)
	{
		if (!DEBUG && !CSettings::DefaultRes)
		{
			RestoreVideoMode();
		}
	}
	winwidth=winheight=0;
}
