//==============================================
// PCX.CPP
// Copyright (C) Davide Pasca 1995-97
//
// TABS=4
//==============================================
#include <stdio.h>
#include <malloc.h>
#include <dos.h>
#include <mem.h>

//#include "LINKFILE.HPP"
#include "PCX.HPP"
#include "VGAPACK.HPP"
#include "BMP.HPP"

typedef struct
{
	char manufacturer;
	char version;
	char encoding;
	char bits_per_pixel;
	short xmin,ymin;
	short xmax,ymax;
	short hres;
	short vres;
	char palette[48];
	char reserved;
	char colour_planes;
	short bytes_per_line;
	short palette_type;
	char filler[58];
}PCXHEAD;

//====================================
static US readLine(FILE *fp, UB *p, US wd, US max)
{
US n=0,c,i;

	do{
		c=fgetc(fp) & 0xff;	// get a key byte
		if((c & 0xc0) == 0xc0)	// if it's a run of bytes field
		{
			i=c & 0x3f;			// and off the high bits
			c=fgetc(fp);		// get the run byte
			while(i--)
			{
				if ( n < wd )
					p[n]=c;	// run the byte
				++n;
			}
		}
		else
		{
			if ( n < wd )
				p[n]=c;			// else just store it
			++n;
		}
	}while(n < max);

	return n;
}

//------------------------------------
#define EXITERR(E)	{err=E; goto exitErr##E;}

long PCX_Read( const char *fnameP, UB **memPP, US *wdP, US *heP, UB *dpP, UB *palP )
{
PCXHEAD	head;
US 		bits;
short	wd,he;
UB		*memP;
FILE	*fp;
short	err,i;
UB		*p;

	if NOT( fp = fopen( fnameP, "rb" ) )
		return( 1 );

	if ( FREAD( &head,sizeof(PCXHEAD),fp) )
		EXITERR( 2 );

	if (head.bits_per_pixel == 1)
		bits = head.colour_planes;
	else
		bits = head.bits_per_pixel;

	if (bits !=8)
		EXITERR( 3 );

	 /* check to make sure it's a picture */
	if (head.manufacturer != 0x0a || head.version != 5 )
		EXITERR( 4 );

	/* Find the palette */
	if NOT( fseek(fp,-769L,SEEK_END) )
	{
		if ( fgetc(fp) != 0x0c )
			EXITERR( 5 );

		if ( palP )
		{
			if ( FREAD( palP, 768, fp ) )
				EXITERR( 5 );
		}
		else
			if ( fseek(fp,768L,SEEK_SET) )
				EXITERR( 5 );
	}
	else
		EXITERR( 5 );

	fseek(fp,128L,SEEK_SET);

	he = (head.ymax-head.ymin)+1;
	wd = (head.xmax-head.xmin)+1; //head.bytes_per_line;

	if ( memPP )
	{
		if ( !wd || !he )				EXITERR( 6 );
		if NOT( NEW( memP, wd*he ) )	EXITERR( 6 );
		p = memP;
		for(i=he; i; --i, p += wd)
			readLine(fp,p,wd,head.bytes_per_line);
	}
	fclose(fp);

	if ( wdP )		*wdP = wd;
	if ( heP )		*heP = he;
	if ( dpP )		*dpP = bits;
	if ( memPP )	*memPP = memP;
	return(0);

exitErr7:
	free( memP );
exitErr6:
exitErr5:
exitErr4:
exitErr3:
exitErr2:
	fclose(fp);

	if ( wdP )		*wdP = wd;
	if ( heP )		*heP = he;
	if ( dpP )		*dpP = bits;
	if ( memPP )	*memPP = memP;
	return( -err );
}

//===========================
long PCX_ReadBitMap( const char *nameP, BitMap *bmP, UB *palP )
{
	if ( PCX_Read( nameP, &bmP->memP, &bmP->wd, &bmP->he, &bmP->dp, palP ) )
		return -1;

	if ( BMP_Alloc( bmP, bmP->wd, bmP->he, bmP->dp, bmP->memP ) )
	{
		SAFE_FREE( bmP->memP );
		return -2;
	}

	return 0;
}

//=======================================
long PCX_ReadBitMaps( const char *fnamep, BitMap *bmlistp, long bobwd, long bobhe, long nframes, UB *palP )
{
BitMap	tmap;
long	x, y, i;
BitMap *bmp;

	if ( bobwd<=0 || bobhe<=0 || nframes<=0 || PCX_ReadBitMap( fnamep, &tmap, palP ) )
		return -1;

	bmp = bmlistp;
	for(i=0; i < nframes; ++i, ++bmp)
	{
		if ( BMP_Alloc( bmp, bobwd, bobhe, 8, 0 ) )
		{
			for(--i, --bmp; i>=0; --bmp)
				BMP_Free( bmp );
			BMP_Free( &tmap );

			return -2;
		}
	}

	x = y = 0;
	bmp = bmlistp;
	BMP_Push();
		for(y=0; y < tmap.he; y += bobhe)
			for(x=0; x < tmap.wd; x += bobwd)
			{
				if ( --nframes < 0 )	goto pazzo;
				BMP_Set( bmp++ );
				BMP_BitBltCP( 0,0, bobwd, bobhe, &tmap, x, y );
			}
pazzo:
	BMP_Pop();
	BMP_Free( &tmap );

	return 0;
} 

//========================================
long PCX_ReadPalette( const char *fnameP, UB *palP )
{
PCXHEAD	head;
US 		bits;
FILE	*fp;

	if NOT( fp = fopen( fnameP, "rb" ) )
		return -1;

	if ( FREAD( &head,sizeof(PCXHEAD),fp) )
		goto errExit;

	if (head.bits_per_pixel == 1)
		bits = head.colour_planes;
	else
		bits = head.bits_per_pixel;

	if (bits !=8 || head.manufacturer != 0x0a || head.version != 5)
		goto errExit;

	if NOT( fseek(fp,-769L,SEEK_END) )	// Find the palette
	{
		if ( fgetc(fp) != 0x0c )
			goto errExit;

		if ( FREAD( palP, 768, fp ) )
			goto errExit;
	}
	else
		goto errExit;

	fclose( fp );
	return 0;

errExit:
	fclose( fp );
	return -1;
}

/*******
static BitMap *bima;

//------------------------------------
static BitMap	*_read_pp_mapP;

inline static void read_putpixel(short x, short y, UB col)
{
	if NOT( _read_pp_mapP->Flags & BM_MODE_X_FLG )
		Cnk_PutPixel( _read_pp_mapP, x, y, col );
	else
		VGAX_PutPixel( x, y, col );
}
***********/
