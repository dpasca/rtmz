//==============================================
// BMP.HPP - raster graphics stuff
// Copyright (C) by Davide Pasca 1995
//==============================================

#ifndef	__BMP_HPP__
#define __BMP_HPP__

#include <string.h>
#include "EXTTYPES.HPP"

extern BitMap		*_BMP_curBitMapP;
#define ACT			_BMP_curBitMapP
#define BMP_MAXPUSH	32

#define	BMP_TF_SUBCH	128
#define	BMP_TF_CHKCH	129

long BMP_TFSet(long fontID);
long BMP_TFGet(void);
void BMP_TFPushSet(long fontID);
void BMP_TFPush(void);
void BMP_TFPop(void);
void BMP_TFDispose(long fontID);
long BMP_TFLoad(const char *fnameP);
long BMP_TFHeight(void);
long BMP_TextWidth(const char *txtP);
void BMP_CharWriteCP( char ch, short x, short y, UB col );
void BMP_TextWriteCP( const char *strP, short x, short y, UB col );
void BMP_TextPrintFCP(short x, short y, UB col, const char *fmtP, ... );

#define	BMP_PIXELPUT(X,Y,C)	*(ACT->offsPP[Y]+X)=C
#define	BMP_PIXELGET(X,Y)	*(ACT->offsPP[Y]+X)

inline UB BMP_PointIsInMap( short x, short y ){ return (US)x < (US)ACT->wd && (US)y < (US)ACT->he; }
inline void BMP_PixelPut( US x, US y, UB col ){ BMP_PIXELPUT( x, y, col ); }
inline void BMP_PixelPutCP( short x, short y, UB col ){ if (BMP_PointIsInMap( x, y)) BMP_PIXELPUT( x, y, col ); }
inline UB BMP_PixelGet( US x, US y ){ return BMP_PIXELGET( x, y ); }

inline void BMP_Set(BitMap *bmP){_BMP_curBitMapP=bmP;}
inline BitMap *BMP_Get(void){return _BMP_curBitMapP;}

void BMP_Clear(void);

void BMP_PushSet(BitMap *bmP);
void BMP_Push(void);
void BMP_Pop(void);
void BMP_OffsPPCreate( BitMap *bmP );
void BMP_Free( BitMap *bmP );
long BMP_Alloc( BitMap *bmP, US wd, US he, UB dp, UB *memP );
void BMP_LineH( US x1, US x2, US y, UB col );
void BMP_LineV( US x, US y1, US y2, UB col );
void BMP_RectFill( US p1x, US p1y, US p2x, US p2y, UB col );
void BMP_RectFillCP( short p1x, short p1y, short p2x, short p2y, UB col );
void BMP_BitBltMskColCP(short xd, short yd, short sdx, short sdy,
						const BitMap *sbmP, short xs, short ys, UB col );
void BMP_BitBltCP(long xd, long yd, long sdx, long sdy,
				  const BitMap *sbmP, long xs, long ys );
void BMP_BitBlt0CP(long xd, long yd, long sdx, long sdy,
				   const BitMap *sbmP, long xs, long ys );
void BMP_BitMapPutCP( const BitMap *sbmP, short dx, short dy );
void BMP_BitMapPut0CP( const BitMap *sbmP, short dx, short dy );
void BMP_BitMapStretchCP( const BitMap *sbmP, short dx, short dy, short wd, short he );
void BMP_BitMapStretch0CP( const BitMap *sbmP, short dx, short dy, short wd, short he );
void BMP_BitMapStretchDestLUT0CP( const BitMap *sbmP, short dx, short dy, short wd, short he, const UB *lutp );

void BMP_ApplyLUT( BitMap *bmP, const UB *lut );

#define	bpush	BMP_PushSet
#define	bpop	BMP_Pop

#undef ACT
#endif
