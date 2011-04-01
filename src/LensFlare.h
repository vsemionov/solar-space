#ifndef LENSFLARE_H
#define LENSFLARE_H

#include "defs.h"

#include "Tensor.h"



class CBody;

class CLensFlare
{
public:
	CLensFlare();
	virtual ~CLensFlare();
	bool Load(CBody *star);
	void Free();
	void Draw();
	void UpdateTime(float seconds);
	void UpdatePos();
	void Restart();
private:
	void Init();
	bool ComputePoints(CBody *star);
	bool IsPointVisible(Vector p);
	bool ParseSpecsFile(CLoader *loader);
	int FindPrevTex(int current);
	Vector *points;
	double modelview[16];
	double projection[16];
	int viewport[4];
	int vispts;
	float flareX, flareY, flareZ;
	float time, lasttime, starttime, endtime;
	float alpha, lastalpha, startalpha, endalpha;
	float DaDt;
	typedef struct { float pos; float size; float color[3]; } flaredata_s;
	flaredata_s *flaredata;
	float sizefactor;
	int num_flares;
	int *textures;
	char **tex_names;
};

#endif
