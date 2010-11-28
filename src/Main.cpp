#include <windows.h>
#include <commctrl.h>

#include <stdio.h>

#include <gl\glu.h>

#include "Settings.h"
#include "Loader.h"
#include "VideoBase.h"
#include "Window.h"
#include "Text.h"
#include "GamePlay.h"
#include "Error.h"
#include "Main.h"


#include "res\resource.h"


#define LOG_TIMEOUT 15
#define LOG_NAME APPNAME " Log.txt"

#define RANDOM_STRING "<Random>"



TScrMode ScrMode=smNone;







static void LogIn()
{
#ifdef USE_ZLOG
	INIT_ZIRON_LOG();
	ZLOG("\"%s\" started.\0",APPNAME);
#endif
}





static void LogOut()
{
#ifdef USE_ZLOG
	ZLOG("\"%s\" normally terminated.",APPNAME);
	END_ZIRON_LOG();
#endif
}





bool MessagePump()
{
	MSG msg;
	while (PeekMessage(&msg,NULL,0,0,PM_REMOVE))
	{
		DispatchMessage(&msg);
		if (msg.message==QUIT_MESSAGE)
			return false;
	}
	return true;
}





static void CenterWindow(HWND hwnd)
{
	RECT rect;
	int x,y;
	HWND hParent=GetParent(hwnd);
	if (!hParent)
		hParent=GetDesktopWindow();
	GetClientRect(hParent,&rect);
	x=rect.right/2;
	y=rect.bottom/2;
	GetClientRect(hwnd,&rect);
	x-=rect.right/2;
	y-=rect.bottom/2;
	SetWindowPos(hwnd,NULL,x,y,0,0,SWP_NOACTIVATE|SWP_NOREPOSITION|SWP_NOSIZE|SWP_NOZORDER);
}





static BOOL CALLBACK PreviewProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_INITDIALOG:
		return TRUE;
	}
	return FALSE;
}





static void StaticPreview(HWND hwndParent)
{
	MSG msg;
	HWND hwnd=CreateDialog(GetModuleHandle(NULL),MAKEINTRESOURCE(IDD_PREVIEW),hwndParent,PreviewProc);
	SetWindowPos(hwnd,NULL,0,0,152,112,SWP_NOACTIVATE|SWP_NOREPOSITION|SWP_NOMOVE|SWP_NOZORDER);
	UpdateWindow(hwnd);
	while (GetMessage(&msg,NULL,0,0))
	{
		DispatchMessage(&msg);
	}
	EndDialog(hwnd,0);
}





static BOOL PrintCloseTime(HWND hwDlg)
{
	static int timeleft=LOG_TIMEOUT+1;
	timeleft--;
	char btxt[32];
	sprintf(btxt,"Show log (%d)",timeleft);
	HWND hwItem=GetDlgItem(hwDlg,IDC_BLOG);
	SetWindowText(hwItem,btxt);
	return (timeleft>0);
}





static BOOL CALLBACK QuestionLogProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_INITDIALOG:
		{
			CenterWindow(hwnd);
			SetTimer(hwnd,0,1000,NULL);
			PrintCloseTime(hwnd);
			return TRUE;
		}
		break;
	case WM_TIMER:
		{
			if (!PrintCloseTime(hwnd))
				EndDialog(hwnd,IDCANCEL);
		}
		break;
	case WM_COMMAND:
		{
			int id=LOWORD(wParam);
			EndDialog(hwnd,id);
			return TRUE;
		}
		break;
	case WM_CLOSE:
		{
			EndDialog(hwnd,IDCANCEL);
			return TRUE;
		}
		break;
	case WM_DESTROY:
		KillTimer(hwnd,0);
		break;
	}
	return FALSE;
}





static void SaveLog()
{
	char filename[MAX_PATH];
	strcpy(filename,CSettings::DataDir);
	strcat(filename,"\\" LOG_NAME);
	FILE *fp=fopen(filename,"wt");
	if (!fp)
	{
		GetTempPath(sizeof(filename),filename);
		strcat(filename,"\\" LOG_NAME);
		fp=fopen(filename,"wt");
	}
	if (fp)
	{
		char error[ERROR_MAXLEN];
		int code;
		const char *type;
		CError::Rewind();
		int c=CError::GetCount();
		int i;
		for (i=0;i<c;i++)
		{
			CError::GetNextError(&code,error);
			switch (code)
			{
			case SUCCESS_CODE:
				type="Success";
				break;
			case WARNING_CODE:
				type="Warning";
				break;
			case ERROR_CODE:
				type="Error";
				break;
			default:
				type="";
				break;
			}
			int n=fprintf(fp,"%s: %s\n",type,error);
			if (n<=0) break;
		}
		fclose(fp);
		if (i==c)
		{
			return; //success
		}
	}
	MessageBox(NULL,"Error saving log.",(APPNAME " error"),MB_OK|MB_ICONEXCLAMATION);
}





static void ViewLog()
{
	char filename[MAX_PATH];
	strcpy(filename,CSettings::DataDir);
	strcat(filename,"\\" LOG_NAME);
	FILE *fp=fopen(filename,"rb");
	if (!fp)
	{
		GetTempPath(sizeof(filename),filename);
		strcat(filename,"\\" LOG_NAME);
		fp=fopen(filename,"rb");
	}
	if (fp)
	{
		fclose(fp);
		HINSTANCE hInst=ShellExecute(NULL,"open",filename,NULL,NULL,SW_SHOWNORMAL);
		if ((int)hInst>32)
		{
			return; //success
		}
	}
	MessageBox(NULL,"Error opening log.",(APPNAME " error"),MB_OK|MB_ICONEXCLAMATION);
}





static void ErrorLogDialog()
{
	int res;
	res=DialogBox(GetModuleHandle(NULL),MAKEINTRESOURCE(IDD_SHOWLOG),NULL,QuestionLogProc);
	if (res!=IDC_BLOG) return;
	SaveLog();
	ViewLog();
}





static void MainLoop()
{
	while (MessagePump())
	{
		CGamePlay::Frame();
	}
}





static void DoSaver(HWND hwnd)
{
	if (CWindow::Create(hwnd))
	{
		if (ScrMode==smSaver)
		{
			if (!DEBUG) ShowCursor(FALSE);
			if (CVideoBase::Init())
			{
				if (CGamePlay::Init())
				{
					MainLoop();
				}
				CGamePlay::ShutDown();
			}
			CVideoBase::ShutDown();
			if (!DEBUG) ShowCursor(TRUE);
		}
		else
		{
			StaticPreview(CSettings::hwnd);
		}
	}
	CWindow::Destroy();
}





static void UpdateRes(HWND hwnd)
{
	BOOL userres=!(IsDlgButtonChecked(hwnd,IDC_CKDEFRES)==BST_CHECKED);
	EnableWindow(GetDlgItem(hwnd,IDC_R640),userres);
	EnableWindow(GetDlgItem(hwnd,IDC_R800),userres);
	EnableWindow(GetDlgItem(hwnd,IDC_R1024),userres);
}





static int AddItem(HWND hwnd, const char *string)
{
	return SendMessage(GetDlgItem(hwnd,IDC_CMBSYSTEMS),CB_ADDSTRING,0,(LPARAM)string);
}





static void FillList(HWND hwnd)
{
	AddItem(hwnd,RANDOM_STRING);
	CSettings::stardrs_s *file;
	int i;
	for (i=0, file=CSettings::filechain.next; i<CSettings::numfiles; i++, file=file->next)
	{
		AddItem(hwnd,file->systemname);
	}
}





static void SelectList(HWND hwnd)
{
	if (CSettings::RandomDataFile)
	{
		SendMessage(GetDlgItem(hwnd,IDC_CMBSYSTEMS),CB_SETCURSEL,0,0);
	}
	else
	{
		char systemname[SYSTEM_NAME_SIZE];
		systemname[0]=0;
		CSettings::stardrs_s *file;
		int i;
		for (i=0, file=CSettings::filechain.next; i<CSettings::numfiles; i++, file=file->next)
		{
			if (!strcmp(CSettings::DataFile,file->filename))
			{
				strcpy(systemname,file->systemname);
				break;
			}
		}
		HRESULT hres=SendMessage(GetDlgItem(hwnd,IDC_CMBSYSTEMS),CB_SELECTSTRING,-1,(LPARAM)systemname);
		if (hres==CB_ERR)
			SendMessage(GetDlgItem(hwnd,IDC_CMBSYSTEMS),CB_SETCURSEL,0,0);
	}
}





static void GetSelectedSystem(HWND hwnd)
{
	HWND hlist=GetDlgItem(hwnd,IDC_CMBSYSTEMS);
	int sel=SendMessage(hlist,CB_GETCURSEL,0,0);
	if (sel==0 || sel==CB_ERR)
	{
		CSettings::RandomDataFile=TRUE;
	}
	else
	{
		CSettings::RandomDataFile=FALSE;
		char systemname[SYSTEM_NAME_SIZE];
		SendMessage(hlist,CB_GETLBTEXT,sel,(LPARAM)systemname);
		CSettings::stardrs_s *file;
		int i;
		for (i=0, file=CSettings::filechain.next; i<CSettings::numfiles; i++, file=file->next)
		{
			if (!strcmp(systemname,file->systemname))
			{
				strcpy(CSettings::DataFile,file->filename);
				break;
			}
		}
	}
}





static BOOL CALLBACK ConfigDialogProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_INITDIALOG:
		CheckRadioButton(hwnd,IDC_R640,IDC_R1024,(IDC_R640+CSettings::VideoMode));
		CheckRadioButton(hwnd,IDC_RLOW,IDC_RHIGH,(IDC_RLOW+CSettings::DetailLevel));
		CheckDlgButton(hwnd,IDC_CKCLOCK,CSettings::ClockOn);
		CheckDlgButton(hwnd,IDC_CKDEFRES,CSettings::DefaultRes);
		CheckDlgButton(hwnd,IDC_CKINFO,CSettings::PlanetInfo);
		UpdateRes(hwnd);
		FillList(hwnd);
		SelectList(hwnd);
		CenterWindow(hwnd);
		return TRUE;
	case WM_COMMAND:
		int id=LOWORD(wParam);
		if (id==IDC_CKDEFRES)
		{
			UpdateRes(hwnd);
		}
		if (id==IDOK)
		{
			int i;
			for (i=0;i<3;i++) if (IsDlgButtonChecked(hwnd,IDC_R640+i)==BST_CHECKED) CSettings::VideoMode=i;
			for (i=0;i<2;i++) if (IsDlgButtonChecked(hwnd,IDC_RLOW+i)==BST_CHECKED) CSettings::DetailLevel=i;
			GetSelectedSystem(hwnd);
			CSettings::ClockOn=(IsDlgButtonChecked(hwnd,IDC_CKCLOCK)==BST_CHECKED);
			CSettings::DefaultRes=(IsDlgButtonChecked(hwnd,IDC_CKDEFRES)==BST_CHECKED);
			CSettings::PlanetInfo=(IsDlgButtonChecked(hwnd,IDC_CKINFO)==BST_CHECKED);
			CSettings::WriteConfigRegistry();
		}
		if (id==IDOK || id==IDCANCEL) EndDialog(hwnd,id);
		break;
	}
	return FALSE;
}





static void ChangePassword(HWND hwnd)
{
	HINSTANCE hmpr=::LoadLibrary("MPR.DLL");
	if (hmpr==NULL)
		return;
	typedef VOID (WINAPI *PWDCHANGEPASSWORD) (LPCSTR lpcRegkeyname,HWND hwnd,UINT uiReserved1,UINT uiReserved2);
	PWDCHANGEPASSWORD PwdChangePassword=(PWDCHANGEPASSWORD)::GetProcAddress(hmpr,"PwdChangePasswordA");
	if (PwdChangePassword==NULL)
	{
		FreeLibrary(hmpr);
		return;
	}
	PwdChangePassword("SCRSAVE",hwnd,0,0);
	FreeLibrary(hmpr);
}






int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	int Ret=IDOK;
	LogIn();
	char *c=GetCommandLine();
	if (*c=='\"') {c++; while (*c!=0 && *c!='\"') c++;} else {while (*c!=0 && *c!=' ') c++;}
	if (*c!=0) c++;
	while (*c==' ') c++;
	HWND hwnd=NULL;
	if (*c==0) {ScrMode=smConfig; hwnd=NULL;}
	else
	{ if (*c=='-' || *c=='/') c++;
		if (*c=='p' || *c=='P' || *c=='l' || *c=='L')
		{ c++; while (*c==' ' || *c==':') c++;
			hwnd=(HWND)atoi(c);
			ScrMode=smPreview;
		}
		else if (*c=='s' || *c=='S') {ScrMode=smSaver; }
		else if (*c=='c' || *c=='C') {c++; while (*c==' ' || *c==':') c++; if (*c==0) hwnd=GetForegroundWindow(); else hwnd=(HWND)atoi(c); ScrMode=smConfig;}
		else if (*c=='a' || *c=='A') {c++; while (*c==' ' || *c==':') c++; hwnd=(HWND)atoi(c); ScrMode=smPassword;}
	}
	if (DEBUG)
		ScrMode=smSaver;
	InitCommonControls();
	CError::Init();
	CSettings::Init();
	if (ScrMode==smNone)
	{
		CError::LogError(ERROR_CODE, "Invalid command-line argument.");
	}
	else if (ScrMode==smPassword)
	{
		ChangePassword(hwnd);
	}
	else
	{
		CSettings::ReadGeneralRegistry();
		CSettings::ReadConfigRegistry();
		if (!CSettings::BuildFileList())
		{
			CError::LogError(ERROR_CODE, "Error enumerating available star systems.");
		}
		else if (CSettings::numfiles > 0)
		{
			if (CSettings::RandomDataFile)
				CSettings::RandomizeDataFile();
			if (ScrMode==smConfig)
			{
				Ret=DialogBox(hInstance,MAKEINTRESOURCE(IDD_CONFIG),hwnd,ConfigDialogProc);
			}
			else if (ScrMode==smSaver || ScrMode==smPreview)
			{
				DoSaver(hwnd);
			}
			else
			{
				// should not happen
			}
		}
		else
		{
			CError::LogError(ERROR_CODE, "No data files found.");
		}
	}
	if (CError::ErrorsOccured())
		ErrorLogDialog();
	CSettings::Free();
	CError::Clear();
	LogOut();
	return Ret;
}
