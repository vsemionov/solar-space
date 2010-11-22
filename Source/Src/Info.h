#ifndef INFO_H
#define INFO_H

#include "defs.h"

#include "Text.h"




class CInfo
{
public:
	CInfo();
	virtual ~CInfo();
	bool Load();
	void Free();
	void Start(float seconds, float duration, char *targetname, CBody *targetbody);
	void Update(float seconds);
	void Restart();
	void Draw();
private:
	void Init();
	void MakeWindow(int list);
	void GetNameCoords(char *text, float *x, float *y);
	void GetInfoCoords(int linenum, float *x, float *y);
	void MakeInfoLine(int linenum, char *line);
	void MakeName(int list, char *targetname);
	void MakeInfo(int list, CBody *targetbody);
	bool loaded;
	CText nametext;
	CText infotext;
	float scrwidth;
	float scrheight;
	int winlist;
	int namelist;
	int infolist;
	float starttime;
	float endtime;
	float fadetime;
	float time;
	float alpha;
};

#endif
