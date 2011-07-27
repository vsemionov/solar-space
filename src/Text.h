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

#ifndef TEXT_H
#define TEXT_H

#include <ft2build.h>
#include <freetype/freetype.h>
#include <freetype/ftglyph.h>
#include <freetype/ftoutln.h>
#include <freetype/fttrigon.h>

#include "Defs.h"


#define NUM_CHARS 128



typedef FT_Error (*ptr_FT_Init_FreeType)(FT_Library *);
typedef FT_Error (*ptr_FT_Done_FreeType)(FT_Library);
typedef FT_Error (*ptr_FT_New_Face)(FT_Library, const char *, FT_Long, FT_Face *);
typedef FT_Error (*ptr_FT_Done_Face)(FT_Face);
typedef FT_Error (*ptr_FT_Set_Char_Size)(FT_Face, FT_F26Dot6, FT_F26Dot6, FT_UInt, FT_UInt);
typedef FT_UInt (*ptr_FT_Get_Char_Index)(FT_Face, FT_ULong);
typedef FT_Error (*ptr_FT_Load_Glyph)(FT_Face, FT_UInt, FT_Int32);
typedef void (*ptr_FT_Done_Glyph)(FT_Glyph);
typedef FT_Error (*ptr_FT_Get_Glyph)(FT_GlyphSlot, FT_Glyph *);
typedef FT_Error (*ptr_FT_Glyph_To_Bitmap)(FT_Glyph *, FT_Render_Mode, FT_Vector *, FT_Bool);





class CText
{
public:
	CText();
	virtual ~CText();
	bool BuildOutlineFont(const char *name, int size, bool bold, bool italic, bool underline, bool strikeout, float thickness);
	static bool Init();
	static void Shutdown();
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
	// FreeType functions:
	static bool FreeType_loaded;
	static HMODULE FreeType;
	static ptr_FT_Init_FreeType pFT_Init_FreeType;
	static ptr_FT_Done_FreeType pFT_Done_FreeType;
	static ptr_FT_New_Face pFT_New_Face;
	static ptr_FT_Done_Face pFT_Done_Face;
	static ptr_FT_Set_Char_Size pFT_Set_Char_Size;
	static ptr_FT_Get_Char_Index pFT_Get_Char_Index;
	static ptr_FT_Load_Glyph pFT_Load_Glyph;
	static ptr_FT_Done_Glyph pFT_Done_Glyph;
	static ptr_FT_Get_Glyph pFT_Get_Glyph;
	static ptr_FT_Glyph_To_Bitmap pFT_Glyph_To_Bitmap;
};

#endif
