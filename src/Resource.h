#ifndef RESOURCE_H
#define RESOURCE_H

#include "defs.h"

#include <stdio.h>

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
		dword foffset;
		dword fsize;
	} drstableentry_t;
	typedef drstableentry_t drstable_t[NUM_RES_ENTRIES];

	drstable_t drstable;
	CNameTree nametree;
	static void PascalStringToCString(char *str);
	static void DecodeTableEntry(drstableentry_t *entry);
	void DestroyNameTree();
	bool BuildNameTree();
	int SearchNameTree(char *name);
	int FindEntryBrutal(char *entryname);
};

#endif
