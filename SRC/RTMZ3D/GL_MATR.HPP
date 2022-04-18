//==============================================
// GL_MATR.HPP - matrix handler
// Copyright (C) Davide Pasca 1995
//==============================================

#ifndef __GL_MATR_HPP__
#define __GL_MATR_HPP__

#include "POLYENG.HPP"

#define GL_MAX_MATR_DEEP	32
#define GL_PROJECTION		1
#define GL_MODELVIEW		2

typedef struct GLMatrix_t
{
 float  mf[4][4];
 char   iIsUpdated;
}GLMatrix_t;

extern GLMatrix_t	*_glCurMatP;

extern GLMatrix_t	*_glGetTransMatrix(void);
inline const GLMatrix_t	*glGetCurMatrix(void){ return _glCurMatP; }

void	glMatrixModel(UL mode);
void    glLoadIdentity (void);
void    glLoadMatrix(const float *m);
void    glSaveMatrix(float *m);

void    glPushMatrix(void);
void    glPopMatrix(void);

void    glTranslate(float x, float y, float z);
void	glScale( float x, float y, float z );
void    glRotate(float angle, char axis);
void    glMultMatrix(const float *m);
void	glMultMatrixInv(const float *m);
void	glInvertMatrix(void);

void	glViewPort( long x, long y, UL wd, UL he );
void	glOrtho( float l, float r, float b, float t, float n, float f );
void	glFrustum( float l, float r, float b, float t, float n, float f );
void	gluPerspective(float fovy, float aspect, float n, float f);
void	glTransformv3f( const float *srcp, float *desp, long cnt );

//===========================================================
inline void glTransform( float *vP, const float *sgP, long cnt )
{
void _M34_TransformF( float *vP, const float *sgP, long cnt );
	_M34_TransformF( vP, sgP, cnt );
}

//===========================================================
inline void glTransform3X3( float *vP, const float *sgP, long cnt )
{
void _glTransform3X3F( float *vP, const float *sgP, long cnt );
	_glTransform3X3F( vP, sgP, cnt );
}

void _glPOEVertsToScreen( POE_Vert_t *vertP, long cnt );

#include "OBJ3D.HPP"
void _glTransformO3DtoPOE( O3D_VertBase_t *bvertP, POE_Vert_t *pvertP, long cnt );
void _glTransformO3DtoPOENormals( O3D_VertBase_t *bvertP, POE_Vert_t *pvertP, long cnt );

void	_glMatrixPrint(void);
#endif
