#ifndef SETTINGS_H
#define SETTINGS_H

#include "defs.h"

#include "Body.h"




class CSettings
{
public:
	CSettings();
	virtual ~CSettings();

	static HWND hwnd;
	static DWORD PasswordDelay;
	static DWORD MouseThreshold;
	static BOOL  MuteSound;
	static POINT InitCursorPos;
	static DWORD InitTime;
	static BOOL  IsDialogActive;
	static BOOL  ReallyClose;
	//user settings:
	static int VideoMode;
	static int DetailLevel;
	static BOOL DefaultRes;
	static BOOL ClockOn;
	static BOOL PlanetInfo;
	static char DataDir[MAX_PATH];
	static char DataFile[MAX_PATH];
	static BOOL RandomDataFile;
	/////
	static void Init();
	static void Free();
	static void ReadGeneralRegistry();
	static void ReadConfigRegistry();
	static void WriteConfigRegistry();
	static void CloseSaverWindow();
	static void StartDialog();
	static void EndDialog();
	static bool BuildFileList();
	static void RandomizeDataFile();
	//////
	struct stardrs_s
	{
		char filename[MAX_PATH];
		char systemname[SYSTEM_NAME_SIZE];
		stardrs_s *prev;
		stardrs_s *next;
	};
	static stardrs_s filechain;
	static int numfiles;
};

#endif
