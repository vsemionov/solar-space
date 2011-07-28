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
#include <string.h>

#include "Log.h"


#define INTERNAL_ERROR_CODE (LOG_INFO-1)



CLog::entry_s CLog::entrychain;
CLog::entry_s *CLog::logmarker=NULL;
int CLog::numentries=1;
int CLog::numproblems=1;
bool CLog::logging=true;





CLog::CLog()
{
}





CLog::~CLog()
{
}





void CLog::Init()
{
	entrychain.level=LOG_INFO;
	CopyString(entrychain.message,"Successful logging.");
	entrychain.prev=entrychain.next=&entrychain;
	logmarker=NULL;
	numentries=1;
	numproblems=0;
	logging=true;
}





void CLog::Clear()
{
	entry_s *cur=entrychain.next;
	while (cur!=&entrychain)
	{
		cur=cur->next;
		free(cur->prev);
	}
	Init();
}





void CLog::Log(int level, const char *message)
{
	if (!logging)
		return;

	if (entrychain.level!=LOG_INFO)
		return;

	entry_s *entry;
	entry=(entry_s*)malloc(sizeof(entry_s));

	if (!entry)
	{
		entrychain.level=INTERNAL_ERROR_CODE;
		CopyString(entrychain.message,"Memory allocation error when logging.");
		return;
	}

	entry->level=level;
	CopyString(entry->message,message);

	entry->prev=entrychain.prev;
	entry->next=&entrychain;
	entrychain.prev->next=entry;
	entrychain.prev=entry;

	numentries++;
	if (level>LOG_INFO)
		numproblems++;
}





void CLog::Rewind()
{
	logmarker=entrychain.next;
}





bool CLog::GetNextEntry(int *level, char *message, int maxlen)
{
	if (!logmarker)
		return false;

	if (level)
		*level=logmarker->level;
	if (message)
		CopyString(message,logmarker->message,maxlen);

	if (logmarker==&entrychain)
		logmarker=NULL;
	else
		logmarker=logmarker->next;

	return true;
}





void CLog::CopyString(char *dest, const char *src, int maxlen)
{
	strncpy(dest,src,maxlen);
	dest[maxlen-1]=0;
}
