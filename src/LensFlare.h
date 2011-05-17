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

#ifndef LENSFLARE_H
#define LENSFLARE_H

#include "defs.h"

#include "Tensor.h"



class CBody;

class CLensFlare
{
public:
	CLensFlare();
	virtual ~CLensFlare();
	bool Load(CBody *star);
	void Free();
	void Draw();
	void UpdateTime(float seconds);
	void UpdatePos();
	void Restart();
private:
	void Init();
	bool ComputePoints(CBody *star);
	bool IsPointVisible(const Vector *p);
	bool ParseSpecsFile(CLoader *loader);
	int FindPrevTex(int current);
	Vector *points;
	double modelview[16];
	double projection[16];
	int viewport[4];
	int vispts;
	float flareX, flareY, flareZ;
	float time, lasttime, starttime, endtime;
	float alpha, lastalpha, startalpha, endalpha;
	float DaDt;
	typedef struct { float pos; float size; float color[3]; } flaredata_s;
	flaredata_s *flaredata;
	float sizefactor;
	int num_flares;
	int *textures;
	char **tex_names;
};

#endif
