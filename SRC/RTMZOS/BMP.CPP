//==============================================
// BMP.CPP - raster graphics stuff
// Copyright (C) by Davide Pasca 1995
//==============================================

#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include "BMP.HPP"
#include "PCX.HPP"

BitMap			*_BMP_curBitMapP;
//static UB		**_BMP_curOffsPP;
static BitMap	*_BMP_stackBitMapP[BMP_MAXPUSH];
static long		_BMP_stackIdx;
#define ACT		_BMP_curBitMapP
//#define OFFS	_BMP_curOffsPP

//=====================================
void BMP_Clear(void)
{
	memset( ACT->memP, 0, ACT->wd * ACT->he );
}

//=====================================
void BMP_ApplyLUT( BitMap *bmP, const UB *lut )
{
UB		*memP;
long	i;

	memP = bmP->memP;
	for(i = bmP->wd * bmP->he; i; --i, ++memP)
		*memP = lut[ *memP ];
}

//=================================
#define	BMP_TF_STACH	32
#define	BMP_TF_ENDCH	129
#define	BMP_TF_NCH		(BMP_TF_ENDCH-BMP_TF_STACH+1)

typedef struct
{
	BitMap	bm;
	US		cwd, che;
} BMP_Font_t;
static BMP_Font_t	*_BMP_curFontP;
static BMP_Font_t	*_BMP_stackFontP[BMP_MAXPUSH];
static long			_BMP_stackFontIdx;
#define FONT		_BMP_curFontP

//=================================
void BMP_TFPushSet(long fontID)
{
	if ( _BMP_stackFontIdx < BMP_MAXPUSH )
	{
		_BMP_stackFontP[_BMP_stackFontIdx++] = FONT;
		FONT = (BMP_Font_t *)fontID;
	}
}
//=================================
void BMP_TFPush(void)
{
	if ( _BMP_stackFontIdx < BMP_MAXPUSH )
		_BMP_stackFontP[_BMP_stackFontIdx++] = FONT;
}
//=================================
void BMP_TFPop(void)
{
	if ( _BMP_stackFontIdx > 0 )
		FONT = _BMP_stackFontP[--_BMP_stackFontIdx];
}
//=================================
void BMP_TFDispose(long fontID)
{
	if NOT( fontID )	return;
	BMP_Free( &((BMP_Font_t *)fontID)->bm );
	
	if ( (BMP_Font_t *)fontID == FONT )
		FONT = 0;

	FREEIF( (BMP_Font_t *)fontID );
}

//=================================
long BMP_TFLoad(const char *fnameP)
{
BMP_Font_t	*fontP;

	if ( NEW(fontP,1) )
	{
		memset( fontP, 0, sizeof(BMP_Font_t) );
		if NOT( PCX_ReadBitMap( fnameP, &fontP->bm, 0 ) )
		{
			fontP->cwd = fontP->bm.wd / BMP_TF_NCH;
			fontP->che = fontP->bm.he;
			return (long)fontP;
		}
		free( fontP );
	}
	return 0;
}
//=================================
long BMP_TFSet(long fontID){long old=(long)FONT; FONT = (BMP_Font_t *)fontID; return old;}
long BMP_TFGet(void){ return (long)FONT; }

//=================================
long BMP_TFHeight(void){ return FONT->che; }
//=================================
long BMP_TextWidth(const char *txtP)
{
	return strlen(txtP)*FONT->cwd;
}

//-----------------------------------
void BMP_CharWriteCP( char ch, short x, short y, UB col )
{
short	dwd;
US		cwd,che;

	if NOT( FONT )	return;
	dwd = ACT->wd;

	if ( x+FONT->cwd >= dwd )	return;
	cwd = FONT->cwd;
	che = FONT->che;

	ch -= BMP_TF_STACH;
	CLAMP(ch,0,BMP_TF_NCH-1);

	BMP_BitBltMskColCP( x,y,cwd-1,che, &FONT->bm,ch*cwd,0, col );
}

//-----------------------------------
void BMP_TextWriteCP( const char *strP, short x, short y, UB col )
{
short	dwd;
US		cwd,che;

	if NOT( FONT )	return;
	dwd = ACT->wd;

	if ( x+FONT->cwd >= dwd )	return;
	cwd = FONT->cwd;
	che = FONT->che;
	while (*strP)
	{
	short	ch = (short)*strP++ - BMP_TF_STACH;
		CLAMP(ch,0,BMP_TF_NCH-1);
		BMP_BitBltMskColCP( x,y,cwd-1,che, &FONT->bm,ch*cwd,0, col );
		if ( (x += cwd) >= dwd )	return;
	}
}

//-----------------------------------
void BMP_TextPrintFCP(short x, short y, UB col, const char *fmtP, ... )
{
char	buf[2048];
va_list	arg;

	va_start( arg, fmtP );
	vsprintf( buf, fmtP, arg );
	va_end( arg );
	BMP_TextWriteCP( buf, x, y, col );
}

//=================================
void BMP_PushSet(BitMap *bmP)
{
	if ( _BMP_stackIdx < BMP_MAXPUSH )
	{
		_BMP_stackBitMapP[_BMP_stackIdx++] = _BMP_curBitMapP;
		ACT = bmP;
		//OFFS = bmP->offsPP;
	}
}
//=================================
void BMP_Push(void)
{
	if ( _BMP_stackIdx < BMP_MAXPUSH )
		_BMP_stackBitMapP[_BMP_stackIdx++] = _BMP_curBitMapP;
}
//=================================
void BMP_Pop(void)
{
	if ( _BMP_stackIdx > 0 )
	{
		ACT = _BMP_stackBitMapP[--_BMP_stackIdx];
		//OFFS = ACT->offsPP;
	}
}

//===========================================
void BMP_Free( BitMap *bmP )
{
	if ( bmP )
	{
		SAFE_FREE( bmP->memP );
		SAFE_FREE( bmP->offsPP );
	}
}
//===========================================
void BMP_OffsPPCreate( BitMap *bmP )
{
short	wd = bmP->wd;
short	he = bmP->he;
UB		*plP = bmP->memP + (long)he * wd;

	while( he--)
	{
		plP -= wd;
		bmP->offsPP[he] = plP;
	}
}
//===========================================
long BMP_Alloc( BitMap *bmP, US wd, US he, UB dp, UB *memP )
{
	if ( memP )
		bmP->memP = memP;
	else
		if NOT( bmP->memP = (UB*)malloc((long)wd*(long)he) )             
			return -1;

	if NOT( bmP->offsPP = (UB **)malloc(he * sizeof(UB*)) )
	{
		free( bmP->memP );
		bmP->memP = 0;
		return -2;
	}

	bmP->wd = wd;
	bmP->he = he;
	bmP->dp = dp;
	bmP->Flags = 0;

	BMP_OffsPPCreate( bmP );
	return 0;
}

//=================================
void BMP_LineH( US x1, US x2, US y, UB col )
{
US	t;
	if ( x2 < x1 )	SWAP( x1, x2, t );
	memset( ACT->offsPP[y] + x1, col, x2-x1+1 );
}

//=================================
void BMP_LineV( US x, US y1, US y2, UB col )
{
UB	*p;
US	wd;

	if ( y2 < y1 )
	{
	US	t=y2;
		y2 = y1;
		y1 = t;
	}
	if NOT( y2 = y2-y1+1 )	return;
	p = ACT->offsPP[y1] + x;
	wd = ACT->wd;
	while(y2--)
	{
		*p = col;
		p += wd;
	}
}

//===========================================
void BMP_RectFill( US p1x, US p1y, US p2x, US p2y, UB col )
{
UB		*pP;
short	mod;

	if ( p2x < p1x )	swap( p2x, p1x );
	if ( p2y < p1y )	swap( p2y, p1y );

	p2y -= p1y-1;
	p2x -= p1x-1;
	pP = ACT->offsPP[p1y] + p1x;
	mod = ACT->wd;
	while( p2y-- )
	{
		memset( pP, col, p2x );
		pP += mod;
	}
}

//===========================================
void BMP_RectFillCP( short p1x, short p1y, short p2x, short p2y, UB col )
{
UB	*pP;

	if ( p2x < p1x )	swap( p2x, p1x );
	if ( p2y < p1y )	swap( p2y, p1y );

	short mx = ACT->wd-1;
	short my = ACT->he-1;

	if ( p2x < 0 ) return;
	else
	if ( p2x > mx ) p2x=mx;

	if ( p2y < 0 ) return;
	else
	if ( p2y > my ) p2y=my;


	if ( p1x < 0 ) p1x=0;
	else
	if ( p1x > mx ) return;

	if ( p1y < 0 ) p1y=0;
	else
	if ( p1y > my ) return;

	if ( (p2y -= p1y-1) <= 0)	return;
	if ( (p2x -= p1x-1) <= 0)	return;

	pP = ACT->offsPP[p1y] + p1x;
	register short mod = ACT->wd;
	while( p2y-- )
	{
		memset( pP, col, p2x );
		pP += mod;
	}
}

//===========================================
void BMP_RectFillLUTCP( short p1x, short p1y, short p2x, short p2y, UB *lutp )
{
UB	*pP;
short	mx,my;
register short mod;
US		t;

	if ( p2x < p1x )	SWAP( p2x, p1x, t );
	if ( p2y < p1y )	SWAP( p2y, p1y, t );

	mx = ACT->wd-1;
	my = ACT->he-1;

	if ( p2x < 0 ) return;
	else
	if ( p2x > mx ) p2x=mx;

	if ( p2y < 0 ) return;
	else
	if ( p2y > my ) p2y=my;


	if ( p1x < 0 ) p1x=0;
	else
	if ( p1x > mx ) return;

	if ( p1y < 0 ) p1y=0;
	else
	if ( p1y > my ) return;

	if ( (p2y -= p1y-1) <= 0)	return;
	if ( (p2x -= p1x-1) <= 0)	return;

	pP = ACT->offsPP[p1y] + p1x;
	mod = ACT->wd - p2x;
	while( p2y-- )
	{
		for ( t = p2x; t; --t, ++pP )
			*pP = lutp[ *pP ];
		pP += mod;
	}
}

//=================================
static inline void memsetmsk( UB *dp, UB *sp, UB col, UL wd )
{
	while( wd-- )
	{
		if ( *sp )	*dp = col;
		++sp;
		++dp;
	}
}

//===========================================
void BMP_BitBltCP(long xd, long yd, long sdx, long sdy,
				  const BitMap *sbmP, long xs, long ys )
{
UB		*sp,*dp;

	US	dwd, swd = sbmP->wd;
	sp = sbmP->offsPP[ys] + xs;

	if ( yd < 0 ) 				// PRIMA LA "Y" PER NON SVALIDARE sdx
		if ( (sdy += yd) <= 0 )
			return;
		else
		{
			sp -= yd * swd;
			yd = 0;
		}

	if ( xd < 0 )
		if ( (sdx += xd) <= 0 )
			return;
		else
		{
			sp -= xd;
			xd = 0;
		}

	if ( xd + sdx >= ACT->wd )
		if ( (sdx = ACT->wd - xd) <= 0 )
			return;

	if ( yd + sdy >= ACT->he )
		if ( (sdy = ACT->he - yd) <= 0 )
			return;

	dp = ACT->offsPP[yd] + xd;

	dwd = ACT->wd;
	if ( sdx <= 64 )
		for(; sdy; --sdy, dp += dwd, sp += swd)
			memcpytiny( dp, sp, sdx );
	else
		for(; sdy; --sdy, dp += dwd, sp += swd)
			memcpy( dp, sp, sdx );
}

//===========================================
void BMP_BitBlt0CP(long xd, long yd, long sdx, long sdy,
				   const BitMap *sbmP, long xs, long ys )
{
UB		*sp,*dp;
long	i;

	US	dwd, swd = sbmP->wd;
	sp = sbmP->offsPP[ys] + xs;

	if ( yd < 0 ) 				// PRIMA LA "Y" PER NON SVALIDARE sdx
		if ( (sdy += yd) <= 0 )
			return;
		else
		{
			sp -= yd * swd;
			yd = 0;
		}

	if ( xd < 0 )
		if ( (sdx += xd) <= 0 )
			return;
		else
		{
			sp -= xd;
			xd = 0;
		}

	if ( xd + sdx >= ACT->wd )
		if ( (sdx = ACT->wd - xd) <= 0 )
			return;

	if ( yd + sdy >= ACT->he )
		if ( (sdy = ACT->he - yd) <= 0 )
			return;

	dp = ACT->offsPP[yd] + xd;

	dwd = ACT->wd - sdx;
	swd -= sdx;
	for(; sdy; --sdy, dp += dwd, sp += swd)
		for (i=sdx; i; --i, ++dp, ++sp )
			if ( *sp )
				*dp = *sp;
}

//===========================================
void BMP_BitBltMskColCP(short xd, short yd, short sdx, short sdy,
						const BitMap *sbmP, short xs, short ys, UB col )
{
UB		*sp,*dp;

	US	swd = sbmP->wd;
	sp = sbmP->offsPP[ys] + xs;

	if ( yd < 0 ) 				// PRIMA LA "Y" PER NON SVALIDARE sdx
		if ( (sdy += yd) <= 0 )
			return;
		else
		{
			sp -= yd * swd;
			yd = 0;
		}

	if ( xd < 0 )
		if ( (sdx += xd) <= 0 )
			return;
		else
		{
			sp -= xd;
			xd = 0;
		}

	if ( xd + sdx >= ACT->wd )
		if ( (sdx = ACT->wd - xd) <= 0 )
			return;

	if ( yd + sdy >= ACT->he )
		if ( (sdy = ACT->he - yd) <= 0 )
			return;

	dp = ACT->offsPP[yd] + xd;

	US	dwd = ACT->wd;
	for(; sdy; --sdy)
	{
		memsetmsk( dp, sp, col, sdx );
		dp += dwd;
		sp += swd;
	}
}

//=================================
void BMP_BitMapPutCP( const BitMap *sbmP, short dx, short dy )
{
UB		*sp,*dp;
long	sdx,sdy, ddx,ddy;

	sdx = sbmP->wd;
	sdy = sbmP->he;

	ddx = ACT->wd;
	ddy = ACT->he;

	sp = (UB *)sbmP->memP;

	if ( dy < 0 ) 				// PRIMA LA "Y" PER NON SVALIDARE sdx
		if ( (sdy += dy) <= 0 )
			return;
		else
		{
			sp -= dy * sdx;
			dy = 0;
		}

	if ( dx < 0 )
		if ( (sdx += dx) <= 0 )
			return;
		else
		{
			sp -= dx;
			dx = 0;
		}


	if ( dx + sdx >= ddx )
		if ( (sdx = ddx - dx) <= 0 )
			return;

	if ( dy + sdy >= ddy )
		if ( (sdy = ddy - dy) <= 0 )
			return;

	dp = ACT->offsPP[dy] + dx;

	ddy = sbmP->wd;
	ddx = ACT->wd;
	for(; sdy; --sdy)
	{
		memcpy( dp, sp, sdx );
		sp += ddy;
		dp += ddx;
	}
}

//=================================
void BMP_BitMapStretchCP( const BitMap *sbmP, short dx, short dy, short wd, short he )
{
long		xs=0,ys=0, stp_x, stp_y;
long		ddx, ddy;
long		dmod;
UB			**soffP, *dp, *sp2, xflip=0;
UL			xxs;
long		i, ysr,oysr;
static UL	idx[801];

	if (wd==0 || he==0)	return;
	if ( wd < 0 )
	{
		wd = -wd;
		xflip = 1;
	}
	stp_x = ((long)sbmP->wd<<16) / wd;
	stp_y = ((long)sbmP->he<<16) / he;

	if ( dy < 0 ) 				// PRIMA LA "Y" PER NON SVALIDARE wd
		if ( (he += dy) <= 0 )
			return;
		else
		{
			ys -= dy*stp_y;
			dy = 0;
		}

	if ( dx < 0 )
		if ( (wd += dx) <= 0 )
			return;
		else
		{
			xs -= dx*stp_x;
			dx = 0;
		}

	ddx = ACT->wd;
	if (ddx > 800 ) ddx=800;
	ddy = ACT->he;

	if ( dx + wd >= ddx )
		if ( (wd = ddx - dx) <= 0 )
			return;

	if ( dy + he >= ddy )
		if ( (he = ddy - dy) <= 0 )
			return;

	soffP = sbmP->offsPP;
	dp = ACT->offsPP[dy] + dx;

	dmod = ACT->wd - wd;


	oysr = ys >> 16;
	sp2 = sbmP->offsPP[ ys >> 16 ];

	if ( xflip )
	{
		for(i=wd, xxs=xs; i; --i,xxs += stp_x)
			*dp++ = sp2[ idx[i] = xxs >> 16 ];
		/*xxs = (wd-1) * stp_x + xs;
		for(i=wd; i; --i, xxs -= stp_x)
			*dp++ = sp2[ idx[i] = xxs >> 16 ];*/
	}
	else
		for(i=wd, xxs=xs; i; --i,xxs += stp_x)
			*dp++ = sp2[ idx[i] = xxs >> 16 ];
	dp += dmod;
	ys += stp_y;

	if ( sbmP->he >= he )
	{
		for(; --he; ys +=stp_y)  // orig HE >= new HE
		{
			sp2 = soffP[ ys >> 16 ];
			for(i=wd; i; --i)
				*dp++ = sp2[ idx[i] ];
			dp += dmod;
		}
	}
	else
	{
		for(oysr=-1; --he; ys +=stp_y)  // orig HE <= new HE (allungato)
		{
			ysr = ys >> 16;
			if ( ysr == oysr )
			{
				memcpy( dp, dp-dmod-wd, wd );
				dp += wd;
			}
			else
			{
				sp2 = soffP[ ysr ];

				for(i=wd; i; --i)
					*dp++ = sp2[ idx[i] ];
				oysr = ysr;
			}
			dp += dmod;
		}
	}
}

//=================================
void BMP_BitMapStretch0CP( const BitMap *sbmP, short dx, short dy, short wd, short he )
{
long		xs=0,ys=0, stp_x, stp_y;
long		ddx, ddy;
long		dmod;
UB			**soffP, *dp, *sp2, c;
UL			xxs;
long		i;
static UL	idx[801];

	if (wd==0 || he==0)	return;
	stp_x = ((long)sbmP->wd<<16) / wd;
	stp_y = ((long)sbmP->he<<16) / he;

	if ( dy < 0 ) 				// PRIMA LA "Y" PER NON SVALIDARE wd
		if ( (he += dy) <= 0 )
			return;
		else
		{
			ys -= dy*stp_y;
			dy = 0;
		}

	if ( dx < 0 )
		if ( (wd += dx) <= 0 )
			return;
		else
		{
			xs -= dx*stp_x;
			dx = 0;
		}

	ddx = ACT->wd;
	if (ddx > 800 ) ddx=800;
	ddy = ACT->he;

	if ( dx + wd >= ddx )
		if ( (wd = ddx - dx) <= 0 )
			return;

	if ( dy + he >= ddy )
		if ( (he = ddy - dy) <= 0 )
			return;

	soffP = sbmP->offsPP;
	dp = ACT->offsPP[dy] + dx;

	dmod = ACT->wd - wd;


	sp2 = sbmP->offsPP[ ys >> 16 ];

	for(i=wd, xxs=xs; i; --i,xxs += stp_x, ++dp)
	{
		c = sp2[ idx[i] = xxs >> 16 ];
		if ( c )	*dp = c;
	}
	dp += dmod;
	ys += stp_y;

	for(; --he; ys +=stp_y)  // orig HE <= new HE (allungato)
	{
		sp2 = soffP[ ys >> 16 ];
		for(i=wd; i; --i, ++dp)
			if ( c = sp2[ idx[i] ] )
				*dp = c;
		dp += dmod;
	}
}

//=================================
void BMP_BitMapPut0CP( const BitMap *sbmP, short dx, short dy )
{
UB		*sp,*dp;
short	sdx,sdy, ddx,ddy;

	sdx = sbmP->wd;
	sdy = sbmP->he;

	ddx = ACT->wd;
	ddy = ACT->he;

	sp = (UB *)sbmP->memP;

	if ( dy < 0 ) 				// PRIMA LA "Y" PER NON SVALIDARE sdx
		if ( (sdy += dy) <= 0 )
			return;
		else
		{
			sp -= dy * sdx;
			dy = 0;
		}

	if ( dx < 0 )
		if ( (sdx += dx) <= 0 )
			return;
		else
		{
			sp -= dx;
			dx = 0;
		}


	if ( dx + sdx >= ddx )
		if ( (sdx = ddx - dx) <= 0 )
			return;

	if ( dy + sdy >= ddy )
		if ( (sdy = ddy - dy) <= 0 )
			return;

	dp = ACT->offsPP[dy] + dx;

	ddy = sbmP->wd - sdx;
	ddx = ACT->wd - sdx;
	for(; sdy; --sdy)
	{
		for(short x=sdx; x; --x, ++dp)
		{
		UB	col;

			if ( col = *sp++ )
				*dp = col;
		}
		sp += ddy;
		dp += ddx;
	}
}

