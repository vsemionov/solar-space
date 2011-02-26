#include <windows.h>

#include <gl\glu.h>
#include <gl\gl.h>

#include <gl\glext.h>

#include "Settings.h"
#include "VideoBase.h"



#define VER(major,minor,mini) ((major<<16)+(minor<<8)+(mini))







bool CVideoBase::ext_bgra=false;
bool CVideoBase::ext_point_params=false;
float CVideoBase::opt_geodetail=0.5f;
bool CVideoBase::opt_linear=false;
bool CVideoBase::opt_mipmaps=false;
bool CVideoBase::opt_shadows=false;
bool CVideoBase::opt_bumpmaps=false;
bool CVideoBase::opt_lensflares=false;
bool CVideoBase::opt_startwinkle=false;





CVideoBase::CVideoBase()
{
}





CVideoBase::~CVideoBase()
{
}





bool CVideoBase::Init()
{
	GetExtensions();
	GetOptions();
	glShadeModel(GL_SMOOTH);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClearDepth(1.0f);
	glDisable(GL_DITHER);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glDisable(GL_NORMALIZE);
	glEnable(GL_LIGHTING);
	glEnable(GL_TEXTURE_2D);
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
	glHint(GL_POINT_SMOOTH_HINT, GL_NICEST);
	glEnable(GL_CULL_FACE);
	glDisable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
	glLightModeli(GL_LIGHT_MODEL_COLOR_CONTROL,GL_SEPARATE_SPECULAR_COLOR);
	return true;
}





void CVideoBase::ShutDown()
{
}





bool CVideoBase::IsInString(const char *string, const char *search)
{
	int i;
	int pos=0;
	int maxpos=strlen(search)-1;
	int len=strlen(string);	
	for (i=0; i<len; i++)
	{
		if ((i==0) || ((i>1) && string[i-1]=='\n'))
		{				// New Extension Begins Here!
			pos=0;													// Begin New Search
			while (string[i]!='\n')
			{								// Search Whole Extension-String
				if (string[i]==search[pos]) pos++;					// Next Position
				if ((pos>maxpos) && string[i+1]=='\n') return true; // We Have A Winner!
				i++;
			}			
		}
	}	
	return false;													// Sorry, Not Found!
}





void CVideoBase::GetVersion(int *major, int *minor, int *mini)
{
	*major=*minor=*mini=0;
	char *verstr=strdup((const char*)glGetString(GL_VERSION));
	char *tok=strtok(verstr,".");
	if (tok!=NULL)
		*major=atoi(tok);
	tok=strtok(NULL,".");
	if (tok!=NULL)
		*minor=atoi(tok);
	tok=strtok(NULL,".");
	if (tok!=NULL)
		*mini=atoi(tok);
	free(verstr);
	verstr=tok=NULL;
}





void CVideoBase::GetExtensions()
{
	int major,minor,mini;
	GetVersion(&major,&minor,&mini);
	char *ext_str=strdup((const char*)glGetString(GL_EXTENSIONS));
	if (ext_str==NULL)
		return;
	int l=strlen(ext_str);
	for (int i=0; i<l; i++)
	{
		if (ext_str[i]==' ')
			ext_str[i]='\n';
	}
	{
		ext_bgra=IsInString(ext_str,"GL_EXT_bgra");
		if (VER(major,minor,mini)>=VER(1,2,0))
			ext_bgra=true;
	}
	{
		ext_point_params=IsInString(ext_str,"GL_ARB_point_parameters");
		if (VER(major,minor,mini)>=VER(1,4,0))
			ext_point_params=true;
	}
	free(ext_str);
	ext_str=NULL;
}





void CVideoBase::GetOptions()
{
	opt_geodetail=0.5f;
	opt_linear=false;
	opt_mipmaps=false;
	opt_shadows=false;
	opt_bumpmaps=false;
	opt_lensflares=false;
	opt_startwinkle=false;
	switch (CSettings::DetailLevel)
	{
	case 2:
		opt_shadows=true;
		opt_bumpmaps=true;
		opt_startwinkle=true;
	case 1:
		opt_geodetail=1.0f;
		opt_mipmaps=true;
	case 0:
		opt_lensflares=true;
		opt_linear=true;
	default:
		break;
	}
}
