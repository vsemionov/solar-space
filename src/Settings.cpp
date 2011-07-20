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


#include "Error.h"
#include "Settings.h"



#define REGSTR_PATH_PLUSSCR (REGSTR_PATH_SETUP "\\Screen Savers")
#define REGSTR_PATH_CONFIG  ("Software\\" TEAM_NAME "\\" APP_NAME)




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
int CSettings::VideoMode;
int CSettings::DetailLevel;
BOOL CSettings::DefaultRes;
BOOL CSettings::ClockOn;
BOOL CSettings::PlanetInfo;
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
	VideoMode=0;
	DetailLevel=0;
	DefaultRes=TRUE;
	ClockOn=FALSE;
	PlanetInfo=FALSE;
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
	IsDialogActive=FALSE;
	LONG res; HKEY skey; DWORD valtype, valsize, val;
	res=RegOpenKeyEx(HKEY_CURRENT_USER,REGSTR_PATH_PLUSSCR,0,KEY_QUERY_VALUE,&skey);
	if (res!=ERROR_SUCCESS) return;
	valsize=sizeof(val); res=RegQueryValueEx(skey,"Password Delay",0,&valtype,(LPBYTE)&val,&valsize); if (res==ERROR_SUCCESS) PasswordDelay=val;
	valsize=sizeof(val); res=RegQueryValueEx(skey,"Mouse Threshold",0,&valtype,(LPBYTE)&val,&valsize);if (res==ERROR_SUCCESS) MouseThreshold=val;
	valsize=sizeof(val); res=RegQueryValueEx(skey,"Mute Sound",0,&valtype,(LPBYTE)&val,&valsize);     if (res==ERROR_SUCCESS) MuteSound=val;
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
	valsize=sizeof(strval); res=RegQueryValueEx(skey,"Data Directory",0,&valtype,(LPBYTE)strval,&valsize);   if (res==ERROR_SUCCESS) strcpy(DataDir,strval);
	RegCloseKey(skey);
	l=strlen(DataDir); if (l>0 && DataDir[l-1]=='\\') DataDir[l-1]=0;
}





void CSettings::ReadConfigRegistry()
{
	VideoMode=2;
	DetailLevel=2;
	DefaultRes=TRUE;
	ClockOn=TRUE;
	PlanetInfo=TRUE;
	strcpy(DataFile,"");
	RandomDataFile=TRUE;
	LONG res; HKEY skey; DWORD valtype, valsize, val;
	char strval[sizeof(DataFile)];
	res=RegOpenKeyEx(HKEY_CURRENT_USER,REGSTR_PATH_CONFIG,0,KEY_QUERY_VALUE,&skey);
	if (res!=ERROR_SUCCESS) return;
	valsize=sizeof(val); res=RegQueryValueEx(skey,"Video Mode",0,&valtype,(LPBYTE)&val,&valsize);   if (res==ERROR_SUCCESS) VideoMode=val;
	valsize=sizeof(val); res=RegQueryValueEx(skey,"Detail Level",0,&valtype,(LPBYTE)&val,&valsize);   if (res==ERROR_SUCCESS) DetailLevel=val;
	valsize=sizeof(val); res=RegQueryValueEx(skey,"Default Resolution",0,&valtype,(LPBYTE)&val,&valsize);   if (res==ERROR_SUCCESS) DefaultRes=val;
	valsize=sizeof(val); res=RegQueryValueEx(skey,"Planet Info",0,&valtype,(LPBYTE)&val,&valsize);   if (res==ERROR_SUCCESS) PlanetInfo=val;
	valsize=sizeof(val); res=RegQueryValueEx(skey,"Clock On",0,&valtype,(LPBYTE)&val,&valsize);   if (res==ERROR_SUCCESS) ClockOn=val;
	valsize=sizeof(strval); res=RegQueryValueEx(skey,"Data File",0,&valtype,(LPBYTE)strval,&valsize);   if (res==ERROR_SUCCESS) strcpy(DataFile,strval);
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
	val=VideoMode; RegSetValueEx(skey,"Video Mode",0,REG_DWORD,(CONST BYTE*)&val,sizeof(val));
	val=DetailLevel; RegSetValueEx(skey,"Detail Level",0,REG_DWORD,(CONST BYTE*)&val,sizeof(val));
	val=DefaultRes; RegSetValueEx(skey,"Default Resolution",0,REG_DWORD,(CONST BYTE*)&val,sizeof(val));
	val=PlanetInfo; RegSetValueEx(skey,"Planet Info",0,REG_DWORD,(CONST BYTE*)&val,sizeof(val));
	val=ClockOn; RegSetValueEx(skey,"Clock On",0,REG_DWORD,(CONST BYTE*)&val,sizeof(val));
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
				CError::LogError(ERROR_CODE,"Failed to list planetary systems - memory allocation error.");
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
