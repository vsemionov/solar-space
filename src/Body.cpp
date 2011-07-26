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
#include "Main.h"
#include "Loader.h"
#include "VideoBase.h"
#include "Error.h"
#include "GamePlay.h"
#include "Body.h"
#include "LensFlare.h"




#define SYSTEM_NAME_RESOURCE CSettings::DataFile
#define SYSTEM_NAME_FILE "SYSTEM.TXT"
#define BODY_DATA_RESOURCE SYSTEM_NAME_RESOURCE
#define BODY_DATA_FILE "BODIES.TXT"
#define BODY_GFX_RESOURCE BODY_DATA_RESOURCE

#define BODY_SEGMENTS_BASE 32
#define SEGMENTS ((int)((float)BODY_SEGMENTS_BASE*CVideoBase::GetOptGeoDetail()))
#define SPECULAR_SEGMENTS SEGMENTS

#define AMBIENT_REFLECTION 0.175f
#define DIFFUSE_REFLECTION (1.0f-AMBIENT_REFLECTION)

#define WATER_SPECULAR 1.00f
#define WATER_SHININESS 48.0f
#define STAR_EMISSION DIFFUSE_REFLECTION
#define RING_AMBIENT 0.30f
#define RING_DIFFUSE (1.0f-RING_AMBIENT)
#define ASTEROID_REFLECTION 0.50f





CLoader CBody::loader;

int CBody::system_format_version=0;
char CBody::systemname[SYSTEM_NAME_SIZE];
char **CBody::textlines=NULL;
int CBody::numlines=0;
int CBody::lineindex=0;
int CBody::numbodies=0;
CBody CBody::ringchain;
CBody **CBody::bodycache=NULL;
float CBody::distmult;
float CBody::radmult;
float CBody::orbtimemult;
float CBody::owntimemult;









CBody::CBody()
{
	Init();
}





CBody::~CBody()
{
	Destroy();
}





void CBody::Init()
{
	id=-1;
	mainbody=false;
	name[0]=0;
	type=(bodytype_e)(-1);
	ZeroMemory(tex_names,sizeof(tex_names));
	ZeroMemory(obj_name,sizeof(obj_name));
	ZeroMemory(info_name,sizeof(info_name));
	ZeroMemory(objects,sizeof(objects));
	ZeroMemory(textures,sizeof(textures));
	numsubbodies=0;
	subbodies=NULL;
	maxchilddist=0.0f;
	ZeroMemory(&info,sizeof(info));
}





bool CBody::LoadSystemData(char *resource, int *format_version, char *buffer,bool quiet)
{
	if (!resource)
		resource=SYSTEM_NAME_RESOURCE;
	CLoader loader;
	if (!loader.WithResource(resource))
	{
		if (!quiet)
			CError::LogError(ERROR_CODE,"Unable to load planetary system - missing or invalid resource.");
		return false;
	}
	if (!loader.LoadText(SYSTEM_NAME_FILE,&textlines,&numlines))
	{
		if (!quiet)
			CError::LogError(ERROR_CODE,"Unable to load planetary system - file missing from resource or internal loader subsystem error.");
		return false;
	}
	if (textlines==NULL)
	{
		if (!quiet)
			CError::LogError(ERROR_CODE,"Unable to load planetary system - internal loader subsystem error.");
		return false;
	}
	if (numlines==0)
	{
		if (!quiet)
			CError::LogError(ERROR_CODE,"Unable to load planetary system - empty data file.");
		return false;
	}
	lineindex=-1;

	int nfield=0;
	while (nfield<2)
	{
		do
		{
			lineindex++;
			if (lineindex>=numlines)
			{
				if (!quiet)
					CError::LogError(ERROR_CODE,"Unable to load planetary system - unexpected end of file.");
				for (int i=0;i<numlines;i++) free(textlines[i]);
				free(textlines); textlines=NULL;
				lineindex=0;
				numlines=0;
				return false;
			}
		} while (strlen(textlines[lineindex])==0 || textlines[lineindex][0]=='/');

		switch (nfield)
		{
		case 0:
			if (!format_version)
				break;
			*format_version=atoi(textlines[lineindex]);
			break;
		case 1:
			if (!buffer)
				break;
			strncpy(buffer,textlines[lineindex],SYSTEM_NAME_SIZE);
			buffer[SYSTEM_NAME_SIZE-1]=0;
			break;
		default:
			break;
		}

		nfield++;
	}

	for (int i=0;i<numlines;i++) free(textlines[i]);
	free(textlines); textlines=NULL;
	lineindex=0;
	numlines=0;

	return true;
}





bool CBody::Load()
{
	int i;
	bool b=true;
	if (textlines==NULL)
	{
		mainbody=true;
		GLfloat AmbientReflection[4]={AMBIENT_REFLECTION,AMBIENT_REFLECTION,AMBIENT_REFLECTION,1.0f};
		GLfloat DiffuseReflection[4]={DIFFUSE_REFLECTION,DIFFUSE_REFLECTION,DIFFUSE_REFLECTION,1.0f};
		GLfloat SpecularReflection[4]={0.0f,0.0f,0.0f,1.0f};
		GLfloat Shininess=128.0f;
		GLfloat ZeroReflection[4]={0.0f,0.0f,0.0f,1.0f};
		glMaterialfv(GL_FRONT_AND_BACK,GL_AMBIENT,AmbientReflection);
		glMaterialfv(GL_FRONT,GL_DIFFUSE,DiffuseReflection);
		glMaterialfv(GL_FRONT,GL_SPECULAR,SpecularReflection);
		glMaterialf(GL_FRONT,GL_SHININESS,Shininess);
		glMaterialfv(GL_BACK,GL_DIFFUSE,ZeroReflection);
		glMaterialfv(GL_BACK,GL_SPECULAR,ZeroReflection);
		glMaterialf(GL_BACK,GL_SHININESS,Shininess);
		if (!LoadSystemData())
		{
			CError::LogError(ERROR_CODE,"Loading of planetary system failed - aborting.");
			return false;
		}
		if (!loader.WithResource(BODY_DATA_RESOURCE))
		{
			CError::LogError(ERROR_CODE,"Unable to load bodies - missing or invalid resource.");
			return false;
		}
		if (!loader.LoadText(BODY_DATA_FILE,&textlines,&numlines))
		{
			CError::LogError(ERROR_CODE,"Unable to load bodies - file missing from resource or internal loader subsystem error.");
			return false;
		}
		if (textlines==NULL)
		{
			CError::LogError(ERROR_CODE,"Unable to load bodies - internal loader subsystem error.");
			return false;
		}
		if (numlines==0)
		{
			CError::LogError(ERROR_CODE,"Unable to load bodies - empty data file.");
			return false;
		}
		bodycache=(CBody**)malloc(numlines*sizeof(CBody*));
		if (!bodycache)
		{
			CError::LogError(ERROR_CODE,"Unable to load bodies - memory allocation failed.");
			return false;
		}
		lineindex=0;
		b=LoadMultipliers();
		if (b)
			b=Load();
		for (i=0;i<numlines;i++) free(textlines[i]);
		free(textlines); textlines=NULL;
		numlines=0;
		if (!b)
		{
			CError::LogError(ERROR_CODE,"Loading of planetary system failed - aborting.");
			return false;
		}
		CalcMaxChildDist();
		bodycache=(CBody**)realloc(bodycache,numbodies*sizeof(CBody*));
		if (!bodycache)
		{
			CError::LogError(ERROR_CODE,"Unable to load bodies - memory allocation failed.");
			return false;
		}
		b=Reload();
		loader.End();
		return b;
	}
	else
	{
		if (!LoadPhys())
		{
			CError::LogError(ERROR_CODE,"Parsing of body data failed - aborting.");
			return false;
		}
		id=numbodies;
		numbodies++;
		if (numsubbodies>0)
		{
			subbodies=new CBody[numsubbodies];
			if (subbodies==NULL)
			{
				CError::LogError(ERROR_CODE,"Unable to load subbody - memory allocation failed.");
				numsubbodies=0;
				return false;
			}
			for (i=0;i<numsubbodies;i++)
			{
				lineindex++;
				if (lineindex>=numlines)
				{
					CError::LogError(ERROR_CODE,"Unable to load subbody - unexpected end of file.");
					numsubbodies=i;
					b=false;
					break;
				}
				b&=subbodies[i].Load();
			}
		}
		return b;
	}
}





bool CBody::LoadMultipliers()
{
	while (sscanf(textlines[lineindex],"%f",&distmult)!=1 || textlines[lineindex][0]=='/')
	{
		lineindex++;
		if (lineindex>=numlines)
		{
			CError::LogError(ERROR_CODE,"Unable to load body - unexpected end of file.");
			return false;
		}
	}
	lineindex++;
	while (sscanf(textlines[lineindex],"%f",&radmult)!=1 || textlines[lineindex][0]=='/')
	{
		lineindex++;
		if (lineindex>=numlines)
		{
			CError::LogError(ERROR_CODE,"Unable to load body - unexpected end of file.");
			return false;
		}
	}
	lineindex++;
	while (sscanf(textlines[lineindex],"%f",&orbtimemult)!=1 || textlines[lineindex][0]=='/')
	{
		lineindex++;
		if (lineindex>=numlines)
		{
			CError::LogError(ERROR_CODE,"Unable to load body - unexpected end of file.");
			return false;
		}
	}
	lineindex++;
	while (sscanf(textlines[lineindex],"%f",&owntimemult)!=1 || textlines[lineindex][0]=='/')
	{
		lineindex++;
		if (lineindex>=numlines)
		{
			CError::LogError(ERROR_CODE,"Unable to load body - unexpected end of file.");
			return false;
		}
	}
	lineindex++;
	return true;
}





void CBody::Destroy()
{
	if (subbodies!=NULL)
	{
		delete [] subbodies;
	}
	FreeGFX();
	if (info.textlines)
	{
		for (int i=0; i<info.numlines; i++)
		{
			if (info.textlines[i])
			{
				free(info.textlines[i]);
			}
		}
		free(info.textlines);
	}
	numbodies--;
	if (mainbody)
	{
		if (bodycache)
			free(bodycache);
		bodycache=NULL;
		numbodies=0;
	}
	Init();
}





void CBody::UpdateOrbit(float seconds)
{
	orbit_rot=orbit_rot_start;
	if (orbit_period)
		orbit_rot+=(360.0f*seconds)/orbit_period;
	float orbit_rot_angle=(orbit_rot*(float)M_PI)/180.0f;
	orb_pos[0]=distance*(float)cos(orbit_rot_angle);
	orb_pos[1]=distance*(float)sin(orbit_rot_angle);
}





void CBody::Update(float seconds)
{
	int i;
	for (i=0;i<numsubbodies;i++)
	{
		subbodies[i].Update(seconds);
	}
	own_rot=own_rot_start;
	if (own_period)
		own_rot+=(360.0f*seconds)/own_period;
	UpdateOrbit(seconds);
	if (type!=rings && textures[3]>0)
	{
		clouds_offset=clouds_rot_start;
		if (clouds_period)
			clouds_offset+=seconds/clouds_period;
	}
}





void CBody::Restart()
{
	int i;
	for (i=0;i<numsubbodies;i++)
	{
		subbodies[i].Restart();
	}
	orbit_rot_start=orbit_rot;
	own_rot_start=own_rot;
	clouds_rot_start=clouds_offset;
	if (type==rings || textures[3]==0)
		clouds_rot_start=0.0f;
	while (orbit_rot_start>180.0f)
		orbit_rot_start-=360.0f;
	while (orbit_rot_start<-180.0f)
		orbit_rot_start+=360.0f;
	while (own_rot_start>180.0f)
		own_rot_start-=360.0f;
	while (own_rot_start<-180.0f)
		own_rot_start+=360.0f;
	while (clouds_rot_start>0.5f)
		clouds_rot_start-=1.0f;
	while (clouds_rot_start<-0.5f)
		clouds_rot_start+=1.0f;
}





void CBody::TransformPosition()
{
	glRotatef(orb_incl[3],orb_incl[0],orb_incl[1],orb_incl[2]);
	if (type!=rings)
	{
		glTranslatef(orb_pos[0],orb_pos[1],0.0f);
		glRotatef(own_incl[3],own_incl[0],own_incl[1],own_incl[2]);
	}
}





void CBody::TransformClouds()
{
	glMatrixMode(GL_TEXTURE);
	glTranslatef(-clouds_offset,0.0f,0.0f);
	glPushMatrix();
	glLoadIdentity();
	glMatrixMode(GL_MODELVIEW);
}





void CBody::DrawSortedNonSolids()
{
	CBody *ring=ringchain.nextring;
	while (ring!=&ringchain)
	{
		glLoadMatrixf(ring->objectmatrix);
		glCallList(ring->objects[0]);
		ring=ring->nextring;
	}
}





void CBody::Draw(CLensFlare *flare)
{
	TransformPosition();
	if (mainbody)
	{
		ringchain.nextring=ringchain.prevring=&ringchain;
	}
	for (int i=0;i<numsubbodies;i++)
	{
		glPushMatrix();
		subbodies[i].Draw(NULL);
		glPopMatrix();
	}
	if (mainbody)
	{
		if (flare)
			flare->UpdatePos();
	}
	glRotatef((type!=rings?own_rot:orbit_rot),0.0f,0.0f,1.0f);
	if (type!=rings)
	{
		if (textures[3]>0)
			TransformClouds();
		DrawGFX();
	}
	else
	{
		SortNonSolidObject();
	}
	if (mainbody)
	{
		DrawSortedNonSolids();
	}
}





bool CBody::Reload()
{
	int i;
	if (mainbody==true)
	{
		if (loader.WithResource(BODY_GFX_RESOURCE)!=true)
		{
			CError::LogError(ERROR_CODE,"Unable to load body GFX - missing or invalid resource.");
			return false;
		}
	}
	FreeGFX();
	if (!LoadGFX())
	{
		CError::LogError(ERROR_CODE,"Failed to load body GFX - aborting.");
		return false;
	}
	bodycache[id]=this;
	if (subbodies!=NULL)
	{
		for (i=0;i<numsubbodies;i++)
		{
			if (!subbodies[i].Reload())
			{
				CError::LogError(ERROR_CODE,"Failed to load subbody GFX - aborting.");
				return false;
			}
		}
	}
	return true;
}





bool CBody::LoadPhys()
{
#define VA_NUM 24
#define VA_FMT "%31s | %d | %f %f | %f | %f %f | %f %f | %f %f | %f %f | %f %f | %d | %12s %12s | %12s | %12s | %12s %12s | %12s | %12s"
#define VA_ARGS				\
	tmpname,				\
	(int *)&type,			\
	&distance,				\
	&radius,				\
	&shape,					\
	&orbit_period,			\
	&own_period,			\
	&orbit_rot_start,		\
	&own_rot_start,			\
	&orb_incl[3],			\
	&own_incl[3],			\
	&orb_incl_dir_angle,	\
	&own_incl_dir_angle,	\
	&clouds_period,			\
	&clouds_rot_start,		\
	&numsubbodies,			\
	tex_names[0][0],		\
	tex_names[0][1],		\
	tex_names[1][0],		\
	tex_names[2][1],		\
	tex_names[3][0],		\
	tex_names[3][1],		\
	obj_name,				\
	info_name
/////////////////////
	float orb_incl_dir_angle, own_incl_dir_angle;
	char tmpname[sizeof(name)];
	int l;
	int i;
	int idx;
	while (sscanf(textlines[lineindex],VA_FMT,VA_ARGS)!=VA_NUM || textlines[lineindex][0]=='/')
	{
		ZeroMemory(tex_names,sizeof(tex_names));
		ZeroMemory(obj_name,sizeof(obj_name));
		ZeroMemory(info_name,sizeof(info_name));
		lineindex++;
		if (lineindex>=numlines)
		{
			CError::LogError(ERROR_CODE,"Unable to load body - unexpected end of file.");
			return false;
		}
	}
	l=strlen(tmpname);
	for (i=0;i<l;i++)
		if (tmpname[i]=='_')
			tmpname[i]=' ';
	for (i=l-1;i>=0;i--)
	{
		if (tmpname[i]!=' ')
			break;
		tmpname[i]=0;
	}
	for (idx=0;idx<l;idx++)
		if (tmpname[idx]!=' ')
			break;
	strcpy(name,tmpname+idx);
	if (obj_name[0]=='.')
		obj_name[0]=0;
	if (info_name[0]=='.')
		info_name[0]=0;
	for (i=0;i<4;i++)
	{
		if (tex_names[i][0][0]=='.')
			tex_names[i][0][0]=0;
		if (tex_names[i][1][0]=='.')
			tex_names[i][1][0]=0;
	}
	if (tex_names[2][1][0]!=0)
		strcpy(tex_names[2][0],tex_names[0][0]);
	if (orbit_rot_start==0.0f)
		orbit_rot_start=(float)(rand()%(360*64))/64.0f;
	if (own_rot_start==0.0f)
		own_rot_start=(float)(rand()%(360*64))/64.0f;
	if (orb_incl_dir_angle==0.0f)
		orb_incl_dir_angle=(float)(rand()%(360*64))/64.0f;
	if (own_incl_dir_angle==0.0f)
		own_incl_dir_angle=(float)(rand()%(360*64))/64.0f;
	orb_incl[0]=radius*(float)cos(orb_incl_dir_angle*M_PI/180);
	orb_incl[1]=radius*(float)sin(orb_incl_dir_angle*M_PI/180);
	orb_incl[2]=0.0f;
	own_incl[0]=radius*(float)cos(own_incl_dir_angle*M_PI/180);
	own_incl[1]=radius*(float)sin(own_incl_dir_angle*M_PI/180);
	own_incl[2]=0.0f;
	if (!orbit_period)
		orbit_period=(float)sqrt(distance*distance*distance);
	if (clouds_rot_start==0.0f)
		clouds_rot_start=(float)(rand()%(360*64))/64.0f;
	clouds_rot_start/=360.0f;
	distance*=(type!=rings?distmult:radmult);
	radius*=radmult;
	orbit_period*=orbtimemult;
	own_period*=owntimemult;
	if (!own_period)
		own_period=orbit_period;
	clouds_period*=owntimemult;
	if (info_name[0])
	{
		if (!LoadInfo())
		{
			CError::LogError(WARNING_CODE,"Failed to load body info - ignoring.");
		}
	}
	return true;
}





bool CBody::LoadInfo()
{
	if (!loader.WithResource(BODY_DATA_RESOURCE))
	{
		CError::LogError(WARNING_CODE,"Unable to load body info - missing or invalid resource.");
		return false;
	}
	if (!loader.LoadText(info_name,&info.textlines,&info.numlines))
	{
		CError::LogError(WARNING_CODE,"Unable to load body info - file missing from resource or internal loader subsystem error.");
		return false;
	}
	if (info.textlines==NULL)
	{
		CError::LogError(WARNING_CODE,"Unable to load body info - internal loader subsystem error.");
		return false;
	}
	if (info.numlines==0)
	{
		CError::LogError(WARNING_CODE,"Unable to load body info - empty data file.");
		return false;
	}
	return true;
}





bool CBody::LoadTextures()
{
	int i;
	for (i=0;i<4;i++)
	{
		if (CGamePlay::UserAbortedLoad())
		{
			CError::LogError(ERROR_CODE,"Loading of body aborted by user.");
			return false;
		}
		textures[i]=loader.LoadTexture(tex_names[i][0],tex_names[i][1],CVideoBase::GetOptMipmaps(),CVideoBase::GetOptLinear(),CVideoBase::GetOptAnisotropic());
		if (textures[i]==0 && (tex_names[i][0][0]!=0 || tex_names[i][1][0]!=0))
		{
			CError::LogError(WARNING_CODE,"Failed to load a body texture - ignoring.");
		}
	}
	return true;
}





void CBody::RenderDisk()
{
	int i;
	float f;
	float angle;
	int segments=(int)(SEGMENTS)*2;
	float r1=distance;
	float r2=distance+radius;
	glBegin(GL_QUAD_STRIP);
	{
		glNormal3f(0.0f,0.0f,1.0f);
		for (i=0;i<=segments;i++)
		{
			f=(float)i/(float)segments;
			angle=(float)(2*M_PI)*f;
			glTexCoord2f(1.0f,1.0f-f);
			glVertex2f(r1*(float)cos(angle),r1*(float)sin(angle));
			glTexCoord2f(0.0f,1.0f-f);
			glVertex2f(r2*(float)cos(angle),r2*(float)sin(angle));
		}
	}
	glEnd();
}





void CBody::MakeStar(GLUquadricObj *quadric)
{
	GLfloat StarEmission[4]={STAR_EMISSION,STAR_EMISSION,STAR_EMISSION, 1.0f};
	glMaterialfv(GL_FRONT,GL_EMISSION,StarEmission);
	MakePlanetoid(quadric);
}





void CBody::MakePlanetoid(GLUquadricObj *quadric)
{
	GLfloat MatSpec[4]={WATER_SPECULAR,WATER_SPECULAR,WATER_SPECULAR,1.0f};
	GLfloat MatShiny=WATER_SHININESS;
	int stacks=((textures[2]>0)?SPECULAR_SEGMENTS:SEGMENTS);
	int slices=stacks*2;
	if (shape!=1.0f)
	{
		glScalef(1.0f,1.0f,shape);
		glEnable(GL_NORMALIZE);
	}
	glBindTexture(GL_TEXTURE_2D,textures[0]);
	gluSphere(quadric,radius,slices,stacks);
	if (textures[1]>0 || textures[2]>0 || textures[3]>0)
	{
		glEnable(GL_BLEND);
		if (textures[1]>0)
		{
			//BUMP MAPPING?
		}
		if (textures[2]>0)
		{
			glMaterialfv(GL_FRONT,GL_SPECULAR,MatSpec);
			glMaterialf(GL_FRONT,GL_SHININESS,MatShiny);
			glBindTexture(GL_TEXTURE_2D,textures[2]);
			gluSphere(quadric,radius,slices,stacks);
		}
		if (textures[3]>0)
		{
			glMatrixMode(GL_TEXTURE);
			glPopMatrix();
			glBindTexture(GL_TEXTURE_2D,textures[3]);
			gluSphere(quadric,radius,slices,stacks);
			glLoadIdentity();
			glMatrixMode(GL_MODELVIEW);
		}
	}
}





void CBody::MakeRings()
{
	GLfloat RingAmbient[4]={RING_AMBIENT,RING_AMBIENT,RING_AMBIENT,1.0f};
	GLfloat RingDiffuse[4]={RING_DIFFUSE,RING_DIFFUSE,RING_DIFFUSE,1.0f};
	glMaterialfv(GL_FRONT_AND_BACK,GL_AMBIENT,RingAmbient);
	glMaterialfv(GL_FRONT_AND_BACK,GL_DIFFUSE,RingDiffuse);
	glLightModeli(GL_LIGHT_MODEL_TWO_SIDE,1);
	glEnable(GL_BLEND);
	glDisable(GL_CULL_FACE);
	glBindTexture(GL_TEXTURE_2D,textures[0]);
	RenderDisk();
}





void CBody::MakeAsteroid()
{
	int i;
	float objsize=0.0f;
	CLoader::object_t *object;
	object=loader.LoadObject(obj_name);
	if (object==NULL)
	{
		CError::LogError(ERROR_CODE,"Unable to load asteroid - file missing from resource or internal loader subsystem error while loading object.");
		return;
	}
	for (i=0;i<object->numvertices;i++)
	{
		float x=object->vertices[i].point[0];
		float y=object->vertices[i].point[1];
		float z=object->vertices[i].point[2];
		float veclen=(float)sqrt(x*x+y*y+z*z);
		if (objsize<veclen)
			objsize=veclen;
	}
	float scale=radius/objsize;
	// these are used only if no texture is present for the asteroid
	float ma[4]={AMBIENT_REFLECTION*ASTEROID_REFLECTION,AMBIENT_REFLECTION*ASTEROID_REFLECTION,AMBIENT_REFLECTION*ASTEROID_REFLECTION,1.0f};
	float md[4]={DIFFUSE_REFLECTION*ASTEROID_REFLECTION,DIFFUSE_REFLECTION*ASTEROID_REFLECTION,DIFFUSE_REFLECTION*ASTEROID_REFLECTION,1.0f};
	if (!textures[0])
	{
		glMaterialfv(GL_FRONT,GL_AMBIENT,ma);
		glMaterialfv(GL_FRONT,GL_DIFFUSE,md);
	}
	glBindTexture(GL_TEXTURE_2D,textures[0]);
	glRotatef(90.0f,1.0f,0.0f,0.0f);
	glScalef(scale,scale,scale);
	glEnable(GL_NORMALIZE);
	bool smooth=(textures[0] || (textures[1] && CVideoBase::GetOptBumpMaps()));
	// smooth looks better when not textured too
	smooth=true;
	glBegin(GL_TRIANGLES);
	{
		for (i=0;i<object->numfaces;i++)
		{
			if (!smooth)
				glNormal3f(object->faces[i].normal[0],object->faces[i].normal[1],object->faces[i].normal[2]);
			for (int j=0;j<3;j++)
			{
				int vertexnum=object->faces[i].vertexnum[j];
				CLoader::vertex_t *vertex=&object->vertices[vertexnum];
				float tx=vertex->tex_coords[0];
				if (tx<0.25f)
				{
					for (int k=0;k<3;k++)
					{
						if (k!=j)
						{
							if (object->vertices[object->faces[i].vertexnum[k]].tex_coords[0]>0.75f)
							{
								tx+=1.0f;
								break;
							}
						}
					}
				}
				if (smooth)
				{
					glNormal3f(vertex->normal[0],vertex->normal[1],vertex->normal[2]);
					if (textures[0])
						glTexCoord2f(tx,vertex->tex_coords[1]);
				}
				glVertex3f(vertex->point[0],vertex->point[1],vertex->point[2]);
			}
		}
	}
	glEnd();
	free(object->vertices);
	free(object->faces);
	free(object); object=NULL;
}





bool CBody::LoadGFX()
{
	if (CGamePlay::UserAbortedLoad())
	{
		CError::LogError(ERROR_CODE,"Loading of body aborted by user.");
		return false;
	}
	CGamePlay::UpdateSplash(name);
	GLUquadricObj *quadric=NULL;
	quadric=gluNewQuadric();
	if (quadric==NULL)
	{
		CError::LogError(ERROR_CODE,"Unable to load body - unable to create an OpenGL quadric.");
		return false;
	}
	gluQuadricNormals(quadric,GLU_SMOOTH);
	gluQuadricOrientation(quadric,GLU_OUTSIDE);
	gluQuadricDrawStyle(quadric,GLU_FILL);
	gluQuadricTexture(quadric,GL_TRUE);
	objects[0]=glGenLists(1);
	if (objects[0]==0)
	{
		CError::LogError(ERROR_CODE,"Unable to load body - unable to create an OpenGL display list.");
		gluDeleteQuadric(quadric);
		return false;
	}
	if (!LoadTextures())
	{
		CError::LogError(ERROR_CODE,"Failed to load body textures.");
		FreeGFX();
		return false;
	}
	glNewList(objects[0],GL_COMPILE);
	{
		glPushAttrib(GL_ENABLE_BIT | GL_LIGHTING_BIT);
		CVideoBase::EnableMultisample();
		if (type==star) MakeStar(quadric);
		if (type==planetoid) MakePlanetoid(quadric);
		if (type==rings) MakeRings();
		if (type==asteroid) MakeAsteroid();
		CVideoBase::DisableMultisample();
		glPopAttrib();
	}
	glEndList();
	gluDeleteQuadric(quadric);
	quadric=NULL;
	return true;
}





bool CBody::CheckGFX()
{
	int i;
	if (glIsList(objects[0])!=GL_TRUE)
		return false;
	for (i=0;i<4;i++)
	{
		if (tex_names[i][0][0]!=0)
		{
			if (glIsTexture((GLuint)textures[i])!=GL_TRUE)
				return false;
		}
	}
	return true;
}





void CBody::FreeGFX()
{
	glDeleteTextures(4,(GLuint *)textures);
	ZeroMemory(textures,sizeof(textures));
	if (glIsList(objects[0]))
		glDeleteLists(objects[0],1);
	ZeroMemory(objects,sizeof(objects));
}





void CBody::DrawGFX()
{
	glCallList(objects[0]);
}





void CBody::SortNonSolidObject()
{
	glGetFloatv(GL_MODELVIEW_MATRIX,objectmatrix);
	objecteyez=objectmatrix[14];
	CBody *cur=ringchain.nextring;
	while (cur!=&ringchain && cur->objecteyez<objecteyez)
	{
		cur=cur->nextring;
	}
	prevring=cur->prevring;
	nextring=cur;
	cur->prevring=this;
	prevring->nextring=this;
}





void CBody::CalcMaxChildDist()
{
	if (type==rings)
		maxchilddist=max((distance+radius),distance);
	else
		maxchilddist=radius;
	for (int i=0;i<numsubbodies;i++)
	{
		subbodies[i].CalcMaxChildDist();
		float childmax=subbodies[i].maxchilddist;
		if (subbodies[i].type!=rings)
			childmax+=subbodies[i].distance;
		if (childmax>maxchilddist)
			maxchilddist=childmax;
	}
}





void CBody::Predict(int bodyid, bool now, float seconds, float *x, float *y, float *z)
{
	if (mainbody)
		glLoadIdentity();
	if (!now)
		UpdateOrbit(seconds);
	//no glLoadIdentity call, so coords are relative to camera
	TransformPosition();
	if (bodyid==id)
	{
		glPushMatrix();
		glGetFloatv(GL_MODELVIEW_MATRIX,objectmatrix);
		*x=objectmatrix[12]; *y=objectmatrix[13]; *z=objectmatrix[14];
		glPopMatrix();
		return;
	}
	// note: no error checking for incorrect results or protection faults
	int i;
	for (i=0; i<numsubbodies;i++)
		if (subbodies[i].id>bodyid)
			break;
	subbodies[--i].Predict(bodyid,now,seconds,x,y,z);
}





float CBody::GetRadius(int bodyid, bool withchildren)
{
	CBody *body=bodycache[bodyid];
	return (withchildren?
				body->maxchilddist:
				(body->type==rings?
					max((body->distance+body->radius),body->distance):
					body->radius));
}
