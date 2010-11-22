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
	static void LogError(int code, char *string);
	static int GetCount() { return numerrors; }
	static bool ErrorsOccured() { return (GetCount()>1); }
	static void Rewind();
	static bool GetNextError(int *code, char *string, int maxlen=ERROR_MAXLEN);
private:
	static void CopyString(char *dest, char *src, int maxlen=ERROR_MAXLEN);
	typedef struct error_s
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
