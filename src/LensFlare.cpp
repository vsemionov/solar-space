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

#ifdef _MSC_VER
#define _USE_MATH_DEFINES
#endif
#include <math.h>

#include "Settings.h"
#include "Window.h"
#include "VideoBase.h"
#include "Loader.h"
#include "Body.h"
#include "Error.h"
#include "GamePlay.h"
#include "LensFlare.h"



#define FLARE_RESOURCE CSettings::DataFile
#define FLARE_SPECS_FILE "FLARES.TXT"

#define SEGMENTS_BASE 4
#define STACKS ((int)((float)SEGMENTS_BASE*CVideoBase::GetOptGeoDetail()))
#define SLICES (2*STACKS)
#define NUM_POINTS (SLICES*(STACKS-1)+2)

#define FADE_TIME 0.25f

#define IS_MAIN_FLARE(num) (flaredata[num].pos==1.0f)







CLensFlare::CLensFlare()
{
	Init();
}





CLensFlare::~CLensFlare()
{
	Free();
}





bool CLensFlare::Load(CBody *star)
{
#define AbortLoad()		\
	{					\
		Free();			\
		return false;	\
	}
////////////////
	if (!ComputePoints(star))
	{
		CError::LogError(WARNING_CODE,"Unable to compute lens flare alpha points.");
		AbortLoad();
	}
	viewport[0]=viewport[1]=0;
	viewport[2]=CWindow::GetWidth();
	viewport[3]=CWindow::GetHeight();
	CLoader loader;
	if (!loader.WithResource(FLARE_RESOURCE))
	{
		CError::LogError(WARNING_CODE,"Unable to load lens flares - missing or invalid resource.");
		AbortLoad();
	}
	if (!ParseSpecsFile(&loader))
	{
		CError::LogError(WARNING_CODE,"Parsing of lens flare failed - skipping.");
		AbortLoad();
	}
	char fs[16];
	int i;
	for (i=0;i<num_flares;i++)
	{
		itoa(i+1,fs,10);
		if (CGamePlay::UserAbortedLoad())
		{
			CError::LogError(ERROR_CODE,"Loading of lens flare aborted by user.");
			AbortLoad();
		}
		CGamePlay::UpdateSplash(fs);
		int prevtex=FindPrevTex(i);
		if (prevtex<i && prevtex>=0)
		{
			textures[i]=textures[prevtex];
		}
		else
		{
			char *entry=tex_names[i];
			textures[i]=loader.LoadTexture(entry,NULL,CVideoBase::GetOptMipmaps(),CVideoBase::GetOptLinear(), false);
			if (textures[i]==0)
			{
				CError::LogError(WARNING_CODE,"Failed to load a lens flare image - ignoring.");
			}
		}
		if (IS_MAIN_FLARE(i))
			flaredata[i].size*=star->GetRadius(0,false)*0.01f;
		else
			flaredata[i].size*=sizefactor*star->radmult*0.5f;
	}
	return true;
}





void CLensFlare::Free()
{
	int i;
	if (points)
		free(points);
	if (flaredata)
		free(flaredata);
	if (tex_names)
	{
		for (i=0;i<num_flares;i++)
		{
			if (tex_names[i])
				free(tex_names[i]);
		}
		free(tex_names);
	}
	if (textures)
	{
		glDeleteTextures(num_flares,(GLuint *)textures);
		free(textures);
	}
	Init();
}





void CLensFlare::UpdateTime(float seconds)
{
	lasttime=time;
	time=seconds;
	lastalpha=alpha;
	alpha=((time-starttime)*DaDt)+startalpha;
	if ((endalpha<startalpha && alpha<endalpha) || (endalpha>startalpha && alpha>endalpha))
		alpha=endalpha;
	CLAMP(alpha,0.0f,1.0f);
}





void CLensFlare::UpdatePos()
{
	if (!num_flares)
		return; //this guarantees we won't get a null ptr reference
	glGetDoublev(GL_MODELVIEW_MATRIX,modelview);
	glGetDoublev(GL_PROJECTION_MATRIX,projection);
	int n=NUM_POINTS;
	int vis=0;
	int i;
	for (i=0;i<n;i++)
	{
		vis+=(int)IsPointVisible(&points[i]);
	}
	double wx,wy,wz;
	gluProject(0.0,0.0,0.0,modelview,projection,viewport,&wx,&wy,&wz);
	flareX=(float)wx;
	flareY=(float)wy;
	flareZ=(float)wz;
	if (flareZ<0.0f || flareZ>1.0f)
	{
		vis=0;
	}
	if (vis!=vispts)
	{
		starttime=lasttime;
		endtime=starttime+FADE_TIME;
		startalpha=lastalpha;
		endalpha=((float)vis/(float)n);
		DaDt=(endalpha-startalpha)/FADE_TIME;
		vispts=vis;
		UpdateTime(time);
	}
}





void CLensFlare::Restart()
{
	starttime-=time;
	endtime-=time;
	lasttime-=time;
	time=0.0f;
}





void CLensFlare::Draw()
{
	if (!num_flares)
		return; //this guarantees we won't get a null ptr reference
	if (alpha==0.0f)
		return; //saves some CPU time
	glPushAttrib(GL_ENABLE_BIT);
	glDisable(GL_LIGHTING);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA,GL_ONE);
	glLoadIdentity();
	float vLight[3];
	vLight[0]=(float)modelview[12];
	vLight[1]=(float)modelview[13];
	vLight[2]=(float)modelview[14];
	float len;
	len=-vLight[2];
	glTranslatef(0.0f,0.0f,-len);
	vLight[2]=0.0f;
	int i;
	for (i=0;i<num_flares;i++)
	{
		flaredata_s *data=&flaredata[i];
		if (textures[i]>0)
		{
			float *color=data->color;
			float k=data->pos;
			float s=data->size;
			glColor4f(color[0],color[1],color[2],alpha);
			glBindTexture(GL_TEXTURE_2D,textures[i]);
			glPushMatrix();
			glTranslatef(vLight[0]*k,vLight[1]*k,0.0f);
			glBegin(GL_QUADS);
			{
				glTexCoord2f(0.0f,0.0f);
				glVertex2f(-s,-s);
				glTexCoord2f(1.0f,0.0f);
				glVertex2f(+s,-s);
				glTexCoord2f(1.0f,1.0f);
				glVertex2f(+s,+s);
				glTexCoord2f(0.0f,1.0f);
				glVertex2f(-s,+s);
			}
			glEnd();
			glPopMatrix();
		}
	}
	glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
	glPopAttrib();
}





void CLensFlare::Init()
{
	points=NULL;
	ZeroMemory(modelview,sizeof(modelview));
	ZeroMemory(projection,sizeof(projection));
	ZeroMemory(viewport,sizeof(viewport));
	flareX=flareY=flareZ=0.0f;
	vispts=0;
	time=lasttime=starttime=endtime=0.0f;
	alpha=lastalpha=startalpha=endalpha=0.0f;
	DaDt=0.0f;
	flaredata=NULL;
	sizefactor=0.0f;
	num_flares=0;
	textures=NULL;
	tex_names=NULL;
}





bool CLensFlare::ComputePoints(CBody *star)
{
	glGetIntegerv(GL_VIEWPORT,viewport);
	int blocksize=NUM_POINTS*sizeof(Vector); //compiler bug-fix
	points=(Vector*)malloc(blocksize);
	if (!points)
	{
		CError::LogError(WARNING_CODE,"Unable to compute lens flare alpha points - memory allocation failed.");
		return false;
	}
	int i,j;
	float radius=star->radius;
	float shape=star->shape;
	int p=0;
	for (i=0;i<=STACKS;i++)
	{
		if (i==0 || i==STACKS)
		{
			points[p][0]=points[p][1]=0.0;
			if (!i)
				points[0][2]=radius*shape;
			else
				points[NUM_POINTS-1][2]=-radius*shape;
			p++;
		}
		else
		{
			double Z=((double)(i*2)/(double)STACKS)-1.0;
			double theta=acos(Z);
			for (j=0;j<SLICES;j++)
			{
				double phi=((double)j/(double)SLICES)*(M_PI*2.0);
				points[p][0]=radius*(cos(phi)*sin(theta));
				points[p][1]=radius*(sin(phi)*sin(theta));
				points[p][2]=radius*shape*cos(theta);
				p++;
			}
		}
	}
	return true;
}





bool CLensFlare::IsPointVisible(const Vector *p)
{
	double winx,winy,winz;
	gluProject((*p)[0],(*p)[1],(*p)[2],modelview,projection,viewport,&winx,&winy,&winz);
	float fZ=(float)winz;
	float bufferZ=0.0f;
	glReadPixels((int)winx,(int)winy,1,1,GL_DEPTH_COMPONENT,GL_FLOAT,&bufferZ);
	if (fZ<=bufferZ && fZ>=0.0f /*&& fZ<=1.0f*/)
		return true;
	else
		return false;
}





bool CLensFlare::ParseSpecsFile(CLoader *loader)
{
////////////////
#define FreeLines()					\
	{								\
		for (int l=0;l<numlines;l++)\
			free(textlines[l]);		\
		free(textlines);			\
		textlines=NULL;				\
		numlines=0;					\
	}
////////////////
#define AbortParse()	\
	{					\
		FreeLines();	\
		return false;	\
	}
////////////////
#define VA_NUM 6
#define VA_FMT "%f  |  %f  | %f %f %f  | %s"
#define VA_ARGS				\
	&flaredata[i].pos,		\
	&flaredata[i].size,		\
	&flaredata[i].color[0],	\
	&flaredata[i].color[1],	\
	&flaredata[i].color[2],	\
	tex_names[i]
/////////////////////
	char **textlines=NULL;
	int numlines=0;
	int lineindex;
	int i;
	if (!loader->LoadText(FLARE_SPECS_FILE,&textlines,&numlines))
	{
		CError::LogError(WARNING_CODE,"Unable to load lens flares - file missing from resource or internal loader subsystem error.");
		return false;
	}
	if (textlines==NULL)
	{
		CError::LogError(WARNING_CODE,"Unable to load lens flares - internal loader subsystem error.");
		return false;
	}
	if (numlines==0)
	{
		CError::LogError(WARNING_CODE,"Unable to load lens flares - empty data file.");
		return false;
	}
	{
		lineindex=0;
		while (sscanf(textlines[lineindex],"%d",&num_flares)!=1 || textlines[lineindex][0]=='/')
		{
			lineindex++;
			if (lineindex>=numlines)
			{
				CError::LogError(WARNING_CODE,"Unable to load lens flares - unexpected end of file.");
				AbortParse();
			}
		}
		lineindex++;
		while (sscanf(textlines[lineindex],"%f",&sizefactor)!=1 || textlines[lineindex][0]=='/')
		{
			lineindex++;
			if (lineindex>=numlines)
			{
				CError::LogError(WARNING_CODE,"Unable to load lens flares - unexpected end of file.");
				AbortParse();
			}
		}
		flaredata=(flaredata_s*)malloc(num_flares*sizeof(flaredata_s));
		if (!flaredata)
		{
			CError::LogError(WARNING_CODE,"Unable to load lens flares - memory allocation failed.");
			AbortParse();
		}
		ZeroMemory(flaredata,num_flares*sizeof(flaredata_s));
		textures=(int*)malloc(num_flares*sizeof(int));
		if (!textures)
		{
			CError::LogError(WARNING_CODE,"Unable to load lens flares - memory allocation failed.");
			AbortParse();
		}
		ZeroMemory(textures,num_flares*sizeof(int));
		tex_names=(char**)malloc(num_flares*sizeof(char*));
		if (!tex_names)
		{
			CError::LogError(WARNING_CODE,"Unable to load lens flares - memory allocation failed.");
			AbortParse();
		}
		ZeroMemory(tex_names,num_flares*sizeof(char*));
		for (i=0;i<num_flares;i++)
		{
			tex_names[i]=(char*)malloc(16);
			if (!tex_names[i])
			{
				CError::LogError(WARNING_CODE,"Unable to load lens flares - memory allocation failed.");
				AbortParse();
			}
			tex_names[i][0]=0;
			lineindex++;
			while (sscanf(textlines[lineindex],VA_FMT,VA_ARGS)!=VA_NUM || textlines[lineindex][0]=='/')
			{
				tex_names[i][0]=0;
				lineindex++;
				if (lineindex>=numlines)
				{
					CError::LogError(WARNING_CODE,"Unable to load lens flares - unexpected end of file.");
					AbortParse();
				}
			}
			strupr(tex_names[i]);
			float *color=flaredata[i].color;
			int j;
			for (j=0;j<3;j++)
			{
				if (color[j]<0.0f) color[j]=0.0f;
				if (color[j]>1.0f) color[j]=1.0f;
			}
		}
	}
	FreeLines();
	return true;
}





int CLensFlare::FindPrevTex(int current)
{
	int prev;
	char *curname=tex_names[current];
	for (prev=0;prev<current;prev++)
	{
		char *prevname=tex_names[prev];
		if (!strcmp(curname,prevname))
			break;
	}
	return prev;
}
