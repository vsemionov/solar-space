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

#ifndef BODY_H
#define BODY_H

#include <gl/glu.h>

#include "Defs.h"

#include "Loader.h"
#include "LensFlare.h"


#define SYSTEM_NAME_SIZE 32




class CBody
{
friend class CCamera;
friend class CLensFlare;
friend class CInfo;
public:
	CBody();
	virtual ~CBody();
	bool Load();
	void Destroy();
	void Update(float seconds);
	void Restart();
	void Draw(CLensFlare *flare);
	bool Reload();
	void Predict(int bodyid, bool now, float seconds, float *x, float *y, float *z);
	float GetRadius(int bodyid, bool withchildren);
	static bool LoadSystemData(char *resource=NULL, int *format_version=&CBody::system_format_version, char *buffer=CBody::systemname,bool quiet=false);
private:
	static int numbodies;
	void Init();
	void UpdateOrbit(float seconds);
	void TransformPosition();
	void TransformClouds();
	void DrawSortedNonSolids();
	static bool LoadMultipliers();
	bool LoadPhys();
	bool LoadInfo();
	bool LoadTextures();
	void RenderDisk();
	void MakeStar(GLUquadricObj *quadric);
	void MakePlanetoid(GLUquadricObj *quadric);
	void MakeRings();
	void MakeAsteroid();
	bool LoadGFX();
	bool CheckGFX();
	void FreeGFX();
	void DrawGFX();
	float maxchilddist;
	static CLoader loader;
	typedef enum {star=0, planetoid=1, rings=2, asteroid=3} bodytype_e;
	static char **textlines;
	static int numlines;
	static int lineindex;
	bool mainbody;
	int id;
	//system data:
	static int system_format_version;
	static char systemname[SYSTEM_NAME_SIZE];
	//body phys data:
	char name[32];
	char tex_names[4][2][16];
	char obj_name[16];
	char info_name[16];
	//base&calculations data:
	bodytype_e type;
	float distance;
	float radius;
	float shape;
	float orbit_period;
	float own_period;
	float own_rot_start, orbit_rot_start;
	float clouds_period, clouds_rot_start;
	static float distmult;
	static float radmult;
	static float orbtimemult;
	static float owntimemult;
	//drawing data:
	float orb_incl[4];
	float orb_pos[2];
	float own_incl[4];
	float own_rot, orbit_rot;
	float clouds_offset;
	//GL stuff:
	int objects[4];
	int textures[4];
	//subbodies:
	int numsubbodies;
	CBody *subbodies;
	static CBody **bodycache;
	//rings z-order chain:
	CBody *nextring, *prevring;
	static CBody ringchain;
	float objectmatrix[16];
	float objecteyez;
	void SortNonSolidObject();
	void CalcMaxChildDist();
	struct
	{
		int numlines;
		char **textlines;
	} info;
};

#endif
