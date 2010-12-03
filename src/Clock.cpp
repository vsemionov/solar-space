#include <windows.h>

#include <gl\glu.h>
#include <gl\gl.h>

#include "Window.h"
#include "Loader.h"
#include "VideoBase.h"
#include "Error.h"
#include "GamePlay.h"
#include "Clock.h"


#define CLOCK_RESOURCE NULL

#define CLOCK_SPECS_FILE "CLOCK.TXT"






CClock::CClock()
{
	Init();
}





CClock::~CClock()
{
	Free();
}





void CClock::Init()
{
	ZeroMemory(textures,sizeof(textures));
	ZeroMemory(cur_tex,sizeof(cur_tex));
	ZeroMemory(tex_names,sizeof(tex_names));
	digit_tex_max=1;
	digit_w=0;
	digit_h=0;
	digit_s=1.0f;
	digit_t=0.0f;
	color_r=0.0f;
	color_g=0.0f;
	color_b=0.0f;
	color_a=0.0f;
}





bool CClock::Load()
{
#define AbortLoad()		\
	{					\
		Free();			\
		return false;	\
	}
////////////////
	CLoader loader;
	if (!loader.WithResource(CLOCK_RESOURCE))
	{
		CError::LogError(WARNING_CODE,"Unable to open clock - missing or invalid resource.");
		return false;
	}
	if (!ParseSpecsFile(&loader))
	{
		CError::LogError(WARNING_CODE,"Clock parse failed - skipping.");
		return false;
	}
	char ds[2];
	ds[1]=0;
	int i;
	for (i=0;i<11;i++)
	{
		ds[0]=(i==10?':':'0'+i);
		if (CGamePlay::UserAbortedLoad())
		{
			CError::LogError(ERROR_CODE,"Clock load aborted by user.");
			AbortLoad();
		}
		CGamePlay::UpdateSplash(ds);
		char *entry=tex_names[i];
		textures[i]=loader.LoadTexture(entry,entry,false);
		if (textures[i]==0)
		{
			CError::LogError(WARNING_CODE,"Failed to load a clock digit - skipping.");
			AbortLoad();
		}
	}
	scrwidth=(float)CWindow::GetWidth();
	scrheight=(float)CWindow::GetHeight();
	return true;
}





void CClock::Free()
{
	for (int i=0;i<12;i++)
		glDeleteTextures(1,(const GLuint*)&textures[i]);
	Init();
}





void CClock::Update()
{
	SYSTEMTIME st;
	GetLocalTime(&st);
	cur_tex[0]=textures[(st.wHour/10)];
	cur_tex[1]=textures[(st.wHour%10)];
	cur_tex[2]=textures[10]*(st.wMilliseconds/500);
	cur_tex[3]=textures[(st.wMinute/10)];
	cur_tex[4]=textures[(st.wMinute%10)];
}





void CClock::Draw()
{
	glDisable(GL_LIGHTING);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	glEnable(GL_BLEND);
	glLoadIdentity();
	glColor4f(color_r,color_g,color_b,color_a);
	for (int i=4;i>=0;i--)
	{
		if (cur_tex[i])
		{
			float x1=scrwidth-(float)(i*digit_w);
			float x2=scrwidth-(float)((i+1)*digit_w);
			float y2=(float)digit_h;
			glBindTexture(GL_TEXTURE_2D,cur_tex[4-i]);
			glBegin(GL_QUADS);
			{
				glTexCoord2f(digit_s,digit_t);	glVertex2f(x1,0.0f);
				glTexCoord2f(0.0f,digit_t);		glVertex2f(x2,0.0f);
				glTexCoord2f(0.0f,1.0f);		glVertex2f(x2,y2);
				glTexCoord2f(digit_s,1.0f);		glVertex2f(x1,y2);
			}
			glEnd();
		}
	}
	glColor3f(1.0f,1.0f,1.0f);
}





bool CClock::ParseSpecsFile(CLoader *loader)
{
#define FreeLines()					\
	{								\
		for (int j=0;j<numlines;j++)\
			free(textlines[j]);		\
		free(textlines);			\
		textlines=NULL;				\
		numlines=0;					\
	}
////////////////
#define AbortParse()	\
	{					\
		FreeLines();	\
		Init();			\
		return false;	\
	}
////////////////
	char **textlines=NULL;
	int numlines=0;
	int lineindex;
	int i;
	if (!loader->LoadText(CLOCK_SPECS_FILE,&textlines,&numlines))
	{
		CError::LogError(WARNING_CODE,"Unable to open clock - file missing from resource or internal loader subsystem error.");
		return false;
	}
	if (textlines==NULL)
	{
		CError::LogError(WARNING_CODE,"Unable to open clock - internal loader subsystem error.");
		return false;
	}
	if (numlines==0)
	{
		CError::LogError(WARNING_CODE,"Unable to open clock - empty data file.");
		return false;
	}
	{
		lineindex=0;
		while (sscanf(textlines[lineindex],"%d",&digit_tex_max)!=1 || textlines[lineindex][0]=='/')
		{
			lineindex++;
			if (lineindex>=numlines)
			{
				CError::LogError(WARNING_CODE,"Unable to load clock - unexpected end of file.");
				AbortParse();
			}
		}
		lineindex++;
		while (sscanf(textlines[lineindex],"%d %d",&digit_w,&digit_h)!=2 || textlines[lineindex][0]=='/')
		{
			lineindex++;
			if (lineindex>=numlines)
			{
				CError::LogError(WARNING_CODE,"Unable to load clock - unexpected end of file.");
				AbortParse();
			}
		}
		lineindex++;
		while (sscanf(textlines[lineindex],"%f %f %f %f",&color_r,&color_g,&color_b,&color_a)!=4 || textlines[lineindex][0]=='/')
		{
			lineindex++;
			if (lineindex>=numlines)
			{
				CError::LogError(WARNING_CODE,"Unable to load clock - unexpected end of file.");
				AbortParse();
			}
		}
		for (i=0;i<11;i++)
		{
			lineindex++;
			while (sscanf(textlines[lineindex],"%s",tex_names[i])!=1 || textlines[lineindex][0]=='/')
			{
				lineindex++;
				if (lineindex>=numlines)
				{
					CError::LogError(WARNING_CODE,"Unable to load clock - unexpected end of file.");
					AbortParse();
				}
			}
		}
	}
	FreeLines();
	{
		if (digit_tex_max<1) digit_tex_max=1;
		if (digit_w<0) digit_w=0;
		if (digit_w>digit_tex_max) digit_w=digit_tex_max;
		if (digit_h<0) digit_h=0;
		if (digit_h>digit_tex_max) digit_h=digit_tex_max;
		if (color_r<0.0f) color_r=0.0f;
		if (color_r>1.0f) color_r=1.0f;
		if (color_g<0.0f) color_g=0.0f;
		if (color_g>1.0f) color_g=1.0f;
		if (color_b<0.0f) color_b=0.0f;
		if (color_b>1.0f) color_b=1.0f;
		digit_s=(float)digit_w/(float)digit_tex_max;
		digit_t=1.0f-((float)digit_h/(float)digit_tex_max);
	}
	return true;
}
