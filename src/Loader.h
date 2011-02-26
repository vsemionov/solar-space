#ifndef LOADER_H
#define LOADER_H

#include "defs.h"

#include "Resource.h"






class CLoader : protected CResource
{
public:

	typedef struct vertex_s
	{
		float point[3];
		float normal[3];
		float tex_coords[2];
	} vertex_t;
	typedef struct face_s
	{
		int vertexnum[3];
		float normal[3];
	} face_t;
	typedef struct object_s
	{
		int numvertices;
		int numfaces;
		vertex_t *vertices;
		face_t *faces;
	} object_t;

	CLoader();
	virtual ~CLoader();
	bool Init();
	void ShutDown();
	bool WithResource(const char *filename);
	void End();
	int LoadRaw(const char *entryname, void **buffer);
	int LoadImage(const char *entryname, int *width, int *height, void **buffer, bool convert_bgr_rgb=true);
	int LoadTexture(const char *imagemap, const char *alphamap, bool mipmaps, bool linear);
	bool ExtractFile(const char *resfile, const char *destfile);
	bool LoadText(const char *entryname, char ***lines, int *numlines);
	object_t *LoadObject(const char *entryname);
private:
	static bool ResizeImage(void **pImage, int *width, int *height);
};

#endif
