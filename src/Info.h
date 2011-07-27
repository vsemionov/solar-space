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

#ifndef INFO_H
#define INFO_H

#include "Defs.h"

#include "Text.h"




class CInfo
{
public:
	CInfo();
	virtual ~CInfo();
	bool Load();
	void Free();
	void Start(float seconds, float duration, char *targetname, CBody *targetbody);
	void Update(float seconds);
	void Restart();
	void Draw();
private:
	void Init();
	void MakeWindow(int list);
	void GetNameCoords(const char *text, int *x, int *y);
	void GetInfoCoords(int linenum, int *x, int *y);
	void MakeInfoLine(int linenum, char *line);
	void MakeName(int list, char *targetname);
	void MakeInfo(int list, CBody *targetbody);
	bool loaded;
	CText nametext;
	CText infotext;
	int scrwidth;
	int scrheight;
	int winlist;
	int namelist;
	int infolist;
	float starttime;
	float endtime;
	float fadetime;
	float time;
	float alpha;
};

#endif
