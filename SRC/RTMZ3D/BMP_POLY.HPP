//==============================================
// BMP_POLY.HPP - raster graphics stuff
// Copyright (C) by Davide Pasca 1995
//==============================================

#ifndef __BMP_POLY_HPP__
#define __BMP_POLY_HPP__

#include "EXTTYPES.HPP"
#include "POLYENG.HPP"

extern BitMap           *_BMP_curBitMapP;
#define ACT                     _BMP_curBitMapP

typedef void (*BMP_PolySetupFuncI_t)(const POE_PolyI_t *);
typedef void (*BMP_PolySetupFunc_t)(const POE_Poly_t *);
typedef void (*BMP_PolySlopeFunc_t)(POE_Vert_t *, POE_Vert_t *, long, long );
typedef void (*BMP_PolyLoopFunc_t)(UB *, long);

//=================================================
void BMP_PolyRoutine( const POE_Poly_t *polyP, BMP_PolySetupFunc_t setupFunc,
												BMP_PolySlopeFunc_t slopeFunc,
												BMP_PolyLoopFunc_t loopFunc );
//-------------------------------------------------
void BMP_PolyRoutineI( const POE_PolyI_t *polyP, BMP_PolySetupFuncI_t setupFunc,
												 BMP_PolySlopeFunc_t slopeFunc,
												 BMP_PolyLoopFunc_t loopFunc );

//=================== FLAT ========================
/*inline void BMP_PolyFlat16( const POE_PolyI_t *polyP )
{
BMP_PolySetupFunc_t     _bmp_pf16setup;
BMP_PolyLoopFunc_t      _bmp_pf16loop;
BMP_PolySlopeFunc_t     _bmp_pf16slope;

	BMP_PolyRoutine( polyP, _bmp_pf16setup, _bmp_pf16slope, _bmp_pf16loop );
}*/
//-------------------------------------------------
inline void BMP_PolyFlat16( const POE_PolyI_t *polyP )
{
extern void _bmp_pf16setupI(const POE_PolyI_t *);
extern void _bmp_pf16slope(POE_Vert_t *, POE_Vert_t *, long, long );
extern void _bmp_pf16loop(UB *, long);

	BMP_PolyRoutineI( polyP, _bmp_pf16setupI, _bmp_pf16slope, _bmp_pf16loop );
}
//-------------------------------------------------
inline void BMP_PolyFlatZ16( const POE_PolyI_t *polyP )
{
extern void _bmp_pf16setupI(const POE_PolyI_t *);
extern void _bmp_pfz16slope(POE_Vert_t *, POE_Vert_t *, long, long );
extern void _bmp_pfz16loop(UB *, long);

	BMP_PolyRoutineI( polyP, _bmp_pf16setupI, _bmp_pfz16slope, _bmp_pfz16loop );
}

//================ GOURAUD =====================
inline void BMP_PolyGouraud16( const POE_PolyI_t *polyP )
{
extern void _bmp_pg16setupI(const POE_PolyI_t *);
extern void _bmp_pg16slope(POE_Vert_t *, POE_Vert_t *, long, long );
extern void _bmp_pg16loop(UB *, long);

	BMP_PolyRoutineI( polyP, _bmp_pg16setupI, _bmp_pg16slope, _bmp_pg16loop );
}
//-------------------------------------------------
inline void BMP_PolyGouraudZ16( const POE_PolyI_t *polyP )
{
extern void _bmp_pg16setupI(const POE_PolyI_t *);
extern void _bmp_pgz16slope(POE_Vert_t *, POE_Vert_t *, long, long );
extern void _bmp_pgz16loop(UB *, long);

	BMP_PolyRoutineI( polyP, _bmp_pg16setupI, _bmp_pgz16slope, _bmp_pgz16loop );
}

//=================================================
void BMP_ZBufferSet( UL *zbufP );
void BMP_PolyPhong16( const POE_PolyI_t *polyP, const float *lightSrcP, US nShad, US nUsrCols );

//==================================
static inline long floor4(long a){      return a & 0xfffffff0;  }
static inline long ceil4(long a){       return (a+15) & 0xfffffff0;     }

#define LINC(X) lrun.X += ldelta.X
#define RINC(X) rrun.X += rdelta.X

#define POLYROUTINE( NAME, PREMINCHECK, PREWHILE, DELTANAME, DRAWSPAN ) \
void NAME \
{\
short   nVerts;\
long    ymin;\
long    y, top, i;\
const POE_Vert_t        *vertsP, *fvertsP[POE_VERTBUFSIZ];\
const long                      *vertIdxP;\
\
	if ( (nVerts=polyP->nVerts) < 3 )       return;\
\
	vertsP = *polyP->vertH;\
	vertIdxP = polyP->vertIdx;\
PREMINCHECK;\
	ymin = LONG_MAX;\
    for (i=nVerts-1; i >= 0; --i)\
	{\
		fvertsP[i] = &vertsP[ vertIdxP[i] ];\
		if ( fvertsP[i]->screen[1] < ymin )\
		{\
			ymin = fvertsP[i]->screen[1];\
			top = i;\
		}\
	}\
\
long    ly,ry,li,ri,rem;\
\
    li = ri = top;\
    rem = nVerts;\
    y = ceil4(ymin-8);\
    ly = ry = y-16;\
\
POE_Vert_t      lrun,ldelta,rrun,rdelta;\
long    bmWd = ACT->wd;\
PREWHILE;\
\
	while (rem>0)\
	{\
		while (ly<=y && rem>0)\
		{\
			--rem;\
			if ( (i = li-1) < 0)            i = nVerts-1;\
			DELTANAME(y, &lrun, &ldelta, fvertsP[li], fvertsP[i], li,i);\
			ly = floor4(fvertsP[i]->screen[1]+8);\
			li = i;\
		}\
		while (ry<=y && rem>0)\
		{\
			--rem;\
			if ( (i = ri+1) >= nVerts )     i = 0;\
			DELTANAME(y, &rrun, &rdelta, fvertsP[ri], fvertsP[i], ri,i);\
			ry = floor4(fvertsP[i]->screen[1]+8);\
			ri = i;\
		}\
\
		UB      *mapP = ACT->offsPP[y>>4];\
		ymin = MIN(ly,ry);\
		while ( y < ymin )\
		{\
		long    xl,xr;\
		POE_Vert_t      *l,*r;\
		\
			if ( rrun.screen[0] < lrun.screen[0] )\
				{ l=&rrun; r=&lrun; }\
			else\
				{ l=&lrun; r=&rrun; }\
			xl = l->screen[0] + 8*16 >>16;\
			xr = r->screen[0] >> 16;\
			DRAWSPAN;\
			LINC(screen[0]);        RINC(screen[0]);\
			mapP += bmWd;\
			y += 16;\
		}\
    }\
}

#undef ACT

#include "BMP_POTX.HPP"

#endif


