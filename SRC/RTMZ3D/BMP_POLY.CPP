//==============================================
// BMP_POLY.CPP - raster graphics stuff
// Copyright (C) Davide Pasca 1995-97
//
// See "readme.txt" for other credits
//
// TABS=4
//==============================================

#include <stdio.h>
#include <conio.h>
#include <limits.h>
#include "BMP.HPP"
#include "BMP_POLY.HPP"

#define ACT			_BMP_curBitMapP

#define	LINCPR(__X__)	_bmp_pr_lrun.__X__ += _bmp_pr_ldelta.__X__
#define	RINCPR(__X__)	_bmp_pr_rrun.__X__ += _bmp_pr_rdelta.__X__

//==================================
US	*_bmp_zbufferP;
void BMP_ZBufferSet( UL *zbufP )
{
	_bmp_zbufferP = (US *)zbufP;
}

//==========================================
POE_Vert_t			_bmp_pr_lrun, _bmp_pr_ldelta, _bmp_pr_rrun, _bmp_pr_rdelta;
long				_bmp_pr_y;
const POE_Vert_t	*_bmp_pr_fvertsP[POE_VERTBUFSIZ];

void BMP_PolyRoutineI( const POE_PolyI_t *polyP, BMP_PolySetupFuncI_t setupFunc,
												 BMP_PolySlopeFunc_t slopeFunc,
												 BMP_PolyLoopFunc_t loopFunc )
{
short	nVerts;
long	ymin;
long	top, i;
const POE_Vert_t	*vertsP, *fvertsP[POE_VERTBUFSIZ];
const long			*vertIdxP;

	if ( (nVerts=polyP->nVerts) < 3 )	return;
	if ( setupFunc )	setupFunc( polyP );

	vertsP = *polyP->vertH;
	vertIdxP = polyP->vertIdx;

	ymin = LONG_MAX;
    for (i=nVerts-1; i >= 0; --i)
	{
		_bmp_pr_fvertsP[i] = &vertsP[ vertIdxP[i] ];
		if ( _bmp_pr_fvertsP[i]->screen[1] < ymin )
		{	ymin = _bmp_pr_fvertsP[i]->screen[1];
			top = i;
		}
	}

long	ly,ry,li,ri,rem;

    li = ri = top;
    rem = nVerts;
    _bmp_pr_y = ceil4(ymin-8);
    ly = ry = _bmp_pr_y-16;

	while (rem>0)
	{
		while (ly<=_bmp_pr_y && rem>0)
		{
			--rem;
			if ( (i = li-1) < 0)		i = nVerts-1;
			slopeFunc(&_bmp_pr_lrun, &_bmp_pr_ldelta, li,i);
			ly = floor4(_bmp_pr_fvertsP[i]->screen[1]+8);
			li = i;
		}
		while (ry<=_bmp_pr_y && rem>0)
		{
			--rem;
			if ( (i = ri+1) >= nVerts )	i = 0;
			slopeFunc(&_bmp_pr_rrun, &_bmp_pr_rdelta, ri,i);
			ry = floor4(_bmp_pr_fvertsP[i]->screen[1]+8);
			ri = i;
		}

		ymin = MIN(ly,ry);
		loopFunc( ACT->offsPP[_bmp_pr_y>>4], ymin );
    }
}

//==========================================
void BMP_PolyRoutine( const POE_Poly_t *polyP, BMP_PolySetupFunc_t setupFunc,
											   BMP_PolySlopeFunc_t slopeFunc,
											   BMP_PolyLoopFunc_t loopFunc )
{
short	nVerts;
long	ymin;
long	top, i;
const POE_Vert_t	*vertsP, *fvertsP[POE_VERTBUFSIZ];

	if ( (nVerts=polyP->nVerts) < 3 )	return;
	if ( setupFunc )	setupFunc( polyP );

	ymin = LONG_MAX;
    for (i=nVerts-1; i >= 0; --i)
	{
		_bmp_pr_fvertsP[i] = &polyP->verts[i];
		if ( _bmp_pr_fvertsP[i]->screen[1] < ymin )
		{	ymin = _bmp_pr_fvertsP[i]->screen[1];
			top = i;
		}
	}

long	ly,ry,li,ri,rem;

    li = ri = top;
    rem = nVerts;
    _bmp_pr_y = ceil4(ymin-8);
    ly = ry = _bmp_pr_y-16;

	while (rem>0)
	{
		while (ly<=_bmp_pr_y && rem>0)
		{
			--rem;
			if ( (i = li-1) < 0)		i = nVerts-1;
			slopeFunc(&_bmp_pr_lrun, &_bmp_pr_ldelta, li,i);
			ly = floor4(_bmp_pr_fvertsP[i]->screen[1]+8);
			li = i;
		}
		while (ry<=_bmp_pr_y && rem>0)
		{
			--rem;
			if ( (i = ri+1) >= nVerts )	i = 0;
			slopeFunc(&_bmp_pr_rrun, &_bmp_pr_rdelta, ri,i);
			ry = floor4(_bmp_pr_fvertsP[i]->screen[1]+8);
			ri = i;
		}

		ymin = MIN(ly,ry);
		loopFunc( ACT->offsPP[_bmp_pr_y>>4], ymin );
    }
}

//==========================================
//             FLAT SHADING
//==========================================
static UB	_bmp_pf16_col;
//------------------------------------------
void _bmp_pf16setupI( const POE_PolyI_t *polyP )
{
	_bmp_pf16_col = polyP->c[0];
}
//------------------------------------------
void _bmp_pf16setup( const POE_Poly_t *polyP )
{
	_bmp_pf16_col = polyP->verts[0].c;
}
//------------------------------------------
void _bmp_pf16slope(POE_Vert_t *run, POE_Vert_t *delta, long i1, long i2)
{
const POE_Vert_t *p1=_bmp_pr_fvertsP[i1], *p2=_bmp_pr_fvertsP[i2];
	
	delta->screen[0] = p2->screen[0] - p1->screen[0] << 16;
	if ( delta->screen[1] = p2->screen[1] - p1->screen[1] )
		delta->screen[0] /= delta->screen[1];
	run->screen[0] = (p1->screen[0] << 12) + (delta->screen[0] * (_bmp_pr_y - p1->screen[1]+8) >> 4);
}
//---------------------------------------------------------
extern void _bmp_memfill(const UB *d, UL col, long wd);
#pragma aux _bmp_memfill =   \
"	inc		ecx"\
"	jle		esci"\
"	cld"\
"	mov		edx,ecx"\
"	shr		ecx,2"\
"	rep		stosd"\
"	and		edx,0x3"\
"	mov		ecx,edx"\
"	rep		stosb"\
"esci:"\
parm caller [edi][eax][ecx]\
modify [edi al ecx edx];
//---------------------------------------------------------
void _bmp_pf16loop( UB *mapp, long ymin )
{
long	wd = ACT->wd;
long	col;

	col = _bmp_pf16_col | ((long)_bmp_pf16_col << 8);
	col = col | (col << 16);
	while ( _bmp_pr_y < ymin )
	{
	long		xl,xr,t;

		xl = _bmp_pr_lrun.screen[0];
		xr = _bmp_pr_rrun.screen[0];
		if ( xr < xl )	SWAP( xl, xr, t );
		xl = xl + 32768 >> 16;
		xr = xr - 32768 >> 16;

		_bmp_memfill( mapp+xl, col, xr-xl );

		LINCPR(screen[0]);	RINCPR(screen[0]);
		mapp += wd;
		_bmp_pr_y += 16;
	}
}

//==========================================
//       FLAT SHADING + Z-BUFFER
//==========================================
//------------------------------------------
void _bmp_pfz16slope(POE_Vert_t *run, POE_Vert_t *delta, long i1, long i2)
{
const POE_Vert_t *p1=_bmp_pr_fvertsP[i1], *p2=_bmp_pr_fvertsP[i2];
	
	delta->screen[0] = p2->screen[0] - p1->screen[0] << 16;
	delta->sz = p2->sz - p1->sz;
	if ( delta->screen[1] = p2->screen[1] - p1->screen[1] )
	{
		delta->screen[0] /= delta->screen[1];
		delta->sz <<= 4; delta->sz /= delta->screen[1];
	}

	long coe = _bmp_pr_y - p1->screen[1]+8;
	run->screen[0] = (p1->screen[0] << 12) + (delta->screen[0] * coe >> 4);
	run->sz = p1->sz + (delta->sz * coe >> 4);
}
//------------------------------------------
static US	*_zBufP;
static inline void _bmp_memfillZ(UB *d, UB col, long z1, long z2, long wd)
{
	if ( wd > 0 )
	{
	US		*zbp = _zBufP;
	US		*zbendp;
	long	stpZ = (z2-z1) / wd;
	US		z;

		/*if ( wd < 16 )*/
		{
			zbendp = zbp + wd;
			do{
				z = z1 >> 14;
				if (z > *zbp)
				{
					*zbp = z;
					*d = col;
				}
				z1 += stpZ; ++d;
			}while( ++zbp < zbendp );
		}
	/*	else
		{
		UL		col2 = (col | ((UL)col << 8));
		long	stpZ4 = stpZ * 4;

			col2  = col2 | (col2 << 16);
			while ( (long)d & 3 )
			{
				z = z1 >> 14;
				if (z > *zbp)
				{
					*zbp = z;
					*d = col;
				}
				z1 += stpZ;
				++d;
				++zbp;
				--wd;
			}

			zbendp = zbp + (wd & ~3);
			do{
				z = z1 >> 14;
				if (z > *zbp)
				{
					zbp[0] = zbp[1] = zbp[2] = zbp[3] = z;
					*(UL *)d = col2;
				}
				z1 += stpZ4;
				d += 4;
				zbp += 4;
			}while( zbp != zbendp );

			wd &= 3;
			while ( wd )
			{
				z = z1 >> 14;
				if (z > *zbp)
				{
					*zbp = z;
					*d = col;
				}
				z1 += stpZ;
				++d;
				++zbp;
				--wd;
			}
		}*/
	}
}

//---------------------------------------------------------
void _bmp_pfz16loop( UB *mapp, long ymin )
{
long	wd = ACT->wd;

	while ( _bmp_pr_y < ymin )
	{
	long		xl,xr;
	POE_Vert_t	*l, *r;

		if ( _bmp_pr_lrun.screen[0] <= _bmp_pr_rrun.screen[0] )
		{	l = &_bmp_pr_lrun;
			r = &_bmp_pr_rrun;
		}
		else
		{	l = &_bmp_pr_rrun;
			r = &_bmp_pr_lrun;
		}
		xl = l->screen[0] + 32768 >> 16;
		xr = r->screen[0] - 32768 >> 16;

		_zBufP = _bmp_zbufferP + ACT->wd*(_bmp_pr_y>>4) + xl;
		_bmp_memfillZ( mapp+xl, _bmp_pf16_col, l->sz, r->sz, xr-xl+1 );
		LINCPR(sz);	RINCPR(sz);

		LINCPR(screen[0]);	RINCPR(screen[0]);
		mapp += wd;
		_bmp_pr_y += 16;
	}
}

//==========================================
//           GOURAUD SHADING
//==========================================
static const UB *_pg16_colorsP;
void _bmp_pg16setupI( const POE_PolyI_t *polyP )
{
	_pg16_colorsP=polyP->c;
}
//------------------------------------------
static inline void pg16_fillRainbow( UB *mapP, long c1, long c2, long wd )
{
long	stpC;
UB		*endp;

	stpC = (c2-c1) / wd;
	endp = mapP + wd;
	while ( mapP < endp )
	{
		*mapP++ = c1 >> 16; 
		c1 += stpC;
	}
}

//------------------------------------------
void _bmp_pg16slope(POE_Vert_t *run, POE_Vert_t *delta, long i1, long i2)
{
const POE_Vert_t *p1=_bmp_pr_fvertsP[i1], *p2=_bmp_pr_fvertsP[i2];
long	coe;

	delta->screen[0] = p2->screen[0] - p1->screen[0] << 16;
	delta->c = (long)_pg16_colorsP[i2] - _pg16_colorsP[i1] << 16;

	if ( delta->screen[1] = p2->screen[1] - p1->screen[1] )
	{
		delta->screen[0] /= delta->screen[1];
		delta->c <<= 4;	delta->c /= delta->screen[1];
	}

	coe = _bmp_pr_y - p1->screen[1]+8;
	run->screen[0] = (p1->screen[0] << 12) + (delta->screen[0] * coe >> 4);
	run->c = (_pg16_colorsP[i1] << 16) + (delta->c * coe >> 4);
}
//---------------------------------------------------------
void _bmp_pg16loop( UB *mapp, long ymin )
{
long	wd = ACT->wd;

	while ( _bmp_pr_y < ymin )
	{
	long		xl,xr;
	POE_Vert_t	*l, *r;

		if ( _bmp_pr_lrun.screen[0] <= _bmp_pr_rrun.screen[0] )
		{	l = &_bmp_pr_lrun;
			r = &_bmp_pr_rrun;
		}
		else
		{	l = &_bmp_pr_rrun;
			r = &_bmp_pr_lrun;
		}
		xl = l->screen[0] + 32768 >> 16;
		xr = r->screen[0] - 32768 >> 16;

		long	spawd = xr-xl+1;
		if ( spawd > 0 )	pg16_fillRainbow( mapp+xl, l->c, r->c, spawd );
		LINCPR(c);	RINCPR(c);

		LINCPR(screen[0]);	RINCPR(screen[0]);
		mapp += wd;
		_bmp_pr_y += 16;
	}
}

//==========================================
//       GOURAUD SHADING + Z-BUFFER
//==========================================
static inline void pgz16_fillRainbow(UB *d, long c1, long c2, long z1, long z2, long wd)
{
	if ( wd > 0 )
	{
	long	stpC;
	long	stpZ;
	UB		*endp;
	US		*zbp = _zBufP;
	US		*zbendp;
	US		z;

		stpC = (c2-c1) / wd;
		stpZ = (z2-z1) / wd;
		zbendp = zbp + wd;
		do{
			z = z1 >> 14;
			if (z > *zbp)
			{
				*zbp = z;
				*d = c1 >> 16;
			}
			z1 += stpZ; ++d;
			c1 += stpC;
		}while( ++zbp < zbendp );
	}
}

//------------------------------------------
void _bmp_pgz16slope(POE_Vert_t *run, POE_Vert_t *delta, long i1, long i2)
{
const POE_Vert_t *p1=_bmp_pr_fvertsP[i1], *p2=_bmp_pr_fvertsP[i2];
long	coe;

	delta->screen[0] = p2->screen[0] - p1->screen[0] << 16;
	delta->c = (long)_pg16_colorsP[i2] - _pg16_colorsP[i1] << 16;
	delta->sz = p2->sz - p1->sz;

	if ( delta->screen[1] = p2->screen[1] - p1->screen[1] )
	{
		delta->screen[0] /= delta->screen[1];
		delta->c <<= 4;	delta->c /= delta->screen[1];
		delta->sz <<= 4; delta->sz /= delta->screen[1];
	}

	coe = _bmp_pr_y - p1->screen[1]+8;
	run->screen[0] = (p1->screen[0] << 12) + (delta->screen[0] * coe >> 4);
	run->c = (_pg16_colorsP[i1] << 16) + (delta->c * coe >> 4);
	run->sz = p1->sz + (delta->sz * coe >> 4);
}


//---------------------------------------------------------
void _bmp_pgz16loop( UB *mapp, long ymin )
{
long	wd = ACT->wd;

	while ( _bmp_pr_y < ymin )
	{
	long		xl,xr;
	POE_Vert_t	*l, *r;

		if ( _bmp_pr_lrun.screen[0] <= _bmp_pr_rrun.screen[0] )
		{	l = &_bmp_pr_lrun;
			r = &_bmp_pr_rrun;
		}
		else
		{	l = &_bmp_pr_rrun;
			r = &_bmp_pr_lrun;
		}
		xl = l->screen[0] + 32768 >> 16;
		xr = r->screen[0] - 32768 >> 16;

		_zBufP = _bmp_zbufferP + ACT->wd*(_bmp_pr_y>>4) + xl;
		pgz16_fillRainbow( mapp+xl, l->c, r->c, l->sz, r->sz, xr-xl+1 );
		LINCPR(c);	RINCPR(c);
		LINCPR(sz);	RINCPR(sz);
		LINCPR(screen[0]);	RINCPR(screen[0]);
		mapp += wd;
		_bmp_pr_y += 16;
	}
}


//==========================================
//              PHONG SHADING
//==========================================
static float		_pp16_nshad1, _pp16_light[3];
static UB			_pp16_color;
static const float	*_pp16_lsrcP;

//==========================================
static inline void pp16_fill( UB *mapP, const float *n1, const float *n2, long wd )
{
float	n[3], dn[3];
float	mag, alpha, alpha_stp;
float	aco_sta, aco_end, aco_stp;
float	asi_sta, asi_end, asi_stp;
float	z1,z2, nl1[3], nl2[3];

	vec3_mul( nl1, n1, 1. );//_pp16_light );
	vec3_mul( nl2, n2, 1. );//_pp16_light );

	asi_sta = atan2( nl1[1], nl1[0] );
	asi_end = atan2( nl2[1], nl2[0] );

	float	rho;
	if NOT( rho = vec3_mag( nl1 ) )	rho = .01;
	aco_sta = acos( nl1[2] / rho );

	if NOT( rho = vec3_mag( nl2 ) )	rho = .01;
	aco_end = acos( nl2[2] / rho );

	aco_stp = (aco_end - aco_sta) / wd;
	asi_stp = (asi_end - asi_sta) / wd;

	do{
		alpha = cos( aco_sta ) * sin( asi_sta );
		aco_sta += aco_stp;
		asi_sta += asi_stp;

		if ( alpha <= 0. )
			*mapP = _pp16_color;
		else
			*mapP = _pp16_color + (UB)(alpha * _pp16_nshad1);

		++mapP;
	}while(--wd);
}
#if 0

//=============================================
extern long fxsqr(long l);
#pragma aux fxsqr =             \
        "imul eax"              \
        "add eax, 8000h"        \
        "adc edx, 0"            \
        "shrd eax, edx, 16"     \
        parm caller [eax]       \
        value [eax]             \
        modify [eax edx];

//=============================================
long TabA[]={
6949350,
4913933,
3474675,
2456966,
1737338,
1228483,
868669,
614242,
434334,
307121,
217167,
153560,
108584,
76780,
54292,
38390,
27146,
19195,
13573,
9598,
6786,
4799,
3393,
2399,
1697,
1200,
848,
600,
424,
300,
212,
150,
106 };

long TabB[]={
152,
215,
304,
430,
608,
860,
1217,
1721,
2433,
3441,
4867,
6883,
9734,
13765,
19467,
27531,
38934,
55061,
77868,
110122,
155736,
220244,
311472,
440488,
622945,
880977,
1245890,
1761954,
2491779,
3523908,
4983558,
7047816,
9967116 };

extern long		_tr_sqr[];
//, _tr_bsr[];
//extern long		*_tr_sqrP;
extern UB		_tr_sqrt[];

extern long fxvec3len(long *a);
#pragma aux fxvec3len = \
		"mov eax, dword ptr [esi]"\
		"imul eax"\
		"mov ebx,eax"\
		"mov ecx,edx"\
\
		"mov eax, dword ptr [esi+4]"\
		"imul eax"\
		"add ebx,eax"\
		"adc ecx,edx"\
\
		"mov eax, dword ptr [esi+8]"\
		"imul eax"\
		"add eax,ebx"\
		"adc edx,ecx"\
		"shrd eax,edx,16"\
		"jle zero"\
\
		"mov ebx,[_tr_bsr+eax*4]"\
		"mul dword ptr [TabA+ebx*4]"\
		"shrd eax,edx,10h"\
		"add eax, [TabB+ebx*4]"\
"zero:"\
        parm caller [esi] \
        value [eax]       \
        modify [eax ebx ecx edx esi];

//=============================================
static inline void pp16_fill( UB *mapP, const float *n1, const float *n2, long wd )
{
long	n[3], dn[3];
long	mag, alpha, alpha_stp;
float	dnf[3];

	vec3_sub( dnf, n2, n1 );
	alpha	  = vec3_dot( n1, _pp16_light ) * 65536.;
	vec3_mul( dnf, dnf, 65536./wd );
	alpha_stp = vec3_dot( dnf, _pp16_light );
	vec3_equ( dn, dnf );

	n[0] = n1[0] * 65536.;
	n[1] = n1[1] * 65536.;
	n[2] = n1[2] * 65536.;
	do{
		if ( *((UL *)&alpha) & 0x80000000 )
			*mapP = _pp16_color;
		else
		{
			if NOT( mag = fxvec3len( n ) )
				mag = 1;
			*mapP = _pp16_color + (alpha / mag);
		}

		++mapP;
		n[0] += dn[0];
		n[1] += dn[1];
		n[2] += dn[2];
		alpha += alpha_stp;
	}while(--wd);
}
#endif

//--------------------------------
static inline void pp16_calcSideDataPoly(long y, POE_Vert_t *run, POE_Vert_t *delta,
										 const POE_Vert_t *p1, const POE_Vert_t *p2,
										 long i1, long i2)
{
long	coe;

	delta->screen[0] = p2->screen[0] - p1->screen[0] << 16;
	vec3_sub( delta->nor, p2->nor, p1->nor );
	//vec3_mul( delta->nor, delta->nor, _pp16_light );
	if ( delta->screen[1] = p2->screen[1] - p1->screen[1] )
	{
		delta->screen[0] /= delta->screen[1];
		vec3_mul( delta->nor, delta->nor, 16./delta->screen[1] );
	}

	coe = y - p1->screen[1]+8;
	run->screen[0] = (p1->screen[0] << 12) + (delta->screen[0] * coe >> 4);

	float	coef = coe * (1./16.);
	run->nor[0] = p1->nor[0] + delta->nor[0] * coef;
	run->nor[1] = p1->nor[1] + delta->nor[1] * coef;
	run->nor[2] = p1->nor[2] + delta->nor[2] * coef;
}
#define PP16_ROUT \
long	wd = xr-xl+1;\
	if ( wd > 0 )	pp16_fill( mapP+xl, l->nor, r->nor, wd );\
	LINC(nor[0]);	RINC(nor[0]);\
	LINC(nor[1]);	RINC(nor[1]);\
	LINC(nor[2]);	RINC(nor[2]);

POLYROUTINE(
	BMP_PolyPhong16( const POE_PolyI_t *polyP, const float *lightSrcP, US nShad, US usrCols ),,
	{
		_pp16_lsrcP=lightSrcP;
		_pp16_nshad1=nShad-1;
		_pp16_color = usrCols + polyP->c[0] * nShad;
		vec3_mul( _pp16_light, _pp16_lsrcP, _pp16_nshad1 );
	},
	pp16_calcSideDataPoly, PP16_ROUT );


	/*UB	doPut;

		bufP = (float *)_zBufP + (xl >> 2);
		stpZ = (z2-z1) / (float)wd;

		doPut = 1; //z1 < *bufP ;
		switch ( wd & 3 ) // ~
		{
		case 3:	if ( doPut ){ *d = col; *bufP = z1; } z1 += stpZ; ++d; ++xl;
		case 2:	if ( doPut ){ *d = col; *bufP = z1; } z1 += stpZ; ++d; ++xl;
		case 1:	if ( doPut ){ *d = col; *bufP = z1; } z1 += stpZ; ++d; ++xl;
				break;
		}

		bufP = (float *)_zBufP + (xl >> 2);
		if ( wd >>= 2 )
		{
			stpZ4 = stpZ*4.;
			for (; wd; z1 += stpZ4, d += 4, ++bufP, --wd)
			{
				//*(UL *)d = col;
				if ( z1 < *bufP )
				{
					*(UL *)d = col;
					*bufP = z1;
				}
			}
		}*/

/*extern long fxsqrt(long a);
#pragma aux fxsqrt = \
		"bsr ebx,eax"\
		"jz zero"\
		"mul dword ptr [TabA+ebx*4]"\
		"shrd eax,edx,10h"\
		"add eax, [TabB+ebx*4]"\
		"zero:"\
        parm caller [eax] \
        value [eax]       \
        modify [eax ebx edx esi];
*/

