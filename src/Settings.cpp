#include <windows.h>
#include <regstr.h>


#include "Error.h"
#include "Settings.h"



#define REGSTR_PATH_PLUSSCR (REGSTR_PATH_SETUP "\\Screen Savers")
#define REGSTR_PATH_CONFIG  ("Software\\" TEAMNAME "\\" APPNAME)




HWND CSettings::hwnd;
DWORD CSettings::PasswordDelay;
DWORD CSettings::MouseThreshold;
BOOL  CSettings::MuteSound;
POINT CSettings::InitCursorPos;
DWORD CSettings::InitTime;
BOOL  CSettings::IsDialogActive;
BOOL  CSettings::ReallyClose;
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
	strcpy(filechain.filename,APPNAME);
	strcpy(filechain.systemname,APPNAME);
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
	res=RegOpenKeyEx(HKEY_CURRENT_USER,REGSTR_PATH_PLUSSCR,0,KEY_ALL_ACCESS,&skey);
	if (res!=ERROR_SUCCESS) return;
	valsize=sizeof(val); res=RegQueryValueEx(skey,"Password Delay",0,&valtype,(LPBYTE)&val,&valsize); if (res==ERROR_SUCCESS) PasswordDelay=val;
	valsize=sizeof(val); res=RegQueryValueEx(skey,"Mouse Threshold",0,&valtype,(LPBYTE)&val,&valsize);if (res==ERROR_SUCCESS) MouseThreshold=val;
	valsize=sizeof(val); res=RegQueryValueEx(skey,"Mute Sound",0,&valtype,(LPBYTE)&val,&valsize);     if (res==ERROR_SUCCESS) MuteSound=val;
	RegCloseKey(skey);
}





void CSettings::ReadConfigRegistry()
{
	VideoMode=1;
	DetailLevel=1;
	DefaultRes=TRUE;
	ClockOn=TRUE;
	PlanetInfo=TRUE;
	strcpy(DataDir,".");
	strcpy(DataFile,"");
	RandomDataFile=TRUE;
	LONG res; HKEY skey; DWORD valtype, valsize, val;
	char strval[sizeof(DataDir)];
	res=RegOpenKeyEx(HKEY_LOCAL_MACHINE,REGSTR_PATH_CONFIG,0,KEY_ALL_ACCESS,&skey);
	if (res!=ERROR_SUCCESS) return;
	valsize=sizeof(val); res=RegQueryValueEx(skey,"Video Mode",0,&valtype,(LPBYTE)&val,&valsize);   if (res==ERROR_SUCCESS) VideoMode=val;
	valsize=sizeof(val); res=RegQueryValueEx(skey,"Detail Level",0,&valtype,(LPBYTE)&val,&valsize);   if (res==ERROR_SUCCESS) DetailLevel=val;
	valsize=sizeof(val); res=RegQueryValueEx(skey,"Clock On",0,&valtype,(LPBYTE)&val,&valsize);   if (res==ERROR_SUCCESS) ClockOn=val;
	valsize=sizeof(val); res=RegQueryValueEx(skey,"Planet Info",0,&valtype,(LPBYTE)&val,&valsize);   if (res==ERROR_SUCCESS) PlanetInfo=val;
	valsize=sizeof(val); res=RegQueryValueEx(skey,"Default Resolution",0,&valtype,(LPBYTE)&val,&valsize);   if (res==ERROR_SUCCESS) DefaultRes=val;
	valsize=sizeof(strval); res=RegQueryValueEx(skey,"Data Directory",0,&valtype,(LPBYTE)strval,&valsize);   if (res==ERROR_SUCCESS) strcpy(DataDir,strval);
	valsize=sizeof(strval); res=RegQueryValueEx(skey,"Data File",0,&valtype,(LPBYTE)strval,&valsize);   if (res==ERROR_SUCCESS) strcpy(DataFile,strval);
	RegCloseKey(skey);
	RandomDataFile=(DataFile[0]==0);
}





void CSettings::WriteConfigRegistry()
{
	if (RandomDataFile)
		DataFile[0]=0;
	LONG res; HKEY skey; DWORD val, disp;
	char strval[sizeof(DataDir)];
	res=RegCreateKeyEx(HKEY_LOCAL_MACHINE,REGSTR_PATH_CONFIG,0,NULL,REG_OPTION_NON_VOLATILE,KEY_ALL_ACCESS,NULL,&skey,&disp);
	if (res!=ERROR_SUCCESS) return;
	val=VideoMode; RegSetValueEx(skey,"Video Mode",0,REG_DWORD,(CONST BYTE*)&val,sizeof(val));
	val=DetailLevel; RegSetValueEx(skey,"Detail Level",0,REG_DWORD,(CONST BYTE*)&val,sizeof(val));
	val=ClockOn; RegSetValueEx(skey,"Clock On",0,REG_DWORD,(CONST BYTE*)&val,sizeof(val));
	val=PlanetInfo; RegSetValueEx(skey,"Planet Info",0,REG_DWORD,(CONST BYTE*)&val,sizeof(val));
	val=DefaultRes; RegSetValueEx(skey,"Default Resolution",0,REG_DWORD,(CONST BYTE*)&val,sizeof(val));
	strcpy(strval,DataDir); RegSetValueEx(skey,"Data Directory",0,REG_SZ,(CONST BYTE*)strval,sizeof(strval));
	strcpy(strval,DataFile); RegSetValueEx(skey,"Data File",0,REG_SZ,(CONST BYTE*)strval,sizeof(strval));
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





void CSettings::BuildFileList()
{
	stardrs_s *newsystem;
	char srchstr[MAX_PATH];
	char systemname[SYSTEM_NAME_SIZE];
	strcpy(srchstr,DataDir);
	strcat(srchstr,"\\*.DRS");
	char filename[MAX_PATH];
	WIN32_FIND_DATA wfd;
	HANDLE h=FindFirstFile(srchstr,&wfd);
	BOOL b=(h!=INVALID_HANDLE_VALUE);
	while (b)
	{
		strcpy(filename,wfd.cFileName);
		if (CBody::LoadSystemName(filename,systemname,true))
		{
			newsystem=(stardrs_s*)malloc(sizeof(stardrs_s));
			if (!newsystem)
			{
				CError::LogError(ERROR_CODE,"Failed to list star systems - memory allocation error.");
				break;
			}
			*strrchr(filename,'.')=0;
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
}





void CSettings::RandomizeDataFile()
{
	int i;
	stardrs_s *file;
	if (!numfiles)
		return;
	srand((unsigned int)timeGetTime());
	int num=rand()%numfiles;
	for (i=0, file=filechain.next; i<num; i++)
	{
		file=file->next;
	}
	strcpy(DataFile,file->filename);
}
