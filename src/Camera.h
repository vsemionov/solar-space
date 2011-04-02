#ifndef CAMERA_H
#define CAMERA_H

#include "defs.h"

#include "Info.h"

#define CAMERA_INIT_TIME 3.0f
#define CAMERA_INIT_FADE_TIME 2.0f




class CCamera
{
public:
	CCamera();
	virtual ~CCamera();
	void Init(CBody *refbody, CInfo *info, int scrwidth, int scrheight);
	void Update(float seconds);
	void Set(float x, float y, float z, float yaw, float pitch);
	void Move(float forward, float right, float up, float yaw, float pitch);
	void ApplyRotation();
	void ApplyTranslation();
	void Apply();
	void DrawFade();
	void Get(float *x, float *y, float *z, float *yaw, float *pitch);
	void ApplyFOV();
	double GetFOV() { return fov; };
	void Restart(float seconds);
	bool fade;
private:
	void GenWideCam(float *len, float *pitch, float yaw);
	void ChangeCam();
	void Angles(float x, float y, float z, float *yaw, float *pitch);
	CBody *mainbody;
	CInfo *planetinfo;
	double aspect;
	int scrwidth,scrheight;
	float cx,cy,cz;
	float cyaw,cpitch;
	double fov;
	typedef enum {shooting=0, zoomingout, moving, pointing, zoomingin} action_e;
	typedef struct {bool chaseor,chasepos; bool children;} type_s;
	action_e action;
	type_s type;
	float ex,ey,ez;
	float nx,ny,nz;
	int target;
	float targetsize;
	float starttime,endtime;
	float fadealpha;
};

#endif
