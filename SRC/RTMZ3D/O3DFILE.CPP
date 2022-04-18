//==============================================
// O3DFILE.CPP - 3D file objects
// Copyright (C) Davide Pasca 1994-1995
//==============================================

#include <string.h>
#include "O3DFILE.HPP"
#include "MATELIB.HPP"

#define ACT __O3D_actObjP
extern O3D_Object_t	*__O3D_actObjP;

extern void vec3_normal( float *nor, const float *p0, const float *p1, const float *p2 );

//==============================================
static long O3D_ValidatePolys( UB orderPolys )
{
O3D_VertBase_t	*bvertsP;
long			*vn;
POE_PolyI_t		*polyP;
long			badPolys=0;

	polyP = ACT->PolysP;
	bvertsP = ACT->baseVertsP;
	for(long npol = 0; npol < ACT->curPolyNum; ++npol, ++polyP)
	{
	long	nPVerts = polyP->nVerts;

		if ( nPVerts > 3 )
		{
		float	nor1[3], nor2[3];
		long	j,j1,j2;

			vn = polyP->vertIdx;
			vec3_normal( nor1, bvertsP[vn[0]].vert,
							   bvertsP[vn[1]].vert,
							   bvertsP[vn[2]].vert );
			for(j=1; j < nPVerts; ++j)
			{
			long	v0,v1,v2;

				if ( (j1 = j+1) >= nPVerts )	j1 -= nPVerts;
				if ( (j2 = j+2) >= nPVerts )	j2 -= nPVerts;
				v0 = vn[j];
				v1 = vn[j1];
				v2 = vn[j2];
				vec3_normal( nor2, bvertsP[v0].vert,
								   bvertsP[v1].vert,
								   bvertsP[v2].vert );
				vec3_sub( nor2, nor2, nor1 );
				if ( ABS(nor2[0]) > .05 || ABS(nor2[1]) > .05 || ABS(nor2[2]) > .05 )
				{
				long		otherPts;
				POE_PolyI_t	savePoly = *polyP;

					vec3_set ( vn, v0, v1, v2 );
					polyP->nVerts = 3;

					otherPts = savePoly.nVerts - 3;
					if NOT( O3D_PolyBegin( savePoly.materialID, savePoly.flags ) )
					{
						while (j2 != j)
						{
							O3D_PolyVertNew( savePoly.vertIdx[j2] );
							if ( ++j2 >= savePoly.nVerts )	j2 -= savePoly.nVerts;
						}
						O3D_PolyVertNew( savePoly.vertIdx[j2] );
					O3D_PolyEnd();
					}
					++badPolys;

					if ( orderPolys )
					{
						savePoly = ACT->PolysP[ ACT->curPolyNum-1 ];	// salva l'ultimo
						memmove( ACT->PolysP + npol+2, ACT->PolysP + npol+1,
								(ACT->curPolyNum-npol-2) * sizeof(*ACT->PolysP) );
						ACT->PolysP[ npol+1 ] = savePoly;
					}

					polyP = &ACT->PolysP[ npol ];
					break;
				}
			}
		}
	}

	return badPolys;
}

//=======================================
static long createGEOMat( short col )
{
static UB gpal[16][3]=
{
	0x2f,0x2f,0x2f,	0x00,0x6f,0x9f,	0x00,0x8f,0x00,	0x00,0x7f,0x4f,
	0x9f,0x00,0x00,	0x4f,0x3f,0x4f,	0x9f,0x7f,0x00,	0x7f,0x7f,0x7f,
	0xFf,0xFf,0xFf,	0x1f,0xAf,0xDf,	0x1f,0xCf,0x10,	0x1f,0xBf,0x7f,
	0xDf,0x2f,0x2f,	0x7f,0x6f,0x7f,	0xDf,0xCf,0x1f,	0xCf,0xCf,0xCf
};
char	buf[32];
long	matID;
static float ambient[4]={.2,.2,.25,.05}, diffuse[4]={.1,.1,.1,.95}, specular[]={.4,.4,.4,.5}, shininess=20.;

	sprintf( buf, "GEO%ld", (long)col );
	vec3_set( diffuse, gpal[col][0]/255., gpal[col][1]/255., gpal[col][2]/255. );
	MLB_MaterialSetDefault( MLB_SHININESS_PTR, &shininess, TAG_END );
	if ( (matID = MLB_MaterialAdd(	MLB_NAME_PTR, buf,			MLB_AMBIENT_PTR, ambient,
									MLB_DIFFUSE_PTR, diffuse,	MLB_SPECULAR_PTR, specular,
									MLB_SHININESS_PTR, &shininess, TAG_END )) < 0 )
		return 0;

	return matID;
}

//=======================================
#define DFL	16
static long lgeo_loadNextPoly( FILE	*fh, UL flags )
{
long	i, cnt, tmpL;
long	vertIdx[64];
UL		col;
long	havechild;

	if ( fscanf( fh, "%ld", &cnt ) == EOF )
		return 0;
	if ( cnt <= 0 )   return -1;

	if ( cnt > 64 )	return 0;
	for (i=0; i < cnt; ++i)
		if ( fscanf( fh, "%ld", &vertIdx[i] ) == EOF )
			return 0;

	if ( fscanf( fh, "%ld", &tmpL ) == EOF )
		return 0;

	if ( tmpL < 0 )
	{
		col = -tmpL;
		havechild = 1;
	}
	else
	{
		col = tmpL;
		havechild = 0;
	}

	if ( col & 32 )		flags |= POE_FLG_MAXLITE | POE_FLG_WIRE;
	if ( col & DFL )	flags |= POE_FLG_WIREFILL;
	O3D_PolyBegin( createGEOMat(col&15), flags );
		for (i=0; i < cnt; ++i)
			O3D_PolyVertNew( vertIdx[i] );
	O3D_PolyEnd();

	return havechild;
}

//---------------------------------
long O3D_LoadGEO( const char *fileName, long *badPolysP )
{
char	myStr[10];
long	i, err=0;
FILE	*fh;

	if NOT( fh = fopen(fileName, "r") )
		return LGO3D_CANNOTOPENFILE;

	O3D_Init();

	fscanf( fh, "%5s", myStr );
	if ( strcmp( myStr, "3DG1" ) )
	{
		err = LGO3D_UNKNOWFILE;
		goto exUnkFile;
	}

	fscanf( fh, "%ld", &i );
	if ( O3D_AllocVerts(i) )
	{
		err=LGO3D_NOMEM;
		goto exNoMem;
	}

	for (i=ACT->NVerts; i; --i)
	{
	float	x, y, z;

		fscanf( fh, "%f %f %f", &x, &y, &z);
		O3D_VertNew( x, y, -z );
	}

	while NOT( feof(fh) )
	{
	long	havechild;

		if ( (havechild = lgeo_loadNextPoly(fh,0)) < 0 )
			goto exNoVerts;

		if ( havechild )
		{
		long	nSub;

			fscanf( fh, "%ld", &nSub );
			for (; nSub; --nSub)
				if ( (havechild = lgeo_loadNextPoly(fh,POE_FLG_ISCHILD)) < 0 ) // no more childs !!!
					goto exNoVerts;
		}
	}

	if (badPolysP)	*badPolysP = O3D_ValidatePolys(1);
	else						 O3D_ValidatePolys(1);
	O3D_Update();

goodExit:
	fclose( fh );
	return( err );

exNoVerts:
exNoMem:
	O3D_Free( (long)ACT );
exUnkFile:
	goto goodExit;
}

//---------------------------------
long O3D_LoadGEOM( const char *fileName, long *badPolysP )
{
long	    i, j, dummy, matID;
long		err=0;
FILE		*fh;

	if NOT( fh = fopen(fileName, "r") )
		return( LGO3D_CANNOTOPENFILE );

	O3D_Init();
	matID = MLB_MaterialAdd( MLB_NAME_PTR, "GEOM_MATERIO", TAG_END );

	fscanf( fh, "%ld %ld %ld", &i, &j, &dummy );
	if ( O3D_AllocVerts(i) || O3D_AllocPolys(j) )
	{
		err=LGO3D_NOMEM;
		goto exNoMem;
	}

	for (i=ACT->NVerts; i; --i)
	{
	float	x, y, z;

		fscanf( fh, "%f %f %f", &x, &y, &z);
		O3D_VertNew( x, y, z );
	}

	for (i=ACT->NPolys; i; --i)
	{
		if ( fscanf( fh, "%ld", &j ) == EOF )
			break;
		/*if ( j > POE_MAXVERTS )
		{
			err = LGO3D_TOOMUCHVERTS;
			goto exNoVerts;
		}*/

		O3D_PolyBegin( matID, 0 );
			for(; j > 0; --j)
			{
			long	vn;
				
				fscanf( fh, "%ld", &vn );
				O3D_PolyVertNew( vn-1 );
			}
		O3D_PolyEnd();
	}
	if (badPolysP)	*badPolysP = O3D_ValidatePolys(0);
	else						 O3D_ValidatePolys(0);
	O3D_Update();

goodExit:
	fclose( fh );
	return( err );

exNoVerts:
exNoMem:
	O3D_Free( (long)ACT );
exUnkFile:
	goto goodExit;
}

