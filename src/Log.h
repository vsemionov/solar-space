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

#ifndef LOG_H
#define LOG_H

#include "Defs.h"


#define MESSAGE_MAXLEN 256

#define LOG_INFO 0
#define LOG_WARN (LOG_INFO+1)
#define LOG_ERROR (LOG_WARN+1)
#define LOG_FATAL (LOG_ERROR+1)





class CLog
{
public:
	CLog();
	virtual ~CLog();
	static void Init();
	static void Clear();
	static void Log(int level, const char *message);
	static int GetCount() { return numentries; }
	static bool ProblemsOccured() { return (numproblems!=0); }
	static void Rewind();
	static bool GetNextEntry(int *level, char *message, int maxlen=MESSAGE_MAXLEN);
	static void Pause() { logging=false; }
	static void Continue() { logging=true; }
private:
	static void CopyString(char *dest, const char *src, int maxlen=MESSAGE_MAXLEN);
	struct entry_s
	{
		int level;
		char message[256];
		entry_s *prev;
		entry_s *next;
	};
	static entry_s entrychain;
	static entry_s *logmarker;
	static int numentries;
	static int numproblems;
	static bool logging;
};

#endif
