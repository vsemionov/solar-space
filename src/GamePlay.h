#ifndef GAMEPLAY_H
#define GAMEPLAY_H

#include "defs.h"

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
	static bool flares;
	static bool planetinfo;
};

#endif
