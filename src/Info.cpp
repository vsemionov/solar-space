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
#define NAME_FONT_SIZE (int)(NAME_FONT_SIZE_AT_H600*scrheight/600)

#define NAME_TEXT_COLOR_R 1.00f
#define NAME_TEXT_COLOR_G 1.00f
#define NAME_TEXT_COLOR_B 1.00f
#define NAME_TEXT_COLOR_A 0.50f

#define INFO_FONT_NAME "Arial"
#define INFO_FONT_SIZE_AT_H600 16
#define INFO_FONT_SIZE (int)(INFO_FONT_SIZE_AT_H600*scrheight/600)

#define INFO_TEXT_COLOR_R 1.00f
#define INFO_TEXT_COLOR_G 1.00f
#define INFO_TEXT_COLOR_B 1.00f
#define INFO_TEXT_COLOR_A 0.50f

#define SCREEN_SIZE_RATIO_STD (4.0f/3.0f)
#define FONT_SIZE_COEFF (4.6f/3.0f) // do not change!
#define SPACING_COEF 1.15f
#define LINES_AFTER_NAME 1.125f


#define WINDOW_COLOR_R 0.50f
#define WINDOW_COLOR_G 0.50f
#define WINDOW_COLOR_B 0.50f
#define WINDOW_COLOR_A 0.25f

#define MAX_FADE_TIME 3.0f
#define FADE_TIME_RATIO 0.10f
#define FADE_TIME(totaltime) min(MAX_FADE_TIME, (totaltime)*FADE_TIME_RATIO)


#define WINDOW_BORDER_REL 0.0125f
#define WINDOW_BORDER (int)(WINDOW_BORDER_REL*scrheight)

#define MARGIN_TOP_REL 0.0250f
#define MARGIN_BOTTOM_REL 0.0050f
#define MARGIN_LEFT_REL (0.0200f*SCREEN_SIZE_RATIO_STD)
#define MARGIN_LEFT_WIDTH (int)(MARGIN_LEFT_REL*scrheight)
#define MARGIN_TOP_HEIGHT (int)(MARGIN_TOP_REL*scrheight)
#define MARGIN_BOTTOM_HEIGHT (int)(MARGIN_BOTTOM_REL*scrheight)

#define WINDOW_WIDTH_REL_Y (0.3050f*SCREEN_SIZE_RATIO_STD)
#define WINDOW_WIDTH (int)(WINDOW_WIDTH_REL_Y*scrheight)
#define WINDOW_HEIGHT (MARGIN_TOP_HEIGHT+MARGIN_BOTTOM_HEIGHT+(int)(NAME_FONT_SIZE*LINES_AFTER_NAME*SPACING_COEF*FONT_SIZE_COEFF)+3*(int)(INFO_FONT_SIZE*SPACING_COEF*FONT_SIZE_COEFF))

#define WINDOW_POS_X1 (WINDOW_BORDER)
#define WINDOW_POS_Y1 (WINDOW_BORDER)
#define WINDOW_POS_X2 (WINDOW_POS_X1+WINDOW_WIDTH)
#define WINDOW_POS_Y2 (WINDOW_POS_Y1+WINDOW_HEIGHT)





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

	scrwidth=CWindow::GetWidth();
	scrheight=CWindow::GetHeight();

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
		int l=WINDOW_POS_X1;
		int r=WINDOW_POS_X2;
		int b=WINDOW_POS_Y1;
		int t=WINDOW_POS_Y2;
		glDisable(GL_TEXTURE_2D);
		glLoadIdentity();
		glBegin(GL_QUADS);
		{
			glVertex2f((float)l,(float)b);
			glVertex2f((float)r,(float)b);
			glVertex2f((float)r,(float)t);
			glVertex2f((float)l,(float)t);
		}
		glEnd();
	}
	glEndList();
}





void CInfo::GetNameCoords(const char *text, int *x, int *y)
{
	float tw;
	nametext.GetTextSize(text,&tw,NULL);
	int th=NAME_FONT_SIZE;

	if (x) *x=WINDOW_POS_X1+(WINDOW_WIDTH-(int)tw)/2;
	if (y) *y=WINDOW_POS_Y2-MARGIN_TOP_HEIGHT-th;
}





void CInfo::GetInfoCoords(int linenum, int *x, int *y)
{
	int ymargin=WINDOW_POS_Y2-MARGIN_TOP_HEIGHT;

	float nameheight;
	nametext.GetTextSize("",NULL,&nameheight);

	float nameadd;
	nameadd=nameheight*SPACING_COEF*LINES_AFTER_NAME;

	int ioffset=INFO_FONT_SIZE;

	float th;
	infotext.GetTextSize("",NULL,&th);
	th*=SPACING_COEF;
	int thi=(int)th*(linenum-1);

	if (x) *x=WINDOW_POS_X1+MARGIN_LEFT_WIDTH;
	if (y) *y=ymargin-(int)nameadd-ioffset-thi;
}





void CInfo::MakeName(int list, char *targetname)
{
	if (!targetname) return;
	if (*targetname==' ') targetname++;
	int x,y;
	GetNameCoords(targetname,&x,&y);
	glNewList(list,GL_COMPILE);
	{
		glEnable(GL_TEXTURE_2D);
		glLoadIdentity();
		glTranslatef((float)x,(float)y,0);
		nametext.Print(targetname);
	}
	glEndList();
}





void CInfo::MakeInfoLine(int linenum, char *line)
{
	int x,y;
	GetInfoCoords(linenum,&x,&y);
	glPushMatrix();
	glTranslatef((float)x,(float)y,0);
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

	glDisable(GL_LIGHTING);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	glEnable(GL_BLEND);

	glColor4f(WINDOW_COLOR_R,WINDOW_COLOR_G,WINDOW_COLOR_B,WINDOW_COLOR_A*alpha);
	glCallList(winlist);

	glColor4f(NAME_TEXT_COLOR_R,NAME_TEXT_COLOR_G,NAME_TEXT_COLOR_B,NAME_TEXT_COLOR_A*alpha);
	glCallList(namelist);

	glColor4f(INFO_TEXT_COLOR_R,INFO_TEXT_COLOR_G,INFO_TEXT_COLOR_B,INFO_TEXT_COLOR_A*alpha);
	glCallList(infolist);
}
