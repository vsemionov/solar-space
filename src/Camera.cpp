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

#include "Loader.h"
#include "Body.h"
#include "Info.h"
#include "Camera.h"
#include "VideoBase.h"



#define DIST_CLIP_NEAR 0.20
#define DIST_CLIP_FAR 500.0

#define MAX_DIST (DIST_CLIP_FAR*0.5)

#define MIN_FOV 0.001
#define MAX_FOV 66.7


#define INIT_TIME		CAMERA_INIT_TIME
#define INIT_FADE_TIME	CAMERA_INIT_FADE_TIME

#define CAM_DURATION 30.0f
#define ZOOM_SPEED 0.66f
#define MOVE_SPEED 20.0f
#define AIM_SPEED 30.0f

#define WIDE_CAM_GEN_ALT 1 /* 1/0 */
#define WIDE_CAM_TARGET_DIST 0.33f	/*	in halves of planetary system radius		*
									 *	affects deviation of sun from window center	*/

#define VIEW_FACTOR 1.50

#define ANGLE(size,dist) \
	(asin(size/max(dist, size))*(VIEW_FACTOR*2.0*180.0/M_PI))
#define DIST(size,angle) \
	(size/sin(angle*(M_PI/(180.0*2.0*VIEW_FACTOR))))

#define MIN_DIST DIST(mainbody->radius,MAX_FOV)

#define LEN(x,y,z) \
	sqrt(x*x+y*y+z*z)
#define LEN2(p,q) \
	sqrt(p*p+q*q)

#define CORRECT_ANGLES(start,end)	\
	{								\
		if ((start-end)>180.0f)		\
			start-=360.0f;			\
		else						\
		if ((start-end)<-180.0f)	\
			start+=360.0f;			\
	}











CCamera::CCamera()
{
	cx=cy=cz=0.0f;
	cyaw=cpitch=0.0f;
	aspect=4.0/3.0;
	fov=MAX_FOV;
}





CCamera::~CCamera()
{
}





void CCamera::Set(float x, float y, float z, float yaw, float pitch)
{
	cx=x;
	cy=y;
	cz=z;
	cyaw=yaw;
	cpitch=pitch;
}





void CCamera::Move(float forward, float right, float up, float yaw, float pitch)
{
	cyaw+=yaw;
	while (cyaw>180.0f)
		cyaw-=360.0f;
	while (cyaw<-180.0f)
		cyaw+=360.0f;
	cpitch+=pitch;
	if (cpitch>90.0f)
		cpitch=90.0f;
	if (cpitch<-90.0f)
		cpitch=-90.0f;
	float sinyaw=(float)sin(cyaw*(M_PI/180));
	float cosyaw=(float)cos(cyaw*(M_PI/180));
	float sinpitch=(float)sin(cpitch*M_PI/180);
	float cospitch=(float)cos(cpitch*M_PI/180);
	cz+=up+(forward*sinpitch);
	cx+=(forward*cosyaw*cospitch)+(right*sinyaw);
	cy+=(forward*sinyaw*cospitch)-(right*cosyaw);
}





void CCamera::ApplyRotation()
{
	glRotatef(-(cpitch+90.0f),1.0f,0.0f,0.0f);
	glRotatef(-(cyaw-90.0f),0.0f,0.0f,1.0f);
}





void CCamera::ApplyTranslation()
{
	glTranslatef(-cx,-cy,-cz);
}





void CCamera::Apply()
{
	ApplyRotation();
	ApplyTranslation();
}





void CCamera::Get(float *x, float *y, float *z, float *yaw, float *pitch)
{
	if (x) *x=cx;
	if (y) *y=cy;
	if (z) *z=cz;
	if (yaw) *yaw=cyaw;
	if (pitch) *pitch=cpitch;
}





void CCamera::ApplyFOV()
{
	static const double near_view_half=DIST_CLIP_NEAR*tan((MAX_FOV*M_PI/180.0)/2.0);
	double near_dist=near_view_half/tan((fov*M_PI/180.0)/2.0);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(fov,aspect,near_dist,DIST_CLIP_FAR);
	glMatrixMode(GL_MODELVIEW);
}





void CCamera::Init(CBody *refbody, CInfo *info, int scrwidth, int scrheight)
{
	mainbody=refbody;
	planetinfo=info;
	aspect=(double)scrwidth/(double)scrheight;
	this->scrwidth=scrwidth;
	this->scrheight=scrheight;
	float rc=mainbody->GetRadius(0,true)*0.5f;
	Set(-rc,-rc,rc,45.0f,-45.0f);
	fov=MAX_FOV;
	CLAMP(fov,MIN_FOV,MAX_FOV);
	ApplyFOV();
	target=0;
	action=shooting;
	type.chaseor=false;
	type.chasepos=false;
	type.children=false;
	fade=true;
	fadealpha=1.0f;
	endtime=INIT_TIME+INIT_FADE_TIME;
}





void CCamera::Restart(float seconds)
{
	starttime-=seconds;
	endtime-=seconds;
}





void CCamera::GenWideCam(float *len, float *pitch, float yaw)
{
	float rc=mainbody->GetRadius(0,true)*0.5f;
	float a;
	float trot;
	*len=(float)LEN(rc,rc,rc);
#if !WIDE_CAM_GEN_ALT
	trot=yaw;
	*pitch=(float)asin(rc/(*len));
	if (rand()&1) *pitch=-(*pitch);
	a=(float)LEN2(rc,rc)-rc;
#else
	(void)pitch; // avoid unused parameter warning
	(void)yaw; // avoid unused parameter warning
	a=rc*WIDE_CAM_TARGET_DIST;
	trot=(float)(rand()%(360*64))/64.0f;
	trot*=(float)(M_PI/180.0);
#endif
	nx=a*(float)cos(trot);
	ny=a*(float)sin(trot);
	nz=0.0f;
}





void CCamera::ChangeCam()
{
//CAMERA GENERATION IS ESSENTIAL TO VIEWING PLEASURE :)
//if chaseor==false, then camera should have max fov, pointing to 0,0,0
//else it should point to target with appropriate fov
//if chasepos==false, camera is still
//else it is close to target, and position is relative to body

	int gen=rand();
	type.chaseor=((gen&1)!=0);
	type.chasepos=((gen&2)!=0);
	type.children=((gen&4)!=0);
	target=(rand()%(mainbody->numbodies-1))+1; //exclude the star

	if (type.chasepos) type.chaseor=true; //exclude these ugly cams
	targetsize=mainbody->GetRadius(target,type.children);
	if (type.children==true && mainbody->GetRadius(target,false)==targetsize)
		type.children=false; //for correct view info on objects with no subbodies

	float len,yaw,pitch;
	yaw=(float)(rand()%(360*64))/64.0f;
	yaw*=(float)(M_PI/180.0);
	if (type.chasepos)
		len=(float)DIST(targetsize,MAX_FOV);
	else
		len=(float)(rand()%(int)((MAX_DIST-MIN_DIST)*16.0+1.0))/16.0f+(float)MIN_DIST;
	len=(float)min(len,MAX_DIST);
	nx=ny=nz=0.0f;
	double Z=((double)(rand()%(2*8192+1))/8192.0)-1.0;
	pitch=(float)asin(Z);
	if (!type.chasepos && !type.chaseor)
		GenWideCam(&len,&pitch,yaw);

	ex=(float)(cos(pitch)*cos(yaw))*len;
	ey=(float)(cos(pitch)*sin(yaw))*len;
	ez=(float)sin(pitch)*len;
}





void CCamera::Angles(float x, float y, float z, float *yaw, float *pitch)
{
	double l=LEN(x,y,z);
	if (l==0.0)
	{
		*yaw=cyaw;
		*pitch=cpitch;
		return;
	}
	double t=asin(z/l);
	*pitch=(float)(t*180.0/M_PI);
	double c=cos(t);
	if (c==0.0)
	{
		*yaw=cyaw;
		return;
	}
	*yaw=(float)(asin(y/(l*c))*180.0/M_PI);
	if (x<0.0f)
		*yaw=180.0f-*yaw;
}





void CCamera::Update(float seconds)
{
	static float tau;
	static double startfov;
	static float sx,sy,sz;
	static float syaw,spitch,eyaw,epitch;
	float tx=0.0f,ty=0.0f,tz=0.0f;
	float x,y,z;
	float yaw,pitch;
	float f,lr,tp;
	float l;
	switch (action)
	{
	default:
	case shooting:
		if (seconds<endtime)
		{
			if (type.chaseor || type.chasepos)
				mainbody->Predict(target,true,0.0f,&tx,&ty,&tz);
			if (type.chasepos)
			{
				cx=tx+ex; cy=ty+ey; cz=tz+ez;
			}
			x=-cx; y=-cy; z=-cz;
			if (type.chaseor)
			{
				x+=tx; y+=ty; z+=tz;
			}
			if (type.chaseor^type.chasepos)
			{
				Angles(x,y,z,&cyaw,&cpitch);
			}
			if (type.chaseor && !type.chasepos)
			{
				fov=ANGLE(targetsize,LEN(x,y,z));
				CLAMP(fov,MIN_FOV,MAX_FOV);
				ApplyFOV();
			}
		}
		else
		{
			action=zoomingout;
			starttime=endtime;
			startfov=fov;
			Update(seconds);
		}
		break;
	case zoomingout:
		fov=startfov*exp(ZOOM_SPEED*(seconds-starttime));
		CLAMP(fov,MIN_FOV,MAX_FOV);
		ApplyFOV();
		if (fov>=MAX_FOV)
		{
			ChangeCam();
			action=moving;
			if (type.chasepos)
				mainbody->Predict(target,true,0.0f,&tx,&ty,&tz);
			x=ex-cx+tx; y=ey-cy+ty; z=ez-cz+tz;
			sx=cx; sy=cy; sz=cz;
			starttime=starttime+(float)log(fov/startfov)/ZOOM_SPEED;
			tau=(float)LEN(x,y,z)/MOVE_SPEED;
			endtime=starttime+tau;
			Update(seconds);
		}
		break;
	case moving:
		if (seconds<endtime)
		{
			tp=(endtime-seconds);
			if (!type.chasepos)
			{
				f=1.0f-((tp*tp)/(tau*tau));
				cx=sx+(ex-sx)*f; cy=sy+(ey-sy)*f; cz=sz+(ez-sz)*f;
			}
			else
			{
				mainbody->Predict(target,true,0.0f,&tx,&ty,&tz);
				f=tp*tp*MOVE_SPEED/tau;
				x=ex-cx+tx; y=ey-cy+ty; z=ez-cz+tz;
				l=(float)LEN(x,y,z);
				if (l!=0.0f)
				{
					lr=f/l;
					x*=lr; y*=lr; z*=lr;
					cx=ex+tx-x; cy=ey+ty-y; cz=ez+tz-z;
				}
			}
		}
		else
		{
			action=pointing;
			if (type.chasepos || type.chaseor)
				mainbody->Predict(target,true,0.0f,&tx,&ty,&tz);
			cx=ex; cy=ey; cz=ez;
			if (type.chasepos)
			{
				cx+=tx; cy+=ty; cz+=tz;
			}
			x=-cx; y=-cy; z=-cz;
			if (type.chaseor)
			{
				x+=tx; y+=ty; z+=tz;
			}
			x+=nx; y+=ny; z+=nz;
			Angles(x,y,z,&eyaw,&epitch);
			syaw=cyaw; spitch=cpitch;
			CORRECT_ANGLES(syaw,eyaw);
			yaw=eyaw-syaw;
			pitch=epitch-spitch;
			starttime=endtime;
			tau=(float)LEN2(yaw,pitch)/AIM_SPEED;
			endtime=starttime+tau;
			Update(seconds);
		}
		break;
	case pointing:
		if (seconds<endtime)
		{
			mainbody->Predict(target,true,0.0f,&tx,&ty,&tz);
			if (type.chasepos)
			{
				cx=tx+ex; cy=ty+ey; cz=tz+ez;
			}
			tp=(endtime-seconds);
			if (!(type.chasepos^type.chaseor))
			{
				f=1.0f-((tp*tp)/(tau*tau));
				cyaw=syaw+(eyaw-syaw)*f;
				cpitch=spitch+(epitch-spitch)*f;
			}
			else
			{
				f=tp*tp*AIM_SPEED/tau;
				x=-cx; y=-cy; z=-cz;
				if (type.chaseor)
				{
					x+=tx; y+=ty; z+=tz;
				}
				Angles(x,y,z,&eyaw,&epitch);
				CORRECT_ANGLES(cyaw,eyaw);
				yaw=eyaw-cyaw;
				pitch=epitch-cpitch;
				l=(float)LEN2(pitch,yaw);
				if (l!=0.0f)
				{
					lr=f/l;
					yaw*=lr; pitch*=lr;
					cyaw=eyaw-yaw; cpitch=epitch-pitch;
				}
			}
		}
		else
		{
			if (type.chaseor || type.chaseor)
				mainbody->Predict(target,true,0.0f,&tx,&ty,&tz);
			cx=ex; cy=ey; cz=ez;
			if (type.chasepos)
			{
				cx+=tx; cy+=ty; cz+=tz;
			}
			x=-cx; y=-cy; z=-cz;
			if (type.chaseor)
			{
				x+=tx; y+=ty; z+=tz;
			}
			x+=nx; y+=ny; z+=nz;
			Angles(x,y,z,&cyaw,&cpitch);
			if (type.chaseor==true && type.chasepos==false)
			{
				action=zoomingin;
				startfov=fov;
				starttime=endtime;
				Update(seconds);
			}
			else
			{
				action=shooting;
				if (planetinfo)
				{
					starttime=endtime;
					planetinfo->Start(	starttime,
										CAM_DURATION,
										(type.chaseor?CBody::bodycache[target]->name:CBody::systemname),
										(type.chaseor?CBody::bodycache[target]:NULL));
				}
				endtime=starttime+CAM_DURATION;
				Update(seconds);
			}
		}
		break;
	case zoomingin:
		mainbody->Predict(target,true,0.0f,&tx,&ty,&tz);
		x=tx-cx; y=ty-cy; z=tz-cz;
		Angles(x,y,z,&cyaw,&cpitch);
		fov=startfov*exp(-ZOOM_SPEED*(seconds-starttime));
		CLAMP(fov,MIN_FOV,MAX_FOV);
		ApplyFOV();
		if (fov<=ANGLE(targetsize,LEN(x,y,z)) || fov<=MIN_FOV)
		{
			action=shooting;
			starttime=starttime-(float)log(fov/startfov)/ZOOM_SPEED;
			if (planetinfo)
			{
				planetinfo->Start(	starttime,
									CAM_DURATION,
									(type.chaseor?CBody::bodycache[target]->name:CBody::systemname),
									(type.chaseor?CBody::bodycache[target]:NULL));
			}
			endtime=starttime+CAM_DURATION;
			Update(seconds);
		}
		break;
	}

	if (fade)
	{
		fadealpha=1.0f-(seconds/INIT_FADE_TIME);
		if (seconds>INIT_FADE_TIME)
		{
			fade=false;
			fadealpha=0.0f;
		}
	}
}





void CCamera::DrawFade()
{
	if (!fade)
		return;

	glLoadIdentity();
	glPushAttrib(GL_ENABLE_BIT);
	{
		glDisable(GL_LIGHTING);
		glDisable(GL_TEXTURE_2D);
		glDisable(GL_DEPTH_TEST);
		glDisable(GL_CULL_FACE);
		glEnable(GL_BLEND);
		glColor4f(0.0f,0.0f,0.0f,fadealpha);
		glBegin(GL_QUADS);
		{
			glVertex2f(0.0f,0.0f);
			glVertex2f((float)scrwidth,0.0f);
			glVertex2f((float)scrwidth,(float)scrheight);
			glVertex2f(0.0f,(float)scrheight);
		}
		glEnd();
	}
	glPopAttrib();
}
