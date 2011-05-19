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

#ifndef WINDOW_H
#define WINDOW_H

#include "defs.h"




class CWindow
{
public:
	CWindow();
	virtual ~CWindow();
	static int prev_p2(int a);
	static void CenterWindow(HWND hwnd);
	static bool Create(HWND hParent);
	static void Destroy();
	static int GetWidth() { return winwidth; };
	static int GetHeight() { return winheight; };
	static int IsMultisampleActive() { return multisample; };
protected:
	static LRESULT CALLBACK WindowProc(HWND hwnd, UINT umsg, WPARAM wparam, LPARAM lparam);
	static DWORD WINAPI PasswordThread(LPVOID lpParameter);
private:
	static bool ChangeVideoMode(int width, int height);
	static bool RestoreVideoMode();
	static bool isWGLExtensionSupported(const char *extension);
	static bool InitMultisample(HDC hDC, GLuint *pixel_format);
	static bool CreateSaverWindow(HWND hParent, DWORD dwStyle, DWORD dwExStyle, int width, int height, const GLuint *pixel_format);
	static void DestroySaverWindow();
	static HWND hwnd;
	static HDC hDC;
	static HGLRC hRC;
	static int winwidth;
	static int winheight;
	static bool multisample;
};

#endif
