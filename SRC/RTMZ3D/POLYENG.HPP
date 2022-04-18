//==============================================
// POLYENG.HPP
// Copyright (C) Davide Pasca 1995-97
//
// TABS=4
//==============================================
#ifndef __POLYENG_HPP__
#define __POLYENG_HPP__

#include "EXTTYPES.HPP"
#include "CVEC3.HPP"

#define POE_MAXVERTS	5
#define POE_VERTBUFSIZ	(POE_MAXVERTS+5)

#define POE_FLG_GSHADE		1
#define POE_FLG_PSHADE		2
#define POE_FLG_ZBUFFER		4
#define POE_FLG_WIRE		16
#define POE_FLG_MAXLITE		32
#define POE_FLG_WIREFILL	64
#define POE_FLG_ISCHILD		128
#define POE_FLG_HALFLITE	256
#define POE_FLG_PALCOL		512
#define POE_FLG_NOBCULL		1024
#define POE_FLG_TEXTURE		2048
#define POE_FLG_TEXIS3D		4096
#define POE_FLG_TEXCHROME	8192

typedef struct
{
	float	eye[4];
	long	screen[2];
	long	sz;
	float	nor[3];
	long	c;
	long	u,v;
	UB		clipCode;
	UB		pad[64 -(4+3+3+1+2)*4 -1];
}POE_Vert_t;

typedef struct
{
	US			nVerts;
	POE_Vert_t	verts[POE_VERTBUFSIZ];

	UL			flags;
	long		userLong;
	UB			clipCode;
}POE_Poly_t;

typedef struct
{
	POE_Vert_t	**vertH;
	float		nor[3], d;
	long		vertIdx[POE_VERTBUFSIZ];
	UB			c[POE_VERTBUFSIZ];
	short		txPos[POE_VERTBUFSIZ][2];
	long		materialID;
	US			nVerts;

	UL			flags;
	long		userLong;
	UB			clipCode;
	//UB			pad[128 -12-2- 4*POE_VERTBUFSIZ - 4-4 -1*POE_VERTBUFSIZ -4-4-1];
}POE_PolyI_t;

//=====================================
extern POE_Poly_t	__POE_standardPoly, *__POE_curPolyP;
extern POE_Vert_t	*__POE_curVertP, *__POE_curVertP_c, *__POE_curVertP_n, *__POE_curVertP_t;

//=====================================
inline POE_Poly_t *POE_GetCurPoly(void){return __POE_curPolyP;}

//=====================================
inline void POE_BeginPolygon(POE_Poly_t *polyP=&__POE_standardPoly)
{
	polyP->nVerts = 0;
	__POE_curPolyP = polyP;
	__POE_curVertP = __POE_curVertP_c = __POE_curVertP_n = __POE_curVertP_t = polyP->verts;
	//memset(poly,0,sizeof(POE_Poly_t));
}

//=====================================
inline void POE_Vertex2v(const long *v)
{
	__POE_curVertP->eye[0] = v[0];
	__POE_curVertP->eye[1] = v[1];
	++__POE_curVertP;
}
//=====================================
inline void POE_Vertex2(long x, long y)
{
	__POE_curVertP->eye[0] = x;
	__POE_curVertP->eye[1] = y;
	++__POE_curVertP;
}
//=====================================
inline void POE_Vertex3(long x, long y, long z)
{
	vec3_set( __POE_curVertP->eye, x,y,z );
	++__POE_curVertP;
}
//=====================================
inline void POE_Vertex3v(const long *v)
{
	vec3_equ( __POE_curVertP->eye, v );
	++__POE_curVertP;
}

//=====================================
inline void POE_Color1(UB col)
{
	__POE_curVertP_c->c = col;
	++__POE_curVertP_c;
}

//=====================================
inline void POE_Normal3(long x, long y, long z)
{
	__POE_curVertP_n->nor[0] = x << 10;
	__POE_curVertP_n->nor[1] = y << 10;
	__POE_curVertP_n->nor[2] = z << 10;
	++__POE_curVertP_n;
}
//=====================================
inline void POE_Normal3v(const short *n)
{
	__POE_curVertP_n->nor[0] = (long)n[0] << 10;
	__POE_curVertP_n->nor[1] = (long)n[1] << 10;
	__POE_curVertP_n->nor[2] = (long)n[2] << 10;
	++__POE_curVertP_n;
}

//=====================================
inline void POE_Texture2v(const long *v){ __POE_curVertP_t->u = v[0]; __POE_curVertP_t->v = v[1]; ++__POE_curVertP_t; }
inline void POE_Texture2v(const US *v){ __POE_curVertP_t->u = v[0]; __POE_curVertP_t->v = v[1]; ++__POE_curVertP_t; }

//=====================================
inline void POE_EndPolygon(void)
{
	__POE_curPolyP->nVerts = __POE_curVertP-__POE_curPolyP->verts;
}

#endif
