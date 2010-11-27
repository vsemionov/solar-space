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





void CResource::PascalStringToCString(char *str)
{
	int len;
	len=(int)str[0];
	memmove((void*)str,(const void*)(str+1),len);
	str[len]=0;
}





void CResource::DecodeTableEntry(drstableentry_t *entry)
{
	int j;
	for (j=1;j<13;j++)
	{
		entry->fname[j]+=75; //-=181;
	}
	entry->foffset-=2430;
	entry->fsize-=2430;
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
	dword size,drssize;
	char *cmpstr;
	const char *idstr="vsALL Resource Phile.";
	CloseResource();
	drs=fopen((const char *)filename, "rb");
	if (drs==NULL)
		return false;
	cmpstr=(char*)malloc(strlen(idstr)+1);
	if (cmpstr!=NULL)
	{
		size=fread((void*)(cmpstr+1),1,strlen(idstr),drs);
		cmpstr[0]=(char)size;
		PascalStringToCString(cmpstr);
		size=strcmp(cmpstr,idstr);
		free(cmpstr);
		if (size==0)
		{
			for (i=0,size=0,drssize=0;i<NUM_RES_ENTRIES;i++)
			{
				size+=(dword)fread(&drstable[i].fname,1,sizeof(drstable[i].fname),drs);
				size+=(dword)fread(&drstable[i].foffset,1,sizeof(drstable[i].foffset),drs);
				size+=(dword)fread(&drstable[i].fsize,1,sizeof(drstable[i].fsize),drs);
				DecodeTableEntry(&drstable[i]);
				PascalStringToCString(drstable[i].fname);
				strupr(drstable[i].fname);
				drssize+=drstable[i].fsize;
			}
			drssize+=size+strlen(idstr);
			if (drssize==(dword)filesize(drs) && size==NUM_RES_ENTRIES*(sizeof(drstable[0].fname)+sizeof(drstable[0].fsize)+sizeof(drstable[0].foffset)))
			{
#ifdef USE_NAMETREES
				if (BuildNameTree())
#endif
				{
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
#ifdef USE_NAMETREES
	DestroyNameTree();
#endif
}





int CResource::FindEntry(const char *entryname)
{
	char uprname[16];
	if (entryname==NULL)
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
