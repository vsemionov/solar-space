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

#include <cstdlib>
#include <ctime>

#include <windows.h>
#include <commctrl.h>

#include <stdio.h>

#include <gl/glu.h>

#include "Settings.h"
#include "Loader.h"
#include "VideoBase.h"
#include "Window.h"
#include "Text.h"
#include "GamePlay.h"
#include "Error.h"
#include "Main.h"


#include "res\resource.h"


#define THUMBNAIL_WIDTH 152
#define THUMBNAIL_HEIGHT 112

#define LOG_TIMEOUT 15
#define LOG_NAME APP_NAME " Log.txt"

#define RANDOM_STRING "<random>"

#define ABOUT_HEADER APP_NAME " " APP_VERSION "\n" APP_DESCRIPTION
#define ABOUT_BODY "For more information, latest news and downloads, please visit the " APP_NAME " website at:" "\n" WEBSITE_URL
#define ABOUT_AUTHORS "Authors and contributors:"
#define ABOUT_TEXT ABOUT_HEADER "\n\n" ABOUT_BODY "\n\n" ABOUT_AUTHORS "\n"

#define AUTHORS_FILE "AUTHORS.txt"
#define AUTHORS_FILE_ERROR "<error: failed to load " AUTHORS_FILE ">"




TScrMode ScrMode=smNone;

HINSTANCE AppInstance=NULL;






static void LogIn()
{
#ifdef USE_ZLOG
	INIT_ZIRON_LOG();
	ZLOG("\"%s\" started.\0",APP_NAME);
#endif
}





static void LogOut()
{
#ifdef USE_ZLOG
	ZLOG("\"%s\" normally terminated.",APP_NAME);
	END_ZIRON_LOG();
#endif
}





bool MessagePump()
{
	MSG msg;
	while (PeekMessage(&msg,NULL,0,0,PM_REMOVE))
	{
		DispatchMessage(&msg);
		if (msg.message==WM_QUIT)
			return false;
	}
	return true;
}





static BOOL CALLBACK PreviewProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_INITDIALOG:
		SetWindowPos(hwnd,NULL,0,0,THUMBNAIL_WIDTH,THUMBNAIL_HEIGHT,SWP_NOACTIVATE|SWP_NOREPOSITION|SWP_NOMOVE|SWP_NOZORDER);
		CWindow::CenterWindow(hwnd);
		return FALSE;
	}
	return FALSE;
}





static void StaticPreview(HWND hwndParent)
{
	MSG msg;
	HWND hwnd=CreateDialog(GetModuleHandle(NULL),MAKEINTRESOURCE(IDD_PREVIEW),hwndParent,PreviewProc);
	while (GetMessage(&msg,NULL,0,0))
	{
		DispatchMessage(&msg);
	}
	DestroyWindow(hwnd);
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
	int id;
	switch (msg)
	{
	case WM_INITDIALOG:
		CWindow::CenterWindow(hwnd);
		SetTimer(hwnd,0,1000,NULL);
		PrintCloseTime(hwnd);
		return TRUE;
	case WM_TIMER:
		if (!PrintCloseTime(hwnd))
			EndDialog(hwnd,IDCANCEL);
		return TRUE;
	case WM_COMMAND:
		id=LOWORD(wParam);
		EndDialog(hwnd,id);
		return TRUE;
	case WM_CLOSE:
		EndDialog(hwnd,IDCANCEL);
		return TRUE;
	case WM_DESTROY:
		KillTimer(hwnd,0);
		return TRUE;
	}
	return FALSE;
}





static bool SaveLog(int level=WARNING_CODE)
{
	char filename[MAX_PATH];
	FILE *fp;
	GetTempPath(sizeof(filename)-strlen(LOG_NAME),filename);
	strcat(filename,LOG_NAME);
	fp=fopen(filename,"wt");
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
			if (code<level)
				continue;
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
			return true;
		}
	}
	return false;
}





static bool ViewLog()
{
	char filename[MAX_PATH];
	FILE *fp;
	GetTempPath(sizeof(filename)-strlen(LOG_NAME),filename);
	strcat(filename,LOG_NAME);
	fp=fopen(filename,"rb");
	if (fp)
	{
		fclose(fp);
		HINSTANCE hInst=ShellExecute(NULL,"open",filename,NULL,NULL,SW_SHOWNORMAL);
		if ((int)hInst>32)
		{
			return true;
		}
	}
	return false;
}





static void ErrorLogDialog()
{
	int res;
	res=DialogBox(GetModuleHandle(NULL),MAKEINTRESOURCE(IDD_SHOWLOG),NULL,QuestionLogProc);
	if (res!=IDC_BLOG) return;
	if (!SaveLog())
	{
		MessageBox(NULL,"Error saving log.",(APP_NAME " Error"),MB_OK|MB_ICONERROR);
		return;
	}
	if (!ViewLog())
	{
		MessageBox(NULL,"Error opening log.",(APP_NAME " Error"),MB_OK|MB_ICONERROR);
		return;
	}
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
	CVideoBase::Init();
	if (CWindow::Create(hwnd))
	{
		if (ScrMode==smSaver)
		{
			if (!DEBUG) ShowCursor(FALSE);
			if (CVideoBase::InitGL())
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





static void ShowAboutBox(HWND hwnd)
{
	char filename[MAX_PATH];
	FILE *fp;
	long tlen;
	long flen;
	long size;
	char *text;
	char *cur;
	const char *mb_title;
	const char *mb_text;
	int mb_type;

	flen=-1;
	tlen=strlen(ABOUT_TEXT);
	size=tlen+1;
	strcpy(filename,CSettings::DataDir);
	strcat(filename,"\\");
	strcat(filename,AUTHORS_FILE);
	fp=fopen(filename,"rt");
	if (fp)
	{
		if (!fseek(fp,0,SEEK_END))
		{
			flen=ftell(fp);
			if (flen>=0)
			{
				size+=flen;
			}
			rewind(fp);
		}
	}

	if (flen<0)
	{
		size+=strlen(AUTHORS_FILE_ERROR);
	}

	text=(char*)malloc(size);
	if (text)
	{
		strcpy(text,ABOUT_TEXT);
		if (flen>=0)
		{
			cur=text+tlen;
			while (fgets(cur,flen+1,fp))
			{
				tlen=strlen(cur);
				if (tlen==0)
					break;
				flen-=tlen;
				cur+=tlen;
			}
			while (cur>text && *(--cur)=='\n')
				*cur=0;
		}
		else
		{
			strcat(text,AUTHORS_FILE_ERROR);
		}
		mb_title=("About " APP_NAME);
		mb_text=text;
		mb_type=MB_OK|MB_ICONINFORMATION;
	}
	else
	{
		mb_title=(APP_NAME " Error");
		mb_text="Memory allocation error.";
		mb_type=MB_OK|MB_ICONERROR;
		MessageBox(hwnd,"Memory allocation error.",(APP_NAME " Error"),MB_OK|MB_ICONERROR);
	}

	if (fp)
		fclose(fp);

	MessageBox(hwnd,mb_text,mb_title,mb_type);

	if (text)
		free(text);
}





static BOOL CALLBACK ConfigDialogProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	int i;
	int id;
	switch (msg)
	{
	case WM_INITDIALOG:
		CheckRadioButton(hwnd,IDC_R640,IDC_R1024,(IDC_R640+CSettings::VideoMode));
		CheckRadioButton(hwnd,IDC_RLOW,IDC_RHIGH,(IDC_RLOW+CSettings::DetailLevel));
		CheckDlgButton(hwnd,IDC_CKDEFRES,CSettings::DefaultRes);
		CheckDlgButton(hwnd,IDC_CKINFO,CSettings::PlanetInfo);
		CheckDlgButton(hwnd,IDC_CKCLOCK,CSettings::ClockOn);
		UpdateRes(hwnd);
		FillList(hwnd);
		SelectList(hwnd);
		CWindow::CenterWindow(hwnd);
		return TRUE;
	case WM_COMMAND:
		id=LOWORD(wParam);
		switch (id)
		{
		case IDC_CKDEFRES:
			UpdateRes(hwnd);
			return TRUE;
		case IDC_ABOUT:
			ShowAboutBox(hwnd);
			return TRUE;
		case IDOK:
			for (i=0;i<3;i++) if (IsDlgButtonChecked(hwnd,IDC_R640+i)==BST_CHECKED) CSettings::VideoMode=i;
			for (i=0;i<2;i++) if (IsDlgButtonChecked(hwnd,IDC_RLOW+i)==BST_CHECKED) CSettings::DetailLevel=i;
			CSettings::DefaultRes=(IsDlgButtonChecked(hwnd,IDC_CKDEFRES)==BST_CHECKED);
			CSettings::PlanetInfo=(IsDlgButtonChecked(hwnd,IDC_CKINFO)==BST_CHECKED);
			CSettings::ClockOn=(IsDlgButtonChecked(hwnd,IDC_CKCLOCK)==BST_CHECKED);
			GetSelectedSystem(hwnd);
			CSettings::WriteConfigRegistry();
			EndDialog(hwnd,id);
			return TRUE;
		case IDCANCEL:
			EndDialog(hwnd,id);
			return TRUE;
		}
		break;
	case WM_NOTIFY:
		id=wParam;
		if (id==IDC_WEBSITE)
		{
			switch (((LPNMHDR)lParam)->code)
			{
			case NM_CLICK:
			case NM_RETURN:
				ShellExecute(NULL, NULL, WEBSITE_URL, NULL, NULL, SW_SHOWNORMAL);
				return TRUE;
			}
		}
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
	int Ret=0;
	AppInstance=hInstance;
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
	srand((unsigned int)time(NULL));
	CError::Init();
	CSettings::Init();
	if (ScrMode==smNone)
	{
		CError::LogError(ERROR_CODE, "Invalid command line argument.");
		Ret=1;
	}
	else if (ScrMode==smPassword)
	{
		ChangePassword(hwnd);
	}
	else
	{
		CSettings::ReadGeneralRegistry();
		CSettings::ReadCommonRegistry();
		CSettings::ReadConfigRegistry();
		if (!CSettings::BuildFileList())
		{
			CError::LogError(ERROR_CODE, "Error enumerating available planetary systems.");
			Ret=2;
		}
		else
		{
			if (ScrMode==smConfig)
			{
				DialogBox(hInstance,MAKEINTRESOURCE(IDD_CONFIG),hwnd,ConfigDialogProc);
			}
			else if (ScrMode==smSaver || ScrMode==smPreview)
			{
				if (CSettings::numfiles > 0 || ScrMode==smPreview)
				{
					if (ScrMode==smSaver && CSettings::RandomDataFile)
						CSettings::RandomizeDataFile();
					DoSaver(hwnd);
				}
				else
				{
					CError::LogError(ERROR_CODE, "No data files found.");
					Ret=3;
				}
			}
			else
			{
				// should not happen
				Ret=4;
			}
		}
	}
	if (CError::ErrorsOccured())
	{
		ErrorLogDialog();
		if (Ret==0)
			Ret=5;
	}
	CSettings::Free();
	CError::Clear();
	LogOut();
	return Ret;
}
