#ifndef VIDEOBASE_H
#define VIDEOBASE_H

#include "defs.h"





class CVideoBase
{
public:
	CVideoBase();
	virtual ~CVideoBase();
	static bool Init();
	static void ShutDown();
	static bool GetExtBGRA() { return ext_bgra; }
	static bool GetExtPointParams() { return ext_point_params; }
	static float GetOptGeoDetail() { return opt_geodetail; }
	static bool GetOptLinear() { return opt_linear; }
	static bool GetOptMipmaps() { return opt_mipmaps; }
	static bool GetOptShadows() { return opt_shadows; }
	static bool GetOptBumpMaps() { return opt_bumpmaps; }
	static bool GetOptLensFlares() { return opt_lensflares; }
	static bool GetOptStarTwinkle() { return opt_startwinkle; }
private:
	static bool IsInString(const char *string, const char *search);
	static void GetVersion(int *major, int *minor, int *patch);
	static void GetExtensions();
	static void GetOptions();
	static bool ext_bgra;
	static bool ext_point_params;
	static float opt_geodetail;
	static bool opt_linear;
	static bool opt_mipmaps;
	static bool opt_shadows;
	static bool opt_bumpmaps;
	static bool opt_lensflares;
	static bool opt_startwinkle;
};

#endif
