#include <windows.h>

#include <gl\gl.h>

#ifdef _MSC_VER
#define _USE_MATH_DEFINES
#endif
#include <math.h>

#include "Window.h"
#include "Body.h"
#include "Error.h"
#include "Info.h"




#define NAME_FONT_NAME "Arial"
#define NAME_FONT_SIZE_AT_H600 24
#define NAME_FONT_SIZE (NAME_FONT_SIZE_AT_H600*scrheight/600)

#define NAME_TEXT_COLOR_R 1.00f
#define NAME_TEXT_COLOR_G 1.00f
#define NAME_TEXT_COLOR_B 1.00f
#define NAME_TEXT_COLOR_A 0.50f

#define INFO_FONT_NAME "Arial"
#define INFO_FONT_SIZE_AT_H600 16
#define INFO_FONT_SIZE (INFO_FONT_SIZE_AT_H600*scrheight/600)

#define INFO_TEXT_COLOR_R 1.00f
#define INFO_TEXT_COLOR_G 1.00f
#define INFO_TEXT_COLOR_B 1.00f
#define INFO_TEXT_COLOR_A 0.50f

#define SPACING_COEF 1.75f
#define LINES_AFTER_NAME 1.00f



#define WINDOW_COLOR_R 0.50f
#define WINDOW_COLOR_G 0.50f
#define WINDOW_COLOR_B 0.50f
#define WINDOW_COLOR_A 0.25f

#define WINDOW_RECT_LEFT 0.0125f
#define WINDOW_RECT_RIGHT 0.32f
#define WINDOW_RECT_BOTTOM 0.0125f
#define WINDOW_RECT_TOP 0.27f

#define WINDOW_MARGIN_TOP 0.100f
#define WINDOW_MARGIN_LEFT 0.075f


#define MAX_FADE_TIME 3.0f
#define FADE_TIME_RATIO 0.10f
#define FADE_TIME(totaltime) min(MAX_FADE_TIME, (totaltime)*FADE_TIME_RATIO)

#define WINDOW_POS_X (WINDOW_RECT_LEFT*scrwidth)
#define WINDOW_POS_Y (WINDOW_RECT_TOP*scrheight)
#define WINDOW_WIDTH ((WINDOW_RECT_RIGHT-WINDOW_RECT_LEFT)*scrwidth)
#define WINDOW_HEIGHT ((WINDOW_RECT_TOP-WINDOW_RECT_BOTTOM)*scrheight)
#define MARGIN_WIDTH (WINDOW_WIDTH*WINDOW_MARGIN_LEFT)
#define MARGIN_HEIGHT (WINDOW_HEIGHT*WINDOW_MARGIN_TOP)






CInfo::CInfo()
{
	Init();
}





CInfo::~CInfo()
{
	Free();
}





void CInfo::Init()
{
	loaded=false;
	scrwidth=0;
	scrheight=0;
	winlist=0;
	namelist=infolist=0;
	time=0;
	starttime=endtime=0;
	fadetime=1;
	alpha=0;
}





void CInfo::Free()
{
	nametext.Free();
	infotext.Free();
	if (winlist)
	{
		if (glIsList(winlist))
			glDeleteLists(winlist,3);
	}
	Init();
}





bool CInfo::Load()
{
	Free();

	scrwidth=(float)CWindow::GetWidth();
	scrheight=(float)CWindow::GetHeight();

	winlist=glGenLists(3);
	if (!winlist)
	{
		CError::LogError(WARNING_CODE,"Unable to load planet info - failed to generate display lists.");
		Free();
		return false;
	}
	namelist=winlist+1;
	infolist=namelist+1;

	MakeWindow(winlist);

	loaded=true;
	loaded&=nametext.BuildFTFont(NAME_FONT_NAME,NAME_FONT_SIZE);
	loaded&=infotext.BuildFTFont(INFO_FONT_NAME,INFO_FONT_SIZE);
	if (!loaded)
	{
		CError::LogError(WARNING_CODE,"Unable to load planet info - failed to load font.");
		Free();
	}

	return loaded;
}





void CInfo::MakeWindow(int list)
{
	glNewList(list,GL_COMPILE);
	{
		float l=scrwidth*WINDOW_RECT_LEFT;
		float r=scrwidth*WINDOW_RECT_RIGHT;
		float b=scrheight*WINDOW_RECT_BOTTOM;
		float t=scrheight*WINDOW_RECT_TOP;
		glDisable(GL_TEXTURE_2D);
		glLoadIdentity();
		glBegin(GL_QUADS);
		{
			glVertex2f(l,b);
			glVertex2f(r,b);
			glVertex2f(r,t);
			glVertex2f(l,t);
		}
		glEnd();
	}
	glEndList();
}





void CInfo::GetNameCoords(const char *text, float *x, float *y)
{
	float tw,th;
	nametext.GetTextSize(text,&tw,&th);
	if (x) *x=WINDOW_POS_X+(WINDOW_WIDTH-tw)*0.5f;
	if (y) *y=WINDOW_POS_Y-MARGIN_HEIGHT-th;
}





void CInfo::GetInfoCoords(int linenum, float *x, float *y)
{
	float namey;
	GetNameCoords("",NULL,&namey);
	float nameadd;
	nametext.GetTextSize("",NULL,&nameadd);
	nameadd*=(SPACING_COEF*LINES_AFTER_NAME);

	float th;
	infotext.GetTextSize("",NULL,&th);
	th*=SPACING_COEF*(float)(linenum-1);

	if (x) *x=WINDOW_POS_X+MARGIN_WIDTH;
	if (y) *y=namey-nameadd-th;
}





void CInfo::MakeName(int list, char *targetname)
{
	if (!targetname) return;
	if (*targetname==' ') targetname++;
	float x,y;
	GetNameCoords(targetname,&x,&y);
	glNewList(list,GL_COMPILE);
	{
		glEnable(GL_TEXTURE_2D);
		glLoadIdentity();
		glTranslatef(x,y,0);
		nametext.Print(targetname);
	}
	glEndList();
}





void CInfo::MakeInfoLine(int linenum, char *line)
{
	float x,y;
	GetInfoCoords(linenum,&x,&y);
	glPushMatrix();
	glTranslatef(x,y,0);
	infotext.Print(line);
	glPopMatrix();
}





void CInfo::MakeInfo(int list, CBody *targetbody)
{
	bool star=(targetbody==NULL);
	if (star) targetbody=CBody::bodycache[0];
	glNewList(list,GL_COMPILE);
	{
		int n=0;
		glEnable(GL_TEXTURE_2D);
		glLoadIdentity();
		if (star)
		{
			char line[64];
			n++;
			sprintf(line,"star name: %s",targetbody->name);
			MakeInfoLine(n,line);
		}
		for (int i=0;i<targetbody->info.numlines;i++)
		{
			if (targetbody->info.textlines[i][0]=='/') continue;
			n++;
			MakeInfoLine(n,targetbody->info.textlines[i]);
		}
	}
	glEndList();
}





void CInfo::Start(float seconds, float duration, char *targetname, CBody *targetbody)
{
	if (!loaded)
		return;
	char name[32];
	strcpy(name,targetname);
	int l=strlen(name);
	for (int i=0;i<l;i++)
		if (name[i]=='_')
			name[i]=' ';
	starttime=seconds;
	endtime=starttime+duration;
	fadetime=FADE_TIME(duration);
	MakeName(namelist,name);
	MakeInfo(infolist,targetbody);
}





void CInfo::Update(float seconds)
{
	time=seconds;
	if (time<(endtime-fadetime))
		alpha=min(1,(time-starttime)/fadetime);
	else
		alpha=max(0,(endtime-time)/fadetime);
}





void CInfo::Restart()
{
	starttime-=time;
	endtime-=time;
	time=0;
}





void CInfo::Draw()
{
	if (!alpha || !loaded)
		return;

	glPushAttrib(GL_ENABLE_BIT);

	glDisable(GL_LIGHTING);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	glDisable(GL_NORMALIZE);
	glEnable(GL_BLEND);

	glColor4f(WINDOW_COLOR_R,WINDOW_COLOR_G,WINDOW_COLOR_B,WINDOW_COLOR_A*alpha);
	glCallList(winlist);

	glColor4f(NAME_TEXT_COLOR_R,NAME_TEXT_COLOR_G,NAME_TEXT_COLOR_B,NAME_TEXT_COLOR_A*alpha);
	glCallList(namelist);

	glColor4f(INFO_TEXT_COLOR_R,INFO_TEXT_COLOR_G,INFO_TEXT_COLOR_B,INFO_TEXT_COLOR_A*alpha);
	glCallList(infolist);

	glPopAttrib();
}
