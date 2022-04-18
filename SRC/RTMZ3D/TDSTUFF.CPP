//==============================================
// TDSTUFF.CPP
// Copyright (C) Davide Pasca 1995-97
//
// TABS=4
//==============================================
#include <stdlib.h>
#include <stdio.h>
#include <io.h>
#include <fcntl.h>
#include <dos.h>
#include <mem.h>
#include <math.h>
#include <ctype.h>

#include "EXTTYPES.HPP"
#include "PASCALIB.HPP"
#include "TDSTUFF.HPP"
#include "VGAPACK.HPP"
#include "POLYENG.HPP"
#include "COLORS.HPP"
#include "TDST_HP.HPP"
#include "TRIG.HPP"
#include "BMP.HPP"
#include "M44.HPP"
#include "GL_MATR.HPP"
#include "MATELIB.HPP"

#define	SCRCOEFF	16.0

W3D_World_t	_W3D_actWorld;
W3D_World_t	*_W3D_actWorldP=&_W3D_actWorld;

#define ACT	_W3D_actWorldP

//=======================================
#define  S		65536.
#define  MAGIC  (((S * S * 16) + (S*.5)) * S)
static inline long float2int( float d )
{
  double dtemp = MAGIC + d;
  return (*(long *)&dtemp) - 0x80000000;
}

//=======================================
void W3D_Initialize(void)
{
	memset(ACT,0,sizeof(W3D_World_t));
	W3D_LightSetPos( -.7,  1., -1. );
	ACT->UseShadePal = 1;
	if ( sizeof(POE_Vert_t) != 64 )
	{
		printf("sizeof(POE_Vert_t)=%ld\n", sizeof(POE_Vert_t) );
		getch();
	}
	if ( sizeof(O3D_VertBase_t) != 32 )
	{
		printf("sizeof(O3D_VertBase_t)=%ld\n", sizeof(O3D_VertBase_t) );
		getch();
	}
	/*if ( sizeof(POE_PolyI_t) != 128 )
	{
		printf("sizeof(POE_PolyI_t)=%ld\n", sizeof(POE_PolyI_t) );
		getch();
	}*/
	MLB_LibraryReset();
	W3D_ShadePaletteSet( &__MLB_actLibP->shadePal );
}

//=======================================
void W3D_LightSetPos( float x, float y, float z )
{
	vec3_set( ACT->lightPos, -x,-y,z );
	vec3_normalize( ACT->lightPos );
}

//=======================================
long W3D_ViewPortSet( US wd, US he )
{
	if ( wd != ACT->bitMap.wd || he != ACT->bitMap.he )
	{
		BMP_Free( &ACT->bitMap );
		if ( BMP_Alloc( &ACT->bitMap, wd, he, 8, 0 ) )	return -1;

		ACT->outWD = wd*SCRCOEFF;
		ACT->outHE = he*SCRCOEFF;

		SAFE_FREE( ACT->zBufferP );
		if ( ACT->zBufferFlg )
			if NOT( ACT->zBufferP = (US *)malloc(sizeof(*ACT->zBufferP)*wd*he) )
				ACT->zBufferFlg = 0;
	}
	return 0;
}

//=======================================
void W3D_PerspectiveSet( float fovy, float aspect, float n, float f )
{
	glMatrixModel( GL_PROJECTION );
	glLoadIdentity();
	gluPerspective( fovy, aspect, n, f );
	glMatrixModel( GL_MODELVIEW );
}
//=======================================
void W3D_ValueSet( UB cmd, long *destP )
{
	switch( cmd )
	{
	case W3D_BITMAP_PTR: ACT->bitMap = *((BitMap *)*destP);	break;
	case W3D_ZBUFFER_FLG:
		SAFE_FREE( ACT->zBufferP );
		if ( ACT->zBufferFlg = *destP )
			if NOT( ACT->zBufferP = (US *)malloc(sizeof(*ACT->zBufferP)*ACT->bitMap.wd*ACT->bitMap.he) )
				ACT->zBufferFlg = 0;
		break;
	}
}

//=======================================
void W3D_ValueGet( UB cmd, long *destP )
{
	switch( cmd )
	{
	case W3D_BITMAP_PTR: *destP = (long)&ACT->bitMap;	break;
	case W3D_ZBUFFER_FLG: *destP = ACT->zBufferFlg;	break;
	}
}

//=======================================
void W3D_ObjectCenter( long objID )
{
O3D_Object_t *oP=(O3D_Object_t *)objID;
float    z;

	z = MAX( ACT->DeeX, oP->totDim[2] );
	if ( z < ACT->DeeX )  z = ACT->DeeX;
	glPushMatrix();
		glLoadIdentity();
		glTranslate( 0,0,z );
		long old=O3D_Set( objID );
			O3D_MatrixSet();
		O3D_Set( old );
	glPopMatrix();
}

//============================================
static inline UB calcLight( const float *norP, const float *lightP, float shadVal )
{
float	cos_theta;

	cos_theta = lightP[0]*norP[0] + lightP[1]*norP[1] + lightP[2]*norP[2];
	if ( !((*(long *)&cos_theta)*2) || ((*(long *)&cos_theta) & 0x80000000) )
		return 0;

	return float2int( shadVal * cos_theta );
}


//===========================================
static UB W3D_ObjectIsVisible( O3D_Object_t *objP, const float *matp )
{
/*
VERT_T			box[8][3];
const VERT_T	*min,*max;

	min = objP->Min;
	max = objP->Max;
	vec3_set( box[0], min[0], min[1], min[2] );
	vec3_set( box[1], max[0], min[1], min[2] );
	vec3_set( box[2], max[0], max[1], min[2] );
	vec3_set( box[3], min[0], max[1], min[2] );

	vec3_set( box[4], min[0], min[1], max[2] );
	vec3_set( box[5], max[0], min[1], max[2] );
	vec3_set( box[6], max[0], max[1], max[2] );
	vec3_set( box[7], min[0], max[1], max[2] );

	glMultMatrix( (float *)objP->mat );
*/
/*	M34_Transform( box[0], box[0], 8 );

	project( box[0], box[0], ACT->DeeX, ACT->DeeY, ACT->outWD>>1, ACT->outHE>>1, _CLP_z1, 8 );

UB	andCode=0xff;
	for(long i=7; i >= 0; --i)
		andCode &= CLP_Code3( box[i] );

	if NOT( andCode )
		return 1;

	return 0;*/

	if ( matp )
		m44_equ( objP->mat, (const float (*)[4])matp );

	m44_equ( objP->matinv, (const float (*)[4])objP->mat );
	m44_invert( objP->matinv );
//	m44_identity( objP->matinv );

	return 1;
}

//=====================================
inline long vec3_dot( const long *a, const long *b ){return a[0]*b[0]+a[1]*b[1]+a[2]*b[2];}

static void W3D_ObjectInsert( O3D_Object_t *objP )
{
float	objView[3], eyeView[3];

	eyeView[0] = objP->mat[0][3];
	eyeView[1] = objP->mat[1][3];
	eyeView[2] = objP->mat[2][3];
	glPushMatrix();
		glLoadMatrix( (float *)objP->matinv );
		glTransform3X3( objView, eyeView, 1 );
	glPopMatrix();


UB				visible;
POE_PolyI_t		*polyP;
long			*zMedP;
O3D_VertBase_t	*bvertsP;

	visible = 0;
	bvertsP = objP->baseVertsP;
	zMedP = &ACT->ZMediumP[ ACT->nActPolys*2 ];
	polyP = objP->PolysP;
	for(long i = objP->curPolyNum; i > 0; --i, ++polyP)
	{
		if ( polyP->flags & POE_FLG_NOBCULL )
			visible = 1;
		else
		if NOT( polyP->flags & POE_FLG_ISCHILD )
		{
			visible = 1;
			if ( polyP->nVerts >= 3 )
			{
			/*float	t;

				visible = *((long *)&t) > *((long *)&polyP->d);*/
				visible = (vec3_dot(polyP->nor, objView) > polyP->d);
			}
		}

		if ( visible )
		{
#if POE_MAXVERTS != 5
#error CAZZO !!!!!
#endif
			long  *idxP = polyP->vertIdx;
			switch ( polyP->nVerts )
			{
			case POE_MAXVERTS:	bvertsP[ *idxP++ ].doTrans = 1;
			/*case 7:	bvertsP[ *idxP++ ].doTrans = 1;
			case 6:	bvertsP[ *idxP++ ].doTrans = 1;
			case 5:	bvertsP[ *idxP++ ].doTrans = 1;*/
			case 4:	bvertsP[ *idxP++ ].doTrans = 1;
			case 3:	bvertsP[ *idxP++ ].doTrans = 1;
			case 2:	bvertsP[ *idxP++ ].doTrans = 1;
			case 1:	bvertsP[ *idxP ].doTrans = 1;	break;
			}

			zMedP[1] = (long)polyP;
			zMedP += 2;
			++ACT->nActPolys;
		}
	}
}

//-----------------------------------
static void W3D_ObjectTranslate( Obj3D *objP )
{
	if NOT( objP->doNotTransVerts )
	{
	float	*tralightP;

		glPushMatrix();
			glLoadMatrix( (float *)objP->matinv );
			glTransform3X3( objP->traLight, ACT->lightPos, 1 );

			vec3_normalize( objP->traLight );
			tralightP = objP->traLight;

			if ( objP->flags & POE_FLG_GSHADE )
			{
			float			maxShade = ACT->ActSPaletteP->nShades-1;
			O3D_VertBase_t	*bvertP = objP->baseVertsP;

				for(long i=objP->curVertNum; i; --i, ++bvertP)
					if ( bvertP->doTrans )
						bvertP->shade = calcLight( bvertP->nor, tralightP, maxShade );
			}

			glLoadMatrix( (float *)objP->mat );
			if ( objP->flags & POE_FLG_TEXCHROME  )
				_glTransformO3DtoPOENormals( objP->baseVertsP, objP->vertsP, objP->curVertNum );
			else
				_glTransformO3DtoPOE( objP->baseVertsP, objP->vertsP, objP->curVertNum );
			objP->doNotTransVerts = 1;
		glPopMatrix();
	}
}

//===========================================
static void W3D_DrawPoly( const POE_PolyI_t *polyP )
{
	if ( polyP->nVerts == 1 )
	{
		if NOT( polyP->clipCode )
		{
		long	*pp;

			pp = (*polyP->vertH)[ polyP->vertIdx[0] ].screen;
			//BMP_PixelPut( pp[0]>>4, pp[1]>>4, polyP->traColor[0] );
		}
	}
	else
		W3D_HandlePoly( polyP );
}

//---------------------------------------------
#define PINVISIBLE	0x7fffffff
static void ProcPolysShades( void )
{
POE_PolyI_t **polyPP, *polyP;
long		i;
UB			lightDeep;
short		nShades;
float		nShades1;

	nShades = ACT->ActSPaletteP->nShades;
	nShades1 = nShades - 1.;

	lightDeep = 0;
	polyPP = (POE_PolyI_t **)ACT->ZMediumP + 1;
	for(i=ACT->nActPolys; i; --i, polyPP += 2)
	{
	US	flags;

		if ( ((long *)polyPP)[-1] == PINVISIBLE )	continue;

		polyP = *polyPP;
		flags = polyP->flags;
		if NOT( flags & POE_FLG_PALCOL )
		{
		short	npts;
		UB		*traColP;

			npts = polyP->nVerts;
			traColP = polyP->c;
			if ( flags & POE_FLG_GSHADE )
			{
			Obj3D	*objP;

				objP = (Obj3D *)polyP->userLong;
				long  *idxP = polyP->vertIdx;

				O3D_VertBase_t	*vshaP = (O3D_VertBase_t *)&objP->baseVertsP->shade;
				/*if ( flags & POE_FLG_HALFLITE )
					for (; npts; --npts)
						*traColP++ = *(UB *)(vshaP + *idxP++) >> 1;
				else*/
					for (; npts; --npts)
						*traColP++ = *(UB *)(vshaP + *idxP++);
			}
			else
			{
				if NOT( flags & POE_FLG_ISCHILD )
				{
					if ( npts > 2 )
						if ( flags & POE_FLG_TEXTURE )
							lightDeep = calcLight( polyP->nor,
												   ((Obj3D *)polyP->userLong)->traLight,
												   31 );
						else
							lightDeep = calcLight( polyP->nor,
												   ((Obj3D *)polyP->userLong)->traLight,
												   nShades1 );
					else
						if ( flags & POE_FLG_TEXTURE )
							lightDeep = 16;
						else
							lightDeep = nShades >> 1;

					//if ( flags & POE_FLG_HALFLITE )
					//	lightDeep >>= 1;
				}
				traColP[0] = lightDeep;
			}
		}
	}
}

//---------------------------------------------
static void ProcPolysColors( void )
{
POE_PolyI_t	**polyPP, *polyP;
long		i;
short		nShadesInt, userColors;

	polyPP = (POE_PolyI_t **)ACT->ZMediumP + 1;
	if ( ACT->UseShadePal )
	{
		userColors = ACT->ActSPaletteP->userColors;
		nShadesInt = ACT->ActSPaletteP->nShades;
		for(i=ACT->nActPolys; i; --i, polyPP += 2)
		{
		short	col;
		US		flags;
		UB		*traColP;

			if ( ((long *)polyPP)[-1] == PINVISIBLE )	continue;

			polyP = *polyPP;
			col = polyP->materialID;
			flags = polyP->flags;
			traColP = polyP->c;

			if ( flags & (POE_FLG_PALCOL|POE_FLG_PSHADE) )
				traColP[0] = col;
			else
			if NOT( flags & POE_FLG_TEXTURE )
			{
				col = col * nShadesInt + userColors;
				if ( flags & POE_FLG_GSHADE )
				{
				UB	*endp = traColP + polyP->nVerts;
	
					while ( traColP < endp )
						*traColP++ += col;	// apply color to all the vertices
				}
				else
					*traColP += col;	// apply color to the 1st vertex
			}
		}
	}
	else
	{
		for(i=ACT->nActPolys; i; --i, polyPP += 2)
		{
			polyP = *polyPP;
			COL_LPA_GetColor( ACT->LevelPalP, polyP->c[0], polyP->c[0] );
		}
	}
}

//---------------------------------------------
static void ProcPolysClipAndZeta( void )
{
POE_PolyI_t	**polyPP, *polyP;
POE_Vert_t	*pvertP;
long		lastParentZMed=0;
long		*idxP;
short		nPVerts, j;

	polyPP = (POE_PolyI_t **)ACT->ZMediumP + 1;
	for(long i=ACT->nActPolys; i; --i, polyPP += 2)
	{
		polyP = *polyPP;
		pvertP = *polyP->vertH; //((Obj3D *)polyP->userLong)->vertsP;
		nPVerts = polyP->nVerts;

		UB	cand = 0xff, cor = 0, tcod;
		idxP = polyP->vertIdx;
		for(j=nPVerts; j > 0; --j, ++idxP)
		{
			tcod = pvertP[ *idxP ].clipCode;
			cand &= tcod;
			cor |= tcod;
		}
		((long *)polyPP)[-1] = PINVISIBLE;
		if NOT( cand )
		{
			polyP->clipCode = cor;

			if NOT( polyP->flags & POE_FLG_ISCHILD )
			{
				/*lastParentZMed = 0.;
				idxP = polyP->vertIdx;
				for(j=nPVerts; j; --j, ++idxP)
					lastParentZMed += pvertP[*idxP].sz;

				lastParentZMed /= nPVerts;*/
				/* use all divs (compiler will transform to >>) to prevent overflow ! */
				switch(nPVerts)
				{
				case 1:	lastParentZMed = pvertP[polyP->vertIdx[0]].sz;		break;

				case 2:	lastParentZMed = (pvertP[polyP->vertIdx[0]].sz/2 +
										  pvertP[polyP->vertIdx[1]].sz/2);	break;

				case 3:	idxP = polyP->vertIdx;
						lastParentZMed = (pvertP[idxP[0]].sz/4 +
										  pvertP[idxP[1]].sz/4 +
										  pvertP[idxP[2]].sz/4 + pvertP[idxP[2]].sz/4);	break;
				default:
						idxP = polyP->vertIdx;
						lastParentZMed = (pvertP[idxP[0]].sz/4 +
										  pvertP[idxP[1]].sz/4 +
										  pvertP[idxP[2]].sz/4 +
										  pvertP[idxP[3]].sz/4);	break;
				}
			}
			else
				lastParentZMed += 1;

			((long *)polyPP)[-1] = lastParentZMed;
		}
	}
}

//---------------------------------------------
static void W3D_ProcPolys(void)
{
	//ACT->nActPolys = 0;
	if NOT( ACT->nActPolys )    return;
	ProcPolysClipAndZeta();
	ProcPolysShades();
	ProcPolysColors();
	//WaitTof();
	//COL_SetOneVGA64( 0, 63,0,0 );
	QSortLX( (long *)ACT->ZMediumP, ACT->nActPolys );
	//COL_SetOneVGA64( 0, 0,0,0 );
}

//==================================
extern void memset4(void *dP, UL pattern, long size );
#pragma aux memset4 =   \
"	shr		ecx,2"\
"	rep		stosd"\
parm caller [edi][eax][ecx]\
modify [edi ecx];

//====================================
void W3D_FrameClear(void)
{
	/*BMP_Clear();*/
	if ( ACT->zBufferFlg )
		memset( ACT->zBufferP, 0x00000000, ACT->bitMap.wd * ACT->bitMap.he * sizeof(*ACT->zBufferP) );
}

//=======================================
void W3D_ObjectDraw( long objID, const float *matp )
{
	if ( ACT->NObjs < W3D_MAXOBJECTS )
	{
	O3D_Object_t *objP=(O3D_Object_t *)objID;

		ACT->NPolys += objP->totPolyNum;
		ACT->objList[ ACT->NObjs ] = objP;

		if ( matp )
		{
			ACT->objMatPresent[ ACT->NObjs ] = 1;
			memcpytiny( &ACT->objMatList[ ACT->NObjs ][0], matp, 16*sizeof(float) );
		}
		else
			ACT->objMatPresent[ ACT->NObjs ] = 0;

		++ACT->NObjs;
	}
}

//====================================
void W3D_FrameCalculate(void)
{
short   i;

	ACT->nActPolys = 0;
	if NOT( ACT->ZMediumP = (long *)malloc( ACT->NPolys*sizeof(long)*2 ) )
	{
		ACT->NPolys = 0;
		return;
	}

	glPushMatrix();
		for(i=0; i < ACT->NObjs; ++i)
		{
		Obj3D	*subObjP = ACT->objList[i];

			while ( subObjP )
			{
				subObjP->doNotTransVerts = 0;
				if ( W3D_ObjectIsVisible( subObjP, ACT->objMatPresent[i] ? &ACT->objMatList[i][0] : 0 ) )
				{
					W3D_ObjectInsert( subObjP );
					W3D_ObjectTranslate( subObjP );
				}
				subObjP = (Obj3D *)subObjP->next;
			}
		}
		W3D_ProcPolys();
	glPopMatrix();
}

//====================================
void W3D_FrameRend(void)
{
long	i;
long	*zMedP;

	zMedP = ACT->ZMediumP;
	if ( !ACT->zBufferFlg || (((POE_PolyI_t *)zMedP[1])->flags & POE_FLG_TEXTURE) )
	{
		for(i=ACT->nActPolys; i; --i, zMedP += 2)
			if ( zMedP[0] != PINVISIBLE )
				W3D_DrawPoly( (POE_PolyI_t *)zMedP[1] );
	}
	else
	{
		zMedP += ACT->nActPolys*2 - 2;
		for(i=ACT->nActPolys; i; --i, zMedP -= 2)
			if ( zMedP[0] != PINVISIBLE )
				W3D_DrawPoly( (POE_PolyI_t *)zMedP[1] );
	}

	SAFE_FREE( ACT->ZMediumP );
	ACT->NPolys = 0;
	ACT->NObjs = 0;
}

//====================================
void W3D_ShadePaletteSet(COL_ShadePalette_t *sPalP)
{
	ACT->ActSPaletteP = sPalP;
	ACT->UseShadePal = 1;
}

//====================================
/*POE_PolyI_t *World3D::PreRendFindPoly(long x, long y)
{
long	i,j;
long	*zMedP;
POE_PolyI_t	*polyP;

	zMedP = ZMediumP + 1;
	for(i = nActPolys-1; i; --i, zMedP += 2)
	{
	const VEC2_t	*vertsP;
	const US		*vertIdxP;

		polyP = (POE_PolyI_t *)zMedP[1];
		vertIdxP = polyP->vertNum;
		vertsP = ((Obj3D *)polyP->objP)->VideoPosP;

		UB	c=0;
		for(i=0, j = polyP->NPoints-1; i < polyP->NPoints; j = i++)
		{
		const VEC2_t	*pi,*pj;

			pi = vertsP + vertIdxP[i] * 3;
			pj = vertsP + vertIdxP[j] * 3;
        	if ( ((pi->y <= y && y < pj->y) || (pj->y <= y && y < pi->y)) )
			{
			long	dx,dy;

				dx = pj->x - pi->x;
				if NOT( dy = pj->y - pi->y )	dy = 1;
				if (x < dx * (y - pi->y) / dy + pi->x)
		          c = !c;
			}
		}
		if ( c )	return polyP;
	}

	return 0;
}
*/
