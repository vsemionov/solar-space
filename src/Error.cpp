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

#include "Error.h"


#define INTERNAL_ERROR_CODE (SUCCESS_CODE-1)



CError::error_s CError::errorchain;
CError::error_s *CError::logmarker=NULL;
int CError::numerrors=1;





CError::CError()
{
}





CError::~CError()
{
}





void CError::Init()
{
	errorchain.code=SUCCESS_CODE;
	CopyString(errorchain.string,"Successful logging.");
	errorchain.prev=errorchain.next=&errorchain;
	logmarker=NULL;
	numerrors=1;
}





void CError::Clear()
{
	error_s *cur=errorchain.next;
	while (cur!=&errorchain)
	{
		cur=cur->next;
		free(cur->prev);
	}
	Init();
}





void CError::LogError(int code, const char *string)
{
	if (errorchain.code!=SUCCESS_CODE)
		return;

	error_s *error;
	error=(error_s*)malloc(sizeof(error_s));

	if (!error)
	{
		errorchain.code=INTERNAL_ERROR_CODE;
		CopyString(errorchain.string,"Memory allocation error when logging.");
		return;
	}

	error->code=code;
	CopyString(error->string,string);

	error->prev=errorchain.prev;
	error->next=&errorchain;
	errorchain.prev->next=error;
	errorchain.prev=error;

	numerrors++;
}





void CError::Rewind()
{
	logmarker=errorchain.next;
}





bool CError::GetNextError(int *code, char *string, int maxlen)
{
	if (!logmarker)
		return false;

	if (code)
		*code=logmarker->code;
	if (string)
		CopyString(string,logmarker->string,maxlen);

	if (logmarker==&errorchain)
		logmarker=NULL;
	else
		logmarker=logmarker->next;

	return true;
}





void CError::CopyString(char *dest, const char *src, int maxlen)
{
	strncpy(dest,src,maxlen);
	dest[maxlen-1]=0;
}
