//==============================================
// TDSTUFF.HPP - world 3d engine
// Copyright (C) Davide Pasca 1995
//==============================================
#ifndef __TDSTUFF_HPP__
#define __TDSTUFF_HPP__

#include <math.h>
#include <malloc.h>
#include "EXTTYPES.HPP"
#include "COLORS.HPP"
#include "OBJ3D.HPP"

#define W3D_MAXOBJECTS	100

#define W3D_REND_CHUNKY	(1<<0)
#define W3D_REND_GORAUD	(1<<1)

//================================
typedef struct
{
	short	NObjs;
	Obj3D	*objList[ W3D_MAXOBJECTS ];
	BO		objMatPresent[ W3D_MAXOBJECTS ];
	float	objMatList[ W3D_MAXOBJECTS ][16];
	long	NPoints;
	long	NPolys;
	long	nActPoints;
	long	nActPolys;
	long	*ZMediumP;
	US		*zBufferP;
	long	zBufferFlg;
	BitMap	bitMap;
	long	outWD, outHE;

#ifdef __USEFLOAT
	float	DeeX, DeeY;
#else
	short	DeeX, DeeY;
#endif

	COL_ShadePalette_t	GeoSPal, GemSPal, *ActSPaletteP;
	UB					UseShadePal;
	COL_LevelPalette_t	*LevelPalP;

	float	lightPos[3];
}W3D_World_t;

extern void project(long *des2P, const float *src3P, float deex, float deey, long hwd, long hhe, float minz, long cnt );

extern W3D_World_t	*_W3D_actWorldP;
#define ACT	_W3D_actWorldP

void W3D_Initialize(void);
void W3D_LightSetPos( float x, float y, float z );
long W3D_ViewPortSet( US wd, US he );
void W3D_PerspectiveSet( float fovy, float aspect, float n, float f );


#define	W3D_BITMAP_PTR	0
#define	W3D_ZBUFFER_FLG	1
void W3D_ValueGet( UB cmd, long *destP );
void W3D_ValueSet( UB cmd, long *destP );

void W3D_FrameClear(void);
void W3D_FrameCalculate(void);
void W3D_FrameRend(void);

void W3D_ObjectDraw( long objID, const float *matp );
void W3D_ObjectCenter( long objID );

void W3D_ShadePaletteSet(COL_ShadePalette_t *sPalP);

#undef ACT

//====================================
inline UB IsCCW( const long *v1, const long *v2, const long *v3 )
{
	if ( (v2[0]-v1[0]) * (v3[1]-v2[1]) >=
		 (v2[1]-v1[1]) * (v3[0]-v2[0]) )
		return 0;

	return 1;
}

#endif
