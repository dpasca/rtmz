//==============================================
// TDST_HP.CPP -
// Copyright (C) Davide Pasca 1995
//==============================================

#include "EXTTYPES.HPP"
#include "POLYENG.HPP"
#include "TDSTUFF.HPP"
#include "CLP.HPP"
#include "TDST_HP.HPP"
#include "COLORS.HPP"
#include "MATELIB.HPP"
#include "CVEC3.HPP"
#include "BMP.HPP"
#include "BMP_POLY.HPP"

#define ACT		_W3D_actWorldP
static UB		_zBufOn;

//==================================
/*extern void memcpy4(void *dP, const void *sP, long size );
#pragma aux memcpy4 =   \
"	shr		ecx,2"\
"	rep		movsd"\
parm caller [edi][esi][ecx]\
modify [edi esi ecx];
*/

/*		extern "C" {
			extern int *asm_PolyStart,*asm_PolyEnd;
			extern unsigned char *asm_Texture,*asm_Video;
			extern int asm_MAXX,asm_MAXY,asm_ROWSZ;
			void asm_LinTexturedPoly(void);
		};
*/

//=================================================
void W3D_HandlePoly( const POE_PolyI_t *polyIP )
{
	if ( polyIP->nVerts < 3 )
		return;

	if ( _zBufOn = ACT->zBufferFlg )
		BMP_ZBufferSet( (UL *)ACT->zBufferP );

	if ( polyIP->clipCode )
		polyIP = CLP_PolyI4D( polyIP );

/*	if ( polyIP->nVerts < 3 )
	{
		//Cnk_LinePoly16( &ACT->bitMap );
	}
	else*/
	{
	long	pflags = polyIP->flags;

		if ( pflags & POE_FLG_GSHADE )
		{
			if ( _zBufOn )	BMP_PolyGouraudZ16( polyIP );
			else			BMP_PolyGouraud16( polyIP );
		}
		else
		if ( pflags & POE_FLG_PSHADE )
		{
			//Cnk_polyIPhongSetData( ACT->ActSPaletteP->nShades, ((Obj3D *)polyIP->userLong)->traLight );
			BMP_PolyPhong16( polyIP,
							 ((O3D_Object_t *)polyIP->userLong)->traLight,
							 ACT->ActSPaletteP->nShades,
							 ACT->ActSPaletteP->userColors );
		}
		else
		if ( pflags & POE_FLG_TEXIS3D )
		{
		MLB_Material_t	*matP;

			matP = MLB_MaterialGetPtr( polyIP->materialID );
			BMP_PolyTextureLUT3D16( polyIP,
								 &matP->textureP->origMap,
								 &MLB_LibraryGetPtr()->levPalP->index[ 256*(31-polyIP->c[0]) ] );
		}
		else
		if ( pflags & POE_FLG_TEXCHROME )
		{
		MLB_Material_t	*matP;

			matP = MLB_MaterialGetPtr( polyIP->materialID );
			BMP_PolyTextureChrome16( polyIP, &matP->textureP->origMap );
		}
		else
		if ( pflags & POE_FLG_TEXTURE )
		{
		MLB_Material_t	*matP;

			matP = MLB_MaterialGetPtr( polyIP->materialID );
			//BMP_PolyTexture2D16( polyIP, &matP->textureP->origMap );
			BMP_PolyTextureLUT2D16( polyIP,
								 &matP->textureP->origMap,
								 &MLB_LibraryGetPtr()->levPalP->index[ 256*(31-polyIP->c[0]) ] );
		/*extern BO	_cazzo;

		if ( _cazzo )
		{
			BMP_PolyTexture2D16( polyIP, &matP->textureP->origMap );
		}
		else
		{
		long		poly[4*16], *polyp, nVerts;
		const short	*txposp;
		const long	*vertIdxP;
		POE_Vert_t	*poep, *vertsP;

			if ( (nVerts=polyIP->nVerts) < 3 )	return;
		
			vertsP = *polyIP->vertH;
			vertIdxP = polyIP->vertIdx;
			txposp = (const short *)polyIP->txPos + nVerts * 2;
	
			polyp = poly;
			for (--nVerts; nVerts >= 0; --nVerts, polyp += 4)
			{
				poep = &vertsP[	vertIdxP[nVerts] ];
				polyp[0] = poep->screen[0] << 12;
				polyp[1] = poep->screen[1] << 12;

				txposp -= 2,
				polyp[2] = txposp[0] << 12;
				polyp[3] = txposp[1] << 12;
			}
		  	asm_PolyStart=(int *)poly; asm_PolyEnd=(int *)polyp;
			asm_Texture = matP->textureP->origMap.memP;
			{
			BitMap	*bmp = BMP_Get();
			
				asm_Video = bmp->memP;
				asm_MAXX=bmp->wd; asm_MAXY=bmp->he; asm_ROWSZ=bmp->wd;
				//asm_MAXX=320; asm_MAXY=200; asm_ROWSZ=320;
				asm_LinTexturedPoly();
			}
		}*/
		
		}
		else
		{
			if ( _zBufOn )	BMP_PolyFlatZ16( polyIP );
			else			BMP_PolyFlat16( polyIP );
		}
	}
}
/*	else
	if ( flags & PD_FLG_WIRE )
	{
		if NOT( flags & PD_FLG_WIREFILL )
		{
			if ( flags & PD_FLG_PSHADE  ){ __POE_standardPoly.verts[1].c = __MLB_actLibP->shadePal.userColors;
											Cnk_DrawpolyIPhong16( &ACT->bitMap ); } else
			if ( flags & PD_FLG_GSHADE ) Cnk_DrawPolyShaded16( &ACT->bitMap ); else
			/*
			MLB_LibraryGetMaterial
			if ( flags & PD_FLG_TEXTURE )
			{
			COL_LevelPalette_t	*lp=polyIP->matP->levPalP;
				Cnk_DrawPolyTexturedLUT2D16( &ACT->bitMap, &polyIP->matP->texMap, &lp->index[(lp->nLevels-1-polyIP->traColor[0]) * 256] );
			}
			else* /
				Cnk_DrawPoly16( &ACT->bitMap );
		}
		__POE_standardPoly.verts[0].c = 0;
		Cnk_LinePoly16( &ACT->bitMap );
	}
*/
	/*if ( flags & PD_FLG_TEXTURE )
	{
	COL_LevelPalette_t	*lp=polyIP->matP->levPalP;
		Cnk_DrawPolyTexturedLUT2D16( &ACT->bitMap, &polyIP->matP->texMap, &lp->index[(lp->nLevels-1-polyIP->traColor[0]) * 256] );
		//Cnk_DrawPolyTextured2D16( &ACT->bitMap, &polyIP->matP->texMap );
	}
	else*/

/*
//=================================================
static inline pi2p_copyvert( POE_Vert_t *d, const POE_Vert_t *s, long col )
{
	vec3_equ( d->scrvert, s->scrvert );
	d->c = col;
}
//-------------------------------------------------
static inline void polyi2poly( const POE_PolyI_t *pi, POE_Poly_t *p )
{
short				i;
const POE_Vert_t	*srcVertsP;
const long			*srcVertIdxP, *srcColsP;
POE_Vert_t			*dstVertP;

	srcColsP = pi->c;
	srcVertIdxP = pi->vertIdx;
	srcVertsP = *pi->vertH;
	dstVertP = p->verts;

	p->userLong = pi->userLong;
	p->flags = pi->flags;
	p->clipCode = pi->clipCode;
	p->nVerts = pi->nVerts;

	switch( p->nVerts )
	{
	case 8: copyvert( &dstVertP[7], &srcVertsP[srcVertIdxP[7]], srcColsP[7] );
	case 7: copyvert( &dstVertP[6], &srcVertsP[srcVertIdxP[6]], srcColsP[6] );
	case 6: copyvert( &dstVertP[5], &srcVertsP[srcVertIdxP[5]], srcColsP[5] );
	case 5: copyvert( &dstVertP[4], &srcVertsP[srcVertIdxP[4]], srcColsP[4] );
	case 4: copyvert( &dstVertP[3], &srcVertsP[srcVertIdxP[3]], srcColsP[3] );
	case 3: copyvert( &dstVertP[2], &srcVertsP[srcVertIdxP[2]], srcColsP[2] );
	case 2: copyvert( &dstVertP[1], &srcVertsP[srcVertIdxP[1]], srcColsP[1] );
	case 1: copyvert( &dstVertP[0], &srcVertsP[srcVertIdxP[0]], srcColsP[0] );
			break;
	}
}
*/
