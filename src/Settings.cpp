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
#include <regstr.h>


#include "Log.h"
#include "Settings.h"



#define REGSTR_PATH_PLUSSCR (REGSTR_PATH_SETUP "\\Screen Savers")
#define REGSTR_PATH_CONFIG  ("Software\\" TEAM_NAME "\\" APP_NAME)

#define VALUE_NAME_PASSWORD_DELAY "Password Delay"
#define VALUE_NAME_MOUSE_THRESHOLD "Mouse Threshold"
#define VALUE_NAME_MUTE_SOUND "Mute Sound"

#define VALUE_NAME_DATA_DIR "Data Directory"

#define VALUE_NAME_RESOLUTION "Resolution"
#define VALUE_NAME_GRAPHICS "Graphics"
#define VALUE_NAME_DESKTOP_RES "Desktop Resolution"
#define VALUE_NAME_SHOW_INFO "Show Info"
#define VALUE_NAME_SHOW_CLOCK "Show Clock"
#define VALUE_NAME_DATA_FILE "Data File"




HWND CSettings::hwnd;
DWORD CSettings::PasswordDelay;
DWORD CSettings::MouseThreshold;
BOOL  CSettings::MuteSound;
POINT CSettings::InitCursorPos;
DWORD CSettings::InitTime;
BOOL  CSettings::IsDialogActive;
BOOL  CSettings::ReallyClose;
BOOL  CSettings::PasswordOK;
//user settings:
int CSettings::Resolution;
int CSettings::Graphics;
BOOL CSettings::DesktopRes;
BOOL CSettings::ShowClock;
BOOL CSettings::ShowInfo;
char CSettings::DataDir[MAX_PATH];
char CSettings::DataFile[MAX_PATH];
BOOL CSettings::RandomDataFile;
CSettings::stardrs_s CSettings::filechain;
int CSettings::numfiles;



CSettings::CSettings()
{
}





CSettings::~CSettings()
{
}





void CSettings::Init()
{
	hwnd=NULL;
	PasswordDelay=0;
	MouseThreshold=0;
	MuteSound=FALSE;
	ZeroMemory(&InitCursorPos,sizeof(InitCursorPos));
	InitTime=0;
	IsDialogActive=FALSE;
	ReallyClose=FALSE;
	PasswordOK=FALSE;
	/////
	Resolution=0;
	Graphics=0;
	DesktopRes=TRUE;
	ShowClock=FALSE;
	ShowInfo=FALSE;
	DataDir[0]=0;
	DataFile[0]=0;
	RandomDataFile=TRUE;
	filechain.next=filechain.prev=&filechain;
	strcpy(filechain.filename,APP_NAME);
	strcpy(filechain.systemname,APP_NAME);
	numfiles=0;
}





void CSettings::Free()
{
	stardrs_s *drs=filechain.next;
	while (drs!=&filechain)
	{
		drs=drs->next;
		free(drs->prev);
	}
	Init();
}





void CSettings::ReadGeneralRegistry()
{
	PasswordDelay=15;
	MouseThreshold=64;
	MuteSound=FALSE;
	IsDialogActive=FALSE;
	LONG res; HKEY skey; DWORD valtype, valsize, val;
	res=RegOpenKeyEx(HKEY_CURRENT_USER,REGSTR_PATH_PLUSSCR,0,KEY_QUERY_VALUE,&skey);
	if (res!=ERROR_SUCCESS) return;
	valsize=sizeof(val); res=RegQueryValueEx(skey,VALUE_NAME_PASSWORD_DELAY,0,&valtype,(LPBYTE)&val,&valsize); if (res==ERROR_SUCCESS) PasswordDelay=val;
	valsize=sizeof(val); res=RegQueryValueEx(skey,VALUE_NAME_MOUSE_THRESHOLD,0,&valtype,(LPBYTE)&val,&valsize);if (res==ERROR_SUCCESS) MouseThreshold=val;
	valsize=sizeof(val); res=RegQueryValueEx(skey,VALUE_NAME_MUTE_SOUND,0,&valtype,(LPBYTE)&val,&valsize);     if (res==ERROR_SUCCESS) MuteSound=val;
	RegCloseKey(skey);
}





void CSettings::ReadCommonRegistry()
{
	int l;
	strcpy(DataDir,".");
	LONG res; HKEY skey; DWORD valtype, valsize;
	char strval[sizeof(DataDir)];
	res=RegOpenKeyEx(HKEY_LOCAL_MACHINE,REGSTR_PATH_CONFIG,0,KEY_QUERY_VALUE,&skey);
	if (res!=ERROR_SUCCESS) return;
	valsize=sizeof(strval); res=RegQueryValueEx(skey,VALUE_NAME_DATA_DIR,0,&valtype,(LPBYTE)strval,&valsize);   if (res==ERROR_SUCCESS) strcpy(DataDir,strval);
	RegCloseKey(skey);
	l=strlen(DataDir); if (l>0 && DataDir[l-1]=='\\') DataDir[l-1]=0;
}





void CSettings::ReadConfigRegistry()
{
	Resolution=2;
	Graphics=2;
	DesktopRes=TRUE;
	ShowClock=TRUE;
	ShowInfo=TRUE;
	strcpy(DataFile,"");
	RandomDataFile=TRUE;
	LONG res; HKEY skey; DWORD valtype, valsize, val;
	char strval[sizeof(DataFile)];
	res=RegOpenKeyEx(HKEY_CURRENT_USER,REGSTR_PATH_CONFIG,0,KEY_QUERY_VALUE,&skey);
	if (res!=ERROR_SUCCESS) return;
	valsize=sizeof(val); res=RegQueryValueEx(skey,VALUE_NAME_RESOLUTION,0,&valtype,(LPBYTE)&val,&valsize);   if (res==ERROR_SUCCESS) Resolution=val;
	valsize=sizeof(val); res=RegQueryValueEx(skey,VALUE_NAME_GRAPHICS,0,&valtype,(LPBYTE)&val,&valsize);   if (res==ERROR_SUCCESS) Graphics=val;
	valsize=sizeof(val); res=RegQueryValueEx(skey,VALUE_NAME_DESKTOP_RES,0,&valtype,(LPBYTE)&val,&valsize);   if (res==ERROR_SUCCESS) DesktopRes=val;
	valsize=sizeof(val); res=RegQueryValueEx(skey,VALUE_NAME_SHOW_INFO,0,&valtype,(LPBYTE)&val,&valsize);   if (res==ERROR_SUCCESS) ShowInfo=val;
	valsize=sizeof(val); res=RegQueryValueEx(skey,VALUE_NAME_SHOW_CLOCK,0,&valtype,(LPBYTE)&val,&valsize);   if (res==ERROR_SUCCESS) ShowClock=val;
	valsize=sizeof(strval); res=RegQueryValueEx(skey,VALUE_NAME_DATA_FILE,0,&valtype,(LPBYTE)strval,&valsize);   if (res==ERROR_SUCCESS) strcpy(DataFile,strval);
	RegCloseKey(skey);
	RandomDataFile=(DataFile[0]==0);
}





void CSettings::WriteConfigRegistry()
{
	if (RandomDataFile)
		DataFile[0]=0;
	LONG res; HKEY skey; DWORD val, disp;
	char strval[sizeof(DataFile)];
	res=RegCreateKeyEx(HKEY_CURRENT_USER,REGSTR_PATH_CONFIG,0,NULL,REG_OPTION_NON_VOLATILE,KEY_SET_VALUE,NULL,&skey,&disp);
	if (res!=ERROR_SUCCESS) return;
	val=Resolution; RegSetValueEx(skey,"Resolution",0,REG_DWORD,(CONST BYTE*)&val,sizeof(val));
	val=Graphics; RegSetValueEx(skey,"Graphics",0,REG_DWORD,(CONST BYTE*)&val,sizeof(val));
	val=DesktopRes; RegSetValueEx(skey,"Desktop Resolution",0,REG_DWORD,(CONST BYTE*)&val,sizeof(val));
	val=ShowInfo; RegSetValueEx(skey,"Show Info",0,REG_DWORD,(CONST BYTE*)&val,sizeof(val));
	val=ShowClock; RegSetValueEx(skey,"Show Clock",0,REG_DWORD,(CONST BYTE*)&val,sizeof(val));
	strcpy(strval,DataFile); RegSetValueEx(skey,"Data File",0,REG_SZ,(CONST BYTE*)strval,strlen(strval)+1);
	RegCloseKey(skey);
}





void CSettings::CloseSaverWindow()
{
	ReallyClose=TRUE;
	PostMessage(hwnd,WM_CLOSE,0,0);
}





void CSettings::StartDialog()
{
	IsDialogActive=TRUE;
	SendMessage(hwnd,WM_SETCURSOR,0,0);
}





void CSettings::EndDialog()
{
	IsDialogActive=FALSE;
	SendMessage(hwnd,WM_SETCURSOR,0,0);
	GetCursorPos(&InitCursorPos);
}





bool CSettings::BuildFileList()
{
	bool res=true;
	stardrs_s *newsystem;
	char srchstr[MAX_PATH];
	char systemname[SYSTEM_NAME_SIZE];
	strcpy(srchstr,DataDir);
	strcat(srchstr,"\\*.d2");
	char filename[MAX_PATH];
	WIN32_FIND_DATA wfd;
	HANDLE h=FindFirstFile(srchstr,&wfd);
	BOOL b=(h!=INVALID_HANDLE_VALUE);
	while (b)
	{
		strcpy(filename,wfd.cFileName);
		*strrchr(filename,'.')=0;
		if (CBody::LoadSystemData(filename,NULL,systemname,true))
		{
			newsystem=(stardrs_s*)malloc(sizeof(stardrs_s));
			if (!newsystem)
			{
				CLog::Log(LOG_ERROR,"Unable to list planetary systems - memory allocation error.");
				res=false;
				break;
			}
			strcpy(newsystem->filename,filename);
			strcpy(newsystem->systemname,systemname);
			newsystem->prev=filechain.prev;
			newsystem->next=&filechain;
			filechain.prev->next=newsystem;
			filechain.prev=newsystem;
			numfiles++;
		}
		b=FindNextFile(h,&wfd);
	}
	FindClose(h);
	return res;
}





void CSettings::RandomizeDataFile()
{
	int i;
	stardrs_s *file;
	if (!numfiles)
		return;
	int num=rand()%numfiles;
	for (i=0, file=filechain.next; i<num; i++)
	{
		file=file->next;
	}
	strcpy(DataFile,file->filename);
}
