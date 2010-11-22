
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





void CError::LogError(int code, char *string)
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





void CError::CopyString(char *dest, char *src, int maxlen)
{
	strncpy(dest,src,maxlen);
	dest[maxlen-1]=0;
}
