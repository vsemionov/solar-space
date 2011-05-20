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

#ifndef CAMERA_H
#define CAMERA_H

#include "defs.h"

#include "Info.h"

#define CAMERA_INIT_TIME 3.0f
#define CAMERA_INIT_FADE_TIME 2.0f // must be greater than RESTART_TIME (in CGamePlay.cpp)




class CCamera
{
public:
	CCamera();
	virtual ~CCamera();
	void Init(CBody *refbody, CInfo *info, int scrwidth, int scrheight);
	void Update(float seconds);
	void Set(float x, float y, float z, float yaw, float pitch);
	void Move(float forward, float right, float up, float yaw, float pitch);
	void ApplyRotation();
	void ApplyTranslation();
	void Apply();
	void DrawFade();
	void Get(float *x, float *y, float *z, float *yaw, float *pitch);
	void ApplyFOV();
	double GetFOV() { return fov; };
	void Restart(float seconds);
	bool fade;
private:
	void GenWideCam(float *len, float *pitch, float yaw);
	void ChangeCam();
	void Angles(float x, float y, float z, float *yaw, float *pitch);
	CBody *mainbody;
	CInfo *planetinfo;
	double aspect;
	int scrwidth,scrheight;
	float cx,cy,cz;
	float cyaw,cpitch;
	double fov;
	typedef enum {shooting=0, zoomingout, moving, pointing, zoomingin} action_e;
	typedef struct {bool chaseor,chasepos; bool children;} type_s;
	action_e action;
	type_s type;
	float ex,ey,ez;
	float nx,ny,nz;
	int target;
	float targetsize;
	float starttime,endtime;
	float fadealpha;
};

#endif
