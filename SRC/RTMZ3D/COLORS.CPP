//==============================================
// Colors.cpp - color handling routines
// Copyright (C) Davide Pasca 1994
//==============================================

#include <dos.h>
#include <conio.h>
#include <malloc.h>
#include <stdio.h>
#include <mem.h>
#include "COLORS.HPP"

#define MAKEBIG(x)	(x = 1L << (sizeof(x)*8-2))

//====================================
void COL_LevelPalette( COL_LevelPalette_t *fogP )
{
	FREEIF( fogP );
}

//====================================
#define MAX_LVLS	256

COL_LevelPalette_t *COL_NewDark(const UB *palP, US nLevels, float coe, US nColors )
{
COL_LevelPalette_t	*fogP;
//UB			*hashp;

	if ( nLevels == 0 || nLevels > MAX_LVLS )	return 0;
	if NOT( fogP = (COL_LevelPalette_t *)malloc( sizeof(COL_LevelPalette_t)+nLevels*256L ) )
		return 0;

	/*
	if NOT( NEW(hashp,32768) )
	{
		free( fogP );
		return 0;
	}
	memset( hashp, 0, 32768 );
	*/

	fogP->nLevels = nLevels;

	if NOT( palP )
		return 0;

UB			tmpPal[MAX_LVLS*3], *rTmpPalP;
const UB	*rPalP;
UB			*indexP;
short		i,j;

	rPalP = palP;
	for (i=0; i < nColors; ++i)
	{
		COL_CreateRangeRGB64( tmpPal, 0, nLevels, rPalP[0], rPalP[1], rPalP[2], rPalP[0]*coe, rPalP[1]*coe, rPalP[2]*coe );
		indexP = fogP->index+i;
		rTmpPalP = tmpPal;
		for(j=nLevels; j; --j)
		{
		UB	r,g,b;

			r = rTmpPalP[0];
			g = rTmpPalP[1];
			b = rTmpPalP[2];
			*indexP = COL_FindBestRGBInPalette( r, g, b, palP, nColors );
			//*indexP = COL_MatchColorHash15( hashp, r, g, b, palP, nColors );
			//hash table = poor resolution + giving me trouble mapping unused colors =
			//= fuck it

			indexP += 256;

			rTmpPalP += 3;
		}
		rPalP += 3;
	}

	//free( hashp );

	return  fogP;
}

//====================================
COL_LevelPalette_t *COL_NewFog(const UB *palP, US nLevels, UB fr, UB fg, UB fb )
{
COL_LevelPalette_t	*fogP;

	if ( nLevels == 0 || nLevels > MAX_LVLS )	return 0;
	if NOT( fogP = (COL_LevelPalette_t *)malloc( sizeof(COL_LevelPalette_t)+nLevels*256L ) )
		return 0;

	fogP->nLevels = nLevels;

	if NOT( palP )
		return 0;

UB			tmpPal[MAX_LVLS*3], *rTmpPalP;
const UB	*rPalP;
UB			*indexP;
short		i,j;

	rPalP = palP;
	for (i=0; i < 256; ++i)
	{
		COL_CreateRangeRGB64( tmpPal, 0, nLevels, rPalP[0], rPalP[1], rPalP[2], fr, fg, fb );
		indexP = fogP->index+i;
		rTmpPalP = tmpPal;
		for(j=nLevels; j; --j)
		{
		UB	r,g,b;

			r = rTmpPalP[0];
			g = rTmpPalP[1];
			b = rTmpPalP[2];
			*indexP = COL_FindBestRGBInPalette( r, g, b, palP, 256 );
			indexP += 256;

			rTmpPalP += 3;
		}
		rPalP += 3;
	}

	return  fogP;
}

//====================================
short COL_FindExactRGB( UB r, UB g, UB b, const UB *palP, US palWd )
{
US		i;

	for (i = 0; i < palWd; ++i, palP += 3)
		if ( palP[0] == r && palP[1] == g && palP[2] == b )
			return i;

	return -1;
}

//====================================
short COL_FindBestRGBInPalette( UB r, UB g, UB b, const UB *palP, US palWd )
{
US		i;
long	dr,dg,db;
long	rl,gl,bl;
long	diff, minDiff, bestI=0;

	rl = r;
	gl = g;
	bl = b;
	MAKEBIG( minDiff );
	for (i = 0; i < palWd && minDiff; ++i, palP += 3)
	{
		dr = (long)palP[0] - rl;
		dg = (long)palP[1] - gl;
		db = (long)palP[2] - bl;

		diff = dr * dr + dg * dg + db * db;
		//diff = (dr << 16) + (dg << 8) + db;
		if (diff < minDiff)
		{
			minDiff = diff;
			bestI = i;
		}
	}

	return bestI;
}

//==============================================================
UB COL_MatchColorHash15( UB *hashp, UB r, UB g, UB b, const UB *palP, US palWd )
{
UL	idx;

	idx = ((UL)r << 7) + ((UL)g << 2) + ((UL)b >> 3);

	if ( idx == 0 )
		return 0;

	if ( hashp[idx] == 0 )
		hashp[idx] = COL_FindBestRGBInPalette( r, g, b, palP, palWd );

	return hashp[idx];
}

//============================================
void COL_CreateShadePalette( COL_ShadePalette_t *spalP, UB *srcPalP, US nColors, US userColors, float minLight, float maxLight)
{
float	shadeVal, shadeStep;
US		nShades;
US		i,j;

	if NOT( nColors )	return;

	spalP->userColors = userColors;
	if ( nColors )	nShades = (256-userColors) / nColors;
	if ( nShades )	shadeStep = (maxLight-minLight) / nShades;

	UB *desPalP = (UB *)spalP->palette + userColors*3;
	for(i=nColors; i; --i)
	{
	float	r,g,b;

		r = srcPalP[0];	g = srcPalP[1];	b = srcPalP[2];	srcPalP += 3;
		shadeVal = minLight;
		for(j=nShades; j; --j)
		{
		float	t;
			t = r * shadeVal; desPalP[0] = MIN(t,63.);
			t = g * shadeVal; desPalP[1] = MIN(t,63.);
			t = b * shadeVal; desPalP[2] = MIN(t,63.);
			desPalP += 3;
			shadeVal += shadeStep;
		}
	}
	spalP->nShades = nShades;
	spalP->nColors = nColors;
}

//====================================
void COL_256to64( UB *p, US cnt )
{
	while( cnt-- )
		*p++ >>= 2;
}

//====================================
void COL_CreateRangeRGB64( UB *palP, US i, US n, UB r1, UB g1, UB b1, UB r2, UB g2, UB b2 )
{
long	stp_r, stp_g, stp_b;
long	r,g,b;

	if NOT(n)
		return;

	stp_r = ((long)r2-r1 << 16) / (long)n;
	stp_g = ((long)g2-g1 << 16) / (long)n;
	stp_b = ((long)b2-b1 << 16) / (long)n;

	r = (long)r1 << 16;
	g = (long)g1 << 16;
	b = (long)b1 << 16;
	palP += i*3;
	do{
		palP[0] = r>>16;
		palP[1] = g>>16;
		palP[2] = b>>16;
		palP += 3;

		r += stp_r;
		g += stp_g;
		b += stp_b;
	}while( --n );
}

//============================================
void COL_CreateTransparencyLUT( long rl, long gl, long bl, const UB *palp, UB *lutp, US nColors )
{
long		i, tr, tg, tb;
const UB	*runpalp;

	runpalp	= palp;
	for(i=nColors; i>0; --i, runpalp += 3)
	{
		tr = (long)((UL)runpalp[0])	+ rl >> 1;
		tg = (long)((UL)runpalp[1])	+ gl >> 1;
		tb = (long)((UL)runpalp[2])	+ bl >> 1;
		*lutp++ = COL_FindBestRGBInPalette( tr, tg, tb, palp, nColors );
	}
}

//=======================================
void COL_CreateRemapLUT( UB *lut, const UB *origpal, const UB *newpal)
{
long	i;

	for(i=256; i; --i, origpal += 3)
		*lut++ = (UB)COL_FindBestRGBInPalette( origpal[0], origpal[1], origpal[2], newpal, 256 );
}

