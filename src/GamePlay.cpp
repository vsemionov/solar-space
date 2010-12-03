#include <windows.h>

#include <gl\gl.h>

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
#define SPLASH_FONT_SIZE 24
#define SPLASH_FONT_BOLD false
#define SPLASH_FONT_ITALIC true
#define SPLASH_FONT_UNDERLINE false
#define SPLASH_FONT_STRIKEOUT false
#define SPLASH_FONT_THICKNESS 0.1f

#define SPLASH_TEXT_X 16.0f
#define SPLASH_TEXT_Y 16.0f
#define SPLASH_TEXT_COLOR_R 0.75f
#define SPLASH_TEXT_COLOR_G 0.75f
#define SPLASH_TEXT_COLOR_B 1.00f

#define RESTART_ALLOWED true
#define RESTART_TIME 10.0f




int CGamePlay::splash_tex=0;
char CGamePlay::load_text[256];
CBody CGamePlay::mainbody;
CCamera CGamePlay::camera;
CStarMap CGamePlay::starmap;
CClock CGamePlay::clock;
CLensFlare CGamePlay::lensflare;
bool CGamePlay::flares=false;
bool CGamePlay::planetinfo=false;
CText CGamePlay::splashtext;
CInfo CGamePlay::info;









CGamePlay::CGamePlay()
{
}





CGamePlay::~CGamePlay()
{
}





bool CGamePlay::Init()
{
	if (!splashtext.BuildOutlineFont(	SPLASH_FONT_NAME,
										SPLASH_FONT_SIZE,
										SPLASH_FONT_BOLD,
										SPLASH_FONT_ITALIC,
										SPLASH_FONT_UNDERLINE,
										SPLASH_FONT_STRIKEOUT,
										0.1f))
		CError::LogError(WARNING_CODE,"Failed to load the splash text font - ignoring.");
	if (!InitScene())
	{
		if (UserAbortedLoad())
			OnUserAbortLoad();
		else
			CError::LogError(ERROR_CODE,"Unable to load scene critical data - aborting.");
		DestroyScene();
		return false;
	}
	splashtext.Free();
	return true;
}





void CGamePlay::ShutDown()
{
	DestroyScene();
}





bool CGamePlay::LoadSplash()
{
	CLoader loader;
	loader.WithResource(SPLASH_RESOURCE);
	splash_tex=loader.LoadTexture(SPLASH_FILE,NULL,false);
	return (splash_tex>0);
}





void CGamePlay::FreeSplash()
{
	glDeleteTextures(1,(GLuint*)&splash_tex);
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
	CError::LogError(ERROR_CODE,"User aborted load.");
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
	SetSplashText("Loading splash screen... ");
	if (!LoadSplash())
		CError::LogError(WARNING_CODE,"Failed to load the splash screen - ignoring.");

	{
		SetSplashText("Loading bodies... ");
		if (UserAbortedLoad())
		{
			OnUserAbortLoad();
			return false;
		}
		if (!mainbody.Load())
		{
			CError::LogError(ERROR_CODE,"Failed to load the bodies - aborting.");
			return false;
		}
	}

	flares=CVideoBase::GetOptLensFlares();
	if (flares)
	{
		SetSplashText("Loading lens flares... ");
		if (UserAbortedLoad())
		{
			OnUserAbortLoad();
			return false;
		}
		if (!lensflare.Load(&mainbody))
		{
			CError::LogError(WARNING_CODE,"Failed to load the lens flares - ignoring.");
			flares=false;
		}
	}

	planetinfo=(CSettings::PlanetInfo==TRUE);
	if (planetinfo)
	{
		SetSplashText("Loading info text font... ");
		if (UserAbortedLoad())
		{
			OnUserAbortLoad();
			return false;
		}
		if (!info.Load())
		{
			CError::LogError(WARNING_CODE,"Failed to load the planet info - ignoring.");
			planetinfo=false;
		}
	}

	{
		SetSplashText("Loading starmap... ");
		if (UserAbortedLoad())
		{
			OnUserAbortLoad();
			return false;
		}
		if (!starmap.Load())
			CError::LogError(WARNING_CODE,"Failed to load the starmap - ignoring.");
	}

	if (CSettings::ClockOn)
	{
		SetSplashText("Loading clock... ");
		if (UserAbortedLoad())
		{
			OnUserAbortLoad();
			return false;
		}
		if (!clock.Load())
			CError::LogError(WARNING_CODE,"Failed to load the clock - ignoring.");
	}

	SetSplashText("Done.");

	srand((unsigned int)timeGetTime());
	InitLight();
	camera.Init(&mainbody,(planetinfo?&info:NULL));
	return FadeOutSplash();
}





void CGamePlay::DestroyScene()
{
	starmap.Free();
	lensflare.Free();
	flares=false;
	if (CSettings::ClockOn)
		clock.Free();
	mainbody.Destroy();
	FreeSplash();
	info.Free();
	planetinfo=false;
}





void CGamePlay::UpdateScene()
{
	float seconds;
	static DWORD starttime=timeGetTime();
	int milidelta;
	milidelta=(timeGetTime()-starttime);
	seconds=milidelta*0.001f;
	mainbody.Update(seconds);
	camera.Update(seconds);
	if (flares)
		lensflare.UpdateTime(seconds);
	if (planetinfo)
		info.Update(seconds);
	if (CSettings::ClockOn==TRUE && milidelta>=500)
		clock.Update();
	if (RESTART_ALLOWED)
	{
		if (seconds>=max(RESTART_TIME,CAMERA_INIT_FADE_TIME))
		{
			mainbody.Restart();
			camera.Restart(seconds);
			if (flares)
				lensflare.Restart();
			if (planetinfo)
				info.Restart();
			starttime=starttime+milidelta;
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
	mainbody.Draw(flares?&lensflare:NULL);
	if (flares)
	{
		lensflare.Draw();
	}
	if (planetinfo || CSettings::ClockOn)
	{
		Prepare2D(CWindow::GetWidth(),CWindow::GetHeight());
		if (planetinfo)
			info.Draw();
		if (CSettings::ClockOn)
			clock.Draw();
		Restore3D();
	}
	camera.DrawFade();
}





void CGamePlay::Frame()
{
	UpdateScene();
	SwapBuffers(wglGetCurrentDC());
	DrawScene();
	glFlush();
}





void CGamePlay::Prepare2D(int width, int height)
{
	glPushAttrib(GL_ENABLE_BIT);
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	gluOrtho2D(0,width,0,height);
	glMatrixMode(GL_MODELVIEW);
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
	float w=800.0f;
	float h=600.0f;
	glLoadIdentity();
	glDisable(GL_LIGHTING);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	if (splash_tex>0)
	{
		glEnable(GL_TEXTURE_2D);
		glColor4f(1,1,1,1);
		glBindTexture(GL_TEXTURE_2D,splash_tex);
		glBegin(GL_QUADS);
		{
			glTexCoord2f(0,0.25f);
			glVertex2f(0,0);
			glTexCoord2f(1,0.25f);
			glVertex2f(w,0);
			glTexCoord2f(1,1);
			glVertex2f(w,h);
			glTexCoord2f(0,1);
			glVertex2f(0,h);
		}
		glEnd();
	}
	glDisable(GL_TEXTURE_2D);
	glTranslatef(SPLASH_TEXT_X,SPLASH_TEXT_Y,0.0f);
	glScalef(SPLASH_FONT_SIZE,SPLASH_FONT_SIZE,SPLASH_FONT_SIZE);
	glColor4f(SPLASH_TEXT_COLOR_R,SPLASH_TEXT_COLOR_G,SPLASH_TEXT_COLOR_B,1);
	splashtext.Print(text);
}





void CGamePlay::DrawSplash(const char *text)
{
	Prepare2D(800,600);
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
	int starttime=timeGetTime();
	float seconds;
	do
	{
		if (UserAbortedLoad())
		{
			OnUserAbortLoad();
			return false;
		}
		seconds=(float)(timeGetTime()-starttime)*0.001f;
		float alpha=min(seconds/CAMERA_INIT_FADE_TIME,1.0f);
		{
			Prepare2D(800,600);
			glPushAttrib(GL_POLYGON_BIT);
			glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
			RenderSplashInner(load_text);
			glLoadIdentity();
			glDisable(GL_DEPTH_TEST);
			glEnable(GL_BLEND);
			glColor4f(0,0,0,alpha);
			glBegin(GL_QUADS);
			{
				glVertex2f(0,0);
				glVertex2f(800,0);
				glVertex2f(800,600);
				glVertex2f(0,600);
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
