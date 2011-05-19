/*
 * Copyright (C) 2003-2011 Victor Semionov
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *  * Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *  * Neither the name of the copyright holder nor the names of the contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <windows.h>

#include <gl/gl.h>

#include <gl/glext.h>
#include <gl/wglext.h>

#include "Settings.h"
#include "VideoBase.h"
#include "Main.h"
#include "Error.h"
#include "Window.h"




#define COLOR_BUFFER_BITS 24
#define ALPHA_BUFFER_BITS 8
#define Z_BUFFER_BITS 16
#define MULTI_SAMPLES 4
#define MULTI_SAMPLES_FALLBACK 2

#define USE_PASSWORD_THREAD false

#define WINDOW_CLASS_NAME APPNAME " window class"






HWND CWindow::hwnd=NULL;
HDC CWindow::hDC=NULL;
HGLRC CWindow::hRC=NULL;
int CWindow::winwidth=0;
int CWindow::winheight=0;
bool CWindow::multisample=false;








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





bool CWindow::isWGLExtensionSupported(const char *extension)
{
	const size_t extlen = strlen(extension);
	const char *supported = NULL;

	// Try To Use wglGetExtensionStringARB On Current DC, If Possible
	PROC wglGetExtString = wglGetProcAddress("wglGetExtensionsStringARB");

	if (wglGetExtString)
		supported = ((char*(__stdcall*)(HDC))wglGetExtString)(wglGetCurrentDC());

	// If That Failed, Try Standard Opengl Extensions String
	if (supported == NULL)
		supported = (char*)glGetString(GL_EXTENSIONS);

	// If That Failed Too, Must Be No Extensions Supported
	if (supported == NULL)
		return false;

	// Begin Examination At Start Of String, Increment By 1 On False Match
	for (const char* p = supported; ; p++)
	{
		// Advance p Up To The Next Possible Match
		p = strstr(p, extension);

		if (p == NULL)
			return false;															// No Match

		// Make Sure That Match Is At The Start Of The String Or That
		// The Previous Char Is A Space, Or Else We Could Accidentally
		// Match "wglFunkywglExtension" With "wglExtension"

		// Also, Make Sure That The Following Character Is Space Or NULL
		// Or Else "wglExtensionTwo" Might Match "wglExtension"
		if ((p==supported || p[-1]==' ') && (p[extlen]=='\0' || p[extlen]==' '))
			return true;															// Match
	}
}





bool CWindow::InitMultisample(HDC hDC, GLuint *pixel_format)
{
	if (!isWGLExtensionSupported("WGL_ARB_multisample"))
	{
		return false;
	}

	PFNWGLCHOOSEPIXELFORMATARBPROC wglChoosePixelFormatARB = (PFNWGLCHOOSEPIXELFORMATARBPROC)wglGetProcAddress("wglChoosePixelFormatARB");
	if (!wglChoosePixelFormatARB)
	{
		return false;
	}

	int		pixelFormat;
	int		valid;
	UINT	numFormats;
	float	fAttributes[] = {0,0};

	int iAttributes[] =
	{
		WGL_DRAW_TO_WINDOW_ARB,		GL_TRUE,
		WGL_ACCELERATION_ARB,		WGL_FULL_ACCELERATION_ARB,
		WGL_SUPPORT_OPENGL_ARB,		GL_TRUE,
		WGL_DOUBLE_BUFFER_ARB,		GL_TRUE,
		WGL_PIXEL_TYPE_ARB,			WGL_TYPE_RGBA_ARB,
		WGL_COLOR_BITS_ARB,			COLOR_BUFFER_BITS,
		WGL_ALPHA_BITS_ARB,			ALPHA_BUFFER_BITS,
		WGL_DEPTH_BITS_ARB,			Z_BUFFER_BITS,
		WGL_STENCIL_BITS_ARB,		0,
		WGL_SAMPLE_BUFFERS_ARB,		GL_TRUE,
		WGL_SAMPLES_ARB,			MULTI_SAMPLES,
		0,0
	};

	valid = wglChoosePixelFormatARB(hDC,iAttributes,fAttributes,1,&pixelFormat,&numFormats);

	if (valid && numFormats >= 1)
	{
		*pixel_format = pixelFormat;
		return true;
	}

	iAttributes[21] = MULTI_SAMPLES_FALLBACK;
	valid = wglChoosePixelFormatARB(hDC,iAttributes,fAttributes,1,&pixelFormat,&numFormats);
	if (valid && numFormats >= 1)
	{
		*pixel_format = pixelFormat;
		return true;
	}

	return  false;
}





bool CWindow::CreateSaverWindow(HWND hParent, DWORD dwStyle, DWORD dwExStyle, int width, int height, const GLuint *pformat)
{
	if (hwnd!=NULL)
	{
		CError::LogError(ERROR_CODE,"Window already exists, can not create new.");
		return false;
	}

	if (!(hwnd=CreateWindowEx(	dwExStyle,			// Extended Style For The Window
								WINDOW_CLASS_NAME,	// Class Name
								APPNAME,			// Window Title
								dwStyle,			// Defined Window Style
								0, 0,				// Window Position
								width,				// Selected Width
								height,				// Selected Height
								hParent,			// Parent Window
								NULL,				// No Menu
								AppInstance,		// Instance
								NULL )))			// Dont Pass Anything To WM_CREATE
	{
		CError::LogError(ERROR_CODE,"Window creation error.");
		return false;
	}

	if (!(hDC=GetDC(hwnd)))
	{
		CError::LogError(ERROR_CODE,"Unable to retrieve window device context.");
		return false;
	}

	GLuint PixelFormat;

	PIXELFORMATDESCRIPTOR pfd=						// pfd Tells Windows How We Want Things To Be
	{
		sizeof(PIXELFORMATDESCRIPTOR),				// Size Of This Pixel Format Descriptor
		1,											// Version Number
		PFD_DRAW_TO_WINDOW |						// Format Must Support Window
		PFD_SUPPORT_OPENGL |						// Format Must Support OpenGL
		PFD_DOUBLEBUFFER,							// Must Support Double Buffering
		PFD_TYPE_RGBA,								// Request An RGBA Format
		COLOR_BUFFER_BITS,	//bpp					// Select Our Color Depth
		0, 0, 0, 0, 0, 0,							// Color Bits Ignored
		ALPHA_BUFFER_BITS,							// Alpha Buffer
		0,											// Shift Bit Ignored
		0,											// No Accumulation Buffer
		0, 0, 0, 0,									// Accumulation Bits Ignored
		Z_BUFFER_BITS,								// Z-Buffer (Depth Buffer)
		0,											// No Stencil Buffer
		0,											// No Auxiliary Buffer
		PFD_MAIN_PLANE,								// Main Drawing Layer
		0,											// Reserved
		0, 0, 0										// Layer Masks Ignored
	};
	pfd.cColorBits=(unsigned char)0;

	if (!pformat)
	{
		if (!(PixelFormat=ChoosePixelFormat(hDC,&pfd)))
		{
			CError::LogError(ERROR_CODE,"No suitable OpenGL pixel format found.");
			return false;
		}
	}
	else
	{
		PixelFormat=*pformat;
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

	return true;
}





void CWindow::DestroySaverWindow()
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
}





int CWindow::prev_p2(int a)
{
	int p=1;
	while (1<<p<=a) p++;
	return 1<<(p-1);
}





bool CWindow::Create(HWND hParent)
{
	WNDCLASS wc;
	ZeroMemory(&wc,sizeof(wc));
	wc.style				= CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wc.lpfnWndProc			= WindowProc;
	wc.cbClsExtra			= 0;
	wc.cbWndExtra			= 0;
	wc.hInstance			= AppInstance;
	wc.hIcon				= NULL;
	wc.hCursor				= NULL;
	wc.hbrBackground		= (HBRUSH)GetStockObject(BLACK_BRUSH);
	wc.lpszMenuName			= NULL;
	wc.lpszClassName		= WINDOW_CLASS_NAME;
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
			width=prev_p2(width-1); height=width*3/4;
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

	dwStyle|=WS_CLIPSIBLINGS;	// Required Window Style
	dwStyle|=WS_CLIPCHILDREN;	// Required Window Style

	RECT win_rect;
	win_rect.top=win_rect.left=0;
	win_rect.right=width; win_rect.bottom=height;
	AdjustWindowRectEx(&win_rect,dwStyle,FALSE,dwExStyle);
	wadd=(win_rect.right-win_rect.left)-width;
	hadd=(win_rect.bottom-win_rect.top)-height;

	if (!CreateSaverWindow(hParent, dwStyle, dwExStyle, width+wadd, height+hadd, NULL))
	{
		return false;
	}

	multisample=false;
	if (CVideoBase::GetOptAntialiasing())
	{
		GLuint pixel_format;
		if (InitMultisample(hDC, &pixel_format))
		{
			DestroySaverWindow();
			if (!CreateSaverWindow(hParent, dwStyle, dwExStyle, width+wadd, height+hadd, &pixel_format))
			{
				return false;
			}
			multisample=true;
		}
	}

	glViewport(0, 0, width, height);

	ShowWindow(hwnd, SW_SHOWNORMAL);

	winwidth=width;
	winheight=height;

	return true;
}





void CWindow::Destroy()
{
	DestroySaverWindow();

	UnregisterClass(WINDOW_CLASS_NAME, AppInstance);

	if (ScrMode==smSaver)
	{
		if (!DEBUG && !CSettings::DefaultRes)
		{
			RestoreVideoMode();
		}
	}

	multisample=false;
	winwidth=winheight=0;
}
