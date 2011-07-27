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

#ifndef LOADER_H
#define LOADER_H

#include "Defs.h"

#include "Resource.h"






class CLoader : public CResource
{
public:

	typedef struct vertex_s
	{
		float point[3];
		float normal[3];
		float tex_coords[2];
	} vertex_t;
	typedef struct face_s
	{
		int vertexnum[3];
		float normal[3];
	} face_t;
	typedef struct object_s
	{
		int numvertices;
		int numfaces;
		vertex_t *vertices;
		face_t *faces;
	} object_t;

	CLoader();
	virtual ~CLoader();
	bool Init();
	void ShutDown();
	bool WithResource(const char *filename);
	void End();
	int LoadRaw(const char *entryname, void **buffer);
#ifndef LOADER_NO_GFX
	int LoadImage(const char *entryname, int *width, int *height, void **buffer, bool convert_bgr_rgb=true);
	int LoadTexture(const char *imagemap, const char *alphamap, bool mipmaps, bool linear, bool anisotropic);
#endif // LOADER_NO_GFX
	bool ExtractFile(const char *entryname, const char *destfile);
	bool LoadText(const char *entryname, char ***lines, int *numlines);
	object_t *LoadObject(const char *entryname);
private:
#ifndef LOADER_NO_GFX
	static bool ResizeImage(void **pImage, int *width, int *height);
#endif // LOADER_NO_GFX
};

#endif
