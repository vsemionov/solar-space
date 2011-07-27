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

#ifndef GAMEPLAY_H
#define GAMEPLAY_H

#include "Defs.h"

#include "Body.h"
#include "Camera.h"
#include "StarMap.h"
#include "Clock.h"
#include "Text.h"
#include "Info.h"
#include "LensFlare.h"


class CGamePlay
{
public:
	CGamePlay();
	virtual ~CGamePlay();
	static bool Init();
	static void ShutDown();
	static void Frame();
	static void UpdateSplash(const char *subtext);
	static bool UserAbortedLoad();
	static void OnUserAbortLoad();
private:
	struct splash_rect
	{
		int x1;
		int x2;
		int y1;
		int y2;
	};
	static void InitTimer();
	static void ShutdownTimer();
	static void CalcSplashRect();
	static bool LoadSplash();
	static void FreeSplash();
	static void RenderSplashInner(const char *text);
	static void DrawSplash(const char *text);
	static void SetSplashText(const char *text);
	static bool FadeOutSplash();
	static void InitLight();
	static bool InitScene();
	static void DestroyScene();
	static void UpdateScene();
	static void DrawScene();
	static void Prepare2D(int width, int height);
	static void Restore3D();
	static int splash_tex;
	static char load_text[256];
	static splash_rect splash_pos;
	static CBody mainbody;
	static CCamera camera;
	static CStarMap starmap;
	static CClock clock;
	static CLensFlare lensflare;
	static CText splashtext;
	static CInfo info;
	static bool have_flares;
	static bool have_info;
	static bool have_clock;
	static UINT timer_res;
};

#endif
