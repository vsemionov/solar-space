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

#ifndef RESOURCE_H
#define RESOURCE_H

#include <windows.h>

#include "defs.h"

#include <stdio.h>
#include <stdint.h>

#include "NameTree.h"



#define NUM_RES_ENTRIES 512





class CResource
{
public:
	CResource();
	virtual ~CResource();
	bool OpenResource(const char *filename);
	void CloseResource();
	int FindEntry(const char *entryname);
	bool ReadEntry(void *buffer, int entrynum);
	bool ReadEntryEx(void *buffer, int entrynum, int offset, int size);
	const char *GetEntryName(int entrynum);
	int GetEntrySize(int entrynum);
protected:
	FILE *drs;
private:
	bool Init();
	void ShutDown();

	typedef struct
	{
		char fname[13];
		uint32_t foffset;
		uint32_t fsize;
	} drstableentry_t;
	typedef drstableentry_t drstable_t[NUM_RES_ENTRIES];

	drstable_t drstable;
	CNameTree nametree;
	char path[MAX_PATH];
	void DestroyNameTree();
	bool BuildNameTree();
	int SearchNameTree(const char *name);
	int FindEntryBrutal(const char *entryname);
};

#endif
