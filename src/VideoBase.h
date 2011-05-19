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

#ifndef VIDEOBASE_H
#define VIDEOBASE_H

#include "defs.h"


#define MAX_ANISOTROPY 4.0f







class CVideoBase
{
public:
	CVideoBase();
	virtual ~CVideoBase();
	static void Init();
	static bool InitGL();
	static void ShutDown();
	static bool GetExtBGRA() { return ext_bgra; }
	static bool GetExtPointParams() { return ext_point_params; }
	static bool GetExtAnisotropic() { return ext_anisotropic; }
	static float GetOptGeoDetail() { return opt_geodetail; }
	static bool GetOptLinear() { return opt_linear; }
	static bool GetOptMipmaps() { return opt_mipmaps; }
	static bool GetOptShadows() { return opt_shadows; }
	static bool GetOptBumpMaps() { return opt_bumpmaps; }
	static bool GetOptLensFlares() { return opt_lensflares; }
	static bool GetOptStarTwinkle() { return opt_startwinkle; }
	static bool GetOptAntialiasing() { return opt_antialiasing; }
	static bool GetOptAnisotropic() { return opt_anisotropic; }
	static void EnableMultisample();
	static void DisableMultisample();
private:
	static bool IsExtensionSupported(const char *string, const char *search);
	static void GetVersion(int *major, int *minor, int *patch);
	static void GetExtensions();
	static void GetOptions();
	static bool ext_bgra;
	static bool ext_point_params;
	static bool ext_anisotropic;
	static float opt_geodetail;
	static bool opt_linear;
	static bool opt_mipmaps;
	static bool opt_shadows;
	static bool opt_bumpmaps;
	static bool opt_lensflares;
	static bool opt_startwinkle;
	static bool opt_antialiasing;
	static bool opt_anisotropic;
};

#endif
