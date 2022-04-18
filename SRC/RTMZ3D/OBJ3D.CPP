//==============================================
// OBJ3D.CPP - 3D objects
// Copyright (C) Davide Pasca 1994-1995
//==============================================

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <conio.h>
#include <string.h>
#include <mem.h>
#include "OBJ3D.HPP"
#include "CVEC3.HPP"
#include "PCX.HPP"
#include "PASCALIB.HPP"
#include "MATELIB.HPP"
#include "PASCALIB.HPP"
#include "GL_MATR.HPP"
#include "M44.HPP"

#define ACT __O3D_actObjP
O3D_Object_t	*__O3D_actObjP;

//==============================================
void vec3_normal( float *nor, const float *p0, const float *p1, const float *p2 )
{
float	vd1[3], vd2[3], t[3];

	vec3_sub( vd1, p2, p1 );
	vec3_sub( vd2, p0, p1 );
	vec3_xprd( t, vd1, vd2 );
	vec3_equ( nor, t );
	vec3_normalize( nor );
}

//==============================================
void O3D_ValueGet( UB cmd, long *valP )
{
	switch(cmd)
	{
	case O3D_TOTAL_POLYS: *valP = ACT->totPolyNum;	break;
	case O3D_TOTAL_VERTS: *valP = ACT->totVertNum;	break;
	}
}

//==============================================
void O3D_ValueGet( UB cmd, float *valP )
{
	switch(cmd)
	{
	case O3D_SPHERE_RAD: 		*valP = ACT->sphererad;		break;
	case O3D_TOTAL_SPHERE_RAD:	*valP = ACT->totsphererad;	break;
	}
}

//==============================================
long O3D_AllocPolys(UL npolys)
{
POE_PolyI_t	*polyP;

	if ( polyP = (POE_PolyI_t *)realloc(ACT->PolysP,npolys*sizeof(*polyP)) )
	{
		ACT->PolysP = polyP;
		ACT->NPolys = npolys;
		return 0;
	}

	return -1;
}

//---------------------------------
void O3D_Init(void)
{
	memset( ACT, 0, sizeof(Obj3D) );
	m44_identity( ACT->mat );
}
//---------------------------------
void O3D_Init(const char *nameP)
{
	O3D_Init();
	strcpymaxsize( ACT->name, nameP, sizeof(ACT->name) );
}
//---------------------------------
long O3D_New(void)
{
O3D_Object_t	*o;

	if ( NEW(o,1) )
	{
	long	old = O3D_Get();
		O3D_Set((long)o);
		O3D_Init();
		O3D_Set(old);
	}

	return (long)o;
}
//---------------------------------
long O3D_New(const char *nameP)
{
O3D_Object_t	*o;

	if ( NEW(o,1) )
	{
	long	old = O3D_Get();
		O3D_Set((long)o);
		O3D_Init(nameP);
		O3D_Set(old);
	}

	return (long)o;
}

//---------------------------------
long O3D_VertNew(float x, float y, float z)
{
	if ( ACT->curVertNum >= ACT->NVerts )
		if ( O3D_AllocVerts(ACT->NVerts+10) )
			return -1;

	return _O3D_VertNew( x, y, z );
}
//---------------------------------
long O3D_VertNew(const float *v)
{
	if ( ACT->curVertNum >= ACT->NVerts )
		if ( O3D_AllocVerts(ACT->NVerts+10) )
			return -1;

	return _O3D_VertNew( v[0],v[1],v[2] );
}

//=================================
long O3D_PolyBegin(long mat, US flags)
{
	if ( ACT->curPolyNum >= ACT->NPolys )
		if ( O3D_AllocPolys(ACT->NPolys+16) )
			return -1;

	ACT->curPolyP = &ACT->PolysP[ ACT->curPolyNum ];
	_O3D_PolyBegin(mat, flags);
	ACT->flags |= flags;
	return 0;
}
//---------------------------------
long O3D_PolyVertNew(long vertIdx, long u, long v)
{
POE_PolyI_t	*polyP;

	polyP = &ACT->PolysP[ ACT->curPolyNum ];
	if ( polyP->nVerts >= POE_MAXVERTS )
	{
	POE_PolyI_t	oldpoly=*polyP;

		O3D_PolyEnd();
		if ( O3D_PolyBegin(0,0) )
		{	// un_end poly
			--ACT->curPolyP;
			--ACT->curPolyNum;
		}
		polyP = ACT->curPolyP;

		oldpoly.vertIdx[1] = oldpoly.vertIdx[ oldpoly.nVerts - 1 ];
		oldpoly.nVerts = 2;
		*polyP = oldpoly;
	}

	polyP->vertIdx[polyP->nVerts] = vertIdx;
	polyP->txPos[polyP->nVerts][0] = u;
	polyP->txPos[polyP->nVerts++][1] = v;

	return 0;
}

//=================================
long O3D_AllocVerts(UL nverts)
{
void	*t;
UB		noMem=0;

	if ( t = realloc(ACT->vertsP,nverts*sizeof(*ACT->vertsP)) )
		ACT->vertsP = (POE_Vert_t *)t;
	else	noMem = 1;

	if ( t = realloc(ACT->baseVertsP,nverts*sizeof(*ACT->baseVertsP)) )
		ACT->baseVertsP = (O3D_VertBase_t *)t;
	else	noMem = 1;

	if ( noMem )
		return -1;
	else
	{
		ACT->NVerts = nverts;
		return 0;
	}
}

//-------------------------------
void O3D_Free( long objID )
{
O3D_Object_t	*o=(O3D_Object_t *)objID;

	while( o )
	{
		SAFE_FREE( o->PolysP );
		SAFE_FREE( o->vertsP );
		SAFE_FREE( o->baseVertsP );
		o->NVerts = o->NPolys = 0;
		o = (O3D_Object_t *)o->next;
	}
}
//------------------------------
void O3D_Dispose( long objID )
{
	O3D_Free( objID );

O3D_Object_t	*o=(O3D_Object_t *)objID, *t;

	while( o )
	{
		t = (O3D_Object_t *)o->next;
		FREEIF( o );
		o = t;
	}
}

//------------------------------
static void O3D_CalcPolyNormals(void)
{
O3D_VertBase_t	*bvertsP;
POE_PolyI_t		*polyP;

	bvertsP = ACT->baseVertsP;
	polyP = ACT->PolysP;
	for(long i=ACT->curPolyNum; i > 0; --i, ++polyP )
		if ( polyP->nVerts >= 3 )
		{
		long	*vIdxP;

			vIdxP = polyP->vertIdx;
			vec3_normal( polyP->nor, bvertsP[vIdxP[0]].vert,
									 bvertsP[vIdxP[1]].vert,
									 bvertsP[vIdxP[2]].vert	);
			polyP->d = -vec3_dot(bvertsP[vIdxP[0]].vert, polyP->nor);
		}
		else
		{
			vec3_set( polyP->nor, 0.,0.,0. );
			polyP->d = 0.;
		}
}

//------------------------------
static void O3D_CalcVertNormals(void)
{
O3D_VertBase_t	*bvertsP;
POE_PolyI_t		*polyP;
long			i,j;

	bvertsP = ACT->baseVertsP;
	for(i=ACT->curVertNum; i > 0; --i, ++bvertsP)
		vec3_set( bvertsP->nor, 0,0,0 );

	bvertsP = ACT->baseVertsP;
	polyP = ACT->PolysP;
	for(i = ACT->curPolyNum; i; --i, ++polyP)
	{
	long	*vIdxP;
	float	mid[3];

		vec3_equ( mid, polyP->nor );
		vIdxP = polyP->vertIdx;
		for(j=polyP->nVerts; j > 0; --j)
		{
			float *v = bvertsP[ *vIdxP++ ].nor;
			vec3_add( v, v, mid );
		}
	}

	for(i=ACT->curVertNum; i > 0; --i, ++bvertsP)
		vec3_normalize( bvertsP->nor );
}

//==============================================
void O3D_OptimizeTriObject(void)
{
O3D_Object_t	*oldP=ACT;

	while (ACT)
	{
	POE_PolyI_t	*polyP;
	long	i;
	UB		doCompact=0;

		polyP = ACT->PolysP;
		for(i = ACT->curPolyNum-1; i > 0; --i, ++polyP)
		{
			if ( polyP->nVerts == 3 && polyP[1].nVerts == 3 )
			{
				if ( polyP->materialID == polyP[1].materialID )
				{
				long	n1[3], n2[3];
				float	nf1[3], nf2[3];

					vec3_equ( nf1, polyP[0].nor );
					vec3_equ( nf2, polyP[1].nor );
					vec3_mul( nf1, nf1, 1000. ); vec3_equ( n1, nf1 );
					vec3_mul( nf2, nf2, 1000. ); vec3_equ( n2, nf2 );
					if ( n1[0]==n2[0] && n1[1]==n2[1] && n1[2]==n2[2] )
					{
					long	*v1, *v2;
					long	vLink1=-1, vLink2=-1;
					long	unlink1Idx, unlink2Idx, link1Idx;
	
						v1 = polyP->vertIdx; v2 = polyP[1].vertIdx;
						for(long j1=2; j1 >= 0; --j1)
							for(long j2=2; j2 >= 0; --j2)
								if ( v1[j1] == v2[j2] )
								{
									if ( vLink1 >= 0 )
									{
										vLink2 = v1[j1];
										goto esciLoop;
									}
									else
									{
										vLink1 = v1[link1Idx=j1];
										break;
									}
								}
esciLoop:				if ( vLink1 >= 0 && vLink2 >= 0 )
						{
						US	vUnlinkV1, vUnlinkV2;
	
							doCompact = 1;
							for(long j=2; j >= 0; --j)
							{
								if ( v1[j] != vLink1 && v1[j] != vLink2 )	vUnlinkV1 = v1[unlink1Idx=j];
								if ( v2[j] != vLink1 && v2[j] != vLink2 )	vUnlinkV2 = v2[unlink2Idx=j];
							}

							if ( v2[unlink2Idx == 0 ? 2 : unlink2Idx-1] != vLink1 )
							{
								v2[3] = vLink1;
								v2[1] = vLink2;
							}
							else
							{
								v2[1] = vLink1;
								v2[3] = vLink2;
							}
							v2[0] = vUnlinkV1;
							v2[2] = vUnlinkV2;
							polyP[1].nVerts = 4;
							polyP[0].nVerts = 0;
						}
					}
				}
			}
		}

		if ( doCompact )
		{
			POE_PolyI_t	*desP;
			desP = polyP = ACT->PolysP;
			for(i=0; i < ACT->curPolyNum; ++i)
			{
				while NOT( polyP->nVerts )
				{
					if ( --ACT->curPolyNum <= 0 )
						goto esciAlVolo;
					++polyP;
				}
				*desP++ = *polyP++;
			}
esciAlVolo:
			O3D_AllocPolys( ACT->curPolyNum );
		}
		ACT = (O3D_Object_t *)ACT->next;
	}

	ACT = oldP;
}

//------------------------------
static void copyNormals( O3D_Object_t *o )
{
O3D_VertBase_t	*bvertsP = o->baseVertsP;
POE_Vert_t		*pvertsP = o->vertsP;

	for(long i=ACT->curVertNum; i; --i, ++bvertsP, ++pvertsP)
		vec3_equ( pvertsP->nor, bvertsP->nor );
}

//------------------------------
void O3D_CalcNormals_NoList(void)
{
	O3D_CalcPolyNormals();
	O3D_CalcVertNormals();
	copyNormals( ACT );
}
//------------------------------
void O3D_CalcNormals(void)
{
O3D_Object_t	*o=ACT;

	while (ACT)
	{
		O3D_CalcNormals_NoList();
		ACT = (Obj3D *)ACT->next;
	}
	ACT = o;
}

//=====================================
long O3D_GetByName( const char *nameP )
{
O3D_Object_t	*oP=ACT;

	while (oP)
	{
		if NOT( strcmp( nameP, oP->name ) )
			return (long)oP;
		oP = (O3D_Object_t *)oP->next;
	}

	return 0;
}

//=====================================
void O3D_MatrixSet( long objID, const float mat[4][4] )
{
O3D_Object_t	*oP=(O3D_Object_t *)objID;

	while (oP)
	{
		memcpytiny( oP->mat, mat, 16*sizeof(float));
		oP->doNotTransVerts = 0;
		oP = (O3D_Object_t *)oP->next;
	}
}
//------------------------------
void O3D_PolysFlags_NoList( UB cmd, US flags )
{
POE_PolyI_t	*polyP;
UL			npol;

	polyP = ACT->PolysP;
	npol = ACT->curPolyNum;

	switch( cmd )
	{
	case O3D_PF_SET:
		while( npol-- )	(polyP++)->flags = flags;	ACT->flags = flags;	break;

	case O3D_PF_AND:
		while( npol-- )	(polyP++)->flags &= flags;	ACT->flags &= flags;	break;

	case O3D_PF_OR:
		while( npol-- )	(polyP++)->flags |= flags;	ACT->flags |= flags;	break;

	case O3D_PF_TOGGLE:
		for(; npol--; ++polyP )
			if ( polyP->flags & flags )
				polyP->flags &= ~flags;
			else
				polyP->flags |= flags;
	
		if ( ACT->flags & flags )		ACT->flags &= ~flags;
		else							ACT->flags |= flags;
		break;
	}
}
void O3D_PolysFlags( UB cmd, US flags )
{
O3D_Object_t	*old;

	old = ACT;
	while ( ACT )
	{
		O3D_PolysFlags_NoList( cmd, flags );
		ACT = (O3D_Object_t *)ACT->next;
	}
	ACT = old;
}

//=======================================================
//=================== TEXTURE STUFF =====================
//=======================================================
#ifndef PI
#define PI	3.14159265359
#endif
static void tdsphere( long txwd, long txhe, short *tPosRunP )
{
long			i;
float			wd,he, redim[3];
O3D_VertBase_t	*bvertP;

	wd = (txwd-1) * 16.;
	he = (txhe-1) * 16.;

	vec3_equ( redim, ACT->Dim );
	if ( redim[0] )	redim[0] = 1./redim[0];
	if ( redim[1] )	redim[1] = 1./redim[1];
	if ( redim[2] )	redim[2] = 1./redim[2];

	bvertP = ACT->baseVertsP;
	for(i=ACT->curVertNum; i; --i, ++bvertP)
	{
	float			vert[3], t;
	//static float	ne[3]={-1.,0.,0.}, nnp[3]={0.,-1.,0.};

		vec3_sub( vert, bvertP->vert, ACT->Min );
		vec3_mul( vert, vert, redim );
		CLAMP(vert[0],0.,1.);
		CLAMP(vert[1],0.,1.);
		CLAMP(vert[2],0.,1.);

		t = vert[1] * 2 - 1;	CLAMP(t,-1.,1.);
		tPosRunP[0] = wd * acos( t ) / PI;
		t = vert[2] * 2 - 1;	CLAMP(t,-1.,1.);
		tPosRunP[1] = he * acos( t ) / PI;
		CLAMP(tPosRunP[0],0,wd);
		CLAMP(tPosRunP[1],0,he);
		tPosRunP += 2;
	}
}

//=============================================
static void tdcylinder( long txwd, long txhe, short *tPosRunP )
{
long			i;
float			wd,he, redim[3];
O3D_VertBase_t	*bvertP;

	wd = (txwd-1) * 16.;
	he = (txhe-1) * 16.;

	vec3_equ( redim, ACT->Dim );
	if ( redim[0] )	redim[0] = 1./redim[0];
	if ( redim[1] )	redim[1] = 1./redim[1];
	if ( redim[2] )	redim[2] = 1./redim[2];

	bvertP = ACT->baseVertsP;
	for(i=ACT->curVertNum; i; --i, ++bvertP)
	{
	float	theta, vert[3];

		//vec3_sub( vert, bvertP->vert, ACT->Min );
		vec3_equ( vert, bvertP->vert );
		vec3_mul( vert, vert, redim );
		//vec3_normalize( vert );
		CLAMP( vert[0], -1., 1. );
		CLAMP( vert[1], -1., 1. );
		CLAMP( vert[2], -1., 1. );

		theta = atan2( vert[2], vert[0] );
		if ( vert[0] > 0. && (theta >= -PI-.01 && theta <= -PI+.01) )
			theta = PI;

		tPosRunP[0] = wd * (PI + theta) / (2*PI);
		tPosRunP[1] = he/2 + he * vert[1];
		CLAMP(tPosRunP[0],0,wd);
		CLAMP(tPosRunP[1],0,he);
		tPosRunP += 2;
	}
}

//=============================================
long O3D_TextureDisplace_NoList( long txwd, long txhe, UL td_kind )
{
short	*tPosP;

	if NOT( tPosP = (short *)malloc( ACT->curVertNum*2*sizeof(short) ) )
		return -1;


	switch( td_kind )
	{
	case O3D_TD_SPHERE:		tdsphere( txwd, txhe, tPosP );		break;
	case O3D_TD_CYLINDER:	tdcylinder( txwd, txhe, tPosP );	break;
	}


POE_PolyI_t	*polyP;

	polyP = ACT->PolysP;
	for(long i=ACT->curPolyNum; i; --i, ++polyP)
	{
	long	j = polyP->nVerts;

		if ( j >= 3 )
			for(--j; j >= 0; --j)
			{
				polyP->txPos[j][0] = tPosP[ polyP->vertIdx[j]*2L+0 ];
				polyP->txPos[j][1] = tPosP[ polyP->vertIdx[j]*2L+1 ];
			}
	}

	free( tPosP );

	return 0;
}

//=============================================
long O3D_TextureDisplace( long txwd, long txhe, UL td_kind )
{
long			err;
O3D_Object_t	*oldobjp;

	oldobjp = ACT;
	while(ACT)
	{
		if ( err = O3D_TextureDisplace_NoList( txwd, txhe, td_kind ) )
		{
			ACT = oldobjp;
			return err;
		}
		ACT = (O3D_Object_t *)ACT->next;
	}
	ACT = oldobjp;

	return 0;
}

//------------------------------
void O3D_GetMinMaxDim( float *min, float *max, float *dim )
{
	if (min)	vec3_equ( min, ACT->totMin );
	if (max)	vec3_equ( max, ACT->totMax );
	if (dim)	vec3_equ( dim, ACT->totDim );
}

//------------------------------
void O3D_CalcMinMaxDim_NoList(void)
{
float			minx,miny,minz, maxx,maxy,maxz;
O3D_VertBase_t	*bvertP;

	bvertP = ACT->baseVertsP;
	minx = maxx = bvertP->vert[0];
	miny = maxy = bvertP->vert[1];
	minz = maxz = bvertP->vert[2];
	for(long i=ACT->curVertNum; i; --i, ++bvertP)
	{
	float	*v=bvertP->vert;
		minx = MIN( minx, v[0] );
		maxx = MAX( maxx, v[0] );
		miny = MIN( miny, v[1] );
		maxy = MAX( maxy, v[1] );
		minz = MIN( minz, v[2] );
		maxz = MAX( maxz, v[2] );
	}
	vec3_set( ACT->Min, minx, miny, minz );
	vec3_set( ACT->Max, maxx, maxy, maxz );
	vec3_sub( ACT->Dim, ACT->Max, ACT->Min );
	ACT->sphererad = sqrt( vec3_dot( ACT->Dim, ACT->Dim ) ) / 2.;
}

//------------------------------
void O3D_CalcMinMaxDim(void)
{
Obj3D	*o, *old = ACT;

	while(ACT)
	{
		O3D_CalcMinMaxDim_NoList();
		ACT = (Obj3D *)ACT->next;
	}

	o = old;
float	*tmin=o->totMin, *tmax=o->totMax, *tdim=o->totDim;

	vec3_equ( tmin, o->Min );
	vec3_equ( tmax, o->Max );
	while ( o )
	{
		tmin[0] = MIN( tmin[0], o->Min[0] );
		tmax[0] = MAX( tmax[0], o->Max[0] );
		tmin[1] = MIN( tmin[1], o->Min[1] );
		tmax[1] = MAX( tmax[1], o->Max[1] );
		tmin[2] = MIN( tmin[2], o->Min[2] );
		tmax[2] = MAX( tmax[2], o->Max[2] );
		o = (Obj3D *)o->next;
	}
	vec3_sub( tdim, tmax, tmin );

	o = (Obj3D *)old->next;
	while ( o )
	{
		vec3_equ( o->totMin, tmin );
		vec3_equ( o->totMax, tmax );
		vec3_equ( o->totDim, tdim );
		o = (Obj3D *)o->next;
	}
	ACT = old;

	ACT->totsphererad = sqrt( vec3_dot( ACT->totDim, ACT->totDim ) ) / 2.;
	o = (Obj3D *)old->next;
	while ( o )
	{
		o->totsphererad = ACT->totsphererad;
		o = (Obj3D *)o->next;
	}
}

//---------------------------------
void O3D_ObjectRecalcStructure(void)
{
	O3D_CalcMinMaxDim();

O3D_Object_t	*o=ACT;
long			totVertNum=0, totPolyNum=0;

	while(o)
	{
		totVertNum += o->curVertNum;
		totPolyNum += o->curPolyNum;
		o = (O3D_Object_t *)o->next;
	}

	o = ACT;
	while ( o )
	{
		o->totVertNum = totVertNum;
		o->totPolyNum = totPolyNum;
		o = (O3D_Object_t *)o->next;
	}
}

//===============================
void O3D_Update(void)
{
	O3D_CalcNormals();
	O3D_ObjectRecalcStructure();
}

