//==============================================
// PASCALIB.CPP
// Copyright (C) Davide Pasca 1995-97
//
// TABS=4
//==============================================
#include <dos.h>
#include <stdlib.h>
#include <stdio.h>
#include <TIME.H>
#include <mem.h>
#include <malloc.H>
#include <CONIO.H>

#include "PASCALIB.HPP"
#include "BMP.HPP"
#include "PCX.HPP"

//=======================================
char *strcpymaxsize(char *dst, const char *src, UL max)
{
char	*oldDst=dst;

	if (max)
		while(1)
		{
			if NOT(--max)
			{
				*dst = 0;
				break;
			}
	
			if NOT( *dst++ = *src++ )
				break;
		}

	return oldDst;
}

//=======================================
static short _RS_seed;
void RandomizeShort( short rd )
{
	_RS_seed = rd;
}

//---------------------------------------
short RandShort(void)
{
	_RS_seed = (_RS_seed+56791) * 13579;
	return _RS_seed & 0x7fff;
}

//======================================
long timer( void )
{
	//return( (long)(clock() * 1000.0 / CLK_TCK) );
	return 0;
}

//======================================
static void QSortLX_rec( long *left, long *right )
{
long		*j;
static long	*i, piv ,x;

	piv = *(left + (((UL)right - (UL)left >> 3 ) & 0xfffffffe) );

	i = left;
	j = right;
	for(;;)
	{
		while ( i[0] < piv )	i += 2;
		while ( j[0] > piv )	j -= 2;

		if ( i >= j )
		{
			if ( left < j )		QSortLX_rec( left, j );
			j += 2;
			if ( j < right )	QSortLX_rec( j, right );
			return;
		}

		SWAP( j[0], i[0], x );
		SWAP( j[1], i[1], x );
		i += 2;
		j -= 2;
	}
}
//----------------------------------------
void QSortLX( long *aP, long right )
{
long	*endP;

	endP = aP+(right-1)*2;
	if ( aP < endP )
		QSortLX_rec( aP, endP );
}

//======================================
static void QSortLX_rec( float *left, float *right )
{
float			*j;
static float	*i, piv ,x;

	piv = *(left + (((UL)right - (UL)left >> 3 ) & 0xfffffffe) );

	i = left;
	j = right;
	for(;;)
	{
		while ( i[0] < piv )	i += 2;
		while ( j[0] > piv )	j -= 2;

		if ( i >= j )
		{
			if ( left < j )		QSortLX_rec( left, j );
			j += 2;
			if ( j < right )	QSortLX_rec( j, right );
			return;
		}

		SWAP( j[0], i[0], x );
		SWAP( j[1], i[1], x );
		i += 2;
		j -= 2;
	}
}

//----------------------------------------
void QSortLX( float *aP, long right )
{
float	*endP;

	endP = aP+(right-1)*2;
	if ( aP < endP )
		QSortLX_rec( aP, endP );
}

//=============================================
static long	*_qx_aP;
static long	*_qx_idxP;

//#define QLX_GET_L(a,i)	(*(long *)(a + i))

static void QSortLX_rec( long left, long right )
{
long					j;
static long				i;
static long				piv,x;

	if ( left >= right )	return;

	piv = _qx_aP[left];

	i = left + 1;
	j = right;
	for(;;)
	{
		while ( _qx_aP[i] < piv )	++i;
		while ( _qx_aP[j] > piv )	--j;
		if ( i < j )
		{
			SWAP( _qx_aP[j], _qx_aP[i], x );
			SWAP( _qx_idxP[j], _qx_idxP[i], x );
			++i;
			--j;
		}
		else
			break;
	}

	SWAP( _qx_aP[j], _qx_aP[left], x );
	SWAP( _qx_idxP[j], _qx_idxP[left], x );

	QSortLX_rec( left, j-1 );
	QSortLX_rec( j+1,  right );
}

/*--------------------------------------*/
void QSortLX( long *aP, long *idxP, long right )
{
	_qx_aP = aP;
	_qx_idxP = idxP;
	QSortLX_rec( 0, right-1 );
}

//=============================
/*
void FreePix3DArray( Pix3DArray *pixP )
{
	SAFE_FREE( pixP->vecP );
	SAFE_FREE( pixP->colP );
	SAFE_FREE( pixP->transVecP );
}

//----------------------------------------
long MakePix3DArray( const char *fnameP, Pix3DArray *pixP, long sx, long sy, UL flags )
{
BitMap	bm;
long	npix;
long   	*vecP, *transVecP=0, vec[3];
UB		*colP, *scolP, col;
short	x,y,hwd,hhe;


	memset( pixP, 0, sizeof(Pix3DArray) );

	if ( PCX_Read(fnameP, &bm.memP, &bm.wd, &bm.he, 0, 0) )
		return -1;

	scolP = bm.memP;
	npix = (long)bm.wd * bm.he;
	long i=npix;
	while( i-- )
		if NOT( *scolP++ )
			--npix;

	if NOT(vecP=(long *)malloc(npix*3*sizeof(*vecP)))	goto err1;
	if NOT(colP=(UB*)malloc(npix*sizeof(*colP)))	goto err2;

	if ( P3DA_MAKEPTSTRANS_FLG & flags )
		if NOT(transVecP=(long *)malloc(npix*3*sizeof(*transVecP)))		goto err3;

	vec[1] = -bm.he >> 1;
	vec[2] = 0;

	hwd = bm.wd >> 1;
	hhe = bm.he >> 1;

	pixP->vecP = vecP;
	pixP->transVecP = transVecP;
	pixP->colP = colP;
	pixP->npix = npix;

	scolP = bm.memP;
	for(y=0; y < bm.he; ++y)
	{
		vec[1] = -(hhe-y)*sy;
		for(x=0; x < bm.wd; ++x)
		{
			if ( col = *scolP++ )
			{
				vec[0] = -(hwd-x)*sx;
				vec3_equ( vecP += 3, vec );
				*colP++ = col;
			}
		}
	}

	BMP_Free( &bm );
	return 0;

err3:
	free( colP );

err2:
	free( vecP );
err1:
	BMP_Free( &bm );
	return -1;
}
*/
