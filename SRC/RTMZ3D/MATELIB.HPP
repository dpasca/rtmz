//==============================================
// MATELIB.HPP - 
// Copyright (C) Davide Pasca 1995
//==============================================

#ifndef __MATELIB_HPP__
#define __MATELIB_HPP__

#include "EXTTYPES.HPP"
#include "COLORS.HPP"

#include <stdarg.h>

#define	ACT	__MLB_actLibP

#define MLB_MAXMATERIALS	256

#define MLB_NAME_PTR			6L
#define MLB_LIGHT_PTR			1L
#define MLB_AMBIENT_PTR			2L
#define MLB_DIFFUSE_PTR			3L
#define MLB_SPECULAR_PTR		4L
#define MLB_SHININESS_PTR		5L
#define MLB_TXMAP_N_PALETTE_PTR	7L

typedef struct
{
	UB		origPal[768], currPal[768];
	US		origNCols, currNCols;
	BitMap	origMap, currMap;
}MLB_Texture_t;

typedef struct
{
	char			name[32];
	UB				active;
	UB				shadePalCol;
	float			light[4], ambient[4], diffuse[4], specular[4], shininess;
	MLB_Texture_t	*textureP;
}MLB_Material_t;

typedef struct
{
	US					nMaterials, nTextures;
	MLB_Material_t		materials[ MLB_MAXMATERIALS ];
	COL_ShadePalette_t	shadePal;
	COL_LevelPalette_t	*levPalP;
	float				deflight[4], defambient[4], defdiffuse[4], defspecular[4], defshininess;
	MLB_Texture_t		deftexture;
}MLB_Library_t;

extern MLB_Library_t	*__MLB_actLibP;

inline MLB_Material_t *MLB_MaterialGetPtr( long matID )
{
	return &__MLB_actLibP->materials[matID];
}

void MLB_LibraryReset(void);
inline MLB_Library_t *MLB_LibraryGetPtr(void)
{
	return ACT;
}
inline MLB_Material_t *MLB_LibraryGetMaterial( US i ){ return &__MLB_actLibP->materials[i]; }

long MLB_MaterialFind( const char *nameP );
long MLB_MaterialSetAll( long tag, ... );
void MLB_MaterialGetDefault( long tag, ... );
void MLB_MaterialSetDefault( long tag, ... );
long MLB_MaterialAdd( long tag, ... );
long MLB_MaterialSetOrAdd( long tag, ... );
void MLB_MaterialDisposeAll( void );

void MLB_PaletteCompute( US userColors );
UB *MLB_PaletteGet(long *stacolp, long *ncolsp);

#undef	ACT

#endif

