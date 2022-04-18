//==============================================
// BMP_POTX.CPP - texture-mapped polygons
// Copyright (C) by Davide Pasca 1995
//==============================================

#include <stdio.h>
#include <conio.h>
#include <limits.h>
#include <float.h>
#include "BMP.HPP"
#include "BMP_POTX.HPP"

#define ACT			_BMP_curBitMapP

#define	LINCPR(__X__)	_bmp_pr_lrun.__X__ += _bmp_pr_ldelta.__X__
#define	RINCPR(__X__)	_bmp_pr_rrun.__X__ += _bmp_pr_rdelta.__X__

extern long				*_bmp_zbufferP;
extern POE_Vert_t		_bmp_pr_lrun, _bmp_pr_ldelta, _bmp_pr_rrun, _bmp_pr_rdelta;
extern long				_bmp_pr_y;
extern const POE_Vert_t	*_bmp_pr_fvertsP[POE_VERTBUFSIZ];

//=======================================
#define  MAGIC  (((65536. * 65536. * 16) + (65536.*.5)) * 65536.)
static inline long float2int( float d )
{
  double dtemp = MAGIC + d;
  return (*(long *)&dtemp) - 0x80000000;
}

//==========================================
//           TEXTURE MAPPING 2D
//==========================================
static const short	*_txPosP;
static const UB		**_tmOffsPP;
//------------------------------------------
void _bmp_pt2d16setupI(const POE_PolyI_t *polyP, const BitMap *tmP)
{
	_tmOffsPP = (const UB **)tmP->offsPP;
	_txPosP = (const short *)polyP->txPos;
}

//------------------------------------------
void _bmp_pt2d16slope(POE_Vert_t *run, POE_Vert_t *delta, long i1, long i2)
{
const POE_Vert_t *p1=_bmp_pr_fvertsP[i1], *p2=_bmp_pr_fvertsP[i2];

	delta->screen[0] = p2->screen[0] - p1->screen[0] << 16;
	delta->u = _txPosP[i2*2+0] - (long)_txPosP[i1*2+0] << 12;
	delta->v = _txPosP[i2*2+1] - (long)_txPosP[i1*2+1] << 12;
	if ( delta->screen[1] = p2->screen[1] - p1->screen[1] )
	{
		delta->screen[0] /= delta->screen[1];
		delta->u = (delta->u << 4) / delta->screen[1];
		delta->v = (delta->v << 4) / delta->screen[1];
	}

	long coe = _bmp_pr_y - p1->screen[1]+8;
	run->screen[0] = (p1->screen[0] << 12) + (delta->screen[0] * coe >> 4);
	run->u = ((long)_txPosP[i1*2+0] << 12) + (delta->u * coe >> 4);
	run->v = ((long)_txPosP[i1*2+1] << 12) + (delta->v * coe >> 4);
}

//---------------------------------------------------------
void _bmp_pt2d16loop( UB *mapp, long ymin )
{
long			wd = ACT->wd;
const UB		**tmOffsPP=_tmOffsPP;

	while ( _bmp_pr_y < ymin )
	{
	long		xl,xr;
	POE_Vert_t	*l, *r;

		if ( _bmp_pr_lrun.screen[0] <= _bmp_pr_rrun.screen[0] )
		{	l = &_bmp_pr_lrun;	r = &_bmp_pr_rrun;	}
		else
		{	l = &_bmp_pr_rrun;	r = &_bmp_pr_lrun;	}

		xl = l->screen[0] + 32768 >> 16;
		xr = r->screen[0] - 32768 >> 16;

		long	spawd = xr-xl+1;
		if ( spawd > 0 )
		{
		long	u,v, delTx,delTy;
		UB		*runmapp, *endp;

			u = l->u;
			v = l->v;
			delTx = (r->u - u) / spawd;
			delTy = (r->v - v) / spawd;
			runmapp = mapp+xl;
			endp = runmapp + spawd;
			/*if ( spawd > 16 )
			{
			const UB	*myendp;
			
				myendp = endp;
				while ( (long)runmapp & 3 )
				{	*runmapp++ = tmOffsPP[v>>16][u>>16];
					u += delTx; v += delTy;
					--myendp;
				}
				myendp = (const UB *)( (long)myendp & ~3 );
				while ( runmapp < myendp )
				{
				register long	c;

					c  = tmOffsPP[v>>16][u>>16];		u += delTx; v += delTy;
					c |= tmOffsPP[v>>16][u>>16] << 8;	u += delTx; v += delTy;
					c |= tmOffsPP[v>>16][u>>16] << 16;	u += delTx; v += delTy;
					c |= tmOffsPP[v>>16][u>>16] << 24;	u += delTx; v += delTy;
					*(long *)runmapp = c;
					runmapp += 4;
				}
			}*/

			while ( runmapp < endp )
			{	*runmapp++ = tmOffsPP[v>>16][u>>16];
				u += delTx; v += delTy;
			}
		}
		LINCPR(u); RINCPR(u);
		LINCPR(v); RINCPR(v);
		LINCPR(screen[0]);	RINCPR(screen[0]);
		mapp += wd;
		_bmp_pr_y += 16;
	}
}

//==========================================
//         TEXTURE MAPPING 2D LUT
//==========================================
static const UB	*_txlutP;
void _bmp_pt2dlut16setupI(const POE_PolyI_t *polyP, const BitMap *tmP, const UB *lutP)
{
	_tmOffsPP = (const UB **)tmP->offsPP;
	_txPosP = (const short *)polyP->txPos;
	_txlutP = lutP;
}
//---------------------------------------------------------
void _bmp_pt2dlut16loop( UB *mapp, long ymin )
{
long		wd = ACT->wd;
const UB	*txlutP=_txlutP;
const UB	**tmOffsPP=_tmOffsPP;

	while ( _bmp_pr_y < ymin )
	{
	long		xl,xr;
	POE_Vert_t	*l, *r;

		if ( _bmp_pr_lrun.screen[0] <= _bmp_pr_rrun.screen[0] )
		{	l = &_bmp_pr_lrun;	r = &_bmp_pr_rrun;	}
		else
		{	l = &_bmp_pr_rrun;	r = &_bmp_pr_lrun;	}

		xl = l->screen[0] + 32768 >> 16;
		xr = r->screen[0] - 32768 >> 16;

		long	spawd = xr-xl+1;
		if ( spawd > 0 )
		{
		long	u,v, delTx,delTy;
		UB		*runmapp, *endp;

			u = l->u;
			v = l->v;
			delTx = (r->u - u) / spawd;
			delTy = (r->v - v) / spawd;
			runmapp = mapp+xl;
			endp = runmapp + spawd;
			while ( runmapp < endp )
			{	*runmapp++ = txlutP[ tmOffsPP[v>>16][u>>16] ];
				u += delTx; v += delTy;
			}
		}
		LINCPR(u); RINCPR(u);
		LINCPR(v); RINCPR(v);
		LINCPR(screen[0]);	RINCPR(screen[0]);
		mapp += wd;
		_bmp_pr_y += 16;
	}
}

//==========================================
//         TEXTURE MAPPING CHROME
//==========================================
static float	_mapwd, _maphe, _mapwdh, _mapheh;
void _bmp_ptchrome16setupI(const POE_PolyI_t *polyP, const BitMap *tmP)
{
	_mapwd = tmP->wd << 16;
	_maphe = tmP->he << 16;
	_mapwdh = _mapwd / 2.;
	_mapheh = _maphe / 2.;
	_tmOffsPP = (const UB **)tmP->offsPP;
}
//------------------------------------------
void _bmp_ptchrome16slope(POE_Vert_t *run, POE_Vert_t *delta, long i1, long i2)
{
const POE_Vert_t *p1=_bmp_pr_fvertsP[i1], *p2=_bmp_pr_fvertsP[i2];
long  u1, u2, v1, v2;
//float	z;

	//if NOT( z = p1->nor[2] )	z = .1;
	u1 = (p1->nor[0]+1.) * _mapwdh;
	v1 = (p1->nor[1]+1.) * _mapheh;
	//if NOT( z = p2->nor[2] )	z = .1;
	u2 = (p2->nor[0]+1.) * _mapwdh;
	v2 = (p2->nor[1]+1.) * _mapheh;
	CLAMP( u1, 0, _mapwd-16 );
	CLAMP( u2, 0, _mapwd-16 );
	CLAMP( v1, 0, _maphe-16 );
	CLAMP( v2, 0, _maphe-16 );

	delta->screen[0] = p2->screen[0] - p1->screen[0] << 16;
	delta->u = u2 - u1;
	delta->v = v2 - v1;
	if ( delta->screen[1] = p2->screen[1] - p1->screen[1] )
	{
		delta->screen[0] /= delta->screen[1];
		delta->u = (delta->u << 4) / delta->screen[1];
		delta->v = (delta->v << 4) / delta->screen[1];
	}

	long coe = _bmp_pr_y - p1->screen[1]+8;
	run->screen[0] = (p1->screen[0] << 12) + (delta->screen[0] * coe >> 4);
	run->u = u1 + (delta->u * coe >> 4);
	run->v = v1 + (delta->v * coe >> 4);
}


//==========================================
//         TEXTURE MAPPING 3D LUT
//==========================================
void _bmp_pt3d16slope(POE_Vert_t *run, POE_Vert_t *delta, long i1, long i2)
{
const POE_Vert_t *p1=_bmp_pr_fvertsP[i1], *p2=_bmp_pr_fvertsP[i2];
long  u1, u2, v1, v2;

	float w1, w2;

	if ( w1 = p1->eye[3] ) w1 = 1. / w1;
	if ( w2 = p2->eye[3] ) w2 = 1. / w2;

	delta->screen[0] = p2->screen[0] - p1->screen[0] << 16;
	run->eye[0] = _txPosP[i1*2+0] * w1;
	run->eye[1] = _txPosP[i1*2+1] * w1;
	if ( delta->screen[1] = p2->screen[1] - p1->screen[1] )
	{
		delta->screen[0] /= delta->screen[1];
		float	hef = 1. / delta->screen[1];
		delta->eye[0] = (_txPosP[i2*2+0]*w2 - run->eye[0]) * hef;
		delta->eye[1] = (_txPosP[i2*2+1]*w2 - run->eye[1]) * hef;
		delta->eye[3] = (w2 - w1)*16. * hef;
	}

	long coe = _bmp_pr_y - p1->screen[1]+8;
	run->screen[0] = (p1->screen[0] << 12) + (delta->screen[0] * coe >> 4);

	float	coef = coe * (1./16.);
	run->eye[0] = run->eye[0] * (1./16.) + delta->eye[0] * coef;
	run->eye[1] = run->eye[1] * (1./16.) + delta->eye[1] * coef;
	run->eye[3] = w1 + delta->eye[3] * coef;
}

//---------------------------------------------------------
void _bmp_pt3dlut16loop( UB *mapp, long ymin )
{
long		wd = ACT->wd;
const UB	*txlutP=_txlutP;
const UB	**tmOffsPP=_tmOffsPP;

	while ( _bmp_pr_y < ymin )
	{
	long		xl,xr;
	POE_Vert_t	*l, *r;

		if ( _bmp_pr_lrun.screen[0] <= _bmp_pr_rrun.screen[0] )
		{	l = &_bmp_pr_lrun;	r = &_bmp_pr_rrun;	}
		else
		{	l = &_bmp_pr_rrun;	r = &_bmp_pr_lrun;	}

		xl = l->screen[0] + 32768 >> 16;
		xr = r->screen[0] - 32768 >> 16;

		long	spawd = xr-xl+1;
		if ( spawd > 0 )
		{
		float	x,y,w, dx,dy,dw, dx16,dy16,dw16, iwd = 1. / spawd, carryf, carry;
		float	iw;
		UB		*runmapp, *endp;
		long	u, v, du, dv;

			dx = (r->eye[0] - (x = l->eye[0])) * iwd;
			dy = (r->eye[1] - (y = l->eye[1])) * iwd;
			dw = (r->eye[3] - (w = l->eye[3])) * iwd;
			dw *= 1./65536.; w *= 1./65536.;
			
			dx16 = dx * 16.;
			dy16 = dy * 16.;
			dw16 = dw * 16.;
			runmapp = mapp + xl;
			endp = mapp + xl + (spawd & ~15);
			while ( runmapp < endp )
			{
			long	i;

				iw = 1. / w;
				u = float2int( x * iw );
				v = float2int( y * iw );
				x += dx16; y += dy16; w += dw16;
				iw = 1. / w;
				du = float2int(x * iw) - u >> 4;
				dv = float2int(y * iw) - v >> 4;

				for (i=4; i; --i)
				{
					runmapp[0] = txlutP[ tmOffsPP[ v >> 16 ][ u >> 16 ] ]; u += du; v += dv;
					runmapp[1] = txlutP[ tmOffsPP[ v >> 16 ][ u >> 16 ] ]; u += du; v += dv;
					runmapp[2] = txlutP[ tmOffsPP[ v >> 16 ][ u >> 16 ] ]; u += du; v += dv;
					runmapp[3] = txlutP[ tmOffsPP[ v >> 16 ][ u >> 16 ] ]; u += du; v += dv;
					runmapp += 4;
				}
			}

			if ( spawd & 15 )
			{
				carryf = spawd & 15;
				iw = 1. / w;
				u = x * iw;
				v = y * iw;
				x += dx * carryf; y += dy * carryf; w += dw * carryf;
				
				iw = 1. / w;
				carryf = 1. / carryf;
				du = float2int(((x * iw) - u) * carryf);
				dv = float2int(((y * iw) - v) * carryf);
				endp = mapp + xl + spawd;
				while ( runmapp < endp )
				{
					*runmapp++ = txlutP[ tmOffsPP[ v >> 16 ][ u >> 16 ] ];
					u += du;
					v += dv;
				}
			}
		}
		LINCPR(eye[0]);  RINCPR(eye[0]);
		LINCPR(eye[1]);  RINCPR(eye[1]);
		LINCPR(eye[3]);  RINCPR(eye[3]);
		LINCPR(screen[0]);	RINCPR(screen[0]);
		mapp += wd;
		_bmp_pr_y += 16;
	}
}
//------------------------------------------
void BMP_PolyTextureLUT3D16( const POE_PolyI_t *polyP, const BitMap *tmP, const UB *lut )
{
float  				wmin, wmax, w;
const POE_Vert_t	*vertsP = *polyP->vertH;
const long			*vertIdxP = polyP->vertIdx;

	wmin = FLT_MAX;	wmax = FLT_MIN;
	for (long i=polyP->nVerts; i; --i)
	{
		w = vertsP[ *vertIdxP++ ].eye[3];
		if ( w < wmin )	wmin = w;
		if ( w > wmax )	wmax = w;
	}

	if ( wmin == 0. || wmax / wmin < 1.12 ) // ABS() ?
	{
		BMP_PolyTextureLUT2D16( polyP, tmP, lut );
		return;
	}

//extern void _bmp_pt2dlut16setupI(const POE_PolyI_t *, const BitMap *, const UB *);
//extern void _bmp_pt3d16slope(POE_Vert_t *, POE_Vert_t *, long, long );
//extern void _bmp_pt3dlut16loop(UB *, long);

	_bmp_pt2dlut16setupI( polyP, tmP, lut );
	BMP_PolyRoutineI( polyP, 0, _bmp_pt3d16slope, _bmp_pt3dlut16loop );

}


#if 0
//==========================================
//           TEXTURE MAPPING 3D
//==========================================
/*extern long fxmul(long l1, long l2);
#pragma aux fxmul =             \
        "imul edx"              \
        "shrd eax, edx, 8"     \
        parm caller [eax] [edx] \
        value [eax]             \
        modify [eax edx];
//	      "add eax, 8000h"
//        "adc edx, 0"


static long fixmul( long a, long b )
{
	return (float)a * (float)b / 1024.;
}*/

//static float	_wcoe;
static inline void pt3d16_calcSideDataPoly(long y, POE_Vert_t *run, POE_Vert_t *delta,
										   const POE_Vert_t *p1, const POE_Vert_t *p2,
										   long i1, long i2)
{
	delta->screen[0] = p2->screen[0] - p1->screen[0] << 16;

	float w1, w2;

	if ( w1 = p1->eye[3] ) w1 = 1. / w1;
	if ( w2 = p2->eye[3] ) w2 = 1. / w2;

	run->eye[0] = _txPosP[i1*2+0] * w1;
	run->eye[1] = _txPosP[i1*2+1] * w1;
	if ( delta->screen[1] = p2->screen[1] - p1->screen[1] )
	{
		delta->screen[0] /= delta->screen[1];
		float	hef = 1. / delta->screen[1];
		delta->eye[0] = (_txPosP[i2*2+0]*w2 - run->eye[0]) * hef;
		delta->eye[1] = (_txPosP[i2*2+1]*w2 - run->eye[1]) * hef;
		delta->eye[3] = (w2 - w1)*16. * hef;
	}

	long coe = y - p1->screen[1]+8;
	run->screen[0] = (p1->screen[0] << 12) + (delta->screen[0] * coe >> 4);

	float	coef = coe * (1./16.);
	run->eye[0] = run->eye[0] * (1./16.) + delta->eye[0] * coef;
	run->eye[1] = run->eye[1] * (1./16.) + delta->eye[1] * coef;
	run->eye[3] = w1 + delta->eye[3] * coef;
}
/*
	//float	w1f, w2f;
	long	w1=1, w2=1;
	w1 = p1->sz;
	w2 = p2->sz;
//	if ( w1f = p1->eye[3] * _wcoe )	w1 = (1L<<20) / w1f;
//	if ( w2f = p2->eye[3] * _wcoe )	w2 = (1L<<20) / w2f;

	run->u = fixmul( _txPosP[i1*2+0], w1 );
	run->v = fixmul( _txPosP[i1*2+1], w1 );
	if ( delta->screen[1] = p2->screen[1] - p1->screen[1] )
	{
		delta->screen[0] /= delta->screen[1];
		delta->u = (fixmul(_txPosP[i2*2+0], w2) - run->u) / delta->screen[1];
		delta->v = (fixmul(_txPosP[i2*2+1], w2) - run->v) / delta->screen[1];
		delta->c = (w2 - w1 >> 4) / delta->screen[1];
	}

	long coe = y - p1->screen[1]+8;
	run->screen[0] = (p1->screen[0] << 12) + (delta->screen[0] * coe >> 4);

	run->u = run->u + delta->u * coe >> 4;
	run->v = run->v + delta->v * coe >> 4;
	run->c  = w1 + (delta->c * coe >> 4);
*/
//------------------------------------------
#define PTLUT3D16ROUT \
long	wd = xr-xl+1;\
	if ( wd > 0 )\
	{\
	UB		*runMapP;\
\
	float	u0,u1,u2, v0,v1,v2, w0,w2;\
	float	a1,a2, b1,b2, dx;\
	long	UD1,UD2,VD1,VD2, v,u;\
\
	u0 = l->eye[0]; u2 = r->eye[0];\
	v0 = l->eye[1]; v2 = r->eye[1];\
	w0 = l->eye[3]; w2 = r->eye[3];\
\
    dx = 1.0 / wd;\
\
    u1 = (u0 + u2) / (w0 + w2);\
    v1 = (v0 + v2) / (w0 + w2);\
    u0 = u0 / w0;               v0 = v0 / w0;\
    u2 = u2 / w2;               v2 = v2 / w2;\
\
	a2 = 2.*(u0 - 2.*u1 + u2);  b2 = 2.*(v0 - 2.*v1 + v2);\
	a1 = (u2 - u0 - a2) * dx;   b1 = (v2 - v0 - b2) * dx;\
	dx *= dx;\
	a2 *= dx;                   b2 *= dx;\
\
    UD1 = 262144. * (a1 + a2);   VD1 = 262144. * (b1 + b2);\
    UD2 = 262144. * 2. * a2;     VD2 = 262144. * 2. * b2;\
    u = 262144. * u0;\
    v = 262144. * v0;\
\
	runMapP = mapP+xl;\
	for(; wd > 0; --wd)\
	{	*runMapP++ = lut[ tmOffsPP[ v >> 18 ][ u >> 18 ] ];\
        u += UD1;\
        v += VD1;\
        UD1 += UD2;\
        VD1 += VD2;\
	}\
	}\
	LINC(eye[0]);  RINC(eye[0]);\
	LINC(eye[1]);  RINC(eye[1]);\
	LINC(eye[3]);  RINC(eye[3]);
// lut[ tmOffsPP[ v >> 18 ][ u >> 18 ] ];
POLYROUTINE(
	BMP_PolyTextureLUT3D16( const POE_PolyI_t *polyP, const BitMap *tmP, const UB *lut ),
	{
	float		wmin;
	float		wmax;
	float		w;
	const long	*idxp;

		wmin = FLT_MAX;	wmax = FLT_MIN;
		idxp = vertIdxP;
		for (i=nVerts; i; --i)
		{
			w = vertsP[ *idxp++ ].eye[3];
			if ( w < wmin )	wmin = w;
			if ( w > wmax )	wmax = w;
		}
		if NOT( wmin )	wmin = .000001;
		if ( wmax / wmin < 1.2 ) // ABS() ?
		{
			BMP_PolyTextureLUT2D16( polyP, tmP, lut );
			return;
		}
	},
	UB	**tmOffsPP;
	{
		tmOffsPP=tmP->offsPP;
		_txPosP = (const short *)polyP->txPos;

		//_wcoe = 1. / ((fvertsP[0]->eye[3] + fvertsP[1]->eye[3] + fvertsP[2]->eye[3])/3.);
	}, pt3d16_calcSideDataPoly, PTLUT3D16ROUT );

/*
    a0 = u0;                            b0 = v0;\
    a1 = (-3*u0 + 4*u1 - u2) * dx;      b1 = (-3*v0 + 4*v1 - v2) * dx;\
    a2 = 2*(u0 - 2*u1 + u2) * dx*dx;    b2 = 2*(v0 - 2*v1 + v2) * dx*dx;\
//------------------------------------------
#define PTLUT3D16ROUT \
long	wd = xr-xl+1;\
	if ( wd > 0 )\
	{\
	long	x,y,w, dx,dy,dw, iwd;\
	UB		*runMapP;\
	dx = ((r->u) - (x = l->u)) / wd;\
	dy = ((r->v) - (y = l->v)) / wd;\
	dw = (r->c - (w = l->c)) / wd;\
	runMapP = mapP+xl;\
	for(; wd > 0; --wd)\
	{	if (w) *runMapP++ = lut[ tmOffsPP[ y/w ][ x/w ] ];\
		x += dx; y += dy; w += dw;\
	}\
	}\
	LINC(u);  RINC(u);\
	LINC(v);  RINC(v);\
	LINC(c);  RINC(c);



	if ( wd > 0 )\
	{\
	float	x,y,w, dx,dy,dw, iwd = 1. / wd;\
	UB		*runMapP;\
	dx = (r->eye[0] - (x = l->eye[0])) * iwd;\
	dy = (r->eye[1] - (y = l->eye[1])) * iwd;\
	dw = (r->eye[3] - (w = l->eye[3])) * iwd;\
	runMapP = mapP+xl;\
	for(; wd > 0; --wd)\
	{	if (w) *runMapP++ = lut[ tmOffsPP[ (long)(y/w) ][ (long)(x/w) ] ];\
		x += dx; y += dy; w += dw;\
	}\
	}\
	LINC(eye[0]);  RINC(eye[0]);\
	LINC(eye[1]);  RINC(eye[1]);\
	LINC(eye[3]);  RINC(eye[3]);

	if ( wd > 0 )\
	{\
	float	x,y,z, dx,dy,dz;\
	float	iwd = 1. / wd;\
	UB		*runMapP;\
\
	x = l->eye[0];\
	y = l->eye[1];\
	z = l->eye[3];\
	dx = (r->eye[0] - l->eye[0]) * iwd;\
	dy = (r->eye[1] - l->eye[1]) * iwd;\
	dz = (r->eye[3] - l->eye[3]) * iwd;\
	runMapP = mapP+xl;\
	for(; wd > 0; --wd)\
	{\
		*runMapP++ = lut[ tmOffsPP[ (long)(y/z) ][ (long)(x/z) ] ];\
		x += dx; y += dy; z += dz;\
	}\
	}\
*/
#endif
