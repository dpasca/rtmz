//==============================================
// OBJ3D.HPP - 3D objects
// Copyright (C) Davide Pasca 1994-1995
//==============================================

#ifndef __OBJ3D_HPP__
#define __OBJ3D_HPP__

typedef float	VEC_T;
typedef float	NOR_T;

#include "EXTTYPES.HPP"
#include "CVEC3.HPP"
#include "COLORS.HPP"
#include "POLYENG.HPP"

//=======================================
typedef struct
{
	UB		rgb[3];

	BitMap	texMap;
	US		palSize;
	UB		mapPalette[256*3];
}O3D_Material_t;

//=======================================
#define O3D_INTPREC				1000.0
#define x3DG1_ID				MAKE_ID('3','D','G','1')
#define LGO3D_CANNOTOPENFILE	-1L
#define LGO3D_UNKNOWFILE		-2L
#define LGO3D_NOMEM				-3L
#define LGO3D_TOOMUCHVERTS		-4L

#define LIST_FIELDS  \
	char name[80];   \
	void *next;

typedef struct
{
	float	vert[3];
	float	nor[3];
	UB		doTrans;
	UB		shade;
	UB		pad[32 -3*4 -3*4 -1 -1];
}O3D_VertBase_t;

typedef struct
{
	LIST_FIELDS
	float			mat[4][4], matinv[4][4];
	void			*worldP;
	NOR_T			traLight[3];

	long			curVertNum, NVerts;		// vertices
	O3D_VertBase_t	*baseVertsP;
	POE_Vert_t		*vertsP;

	long			curPolyNum, NPolys;		// polygons
	POE_PolyI_t		*curPolyP, *PolysP;

	US				nMaterials;
	O3D_Material_t	*materialsP;

	float			sphererad, totsphererad;
	VEC_T			Min[3], Max[3], Dim[3], totMin[3], totMax[3], totDim[3];
	long			totVertNum, totPolyNum;

	UL				oflags;
	US				flags;
	UB				doNotTransVerts;
} Obj3D, O3D_Object_t;

#include "GL_MATR.HPP"

#define ACT __O3D_actObjP
extern O3D_Object_t	*__O3D_actObjP;

inline long O3D_Set(long objID)
{
O3D_Object_t *t=ACT;
	ACT = (O3D_Object_t *)objID;
	return (long)t;
}
inline long O3D_Get(void){return (long)ACT;}

inline void O3D_ResetPolys(void){ ACT->curPolyNum = 0; ACT->curPolyP = ACT->PolysP; }
inline void O3D_ResetVerts(void){ ACT->curVertNum = 0; }
long O3D_AllocVerts(UL nverts);
long O3D_AllocPolys(UL npolys);

//---------------------------------
inline long _O3D_VertNew(float x, float y, float z)
{
	vec3_set( ACT->baseVertsP[ ACT->curVertNum ].vert, x, y, z );
	return ACT->curVertNum++;
}
//---------------------------------
long O3D_VertNew(float x, float y, float z);
long O3D_VertNew(const float *v);

//=================================
long O3D_PolyBegin(long mat, US flags);
inline void _O3D_PolyBegin(long mat, US flags)
{
POE_PolyI_t	*polyP;

	polyP = ACT->curPolyP;
	polyP->nVerts = 0;
	polyP->vertH = &ACT->vertsP;
	polyP->materialID = mat;

	polyP->flags = flags;
	polyP->userLong = (long)ACT;
	polyP->clipCode = 0;
}
//---------------------------------
long O3D_PolyVertNew(long vertIdx,long u=0, long v=0);
inline void _O3D_PolyVertNew(long vertIdx,long u=0, long v=0)
{
POE_PolyI_t	*polyP;

	polyP = ACT->curPolyP;
	polyP->vertIdx[polyP->nVerts] = vertIdx;
	polyP->txPos[polyP->nVerts][0] = u;
	polyP->txPos[polyP->nVerts++][1] = v;
}
//---------------------------------
inline void O3D_PolyEnd(void)
{
	++ACT->curPolyNum;
	++ACT->curPolyP;
}

#define O3D_TOTAL_POLYS	0
#define O3D_TOTAL_VERTS	1
void O3D_ValueGet(UB cmd, long *valP );

#define O3D_SPHERE_RAD			2
#define O3D_TOTAL_SPHERE_RAD	3
void O3D_ValueGet(UB cmd, float *valP );

long O3D_New(void);
long O3D_New(const char *nameP);
void O3D_Dispose( long objID );

inline void O3D_MatrixSet_NoList(long objID=(long)ACT, const float mat[4][4]=(const float (*)[4])_glCurMatP->mf )
{
	memcpytiny( ((O3D_Object_t *)objID)->mat, mat, 16*sizeof(float));
	((O3D_Object_t *)objID)->doNotTransVerts = 0;
}
void O3D_MatrixSet(long objID=(long)ACT, const float mat[4][4]=(const float (*)[4])_glCurMatP->mf );
long O3D_GetByName( const char *nameP );

#define O3D_PF_SET		0
#define O3D_PF_AND		1
#define O3D_PF_OR		2
#define O3D_PF_TOGGLE	3
void O3D_PolysFlags( UB cmd, US flags );
void O3D_PolysFlags_NoList( UB cmd, US flags );

long O3D_LoadGEO( const char *fileName, long *badPolysP );
long O3D_LoadGEOM( const char *fileName, long *badPolysP );

void O3D_Update(void);

void O3D_Init(void);
void O3D_Free( long objID );

#define O3D_TD_SPHERE	1
#define O3D_TD_CYLINDER	2
long O3D_TextureDisplace( long txwd, long txhe, UL td_kind );
long O3D_TextureDisplace_NoList( long txwd, long txhe, UL td_kind );

void O3D_CalcNormals(void);
void O3D_CalcNormals_NoList(void);
void O3D_CalcNormals_NoList( double *vecsP );
void O3D_OptimizeTriObject(void);
void O3D_CalcMinMaxDim(void);
void O3D_CalcMinMaxDim_NoList(void);
void O3D_GetMinMaxDim(float *min, float *max, float *dim );
#undef ACT

#include "READ3DS.HPP"

#endif
