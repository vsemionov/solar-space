#ifndef TEXT_H
#define TEXT_H

#include <ft2build.h>
#include <freetype/freetype.h>
#include <freetype/ftglyph.h>
#include <freetype/ftoutln.h>
#include <freetype/fttrigon.h>

#include "defs.h"


#define NUM_CHARS 128




class CText
{
public:
	CText();
	virtual ~CText();
	bool BuildOutlineFont(const char *name, int size, bool bold, bool italic, bool underline, bool strikeout, float thickness);
	bool Init(bool FreeType=false);
	void Free();
	bool BuildFTFont(const char *name, int size);
	void Print(const char *fmt, ...);
	bool GetTextSize(const char *text, float *width, float *height);
private:
	void Construct();
	static bool MakeFTChar(FT_Face face, char ch, int list_base, int textures[NUM_CHARS], float charsizes[NUM_CHARS][2], bool mipmaps);
	static int next_p2 (int a);
	bool UseFT;
	bool loaded;
	int listbase;
	float charsize[NUM_CHARS][2];
	float charheight;
	int FT_tex[NUM_CHARS];
	float sizescale;
};

#endif
