#include <windows.h>

#include <gl\gl.h>

#include <stdio.h>


#include "Settings.h"
#include "Text.h"



#define FONT_NAME "Arial"
#define FONT_SIZE 16
#define FONT_BOLD true
#define FONT_ITALIC false
#define FONT_UNDERLINE false
#define FONT_STRIKEOUT false
#define FONT_THICKNESS 0.1f

#define ALLOW_FONT_SCALING true
#define FT_FONT_SIZE_THRESHOLD 32












CText::CText()
{
	Construct();
}





CText::~CText()
{
	Free();
}





void CText::Construct()
{
	ZeroMemory(FT_tex,sizeof(FT_tex));
	ZeroMemory(charsize,sizeof(charsize));
	charheight=0.0f;
	listbase=0;
	sizescale=1.0f;
	UseFT=false;
	loaded=false;
}





bool CText::Init(bool FreeType)
{
	if (FreeType)
	{
		return BuildFTFont(FONT_NAME,FONT_SIZE);
	}
	else
	{
		return BuildOutlineFont(FONT_NAME,
								FONT_SIZE,
								FONT_BOLD,
								FONT_ITALIC,
								FONT_UNDERLINE,
								FONT_STRIKEOUT,
								FONT_THICKNESS);
	}
}





void CText::Free()
{
	if (loaded)
	{
		glDeleteLists(listbase,NUM_CHARS);
		if (UseFT)
		{
			glDeleteTextures(NUM_CHARS,(GLuint*)FT_tex);
		}
	}
	Construct();
}





bool CText::BuildOutlineFont(const char *name, int size, bool bold, bool italic, bool underline, bool strikeout, float thickness)
{
	if (loaded)
	{
		Free();
	}
	GLYPHMETRICSFLOAT gmf[NUM_CHARS];
	HFONT font;
	listbase=glGenLists(NUM_CHARS);
	if (listbase)
	{
		HDC hDC=GetDC(CSettings::hwnd);
//		size=-MulDiv(size,GetDeviceCaps(hDC,LOGPIXELSY),72);
		size=-size; // size dont depend on system font size
		if (hDC)
		{
			font=CreateFont(size,							// Height Of Font
							0,								// Width Of Font
							0,								// Angle Of Escapement
							0,								// Orientation Angle
							bold?FW_BOLD:FW_NORMAL,			// Font Weight
							italic,							// Italic
							underline,						// Underline
							strikeout,						// Strikeout
							ANSI_CHARSET,					// Character Set Identifier
							OUT_TT_PRECIS,					// Output Precision
							CLIP_DEFAULT_PRECIS,			// Clipping Precision
							ANTIALIASED_QUALITY,			// Output Quality
							FF_DONTCARE|DEFAULT_PITCH,		// Family And Pitch
							name);							// Font Name
			if (font)
			{
				HGDIOBJ OldObj=SelectObject(hDC,font);
				loaded=(wglUseFontOutlines(	hDC,							// Select The Current DC
											0,								// Starting Character
											NUM_CHARS,						// Number Of Display Lists To Build
											listbase,						// Starting Display Lists
											0.0f,							// Deviation From The True Outlines
											thickness,						// Font Thickness In The Z Direction
											WGL_FONT_POLYGONS,				// Use Polygons, Not Lines
											gmf)!=0);						// Address Of Buffer To Recieve Data
				if (loaded)
				{
					for (int i=0;i<NUM_CHARS;i++)
					{
						charsize[i][0]=gmf[i].gmfCellIncX;
						charsize[i][1]=gmf[i].gmfCellIncY;
					}
					charheight=1.0f;
				}
				SelectObject(hDC,OldObj);
				DeleteObject(font);
			}
			ReleaseDC(CSettings::hwnd,hDC);
		}
	}
	return loaded;
}





bool CText::BuildFTFont(const char *name, int size)
{
	if (loaded)
	{
		Free();
	}

	bool smallfont=(size<FT_FONT_SIZE_THRESHOLD && ALLOW_FONT_SCALING==true);
	if (smallfont)
	{
		sizescale=(float)((double)size/(double)FT_FONT_SIZE_THRESHOLD);
		size=FT_FONT_SIZE_THRESHOLD;
	}

	char fontpath[MAX_PATH];
	GetWindowsDirectory(fontpath,sizeof(fontpath));
	strcat(fontpath,"\\FONTS\\");
	strcat(fontpath,name);
	strcat(fontpath,".TTF");

	FT_Library library;
	if (FT_Init_FreeType(&library))
		return false;

	FT_Face face;
	if (FT_New_Face(library,fontpath,0,&face)) 
	{
		FT_Done_Face(face);
		return false;
	}
	FT_Set_Char_Size(face,size<<6,size<<6,96,96);

	listbase=glGenLists(NUM_CHARS);
	glGenTextures(NUM_CHARS,(GLuint*)FT_tex);

	loaded=true;
	for(int i=0;i<NUM_CHARS;i++)
	{
		if (!MakeFTChar(face, i, listbase, FT_tex, charsize, smallfont))
		{
			loaded=false;
			break;
		}
	}

	if (loaded)
	{
		charheight=(float)size;
	}

	FT_Done_Face(face);
	FT_Done_FreeType(library);

	return loaded;
}





int CText::next_p2 (int a)
{
	int rval=1;
	while(rval<a) rval<<=1;
	return rval;
}





bool CText::MakeFTChar(FT_Face face, char ch, int list_base, int textures[NUM_CHARS], float charsizes[NUM_CHARS][2], bool mipmaps)
{
	const int ich = ch;

	if(FT_Load_Glyph(face,FT_Get_Char_Index(face,ich),FT_LOAD_DEFAULT))
		return false;

	FT_Glyph glyph;
	if(FT_Get_Glyph(face->glyph,&glyph))
		return false;

	bool empty=(FT_Glyph_To_Bitmap(&glyph,FT_RENDER_MODE_NORMAL,0,1)!=0);
	FT_BitmapGlyph bitmap_glyph=(FT_BitmapGlyph)glyph;
	FT_Bitmap& bitmap=bitmap_glyph->bitmap;

	int width=1, height=1;
	if (!empty)
	{

		width=next_p2(bitmap.width);
		height=next_p2(bitmap.rows);

		unsigned char *expanded_data=(unsigned char *)malloc(2*width*height);
		if (!expanded_data)
			return false;

		for(int j=0; j<height; j++)
		{
			for(int i=0; i<width; i++)
			{
				expanded_data[2*(i+j*width)]=255;
				expanded_data[2*(i+j*width)+1] = 
					(i>=bitmap.width || j>=bitmap.rows) ?
					0 : bitmap.buffer[i + bitmap.width*j];
			}
		}

		glBindTexture(GL_TEXTURE_2D, textures[ich]);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
		if (mipmaps)
		{
			glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_NEAREST);
			if (gluBuild2DMipmaps(GL_TEXTURE_2D,2,width,height,GL_LUMINANCE_ALPHA,GL_UNSIGNED_BYTE,expanded_data)!=0)
			{
				free(expanded_data);
				return false;
			}
		}
		else
		{
			glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
			glTexImage2D(	GL_TEXTURE_2D,
							0,
							GL_RGBA,
							width,
							height,
							0,
							GL_LUMINANCE_ALPHA,
							GL_UNSIGNED_BYTE,
							expanded_data);
		}

		free(expanded_data);
	}

	glNewList(list_base+ich,GL_COMPILE);
	{
		if (!empty)
		{
			glBindTexture(GL_TEXTURE_2D,textures[ich]);
			glPushMatrix();

			glTranslatef((float)bitmap_glyph->left, (float)(bitmap_glyph->top-bitmap.rows),0);

			float	x= (float)bitmap.width / (float)width,
					y= (float)bitmap.rows / (float)height;

			glBegin(GL_QUADS);
			{
				glNormal3f(0,0,1);
				glTexCoord2f(0,0); glVertex2f(0,(float)bitmap.rows);
				glTexCoord2f(x,0); glVertex2f((float)bitmap.width, (float)bitmap.rows);
				glTexCoord2f(x,y); glVertex2f((float)bitmap.width, 0);
				glTexCoord2f(0,y); glVertex2f(0,0);
			}
			glEnd();

			glPopMatrix();
		}

		glTranslatef((float)(face->glyph->advance.x>>6), 0, 0);
	}
	glEndList();

	charsizes[ich][0]=((float)face->glyph->advance.x/64.0f);
	charsizes[ich][1]=((float)face->size->metrics.height/64.0f);

	return true;
}





void CText::Print(const char *fmt, ...)					// Custom GL "Print" Routine
{
	char text[256];										// Holds Our String
	va_list ap;											// Pointer To List Of Arguments

	if (!loaded || !fmt)								// If There's No Text
		return;											// Do Nothing

	va_start(ap, fmt);									// Parses The String For Variables
		vsprintf(text, fmt, ap);						// And Converts Symbols To Actual Numbers
	va_end(ap);											// Results Are Stored In Text

	glPushAttrib(GL_LIST_BIT);							// Pushes The Display List Bits
	glListBase(listbase);								// Sets The Base Character to 0
	if (sizescale!=1.0f)
		glScalef(sizescale,sizescale,sizescale);
	glCallLists(strlen(text), GL_UNSIGNED_BYTE, text);	// Draws The Display List Text
	if (sizescale!=1.0f)
		glScalef(1.0f/sizescale,1.0f/sizescale,1.0f/sizescale);
	glPopAttrib();										// Pops The Display List Bits
}





bool CText::GetTextSize(const char *text, float *width, float *height)
{
	if (!loaded || !text || (!width && !height))
		return false;

	float w=0.0f;
	for (const char *c=text; *c; c++)
	{
		w+=charsize[(int)*c][0];
	}

	if (width) *width=w*sizescale;
	if (height) *height=charheight*sizescale;

	return false;
}
