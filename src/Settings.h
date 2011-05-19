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
	static BOOL  PasswordOK;
	//user settings:
	static int VideoMode;
	static int DetailLevel;
	static BOOL DefaultRes;
	static BOOL PlanetInfo;
	static BOOL ClockOn;
	static char DataDir[MAX_PATH];
	static char DataFile[MAX_PATH];
	static BOOL RandomDataFile;
	/////
	static void Init();
	static void Free();
	static void ReadGeneralRegistry();
	static void ReadCommonRegistry();
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
