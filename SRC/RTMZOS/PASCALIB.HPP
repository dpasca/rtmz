//==============================================
// PASCALIB.HPP -
// Copyright (C) Davide Pasca 1995
//==============================================

#ifndef __PASCALIB_HPP__
#define __PASCALIB_HPP__

#include <STDARG.H>
#include "EXTTYPES.HPP"
/*
#include "CVEC3.HPP"

#define	P3DA_MAKEPTSTRANS_FLG	1
typedef struct
{
	long	npix;
	long	*vecP;
	UB		*colP;
	long	*transVecP;
}Pix3DArray;

void FreePix3DArray( Pix3DArray *pixP );
long MakePix3DArray( const char *fnameP, Pix3DArray *pixP, long sx, long sy, UL flags );
*/

//extern "C" void setmemMsk(void *addr, void *mask, UL len, char val);

char *strcpymaxsize(char *dst, const char *src, UL max);
void RandomizeShort( short rd );
short RandShort(void);

long timer( void );
void QSortLX( long *aP, long *idxP, long right );
void QSortLX( float *aP, long right );
void QSortLX( long *aP, long right );

#endif
