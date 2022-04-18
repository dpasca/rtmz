//==============================================
// CLP.CPP - clipping routines
// Copyright (C) Davide Pasca 1995-97
//
// See "readme.txt" for other credits
//
// TABS=4
//==============================================
#include "EXTTYPES.HPP"
#include "GL_MATR.HPP"
#include "CLP.HPP"

static UL	_flags;

//=================================================
static void polyi2poly( const POE_PolyI_t *pi, POE_Poly_t *p )
{
const POE_Vert_t	*srcVertsP;
const long			*sidxP;
const UB			*srcColsP;
const short			*srcTxPosP;
POE_Vert_t			*dstVertP;
long				i;

	srcColsP = pi->c;
	srcTxPosP = (const short *)pi->txPos;
	sidxP = pi->vertIdx;
	srcVertsP = *pi->vertH;
	dstVertP = p->verts;

	p->userLong = pi->userLong;
	p->flags = pi->flags;
	p->clipCode = pi->clipCode;
	p->nVerts = pi->nVerts;

	if ( p->flags & POE_FLG_GSHADE )
	{
		for(i=p->nVerts; i > 0; --i, ++dstVertP)
		{
			*dstVertP = srcVertsP[*sidxP++];
			dstVertP->c = *srcColsP++;
		}
	}
	else
	if ( p->flags & POE_FLG_TEXTURE )
	{
		for(i=p->nVerts; i > 0; --i, ++dstVertP, srcTxPosP += 2)
		{
			*dstVertP = srcVertsP[*sidxP++];
			dstVertP->c = *srcColsP;
			dstVertP->u = srcTxPosP[0];
			dstVertP->v = srcTxPosP[1];
		}
	}
	else
	/*if ( p->flags & (POE_FLG_PSHADE | POE_FLG_TEXCHROME) )
	{
	O3D_VertBase_t	*bvertP = objP->baseVertsP;

		for(i=p->nVerts; i > 0; --i, ++dstVertP, ++sidxP)
		{
			*dstVertP = srcVertsP[*sidxP];
			vec3_equ( dstVertP->nor, bvertP[*sidxP].nor );
			dstVertP->c = *srcColsP;
		}
	}
	else*/
		for(i=p->nVerts; i > 0; --i, ++dstVertP)
		{
			*dstVertP = srcVertsP[*sidxP++];
			dstVertP->c = *srcColsP;
		}
}

//----------------------------------------------
static void poly2polyi( const POE_Poly_t *p, POE_PolyI_t *pi )
{
POE_Vert_t			*desVertsP, *tvp;
long				*didxP, i;
const POE_Vert_t	*srcVertP;
UB					*colP;

	pi->userLong = p->userLong;
	pi->flags = p->flags;
	pi->clipCode = p->clipCode;
	pi->nVerts = p->nVerts;

	didxP = pi->vertIdx;
	desVertsP = *pi->vertH;
	srcVertP = p->verts;
	colP = pi->c;

	if ( p->flags & POE_FLG_GSHADE )
	{
		for(i=p->nVerts; i > 0; --i)
		{
			tvp = &desVertsP[*didxP++];
			*tvp = *srcVertP++;
			*colP++ = tvp->c;
		}
	}
	else
	if ( p->flags & POE_FLG_TEXTURE )
	{
	short	*txP = (short *)pi->txPos;

		*colP = srcVertP->c;
		for(i=p->nVerts; i > 0; --i, txP += 2)
		{
			tvp = &desVertsP[*didxP++];
			*tvp = *srcVertP++;
			txP[0] = tvp->u;
			txP[1] = tvp->v;
		}
	}
	/*else
	if ( p->flags & (POE_FLG_PSHADE | POE_FLG_TEXCHROME) )
	{
	O3D_VertBase_t	*bvertP = objP->baseVertsP;

		for(i=p->nVerts; i > 0; --i, ++dstVertP, ++sidxP)
		{
			tvp = &desVertsP[*didxP++];
			*tvp = *srcVertP++;
			txP[0] = tvp->u;
			txP[1] = tvp->v;

			*dstVertP = srcVertsP[*sidxP];
			vec3_equ( dstVertP->nor, bvertP[*sidxP].nor );
			dstVertP->c = *srcColsP;
		}
	}*/
	else
	{	*colP = srcVertP->c;
		for(i=p->nVerts; i > 0; --i)
			desVertsP[*didxP++] = *srcVertP++;
	}
}

//==============================================
#define CLERP(ELEM)		(newP->ELEM = v1P->ELEM + coe * (v2P->ELEM - v1P->ELEM))
static void clipRecalcVert( POE_Vert_t *newP, POE_Vert_t *v1P, POE_Vert_t *v2P, float coe )
{
	CLERP( eye[0] );
	CLERP( eye[1] );
	CLERP( eye[2] );
	CLERP( eye[3] );

	//if ( _flags & POE_FLG_GSHADE )
		CLERP( c );

	if ( _flags & POE_FLG_TEXTURE )
	{
		CLERP( u );
		CLERP( v );
	}

	if ( _flags & (POE_FLG_PSHADE | POE_FLG_TEXCHROME) )
	{
		CLERP( nor[0] );
		CLERP( nor[1] );
		CLERP( nor[2] );
	}
}

//----------------------------------------------
static void clipSide(POE_Poly_t *p, POE_Poly_t *q, long index, float sign)
{
float		tu, tv;
POE_Vert_t	*v, *u;
long		i;
UB			cur_is_inside, old_is_inside;

	q->nVerts = 0;		// start with u=vert[n-1], v=vert[0]
	u = &p->verts[ p->nVerts - 1 ];

	tu = sign * u->eye[index] - u->eye[3];
	old_is_inside = (tu<=0.);

	for (i=p->nVerts, v=p->verts; i > 0; --i, ++v)
	{
		tv = sign * v->eye[index] - v->eye[3];
		cur_is_inside = (tv<=0.);

		if (old_is_inside ^ cur_is_inside)
			clipRecalcVert( &q->verts[q->nVerts++], u, v, tu / (tu - tv) );

		if (cur_is_inside)
			q->verts[q->nVerts++] = *v;

		old_is_inside = cur_is_inside;
		tu = tv;
		u = v;
	}
}
//----------------------------------------------
#define CLIP_AND_SWAP(O,N,I,S)	{ clipSide(O,N,I,S); SWAP(O,N,t); }

//==============================================
static POE_Vert_t	_clp_verts[POE_VERTBUFSIZ];
static POE_Vert_t	*_clp_vertsP=_clp_verts;
static POE_PolyI_t	_clp_poly=
{
	&_clp_vertsP,
	0,1,2,3,4,5,6,7,8,9,10,11,12,POE_VERTBUFSIZ-1
};

const POE_PolyI_t *CLP_PolyI4D( const POE_PolyI_t *srcpolyP )
{
POE_Poly_t	poly1, poly2;
POE_Poly_t	*o=&poly1, *n=&poly2, *t;
UB			code;

	if NOT( code = srcpolyP->clipCode )
		return srcpolyP;

	polyi2poly( srcpolyP, o );
	_flags = n->flags = o->flags;

	if (code & CLP_CODE_X1)	CLIP_AND_SWAP( o, n, 0, -1. );
	if (code & CLP_CODE_X2)	CLIP_AND_SWAP( o, n, 0,  1. );
	if (code & CLP_CODE_Y1)	CLIP_AND_SWAP( o, n, 1, -1. );
	if (code & CLP_CODE_Y2)	CLIP_AND_SWAP( o, n, 1,  1. );
	if (code & CLP_CODE_Z1)	CLIP_AND_SWAP( o, n, 2, -1. );

	_glPOEVertsToScreen( o->verts, o->nVerts );

	poly2polyi( o, &_clp_poly );
	_clp_poly.materialID = srcpolyP->materialID;

	return &_clp_poly;
}
