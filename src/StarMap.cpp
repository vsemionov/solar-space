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
#include <gl/glu.h>

#ifdef _MSC_VER
#define _USE_MATH_DEFINES
#endif
#include <math.h>

#include <gl/glext.h>

#include "Settings.h"
#include "Loader.h"
#include "VideoBase.h"
#include "Window.h"
#include "Error.h"
#include "GamePlay.h"
#include "StarMap.h"


//resource stuff
#define STARMAP_RESOURCE CSettings::DataFile
#define STARMAP_FILE "STARS.TXT"

//sphere radius
#define STARMAP_RADIUS 100.0

//num random stars
#define NUM_RANDOM_STARS_BASE 8192
#define NUM_RANDOM_STARS ((int)((float)NUM_RANDOM_STARS_BASE*CVideoBase::GetOptGeoDetail()))

//random star size and color
#define MIN_INTENSITY 0.6f //also affects colorfulness of stars, when generated randomly
#define MIN_MAG 4.75
#define MAX_MAG 7.9

//real-star color levels
#define COLOR_SCALE 0.75f

//magnitude-size conversion
#define STAR_SIZE(star_mag) (0.75+(7.0-star_mag)/2.5)

//point size macros
#define POINT_SIZE_AT_H600 1.25
#define SQRT_QUAD_ATTEN_INV (STARMAP_RADIUS)
#define QUAD_ATTEN_INV (SQRT_QUAD_ATTEN_INV*SQRT_QUAD_ATTEN_INV)
#define QUAD_ATTEN (1.0/QUAD_ATTEN_INV)
#define STAR_DIST(star_size) (CVideoBase::GetExtPointParams()?		\
								(SQRT_QUAD_ATTEN_INV/star_size):	\
								STARMAP_RADIUS)

#define AUTO_SIZE_COEF 0.625









CStarMap::CStarMap()
{
	Init();
}





CStarMap::~CStarMap()
{
	Free();
}





bool CStarMap::Load()
{
	Free();
	if (LoadStars())
	{
		PrepColor();
	}
	else
	{
		CError::LogError(LOG_ERROR,"Failed to load stars, trying to generate randomly.");
		Free();
		if (CGamePlay::UserAbortedLoad())
		{
			CError::LogError(LOG_INFO,"Loading of star map aborted by user.");
			return false;
		}
		CGamePlay::UpdateSplash("generating... ");
		if (!GenStars())
		{
			CError::LogError(LOG_ERROR,"Failed to generate random stars.");
			Free();
			return false;
		}
	}

	PrepData();

	point_size=(float)(POINT_SIZE_AT_H600*((double)CWindow::GetHeight()/600.0));

	twinkle=CVideoBase::GetOptStarTwinkle();
	twinkle=false; //no twinkle effect yet

	InitGL();

	if (!twinkle)
	{
		object = glGenLists(1);
		if (object==0)
		{
			CError::LogError(LOG_ERROR,"Unable to record star map display list - internal OpenGL error.");
			Free();
			return false;
		}

		glNewList(object,GL_COMPILE);
		{
			DrawStars();
		}
		glEndList();
	}

	return true;
}





void CStarMap::Free()
{
	if (glIsList(object))
		glDeleteLists(object,1);
	if (stars)
	{
		free(stars);
	}
	Init();
}





void CStarMap::Draw()
{
	if (!num_stars)
		return;
	if (!twinkle)
	{
		glCallList(object);
	}
	else
	{
		DrawStars();
	}
}





void CStarMap::Init()
{
	object=0;
	twinkle=false;
	stars=NULL;
	num_stars=0;
	point_size=1.0f;
}





bool CStarMap::LoadStars()
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
#define VA_FMT "%lf %lf | %f | %f %f %f"
#define VA_ARGS			\
	&stars[i].Dec,		\
	&stars[i].RA,		\
	&stars[i].mag,		\
	&stars[i].color[0],		\
	&stars[i].color[1],		\
	&stars[i].color[2]
/////////////////////
	const char *eof_msg="Unable to load star data - unexpected end of file.";
	char **textlines=NULL;
	int numlines=0;
	int lineindex;
	int i;
	CLoader loader;
	if (!loader.WithResource(STARMAP_RESOURCE))
	{
		CError::LogError(LOG_ERROR,"Unable to load star map file - missing or invalid resource.");
		return false;
	}
	if (!loader.LoadText(STARMAP_FILE,&textlines,&numlines))
	{
		CError::LogError(LOG_ERROR,"Unable to load star map data - file missing from resource or internal loader subsystem error.");
		return false;
	}
	if (textlines==NULL)
	{
		CError::LogError(LOG_ERROR,"Unable to load star map - internal loader subsystem error.");
		return false;
	}
	if (numlines==0)
	{
		CError::LogError(LOG_ERROR,"Unable to load star map - empty data file.");
		return false;
	}
	{
		lineindex=-1;
		do
		{
			lineindex++;
			if (lineindex>=numlines)
			{
				CError::LogError(LOG_ERROR,eof_msg);
				AbortParse();
			}
		} while (sscanf(textlines[lineindex],"%d",&num_stars)!=1 || textlines[lineindex][0]=='/');
		stars=(stardata_s*)malloc(num_stars*sizeof(stardata_s));
		if (!stars)
		{
			CError::LogError(LOG_ERROR,"Unable to load stars - memory allocation failed.");
			AbortParse();
		}
		ZeroMemory(stars,num_stars*sizeof(stardata_s));
		for (i=0;i<num_stars;i++)
		{
			do
			{
				lineindex++;
				if (lineindex>=numlines)
				{
					CError::LogError(LOG_ERROR,eof_msg);
					AbortParse();
				}
			} while (sscanf(textlines[lineindex],VA_FMT,VA_ARGS)!=VA_NUM || textlines[lineindex][0]=='/');
		}
	}
	FreeLines();
	return true;
}





bool CStarMap::GenStars()
{
	int i;
	num_stars=NUM_RANDOM_STARS;
	stars=(stardata_s*)malloc(num_stars*sizeof(stardata_s));
	if (!stars)
	{
		CError::LogError(LOG_ERROR,"Unable to generate star data - memory allocation failed.");
		return false;
	}
	const float mi=MIN_INTENSITY;
	const float ri=(1.0f-MIN_INTENSITY);
	for (i=0;i<num_stars;i++)
	{
		double Z=((double)(rand()%(2*8192+1))/8192.0)-1.0;
		stars[i].Dec=asin(Z)*(180.0/M_PI);
		stars[i].RA=(double)(rand()%(360*64))/64.0;
		stars[i].mag=(float)(MIN_MAG+(double)(rand()%(int)((MAX_MAG-MIN_MAG)*1000.0+1.0))/1000.0);
		stars[i].color[0]=mi+(float)(rand()%256)*ri/255.0f;
		stars[i].color[1]=mi+(float)(rand()%256)*ri/255.0f;
		stars[i].color[2]=mi+(float)(rand()%256)*ri/255.0f;
	}
	return true;
}





void CStarMap::PrepColor()
{
	int i,j;
	for (i=0;i<num_stars;i++)
	{
		stardata_s *star=&stars[i];
		for (j=0;j<3;j++)
		{
			star->color[j]*=COLOR_SCALE;
		}
	}
}





void CStarMap::PrepData()
{
	int i;
	for (i=0;i<num_stars;i++)
	{
		double theta=(90.0-stars[i].Dec)*(M_PI/180.0);
		double phi=stars[i].RA*(M_PI/180.0);
		double Ctheta=cos(theta);
		double Stheta=sin(theta);
		double Cphi=cos(phi);
		double Sphi=sin(phi);
		double size=STAR_SIZE(stars[i].mag);
		// size correction
		size*=AUTO_SIZE_COEF;
		stars[i].size=(float)size;
		stars[i].pos[0]=(float)(STAR_DIST(size)*Cphi*Stheta);
		stars[i].pos[1]=(float)(STAR_DIST(size)*Sphi*Stheta);
		stars[i].pos[2]=(float)(STAR_DIST(size)*Ctheta);
	}
}





void CStarMap::InitGL()
{
	glPointSize(point_size);
	bool autosize=CVideoBase::GetExtPointParams();
	if (autosize)
	{
		PFNGLPOINTPARAMETERFARBPROC glPointParameterfARB = NULL;
		PFNGLPOINTPARAMETERFVARBPROC glPointParameterfvARB = NULL;
		glPointParameterfARB=(PFNGLPOINTPARAMETERFARBPROC)wglGetProcAddress("glPointParameterfARB");
		glPointParameterfvARB=(PFNGLPOINTPARAMETERFVARBPROC)wglGetProcAddress("glPointParameterfvARB");
		if (glPointParameterfvARB!=NULL)
		{
			GLfloat DistAttenFactors[3] = {0.0f, 0.0f, (float)QUAD_ATTEN};
			glPointParameterfvARB(GL_POINT_DISTANCE_ATTENUATION_ARB,DistAttenFactors);
		}
	}
}





void CStarMap::DrawStars()
{
	int i;
	int lastsize10=(int)(point_size*10.0f);
	bool autosize=CVideoBase::GetExtPointParams();
	glPushAttrib(GL_ENABLE_BIT);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_LIGHTING);
	glDisable(GL_CULL_FACE);
	glEnable(GL_POINT_SMOOTH);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA,GL_ONE);
	{
		glBegin(GL_POINTS);
		{
			for (i=0;i<num_stars;i++)
			{
				if (!autosize)
				{
					int size10=(int)(stars[i].size*10.0f);
					if (size10!=lastsize10)
					{
						glEnd();
						glPointSize(stars[i].size*point_size);
						glBegin(GL_POINTS);
						lastsize10=size10;
					}
				}
				glColor3fv((GLfloat*)&stars[i].color);
				glVertex3fv((GLfloat*)&stars[i].pos);
			}
		}
		glEnd();
	}
	glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
	glPopAttrib();
}
