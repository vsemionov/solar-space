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
#include <olectl.h>

#include <gl/glu.h>
#include <gl/gl.h>

#include <gl/glext.h>

#ifdef _MSC_VER
#define _USE_MATH_DEFINES
#endif
#include <math.h>


#include "Settings.h"
#include "VideoBase.h"
#include "Loader.h"




#define DEFAULT_RESOURCE APP_NAME



CLoader::CLoader()
{
	Init();
}





CLoader::~CLoader()
{
	ShutDown();
}





bool CLoader::Init()
{
	return true;
}





void CLoader::ShutDown()
{
}





bool CLoader::WithResource(const char *filename)
{
	char filepath[MAX_PATH];
	const char *ext=".d2";
	if (filename==NULL)
		filename=DEFAULT_RESOURCE;
	strcpy(filepath,CSettings::DataDir);
	strcat(filepath,"\\");
	strcat(filepath,filename);
	strcat(filepath,ext);
	return CResource::OpenResource(filepath);
}





void CLoader::End()
{
	CResource::CloseResource();
}





int CLoader::LoadRaw(const char *entryname, void **buffer)
{
	int entrynum;
	int size;
	if (buffer!=NULL && entryname!=NULL)
	{
		*buffer=NULL;
		entrynum=CResource::FindEntry(entryname);
		if (entrynum>=0)
		{
			size=CResource::GetEntrySize(entrynum);
			if (size>=0)
			{
				*buffer=malloc(size);
				if (*buffer!=NULL)
				{
					if (CResource::ReadEntry(*buffer,entrynum)==true)
					{
						return size;
					}
					else
					{
						free(*buffer);
						*buffer=NULL;
					}
				}
			}
		}
	}
	return -1;
}





int CLoader::LoadImage(const char *entryname, int *width, int *height, void **buffer, bool convert_bgr_rgb)
{
	int i;
	int entrynum;
	int size;
	if (entryname==NULL || buffer==NULL)
		return -1;
	*buffer=NULL;
	*width=*height=0;
	entrynum=CResource::FindEntry(entryname);
	if (entrynum<0)
		return -1;
	size=CResource::GetEntrySize(entrynum);
	if (size<=0)
		return -1;
	HGLOBAL hGlobal=GlobalAlloc(GMEM_MOVEABLE,size);
	if (hGlobal==NULL)
		return -1;
	void *pData=GlobalLock(hGlobal);
	if (pData==NULL)
	{
		GlobalFree(hGlobal);
		return -1;
	}
	if (CResource::ReadEntry(pData,entrynum)!=true)
	{
		GlobalUnlock(hGlobal);
		GlobalFree(hGlobal);
		return -1;
	}
	GlobalUnlock(hGlobal);
	IStream *pStream=NULL;
	IPicture *pPicture=NULL;
	if (CreateStreamOnHGlobal(hGlobal,TRUE,&pStream)!=S_OK)
	{
		GlobalFree(hGlobal);
		return -1;
	}
	if (OleLoadPicture(pStream,size,FALSE,IID_IPicture,(LPVOID*)&pPicture)!=S_OK)
	{
		pStream->Release();
		return -1;
	}
	pStream->Release();
	HWND hwndDesktop;
	HDC hdcMem,hdcDesktop;
	hwndDesktop=GetDesktopWindow();
	hdcDesktop=GetDC(hwndDesktop);
	hdcMem=CreateCompatibleDC(hdcDesktop);
	if (hdcMem==NULL)
	{
		ReleaseDC(hwndDesktop,hdcDesktop);
		pPicture->Release();
		return -1;
	}
	long hmWidth;
	long hmHeight;
	if (pPicture->get_Width(&hmWidth)!=S_OK || pPicture->get_Height(&hmHeight)!=S_OK)
	{
		DeleteDC(hdcMem);
		ReleaseDC(hwndDesktop,hdcDesktop);
		pPicture->Release();
		return -1;
	}
	int nWidth=MulDiv(hmWidth,GetDeviceCaps(hdcDesktop,LOGPIXELSX),2540);
	int nHeight=MulDiv(hmHeight,GetDeviceCaps(hdcDesktop,LOGPIXELSY),2540);
	BITMAPINFO bi;
	ZeroMemory(&bi, sizeof(bi));
	bi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bi.bmiHeader.biBitCount=32;
	bi.bmiHeader.biWidth=nWidth;
	bi.bmiHeader.biHeight=nHeight;
	bi.bmiHeader.biCompression=BI_RGB;
	bi.bmiHeader.biPlanes=1;
	char *bits=NULL;
	HBITMAP hbmp=CreateDIBSection(hdcMem,&bi,DIB_RGB_COLORS,(void**)&bits,0,0);
	if (hbmp==NULL)
	{
		DeleteDC(hdcMem);
		ReleaseDC(hwndDesktop,hdcDesktop);
		pPicture->Release();
		return -1;
	}
	HGDIOBJ hobjOld=SelectObject(hdcMem,hbmp);
	HRESULT hr=pPicture->Render(hdcMem,0,0,nWidth,nHeight,0,hmHeight,hmWidth,-hmHeight,NULL);
	size=-1;
	if (hr==S_OK)
	{
		size=nWidth*nHeight*4;
		*buffer=malloc(size);
		if (*buffer!=NULL)
		{
			char t;
			for (i=0;i<size;i+=4)
			{
				bits[i+3]=-1;
				if (convert_bgr_rgb!=true)
					continue;
				t=bits[i];
				bits[i]=bits[i+2];
				bits[i+2]=t;
			}
			MoveMemory(*buffer,bits,size);
			if (width!=NULL)
				*width=nWidth;
			if (height!=NULL)
				*height=nHeight;
		}
		else
		{
			size=-1;
		}
	}
	SelectObject(hdcMem,hobjOld);
	DeleteObject(hbmp);
	DeleteDC(hdcMem);
	ReleaseDC(hwndDesktop,hdcDesktop);
	pPicture->Release();
	return size;
}





bool CLoader::ResizeImage(void **pImage, int *width, int *height)
{
	int maxsize;
	int oldwidth, oldheight;
	int newwidth=1, newheight=1;
	if (pImage==NULL || width==NULL || height==NULL)
		return false;
	oldwidth=*width;
	oldheight=*height;
	glGetIntegerv(GL_MAX_TEXTURE_SIZE,&maxsize);
	CLAMP(oldwidth,1,maxsize);
	CLAMP(oldheight,1,maxsize);
	while ((newwidth<<1)<=oldwidth) { newwidth<<=1; }
	while ((newheight<<1)<=oldheight) { newheight<<=1; }
	if (newwidth==*width && newheight==*height)
		return true;
	void *bits=malloc(4*newwidth*newheight);
	if (bits==NULL)
		return false;
	if (!gluScaleImage(GL_RGBA,*width,*height,GL_UNSIGNED_BYTE,*pImage,newwidth,newheight,GL_UNSIGNED_BYTE,bits))
	{
		free(*pImage);
		*pImage=bits;
		*width=newwidth;
		*height=newheight;
		return true;
	}
	else
	{
		free(bits);
		return false;
	}
}





int CLoader::LoadTexture(const char *imagemap, const char *alphamap, bool mipmaps, bool linear, bool anisotropic)
{
	bool b;
	int tex_width[2], tex_height[2], tex_size[2];
	unsigned char *pImage[2];
	int texture;
	int alphachan=0;
	texture=0;
	if (imagemap!=NULL)
	{
		tex_size[0]=LoadImage(imagemap,&tex_width[0],&tex_height[0],(void**)&pImage[0]);
		if (tex_size[0]>0)
		{
			if (alphamap!=NULL)
			{
				if (CLoader::FindEntry(alphamap)==CLoader::FindEntry(imagemap))
				{
					pImage[1]=pImage[0];
					tex_size[1]=tex_size[0]; tex_width[1]=tex_width[0]; tex_height[1]=tex_height[0];
				}
				else
				{
					tex_size[1]=LoadImage(alphamap,&tex_width[1],&tex_height[1],(void**)&pImage[1]);
				}
				if (tex_size[1]>0 && tex_width[1]==tex_width[0] && tex_height[1]==tex_height[0])
				{
					alphachan=1;
					int j;
					for (j=0;j<tex_size[1];j+=4)
					{
						double R=(double)pImage[1][j];
						double G=(double)pImage[1][j+1];
						double B=(double)pImage[1][j+2];
						double lum=(R*0.299)+(G*0.587)+(B*0.114);
						pImage[0][j+3]=(unsigned char)lum;
					}
				}
				if (pImage[1]!=pImage[0])
					free(pImage[1]);
				pImage[1]=NULL;
			}
			glGenTextures(1,(GLuint*)&texture);
			if (texture>0)
			{
				glBindTexture(GL_TEXTURE_2D,texture);
				glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,(linear?GL_LINEAR:GL_NEAREST));
				b=true;
				if (mipmaps)
				{
					glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,(linear?GL_LINEAR_MIPMAP_LINEAR:GL_NEAREST_MIPMAP_LINEAR));
					if (gluBuild2DMipmaps(GL_TEXTURE_2D,3+alphachan,tex_width[0],tex_height[0],GL_RGBA,GL_UNSIGNED_BYTE,pImage[0])!=0)
						b=false;
				}
				else
				{
					glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,(linear?GL_LINEAR:GL_NEAREST));
					if (ResizeImage((void**)&pImage[0],&tex_width[0],&tex_height[0]))
					{
						glGetError();
						glTexImage2D(GL_TEXTURE_2D,0,3+alphachan,tex_width[0],tex_height[0],0,GL_RGBA,GL_UNSIGNED_BYTE,pImage[0]);
						if (glGetError()!=GL_NO_ERROR)
							b=false;
					}
				}
				if (b==true)
				{
					if (anisotropic)
					{
						if (CVideoBase::GetExtAnisotropic())
						{
							glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MAX_ANISOTROPY_EXT,MAX_ANISOTROPY);
						}
					}
				}
				else
				{
					glDeleteTextures(1,(GLuint *)&texture);
					texture=0;
				}
			}
			free(pImage[0]);
			pImage[0]=NULL;
		}
	}
	return texture;
}





bool CLoader::ExtractFile(const char *resfile, const char *destfile)
{
	void *buffer;
	int readsize, writesize;
	FILE *fd;
	bool b=false;
	if (resfile!=NULL && destfile!=NULL)
	{
		readsize=LoadRaw(resfile,&buffer);
		if (readsize>=0)
		{
			fd=fopen(destfile,"wb");
			if (fd!=NULL)
			{
				writesize=fwrite(buffer,1,readsize,fd);
				if (writesize==readsize)
				{
					b=true;
				}
				fclose(fd);
			}
			free(buffer);
		}
	}
	return b;
}





bool CLoader::LoadText(const char *entryname, char ***lines, int *numlines)
{
	bool b=false;
	if (entryname!=NULL && numlines!=NULL && lines!=NULL)
	{
		char *buffer;
		int size=LoadRaw(entryname,(void**)&buffer);
		if (size>0)
		{
			buffer[size-1]=0;
			char **linesptr=(char**)malloc(size*sizeof(char*));
			if (linesptr!=NULL)
			{
				int linesfound=0;
				char del[3]={13,10,0}; //caution: using this set of delimiters will remove empty lines
				//char *del="\n";
				char *token=strtok(buffer,del);
				while (token!=NULL)
				{
					linesptr[linesfound]=(char*)malloc(strlen(token)+1);
					if (linesptr[linesfound]==NULL)
					{
						free(buffer);
						return false;
					}
					strcpy(linesptr[linesfound],token);
					linesfound++;
					token=strtok(NULL,del);
				}
				linesptr=(char**)realloc(linesptr,linesfound*sizeof(char*));
				*lines=linesptr;
				*numlines=linesfound;
				b=true;
			}
			free(buffer);
		}
	}
	return b;
}





CLoader::object_t *CLoader::LoadObject(const char *entryname)
{
	object_t *obj=NULL;
	char **lines;
	int numlines;
	if (LoadText(entryname,&lines,&numlines))
	{
		obj=(object_t*)malloc(sizeof(object_t));
		if (obj!=NULL)
		{
			obj->numvertices=0;
			obj->numfaces=0;
			obj->vertices=(vertex_t*)malloc(numlines*sizeof(vertex_t));
			obj->faces=(face_t*)malloc(numlines*sizeof(face_t));
			int i,j,k;
			if (obj->vertices!=NULL && obj->faces!=NULL)
			{
				for (i=0;i<numlines;i++)
				{
					if (lines[i][0]=='g')
						break;
				}
				int startline=i+1;
				for (i=startline;i<numlines;i++)
				{
					if (lines[i][0]=='g')
						break;
				}
				int endline=i-1;
				for (i=startline;i<=endline;i++)
				{
					if (lines[i][0]=='v' && lines[i][1]==' ')
					{
						vertex_t *pv=&obj->vertices[obj->numvertices];
						sscanf(lines[i]+2,"%f %f %f",&pv->point[0],&pv->point[1],&pv->point[2]);
						obj->numvertices++;
					}
				}
				for (i=startline;i<=endline;i++)
				{
					if (lines[i][0]=='f' && lines[i][1]==' ')
					{
						int vn[4]; vn[3]=0;
						face_t *pf;
						sscanf(lines[i]+2,"%d %d %d %d",&vn[2],&vn[1],&vn[0],&vn[3]);
						if (vn[3]!=0)
						{
							pf=&obj->faces[obj->numfaces];
							pf->vertexnum[0]=vn[3]-1; pf->vertexnum[1]=vn[0]-1; pf->vertexnum[2]=vn[2]-1;
							obj->numfaces++;
						}
						pf=&obj->faces[obj->numfaces];
						for (j=0;j<=2;j++)
							pf->vertexnum[j]=vn[j]-1;
						obj->numfaces++;
					}
				}
			}
			for (i=0;i<numlines;i++) free(lines[i]);
			free(lines); lines=NULL;
			obj->vertices=(vertex_t*)realloc(obj->vertices,obj->numvertices*sizeof(vertex_t));
			obj->faces=(face_t*)realloc(obj->faces,obj->numfaces*sizeof(face_t));
			for (i=0;i<obj->numfaces;i++)
			{
				face_t *pf=&obj->faces[i];
				float vecs[3][3];
				for (j=0;j<=2;j++)
					for (k=0;k<=2;k++)
						vecs[j][k]=obj->vertices[pf->vertexnum[j]].point[k];
				obj->faces[i].normal[0]=
					(((vecs[1][1]-vecs[0][1])*(vecs[0][2]-vecs[2][2]))-
					((vecs[1][2]-vecs[0][2])*(vecs[0][1]-vecs[2][1])));
				obj->faces[i].normal[1]=
					(((vecs[1][2]-vecs[0][2])*(vecs[0][0]-vecs[2][0]))-
					((vecs[1][0]-vecs[0][0])*(vecs[0][2]-vecs[2][2])));
				obj->faces[i].normal[2]=
					(((vecs[1][0]-vecs[0][0])*(vecs[0][1]-vecs[2][1]))-
					((vecs[1][1]-vecs[0][1])*(vecs[0][0]-vecs[2][0])));
				for (j=0;j<3;j++)
					obj->faces[i].normal[j]*=-1.0f;
			}
			for (i=0;i<obj->numvertices;i++)
			{
				vertex_t *pv=&obj->vertices[i];
				for (j=0;j<=2;j++)
					pv->normal[j]=0.0f;
				int c=0;
				for (j=0;j<obj->numfaces;j++)
				{
					if (obj->faces[j].vertexnum[0]==i || obj->faces[j].vertexnum[1]==i || obj->faces[j].vertexnum[2]==i)
					{
						for (k=0;k<=2;k++)
							pv->normal[k]+=obj->faces[j].normal[k];
						c++;
					}
				}
				if (c!=0)
				{
					for (j=0;j<=2;j++)
						pv->normal[j]/=c;
				}
				double r=sqrt((pv->point[0]*pv->point[0])+(pv->point[1]*pv->point[1])+(pv->point[2]*pv->point[2]));
//				pv->tex_coords[1]=(float)(asin(pv->point[2]/r)/M_PI)+0.5f;
//				pv->tex_coords[0]=(float)(atan2(pv->point[1],pv->point[0])/(M_PI*2.0))+0.5f;
				pv->tex_coords[1]=(float)(asin(pv->point[1]/r)/M_PI)+0.5f;
				pv->tex_coords[0]=(float)(atan2(-pv->point[2],pv->point[0])/(M_PI*2.0))+0.5f;
			}
		}
	}
	return obj;
}
