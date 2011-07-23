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

#include <windows.h>

#include <gl/glu.h>
#include <gl/gl.h>

#include <gl/glext.h>
#include <gl/wglext.h>

#include "Settings.h"
#include "Window.h"
#include "VideoBase.h"



#define VER(major,minor,patch) ((major<<16)+(minor<<8)+(patch))







bool CVideoBase::ext_bgra=false;
bool CVideoBase::ext_point_params=false;
bool CVideoBase::ext_anisotropic=false;
float CVideoBase::opt_geodetail=0.5f;
bool CVideoBase::opt_linear=false;
bool CVideoBase::opt_mipmaps=false;
bool CVideoBase::opt_shadows=false;
bool CVideoBase::opt_bumpmaps=false;
bool CVideoBase::opt_lensflares=false;
bool CVideoBase::opt_startwinkle=false;
bool CVideoBase::opt_antialiasing=false;
bool CVideoBase::opt_anisotropic=false;





CVideoBase::CVideoBase()
{
}





CVideoBase::~CVideoBase()
{
}





void CVideoBase::Init()
{
	SetOptions();
}





bool CVideoBase::InitGL()
{
	GetExtensions();
	glShadeModel(GL_SMOOTH);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClearDepth(1.0f);
	glDisable(GL_DITHER);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glDisable(GL_NORMALIZE);
	glEnable(GL_LIGHTING);
	glEnable(GL_TEXTURE_2D);
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
	glHint(GL_POINT_SMOOTH_HINT, GL_NICEST);
	glEnable(GL_CULL_FACE);
	glDisable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
	glLightModeli(GL_LIGHT_MODEL_COLOR_CONTROL,GL_SEPARATE_SPECULAR_COLOR);
	DisableMultisample();
	return true;
}





void CVideoBase::ShutDown()
{
}





bool CVideoBase::IsExtensionSupported(const char *supported, const char *extension)
{
	const size_t extlen = strlen(extension);

	if (supported == NULL)
		return false;

	for (const char* p = supported; ; p++)
	{
		p = strstr(p, extension);

		if (p == NULL)
			return false;

		if ((p==supported || p[-1]==' ') && (p[extlen]==' ' || p[extlen]==0))
			return true;
	}
}





void CVideoBase::GetVersion(int *major, int *minor, int *patch)
{
	*major=*minor=*patch=0;
	char *verstr=strdup((const char*)glGetString(GL_VERSION));
	char *tok=strtok(verstr,".");
	if (tok!=NULL)
		*major=atoi(tok);
	tok=strtok(NULL,".");
	if (tok!=NULL)
		*minor=atoi(tok);
	tok=strtok(NULL,".");
	if (tok!=NULL)
		*patch=atoi(tok);
	free(verstr);
	verstr=tok=NULL;
}





void CVideoBase::GetExtensions()
{
	int major,minor,patch;
	GetVersion(&major,&minor,&patch);
	const char *ext_str=(const char*)glGetString(GL_EXTENSIONS);
	if (ext_str==NULL)
		return;
	{
		ext_bgra=IsExtensionSupported(ext_str,"GL_EXT_bgra");
		if (VER(major,minor,patch)>=VER(1,2,0))
			ext_bgra=true;
	}
	{
		ext_point_params=IsExtensionSupported(ext_str,"GL_ARB_point_parameters");
		if (VER(major,minor,patch)>=VER(1,4,0))
			ext_point_params=true;
	}
	{
		ext_anisotropic=IsExtensionSupported(ext_str,"GL_EXT_texture_filter_anisotropic");
	}
}





void CVideoBase::SetOptions()
{
	opt_geodetail=0.5f;
	opt_linear=false;
	opt_mipmaps=false;
	opt_shadows=false;
	opt_bumpmaps=false;
	opt_lensflares=false;
	opt_startwinkle=false;
	opt_antialiasing=false;
	opt_anisotropic=false;
	switch (CSettings::DetailLevel)
	{
	case 2:
		opt_shadows=true;
		opt_bumpmaps=true;
		opt_startwinkle=true;
		opt_antialiasing=true;
		opt_anisotropic=true;
	case 1:
		opt_geodetail=1.0f;
		opt_mipmaps=true;
	case 0:
		opt_lensflares=true;
		opt_linear=true;
	default:
		break;
	}
}





void CVideoBase::EnableMultisample()
{
	if (CWindow::IsMultisampleActive())
	{
		glEnable(GL_MULTISAMPLE_ARB);
	}
}





void CVideoBase::DisableMultisample()
{
	if (CWindow::IsMultisampleActive())
	{
		glDisable(GL_MULTISAMPLE_ARB);
	}
}
