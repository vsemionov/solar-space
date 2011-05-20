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

#include <math.h>

#include "Settings.h"
#include "Main.h"
#include "VideoBase.h"
#include "Loader.h"
#include "Window.h"
#include "Text.h"
#include "Info.h"
#include "Error.h"
#include "GamePlay.h"



#define SPLASH_RESOURCE NULL
#define SPLASH_FILE "SPLASH.JPG"

#define SPLASH_FONT_NAME "Arial"
#define SPLASH_FONT_SIZE_AT_H600 12
#define SPLASH_FONT_SIZE (int)(SPLASH_FONT_SIZE_AT_H600*CWindow::GetHeight()/600)
#define SPLASH_TEXT_BAND_SIZE_FACTOR 1.25f

#define SPLASH_TEXT_COLOR_R 0.75f
#define SPLASH_TEXT_COLOR_G 0.75f
#define SPLASH_TEXT_COLOR_B 0.75f

#define TIMER_RESOLUTION_MS 1

#define RESTART_ALLOWED true
#define RESTART_TIME 10.0f



int CGamePlay::splash_tex=0;
char CGamePlay::load_text[256];
CGamePlay::splash_rect CGamePlay::splash_pos;
CBody CGamePlay::mainbody;
CCamera CGamePlay::camera;
CStarMap CGamePlay::starmap;
CClock CGamePlay::clock;
CLensFlare CGamePlay::lensflare;
bool CGamePlay::have_flares=false;
bool CGamePlay::have_info=false;
bool CGamePlay::have_clock=false;
CText CGamePlay::splashtext;
CInfo CGamePlay::info;
UINT CGamePlay::timer_res=0;









CGamePlay::CGamePlay()
{
}





CGamePlay::~CGamePlay()
{
}





bool CGamePlay::Init()
{
	bool ret=true;
	InitTimer();
	if (!splashtext.BuildFTFont(SPLASH_FONT_NAME,SPLASH_FONT_SIZE))
		CError::LogError(WARNING_CODE,"Failed to load the splash text font - ignoring.");
	if (!InitScene())
	{
		if (UserAbortedLoad())
			OnUserAbortLoad();
		else
			CError::LogError(ERROR_CODE,"Unable to load scene critical data - aborting.");
		DestroyScene();
		ret=false;
	}
	FreeSplash();
	splashtext.Free();
	return ret;
}





void CGamePlay::ShutDown()
{
	DestroyScene();
	ShutdownTimer();
}





void CGamePlay::InitTimer()
{
	TIMECAPS tc;
	UINT resolution;

	timer_res=0;

	if (timeGetDevCaps(&tc,sizeof(tc))==MMSYSERR_NOERROR)
	{
		resolution=TIMER_RESOLUTION_MS;
		CLAMP(resolution,tc.wPeriodMin,tc.wPeriodMax);
		if (timeBeginPeriod(resolution)==MMSYSERR_NOERROR)
		{
			timer_res=resolution;
		}
	}
}





void CGamePlay::ShutdownTimer()
{
	if (timer_res!=0)
	{
		timeEndPeriod(timer_res);
	}
}





void CGamePlay::CalcSplashRect()
{
	int w=CWindow::GetWidth();
	int h=CWindow::GetHeight();
	if (w*3>=h*4)
	{
		splash_pos.x1=(w-h*4/3)/2;
		splash_pos.x2=(w+h*4/3)/2;
		splash_pos.y1=0;
		splash_pos.y2=h;
	}
	else
	{
		splash_pos.x1=0;
		splash_pos.x2=w;
		splash_pos.y1=(h-w*3/4)/2;
		splash_pos.y2=(h+w*3/4)/2;
	}
}





bool CGamePlay::LoadSplash()
{
	CalcSplashRect();
	CLoader loader;
	loader.WithResource(SPLASH_RESOURCE);
	splash_tex=loader.LoadTexture(SPLASH_FILE,NULL,CVideoBase::GetOptMipmaps(),CVideoBase::GetOptLinear(), false);
	return (splash_tex>0);
}





void CGamePlay::FreeSplash()
{
	glDeleteTextures(1,(GLuint *)&splash_tex);
	splash_tex=0;
}





void CGamePlay::InitLight()
{
	GLfloat LightAmbient[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
	GLfloat LightDiffuse[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
	GLfloat LightSpecular[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
	glLightfv(GL_LIGHT1,GL_AMBIENT,LightAmbient);
	glLightfv(GL_LIGHT1,GL_DIFFUSE,LightDiffuse);
	glLightfv(GL_LIGHT1,GL_SPECULAR,LightSpecular);
	glEnable(GL_LIGHT1);
}





void CGamePlay::OnUserAbortLoad()
{
	// log new error
	CError::LogError(ERROR_CODE,"Loading aborted by user.");
	// get first error, and if it's "user abort", clear the log so it's not printed
	if (CError::ErrorsOccured())
	{
		int code;
		char string[ERROR_MAXLEN];
		CError::Rewind();
		CError::GetNextError(&code,string);
		strlwr(string);
		if (strstr(string,"user") && strstr(string,"abort"))
			CError::Clear();
	}
}





bool CGamePlay::UserAbortedLoad()
{
	static bool userabort=false;
	if (!userabort)
		userabort=!MessagePump();
	return userabort;
}





bool CGamePlay::InitScene()
{
	{
		SetSplashText("Loading splash screen... ");
		if (!LoadSplash())
			CError::LogError(WARNING_CODE,"Failed to load the splash screen - ignoring.");
	}

	{
		if (UserAbortedLoad())
		{
			return false;
		}
		SetSplashText("Loading bodies... ");
		if (!mainbody.Load())
		{
			CError::LogError(ERROR_CODE,"Failed to load the bodies - aborting.");
			return false;
		}
	}

	{
		if (UserAbortedLoad())
		{
			return false;
		}
		SetSplashText("Loading star map... ");
		if (!starmap.Load())
			CError::LogError(WARNING_CODE,"Failed to load the star map - ignoring.");
	}

	have_flares=CVideoBase::GetOptLensFlares();
	if (have_flares)
	{
		if (UserAbortedLoad())
		{
			return false;
		}
		SetSplashText("Loading lens flares... ");
		if (!lensflare.Load(&mainbody))
		{
			CError::LogError(WARNING_CODE,"Failed to load the lens flares - ignoring.");
			have_flares=false;
		}
	}

	have_info=(CSettings::PlanetInfo==TRUE);
	if (have_info)
	{
		if (UserAbortedLoad())
		{
			return false;
		}
		SetSplashText("Loading info text font... ");
		if (!info.Load())
		{
			CError::LogError(WARNING_CODE,"Failed to load the planet info - ignoring.");
			have_info=false;
		}
	}

	have_clock=(CSettings::ClockOn==TRUE);
	if (have_clock)
	{
		if (UserAbortedLoad())
		{
			return false;
		}
		SetSplashText("Loading clock... ");
		if (!clock.Load())
			CError::LogError(WARNING_CODE,"Failed to load the clock - ignoring.");
	}

	if (UserAbortedLoad())
	{
		return false;
	}
	SetSplashText("Done.");

	InitLight();
	camera.Init(&mainbody,(have_info?&info:NULL),CWindow::GetWidth(),CWindow::GetHeight());
	return FadeOutSplash();
}





void CGamePlay::DestroyScene()
{
	clock.Free();
	info.Free();
	have_info=false;
	lensflare.Free();
	have_flares=false;
	starmap.Free();
	mainbody.Destroy();
}





void CGamePlay::UpdateScene()
{
	static DWORD starttime=timeGetTime();
	DWORD millidelta;
	float seconds;

	millidelta=(timeGetTime()-starttime);
	seconds=(float)millidelta*0.001f;

	mainbody.Update(seconds);
	camera.Update(seconds);
	if (have_flares)
		lensflare.UpdateTime(seconds);
	if (have_info)
		info.Update(seconds);
	if (have_clock)
		clock.Update();

	if (RESTART_ALLOWED)
	{
		if (seconds>=max(RESTART_TIME,CAMERA_INIT_FADE_TIME))
		{
			mainbody.Restart();
			camera.Restart(seconds);
			if (have_flares)
				lensflare.Restart();
			if (have_info)
				info.Restart();

			starttime=starttime+millidelta;
		}
	}
}





void CGamePlay::DrawScene()
{
	GLfloat LightPosition[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();
	camera.ApplyRotation();
	starmap.Draw();
	camera.ApplyTranslation();
	glLightfv(GL_LIGHT1,GL_POSITION,LightPosition);
	mainbody.Draw(have_flares?&lensflare:NULL);
	if (have_flares)
	{
		lensflare.Draw();
	}
	if (have_info || have_clock || camera.fade)
	{
		Prepare2D(CWindow::GetWidth(),CWindow::GetHeight());
		if (have_info)
			info.Draw();
		if (have_clock)
			clock.Draw();
		if (camera.fade)
			camera.DrawFade();
		Restore3D();
	}
}





void CGamePlay::Frame()
{
	UpdateScene();
	DrawScene();
	glFlush();
	SwapBuffers(wglGetCurrentDC());
}





void CGamePlay::Prepare2D(int width, int height)
{
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	gluOrtho2D(0,width,0,height);
	glMatrixMode(GL_MODELVIEW);
	glPushAttrib(GL_ENABLE_BIT);
}





void CGamePlay::Restore3D()
{
	glPopAttrib();
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
}





void CGamePlay::UpdateSplash(const char *subtext)
{
	char text[256];
	strcpy(text,load_text);
	strcat(text,subtext);
	DrawSplash(text);
}





void CGamePlay::RenderSplashInner(const char *text)
{
	glLoadIdentity();
	glDisable(GL_LIGHTING);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	glEnable(GL_TEXTURE_2D);
	if (splash_tex>0)
	{
		glColor4f(1,1,1,1);
		glBindTexture(GL_TEXTURE_2D,splash_tex);
		glBegin(GL_QUADS);
		{
			glTexCoord2f(0,0.25f);
			glVertex2f((float)splash_pos.x1,(float)splash_pos.y1);
			glTexCoord2f(1,0.25f);
			glVertex2f((float)splash_pos.x2,(float)splash_pos.y1);
			glTexCoord2f(1,1);
			glVertex2f((float)splash_pos.x2,(float)splash_pos.y2);
			glTexCoord2f(0,1);
			glVertex2f((float)splash_pos.x1,(float)splash_pos.y2);
		}
		glEnd();
	}
	int w=CWindow::GetWidth();
	float th;
	splashtext.GetTextSize(text,NULL,&th);
	int text_under_height=(int)th-SPLASH_FONT_SIZE;
	int band_height=(int)(th*SPLASH_TEXT_BAND_SIZE_FACTOR);
	glDisable(GL_TEXTURE_2D);
	glColor4f(0,0,0,1);
	glBegin(GL_QUADS);
	{
		glVertex2f(0,0);
		glVertex2f((float)w,0);
		glVertex2f((float)w,(float)band_height);
		glVertex2f(0,(float)band_height);
	}
	glEnd();
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_BLEND);
	glTranslatef(0,(float)text_under_height,0);
	glColor4f(SPLASH_TEXT_COLOR_R,SPLASH_TEXT_COLOR_G,SPLASH_TEXT_COLOR_B,1);
	splashtext.Print(text);
}





void CGamePlay::DrawSplash(const char *text)
{
	Prepare2D(CWindow::GetWidth(),CWindow::GetHeight());
	glPushAttrib(GL_POLYGON_BIT);
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	RenderSplashInner(text);
	glPopAttrib();
	Restore3D();
	glFlush();
	SwapBuffers(wglGetCurrentDC());
}





void CGamePlay::SetSplashText(const char *text)
{
	strcpy(load_text,text);
	DrawSplash(text);
}





bool CGamePlay::FadeOutSplash()
{
	int w=CWindow::GetWidth();
	int h=CWindow::GetHeight();
	DWORD starttime=timeGetTime();
	float seconds;
	do
	{
		if (UserAbortedLoad())
		{
			return false;
		}
		seconds=(float)(timeGetTime()-starttime)*0.001f;
		float alpha=min(seconds/CAMERA_INIT_FADE_TIME,1.0f);
		{
			Prepare2D(w,h);
			glPushAttrib(GL_POLYGON_BIT);
			glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
			RenderSplashInner(load_text);
			glLoadIdentity();
			glDisable(GL_DEPTH_TEST);
			glDisable(GL_TEXTURE_2D);
			glEnable(GL_BLEND);
			glColor4f(0,0,0,alpha);
			glBegin(GL_QUADS);
			{
				glVertex2f(0.0f,0.0f);
				glVertex2f((float)w,0.0f);
				glVertex2f((float)w,(float)h);
				glVertex2f(0.0f,(float)h);
			}
			glEnd();
			glPopAttrib();
			Restore3D();
			glFlush();
			SwapBuffers(wglGetCurrentDC());
		}
	}
	while (seconds<CAMERA_INIT_FADE_TIME);
	return true;
}
