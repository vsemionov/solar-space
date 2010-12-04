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
	char *GetEntryName(int entrynum);
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
	int SearchNameTree(char *name);
	int FindEntryBrutal(char *entryname);
};

#endif
