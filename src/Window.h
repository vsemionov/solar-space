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
protected:
	static LRESULT CALLBACK WindowProc(HWND hwnd, UINT umsg, WPARAM wparam, LPARAM lparam);
	static DWORD WINAPI PasswordThread(LPVOID lpParameter);
private:
	static bool ChangeVideoMode(int width, int height);
	static bool RestoreVideoMode();
	static HWND hwnd;
	static HDC hDC;
	static HGLRC hRC;
	static DEVMODE dmOldScreenSettings;
	static int winwidth;
	static int winheight;
};

#endif
