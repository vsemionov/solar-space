#ifndef CLOCK_H
#define CLOCK_H

#include "defs.h"





class CClock
{
public:
	CClock();
	virtual ~CClock();
	bool Load();
	void Free();
	void Update();
	void Draw();
private:
	void Init();
	bool ParseSpecsFile(CLoader *loader);
	int textures[12];
	int cur_tex[6];
	float scrwidth,scrheight;
	int digit_tex_max;
	int digit_w, digit_h;
	float digit_s, digit_t;
	float color_r, color_g, color_b, color_a;
	char tex_names[12][16];
};

#endif
