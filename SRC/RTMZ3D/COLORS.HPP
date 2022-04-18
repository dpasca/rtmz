//==============================================
// COLORS.HPP -
// Copyright (C) Davide Pasca 1995
//==============================================

#ifndef __COLORS_HPP__
#define __COLORS_HPP__

#include "EXTTYPES.HPP"
//#include "winstub.h"

#ifndef _WINDOWS_
#ifndef __WINSTUB_H__
typedef struct tagRGBQUAD {
	UB    rgbBlue;
	UB    rgbGreen;
	UB    rgbRed;
	UB    rgbReserved;
} RGBQUAD;
#endif
#endif

#define COL_GET_R(n)    (((n)&0x00ff0000)>>16)
#define COL_GET_G(n)    (((n)&0x0000ff00)>>8)
#define COL_GET_B(n)     ((n)&0x000000ff)

typedef struct
{
	US      userColors;
	US      nColors;
	US      nShades;
	UB      palette[256][3];
}COL_ShadePalette_t;

typedef struct
{
	US      nLevels;
	UB      index[1];
}COL_LevelPalette_t;

void COL_LevelPalette( COL_LevelPalette_t *fogP );
COL_LevelPalette_t *COL_NewFog(const UB *palP, US nLevels, UB fr, UB fg, UB fb );
COL_LevelPalette_t *COL_NewDark(const UB *palP, US nLevels, float coe, US nColors );
short COL_LPA_GetColor( const COL_LevelPalette_t *lpP, short col, short lev );

UB COL_MatchColorHash15( UB *hashp, UB r, UB g, UB b, const UB *palP, US palWd );
short COL_FindBestRGBInPalette( UB r, UB g, UB b, const UB *palP, US palWd );
short COL_FindExactRGB( UB r, UB g, UB b, const UB *palP, US palWd );

short COL_FindBestRGBQUAD( long rl, long gl, long bl, const RGBQUAD *palP, US palWd );
void COL_CreateTransparencyLUT( long rl, long gl, long bl, const UB *palp, UB *lutp, US nColors );

void COL_CreateShadePalette( COL_ShadePalette_t *spalP, UB *srcPalP, US nColors, US userColors, float minLight, float maxLight);
void COL_256to64( UB *p, US cnt );
void COL_CreateRangeRGB64( UB *palP, US i, US n, UB r1, UB g1, UB b1, UB r2, UB g2, UB b2 );
inline void COL_CreateRangeRgb64( UB *palP, UB i, UB n, UL rgb1, UL rgb2 )
{
	COL_CreateRangeRGB64( palP, i, n, COL_GET_R(rgb1),COL_GET_G(rgb1),COL_GET_B(rgb1),
									  COL_GET_R(rgb2),COL_GET_G(rgb2),COL_GET_B(rgb2) );
}

inline short COL_LPA_GetColor( const COL_LevelPalette_t *lpP, short col, short lev )
{ return lpP->index[col + lev * 256]; }

UB *COL_InverseTableNew(const RGBQUAD *palP, US palWd);
void COL_InverseTableDispose( UB *itp );
void COL_TransLUTCreateFast( long rl, long gl, long bl, const RGBQUAD *palp, UB *invTabP, UB *lutp );

void COL_CreateRemapLUT(UB *lut, const UB *origpal, const UB *newpal);

#endif
