#ifndef WINDOW_H
#define WINDOW_H

#include "defs.h"




class CWindow
{
public:
	CWindow();
	virtual ~CWindow();
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
