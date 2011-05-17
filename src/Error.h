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

#ifndef ERROR_H
#define ERROR_H

#include "defs.h"


#define ERROR_MAXLEN 256

#define SUCCESS_CODE 0
#define WARNING_CODE (SUCCESS_CODE+1)
#define ERROR_CODE (WARNING_CODE+1)




class CError
{
public:
	CError();
	virtual ~CError();
	static void Init();
	static void Clear();
	static void LogError(int code, const char *string);
	static int GetCount() { return numerrors; }
	static bool ErrorsOccured() { return (GetCount()>1); }
	static void Rewind();
	static bool GetNextError(int *code, char *string, int maxlen=ERROR_MAXLEN);
private:
	static void CopyString(char *dest, const char *src, int maxlen=ERROR_MAXLEN);
	struct error_s
	{
		int code;
		char string[256];
		error_s *prev;
		error_s *next;
	};
	static error_s errorchain;
	static error_s *logmarker;
	static int numerrors;
};

#endif
