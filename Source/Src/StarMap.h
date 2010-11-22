#ifndef STARMAP_H
#define STARMAP_H

#include "defs.h"





class CStarMap
{
public:
	CStarMap();
	virtual ~CStarMap();
	bool Load();
	void Free();
	void Draw();
	bool Check();
private:
	typedef struct
	{
		double Dec;
		double RA;
		float Mag;
		float U_B, B_V, R_I;
		float pos[3];
		float size;
		float color[3];
	} stardata_s;
	void Init();
	bool LoadStars();
	bool GenStars();
	void PrepColor();
	void PrepData();
	void InitGL();
	void DrawStars();
	int object;
	int num_stars;
	bool twinkle;
	stardata_s *stars;
	float point_size;
};

#endif
