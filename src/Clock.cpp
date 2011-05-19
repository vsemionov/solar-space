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

#include <gl/glu.h>
#include <gl/gl.h>

#include "Window.h"
#include "Loader.h"
#include "VideoBase.h"
#include "Error.h"
#include "GamePlay.h"
#include "Clock.h"


#define CLOCK_RESOURCE NULL

#define CLOCK_SPECS_FILE "CLOCK.TXT"






CClock::CClock()
{
	Init();
}





CClock::~CClock()
{
	Free();
}





void CClock::Init()
{
	ZeroMemory(textures,sizeof(textures));
	ZeroMemory(cur_tex,sizeof(cur_tex));
	ZeroMemory(tex_names,sizeof(tex_names));
	digit_tex_max=1;
	digit_w=0;
	digit_h=0;
	digit_s=1.0f;
	digit_t=0.0f;
	color_r=0.0f;
	color_g=0.0f;
	color_b=0.0f;
	color_a=0.0f;
}





bool CClock::Load()
{
#define AbortLoad()		\
	{					\
		Free();			\
		return false;	\
	}
////////////////
	CLoader loader;
	if (!loader.WithResource(CLOCK_RESOURCE))
	{
		CError::LogError(WARNING_CODE,"Unable to load clock - missing or invalid resource.");
		return false;
	}
	if (!ParseSpecsFile(&loader))
	{
		CError::LogError(WARNING_CODE,"Parsing of clock failed - skipping.");
		return false;
	}
	char ds[2];
	ds[1]=0;
	int i;
	for (i=0;i<11;i++)
	{
		ds[0]=(i==10?':':'0'+i);
		if (CGamePlay::UserAbortedLoad())
		{
			CError::LogError(ERROR_CODE,"Loading of clock aborted by user.");
			AbortLoad();
		}
		CGamePlay::UpdateSplash(ds);
		char *entry=tex_names[i];
		textures[i]=loader.LoadTexture(entry,entry,false,false,false);
		if (textures[i]==0)
		{
			CError::LogError(WARNING_CODE,"Failed to load clock digit - skipping.");
			AbortLoad();
		}
	}
	scrwidth=CWindow::GetWidth();
	scrheight=CWindow::GetHeight();
	return true;
}





void CClock::Free()
{
	glDeleteTextures(12,(GLuint *)textures);
	Init();
}





void CClock::Update()
{
	SYSTEMTIME st;

	GetLocalTime(&st);

	cur_tex[0]=textures[(st.wHour/10)];
	cur_tex[1]=textures[(st.wHour%10)];
	cur_tex[2]=textures[10]*(st.wMilliseconds/500);
	cur_tex[3]=textures[(st.wMinute/10)];
	cur_tex[4]=textures[(st.wMinute%10)];
}





void CClock::Draw()
{
	glDisable(GL_LIGHTING);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	glEnable(GL_BLEND);
	glLoadIdentity();
	glColor4f(color_r,color_g,color_b,color_a);
	for (int i=4;i>=0;i--)
	{
		if (cur_tex[i])
		{
			int x1=scrwidth-i*digit_w;
			int x2=scrwidth-(i+1)*digit_w;
			int y2=digit_h;
			glBindTexture(GL_TEXTURE_2D,cur_tex[4-i]);
			glBegin(GL_QUADS);
			{
				glTexCoord2f(digit_s,digit_t);	glVertex2f((float)x1,0.0f);
				glTexCoord2f(0.0f,digit_t);		glVertex2f((float)x2,0.0f);
				glTexCoord2f(0.0f,1.0f);		glVertex2f((float)x2,(float)y2);
				glTexCoord2f(digit_s,1.0f);		glVertex2f((float)x1,(float)y2);
			}
			glEnd();
		}
	}
	glColor3f(1.0f,1.0f,1.0f);
}





bool CClock::ParseSpecsFile(CLoader *loader)
{
#define FreeLines()					\
	{								\
		for (int j=0;j<numlines;j++)\
			free(textlines[j]);		\
		free(textlines);			\
		textlines=NULL;				\
		numlines=0;					\
	}
////////////////
#define AbortParse()	\
	{					\
		FreeLines();	\
		Init();			\
		return false;	\
	}
////////////////
	char **textlines=NULL;
	int numlines=0;
	int lineindex;
	int i;
	if (!loader->LoadText(CLOCK_SPECS_FILE,&textlines,&numlines))
	{
		CError::LogError(WARNING_CODE,"Unable to load clock - file missing from resource or internal loader subsystem error.");
		return false;
	}
	if (textlines==NULL)
	{
		CError::LogError(WARNING_CODE,"Unable to load clock - internal loader subsystem error.");
		return false;
	}
	if (numlines==0)
	{
		CError::LogError(WARNING_CODE,"Unable to load clock - empty data file.");
		return false;
	}
	{
		lineindex=0;
		while (sscanf(textlines[lineindex],"%d",&digit_tex_max)!=1 || textlines[lineindex][0]=='/')
		{
			lineindex++;
			if (lineindex>=numlines)
			{
				CError::LogError(WARNING_CODE,"Unable to load clock - unexpected end of file.");
				AbortParse();
			}
		}
		lineindex++;
		while (sscanf(textlines[lineindex],"%d %d",&digit_w,&digit_h)!=2 || textlines[lineindex][0]=='/')
		{
			lineindex++;
			if (lineindex>=numlines)
			{
				CError::LogError(WARNING_CODE,"Unable to load clock - unexpected end of file.");
				AbortParse();
			}
		}
		lineindex++;
		while (sscanf(textlines[lineindex],"%f %f %f %f",&color_r,&color_g,&color_b,&color_a)!=4 || textlines[lineindex][0]=='/')
		{
			lineindex++;
			if (lineindex>=numlines)
			{
				CError::LogError(WARNING_CODE,"Unable to load clock - unexpected end of file.");
				AbortParse();
			}
		}
		for (i=0;i<11;i++)
		{
			lineindex++;
			while (sscanf(textlines[lineindex],"%s",tex_names[i])!=1 || textlines[lineindex][0]=='/')
			{
				lineindex++;
				if (lineindex>=numlines)
				{
					CError::LogError(WARNING_CODE,"Unable to load clock - unexpected end of file.");
					AbortParse();
				}
			}
		}
	}
	FreeLines();
	{
		if (digit_tex_max<1) digit_tex_max=1;
		if (digit_w<0) digit_w=0;
		if (digit_w>digit_tex_max) digit_w=digit_tex_max;
		if (digit_h<0) digit_h=0;
		if (digit_h>digit_tex_max) digit_h=digit_tex_max;
		if (color_r<0.0f) color_r=0.0f;
		if (color_r>1.0f) color_r=1.0f;
		if (color_g<0.0f) color_g=0.0f;
		if (color_g>1.0f) color_g=1.0f;
		if (color_b<0.0f) color_b=0.0f;
		if (color_b>1.0f) color_b=1.0f;
		digit_s=(float)digit_w/(float)digit_tex_max;
		digit_t=1.0f-((float)digit_h/(float)digit_tex_max);
	}
	return true;
}
