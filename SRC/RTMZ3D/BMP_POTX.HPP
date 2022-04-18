//==============================================
// BMP_POTX.HPP - texture-mapped polygons
// Copyright (C) by Davide Pasca 1995
//==============================================

#ifndef	__BMP_POTX_HPP__
#define __BMP_POTX_HPP__

#include "BMP_POLY.HPP"

void BMP_PolyTextureLUT3D16( const POE_PolyI_t *polyP, const BitMap *tmP, const UB *lut );

//================ TEXTURE 2D =====================
inline void BMP_PolyTexture2D16( const POE_PolyI_t *polyP, const BitMap *tmP )
{
extern void _bmp_pt2d16setupI(const POE_PolyI_t *, const BitMap *);
extern void _bmp_pt2d16slope(POE_Vert_t *, POE_Vert_t *, long, long );
extern void _bmp_pt2d16loop(UB *, long);

	_bmp_pt2d16setupI( polyP, tmP );
	BMP_PolyRoutineI( polyP, 0, _bmp_pt2d16slope, _bmp_pt2d16loop );
}
//============== TEXTURE 2D LUT =====================
inline void BMP_PolyTextureLUT2D16( const POE_PolyI_t *polyP, const BitMap *tmP, const UB *lutP )
{
extern void _bmp_pt2dlut16setupI(const POE_PolyI_t *, const BitMap *, const UB *);
extern void _bmp_pt2d16slope(POE_Vert_t *, POE_Vert_t *, long, long );
extern void _bmp_pt2dlut16loop(UB *, long);

	_bmp_pt2dlut16setupI( polyP, tmP, lutP );
	BMP_PolyRoutineI( polyP, 0, _bmp_pt2d16slope, _bmp_pt2dlut16loop );
}
//================ TEXTURE CHROME =====================
inline void BMP_PolyTextureChrome16( const POE_PolyI_t *polyP, const BitMap *tmP )
{
extern void _bmp_ptchrome16setupI(const POE_PolyI_t *, const BitMap *);
extern void _bmp_ptchrome16slope(POE_Vert_t *, POE_Vert_t *, long, long );
extern void _bmp_pt2d16loop(UB *, long);

	_bmp_ptchrome16setupI( polyP, tmP );
	BMP_PolyRoutineI( polyP, 0, _bmp_ptchrome16slope, _bmp_pt2d16loop );
}
//============== TEXTURE 3D LUT =====================
void BMP_PolyTextureLUT3D16( const POE_PolyI_t *polyP, const BitMap *tmP, const UB *lut );

#endif