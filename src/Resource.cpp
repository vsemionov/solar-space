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

#include <malloc.h>
#include <stdarg.h>
#include <string.h>
#include <io.h>

#include "Resource.h"



#define USE_NAMETREES
#define filesize(fileptr) (filelength(fileno(fileptr)))





CResource::CResource()
{
	drs=NULL;
	Init();
}





CResource::~CResource()
{
	ShutDown();
}





bool CResource::Init()
{
	ShutDown();
	return true;
}





void CResource::ShutDown()
{
	CloseResource();
}





void CResource::DestroyNameTree()
{
	nametree.Destroy();
}





bool CResource::BuildNameTree()
{
	int i;
	for (i=0;i<NUM_RES_ENTRIES;i++)
	{
		if (drstable[i].foffset!=0)
		{
			if (!nametree.AddLeaf(drstable[i].fname,i))
			{
				nametree.Destroy();
				return false;
			}
		}
	}
	return true;
}





int CResource::SearchNameTree(char *name)
{
	return nametree.FindLeaf(name);
}





int CResource::FindEntryBrutal(char *entryname)
{
	int i;
	if (drs!=NULL)
	{
		for (i=0;i<NUM_RES_ENTRIES;i++)
		{
			if (!stricmp(entryname,drstable[i].fname))
				return i;
		}
	}
	return -1;
}





bool CResource::OpenResource(const char *filename)
{
	int i;
	int size,drssize;
	const int hdrlen=8;
	char cmphdr[hdrlen];
	char idhdr[hdrlen]={'v','s',' ','d','2',0,0,0};
	if (filename==NULL || filename[0]==0)
		return false;
	if (strcmp(filename, path)==0)
		return true;
	memset(cmphdr, 0, sizeof(cmphdr));
	CloseResource();
	drs=fopen((const char *)filename, "rb");
	if (drs==NULL)
		return false;
	size=fread(cmphdr,1,hdrlen,drs);
	if (size==hdrlen)
	{
		size=memcmp(cmphdr,idhdr, hdrlen);
		if (size==0)
		{
			for (i=0,size=0,drssize=0;i<NUM_RES_ENTRIES;i++)
			{
				memset(drstable[i].fname,0,sizeof(drstable[i].fname));
				size+=fread(&drstable[i].fname,1,sizeof(drstable[i].fname)-1,drs);
				size+=fread(&drstable[i].foffset,1,sizeof(drstable[i].foffset),drs);
				size+=fread(&drstable[i].fsize,1,sizeof(drstable[i].fsize),drs);
				strupr(drstable[i].fname);
				drssize+=drstable[i].fsize;
			}
			drssize+=size+hdrlen;
			if (drssize==filesize(drs) && size==NUM_RES_ENTRIES*((sizeof(drstable[0].fname)-1)+sizeof(drstable[0].fsize)+sizeof(drstable[0].foffset)))
			{
#ifdef USE_NAMETREES
				if (BuildNameTree())
#endif
				{
					strncpy(path, filename, sizeof(path));
					path[sizeof(path)-1]=0;
					return true;
				}
			}
		}
	}
	CloseResource();
	return false;
}





void CResource::CloseResource()
{
	if (drs!=NULL)
	{
		fclose(drs);
	}
	drs=NULL;
	memset(drstable,0,sizeof(drstable));
	path[0]=0;
#ifdef USE_NAMETREES
	DestroyNameTree();
#endif
}





int CResource::FindEntry(const char *entryname)
{
	char uprname[16];
	if (entryname==NULL || entryname[0]==0)
		return -1;
	strncpy(uprname,entryname,sizeof(uprname));
	uprname[sizeof(uprname)-1]=0;
	strupr(uprname);
#ifdef USE_NAMETREES
	return SearchNameTree(uprname);
#else
	return FindEntryBrutal(uprname);
#endif
}





bool CResource::ReadEntry(void *buffer, int entrynum)
{
	if (drs!=NULL && buffer!=NULL && entrynum<NUM_RES_ENTRIES && entrynum>=0)
	{
		if (!fseek(drs,drstable[entrynum].foffset,SEEK_SET))
		{
			if (fread(buffer,1,drstable[entrynum].fsize,drs)==drstable[entrynum].fsize)
				return true;
		}
	}
	return false;
}





bool CResource::ReadEntryEx(void *buffer, int entrynum, int offset, int size)
{
	if (drs!=NULL && buffer!=NULL && entrynum<NUM_RES_ENTRIES && entrynum>=0)
	{
		if (size==0)
			size=drstable[entrynum].fsize;
		if ((offset+size)<=(int)drstable[entrynum].fsize)
		{
			if (!fseek(drs,drstable[entrynum].foffset+offset,SEEK_SET))
			{
				if ((int)fread(buffer,1,size,drs)==size)
					return true;
			}
		}
	}
	return false;
}





char *CResource::GetEntryName(int entrynum)
{
	if (drs==NULL || entrynum>=NUM_RES_ENTRIES || entrynum<0)
		return NULL;
	return drstable[entrynum].fname;
}





int CResource::GetEntrySize(int entrynum)
{
	if (drs==NULL || entrynum>=NUM_RES_ENTRIES || entrynum<0)
		return -1;
	return (int)drstable[entrynum].fsize;
}
