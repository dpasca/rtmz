//==============================================
// GL_MATR.CPP
// Copyright (C) Davide Pasca 1995-97
//
// See "readme.txt" for other credits
//
// TABS=4
//==============================================
#include <mem.h>
#include <stdio.h>
#include "EXTTYPES.HPP"
#include "M44.HPP"
#include "GL_MATR.HPP"
#include "CLP.HPP"
#include "TRIG.HPP"

static GLMatrix_t _identity=
{
	{
		{1.0,0.0,0.0,0.0},
		{0.0,1.0,0.0,0.0},
		{0.0,0.0,1.0,0.0},
		{0.0,0.0,0.0,1.0}
	}
,1};

static GLMatrix_t _glModelMat=
{
	{
		{1.0,0.0,0.0,0.0},
		{0.0,1.0,0.0,0.0},
		{0.0,0.0,1.0,0.0},
		{0.0,0.0,0.0,1.0}
	}
,1};

GLMatrix_t _glProjMat=
{
	{
		{1.0,0.0,0.0,0.0},
		{0.0,1.0,0.0,0.0},
		{0.0,0.0,1.0,0.0},
		{0.0,0.0,0.0,1.0}
	}
,1};
static GLMatrix_t _glTransMat=
{
	{
		{1.0,0.0,0.0,0.0},
		{0.0,1.0,0.0,0.0},
		{0.0,0.0,1.0,0.0},
		{0.0,0.0,0.0,1.0}
	}
,1};

GLMatrix_t	*_glCurMatP=&_glModelMat;

#define ACT	_glCurMatP
#define HEY	_glCurMatP->mf

//=======================================
#define  MAGIC  (((65536. * 65536. * 16) + (65536.*.5)) * 65536.)
static inline long float2int( float d )
{
  double dtemp = MAGIC + d;
  return (*(long *)&dtemp) - 0x80000000;
}

//============================
GLMatrix_t *_glGetTransMatrix(void)
{
	return &_glTransMat;
}
//============================
GLMatrix_t *_glGetMatrix(void)
{
	return &_glTransMat;
}

//============================
static long	_mattop;
static GLMatrix_t _matstack[GL_MAX_MATR_DEEP];

void glPushMatrix(void)
{
	if (_mattop < GL_MAX_MATR_DEEP)
		_matstack[_mattop++] = *ACT;
	else
		printf( "GL_MATR.C - glPushMatrix() overflow\n" );
}

//----------------------------
void glPopMatrix(void)
{
	if (_mattop > 0)
		*ACT = _matstack[--_mattop];
	else
		printf( "GL_MATR.C - glPopMatrix() overflow\n" );
}

//============================
void glMatrixModel(UL mode)
{
	switch ( mode )
	{
	case GL_PROJECTION:	ACT = &_glProjMat;	break;
	case GL_MODELVIEW:	ACT = &_glModelMat;	break;
	}
}

//============================
void glLoadIdentity(void){ *ACT = _identity; }
void glLoadMatrix(const float *m){	memcpytiny(ACT->mf,m, sizeof(float)*4*4); ACT->iIsUpdated = 0; }
void glSaveMatrix(float *m){		memcpytiny(m,ACT->mf, sizeof(float)*4*4); ACT->iIsUpdated = 0; }
//============================
void glMultMatrix(const float *m)
{
	m44_multiply( HEY, (const float (*)[4])HEY, (const float (*)[4])m );
	ACT->iIsUpdated = 0;
}
//============================
void glMultMatrixInv(const float *m)
{
	m44_multiply( HEY, (const float (*)[4])m, (const float (*)[4])HEY );
	ACT->iIsUpdated = 0;
}
//============================
void glTranslate(float x, float y, float z)
{
	HEY[0][3] += x*HEY[0][0] + y*HEY[0][1] + z*HEY[0][2];
	HEY[1][3] += x*HEY[1][0] + y*HEY[1][1] + z*HEY[1][2];
	HEY[2][3] += x*HEY[2][0] + y*HEY[2][1] + z*HEY[2][2];
	HEY[3][3] += x*HEY[3][0] + y*HEY[3][1] + z*HEY[3][2];
	ACT->iIsUpdated = 0;
}

//============================
void glScale( float x, float y, float z )	// yes, can be optimized as for the rotation
{
float	m[16];

#define M(Y,X)	m[Y*4+X]
   M(0,0) = x;  M(0,1) = 0;  M(0,2) = 0;  M(0,3) = 0;
   M(1,0) = 0;  M(1,1) = y;  M(1,2) = 0;  M(1,3) = 0;
   M(2,0) = 0;  M(2,1) = 0;  M(2,2) = z;  M(2,3) = 0;
   M(3,0) = 0;  M(3,1) = 0;  M(3,2) = 0;  M(3,3) = 1;
#undef M
	glMultMatrix( m );
}

//============================
UL	_vpWD, _vpHE;
void glViewPort( long x, long y, UL wd, UL he )
{
	x = y;	// dont annoy me about unused variables
	_vpWD = wd;
	_vpHE = he;
}

//============================
void glOrtho( float l, float r, float b, float t, float n, float f )
{
static float	m[4][4]={
1.,0.,0.,0.,
0.,1.,0.,0.,
0.,0.,1.,0.,
0.,0.,0.,1. };
float	rl, tb, fn;

	if ( (rl = r - l) == 0. || (tb = t - b) == 0. || (fn = f - n) == 0. )
		return;
	m[0][0] = 2 / rl;
	m[1][1] = 2 / tb;
	m[2][2] = - 2 / fn;

	m[0][3] = - ( r + l ) / rl;
	m[1][3] = - ( t + b ) / tb;
	m[2][3] = - ( f + n ) / fn;

	glMultMatrix( (float *)m );
}

//============================
void glFrustum( float l, float r, float b, float t, float n, float f )
{
float	rl, tb, fn;

	if ( (rl = r - l) == 0. || (tb = t - b) == 0. || (fn = f - n) == 0. )
		return;

float	m[4][4]={
	2. * n / rl, 0.,			 (r + l) / rl,	 0.,
	0.,			 2. * n / tb,	 (t + b) / tb,	 0.,
	0.,			 0.,			-(f + n) / fn,	-2. * f * n / fn,
	0.,			 0.,			-1.,			 0. };

	glMultMatrix( (float *)m );
}

//============================
void gluPerspective(float fovy, float aspect, float n, float f)
{
float	bot;

	fovy = TR_GR2RAD(fovy) / 2.;
	if NOT( bot = cos( fovy ) )
		bot = 10000.;
	else
		bot = sin( fovy ) / bot;

	aspect *= bot;
	glFrustum( -aspect, aspect, -bot, bot, n, f );
}

//============================
void glRotate(float angle, char axis)
{
float	cosa, sina, t;
long	i;

	cosa = tr_fcosf(angle);
	sina = tr_fsinf(angle);

	switch(axis)
	{
	case 'x':
	case 'X':	for (i = 0 ; i < 4 ; ++i)
				{	t = HEY[i][1];
					HEY[i][1] = t*cosa - HEY[i][2]*sina;
					HEY[i][2] = t*sina + HEY[i][2]*cosa;
				}
				break;

	case 'y':
	case 'Y':	for (i = 0 ; i < 4 ; ++i)
				{	t = HEY[i][0];
					HEY[i][0] = t*cosa - HEY[i][2]*sina;
					HEY[i][2] = t*sina + HEY[i][2]*cosa;
				}
				break;

	case 'z':
	case 'Z':	for (i = 0 ; i < 4 ; ++i)
				{	t = HEY[i][0];
					HEY[i][0] = t*cosa - HEY[i][1]*sina;
					HEY[i][1] = t*sina + HEY[i][1]*cosa;
				}
				break;
	}
	ACT->iIsUpdated = 0;
}

//=======================================
static UB _glCreateTransMat(void)
{
	if ( !_glProjMat.iIsUpdated || !_glModelMat.iIsUpdated )
	{
		_glProjMat.iIsUpdated = _glModelMat.iIsUpdated = 1;
		m44_multiply( _glTransMat.mf, (const float (*)[4])_glProjMat.mf,
									  (const float (*)[4])_glModelMat.mf );
		return 1;
	}
	return 0;
}

//=======================================
void _glMatrixPrint(void)
{
	for(short j=0; j < 4; ++j)
	{
		for(short i=0; i < 4; ++i)
			printf("% 3.4f ", ACT->mf[j][i] );

		//printf("   ");
		//for(i=0; i < 4; ++i)
		//	printf("%4ld ", ACT->mi[j][i] );

		printf("\n");
	}
	printf("\n");
}

//=======================================
void _glMatrixPrint(float m[4][4])
{
	for(short j=0; j < 4; ++j)
	{
		for(short i=0; i < 4; ++i)
			printf("% 3.4f ", m[j][i] );
		printf("\n");
	}
	printf("\n");
}

//=======================================
void glInvertMatrix(void)
{
	m44_invert(HEY);
	ACT->iIsUpdated = 0;
}

//=======================================
void _glTransform3X3F( float *vP, const float *sgP, long cnt )
{
GLMatrix_t	*m;
float		srcx,srcy,srcz;
float		m00,m01,m02;
float		m10,m11,m12;
float		m20,m21,m22;

	if (cnt<=0)	return;

	m = ACT;
	m00 = m->mf[0][0];  m01 = m->mf[0][1];  m02 = m->mf[0][2];
	m10 = m->mf[1][0];  m11 = m->mf[1][1];  m12 = m->mf[1][2];
	m20 = m->mf[2][0];  m21 = m->mf[2][1];  m22 = m->mf[2][2];

	while( cnt-- )
	{
		srcx = sgP[0];
		srcy = sgP[1];
		srcz = sgP[2];
		vP[0] = m00 * srcx + m01 * srcy + m02 * srcz;
		vP[1] = m10 * srcx + m11 * srcy + m12 * srcz;
		vP[2] = m20 * srcx + m21 * srcy + m22 * srcz;

		sgP += 3;
		vP += 3;
	}
}

//=======================================
void glTransform3X4( float *vP, const float *sgP, long cnt )
{
float	x,y,z;
float	*m;

	if (cnt<=0)	return;

	m = (float *)ACT->mf;
#define M(Y,X)	m[Y*4+X]
	while( cnt-- )
	{
		x = sgP[0];
		y = sgP[1];
		z = sgP[2];
		vP[0] = M(0,0) * x + M(0,1) * y + M(0,2) * z + M(0,3);
		vP[1] = M(1,0) * x + M(1,1) * y + M(1,2) * z + M(1,3);
		vP[2] = M(2,0) * x + M(2,1) * y + M(2,2) * z + M(2,3);
		sgP += 3;
		vP += 3;
	}
#undef M
}

//=======================================
extern long float_cmp(long a, long b);
#pragma aux float_cmp =   \
"MOV ECX,EAX" \
"MOV EDX,EBX" \
"SAR ECX,31" \
"AND EAX,0x7FFFFFFF" \
"SAR EDX,31" \
"AND EBX,0x7FFFFFFF" \
"XOR EAX,ECX" \
"XOR EBX,EDX" \
"SUB EAX,ECX" \
"SUB EBX,EDX" \
"SUB EAX,EBX" \
parm caller [eax][ebx]\
modify [eax ebx ecx edx]\
value [eax];

//=======================================
inline UB CLP_Code4( const float *v )
{
float	nw = -v[3];
UB		code=0;

	if (v[0] < nw)		code |= CLP_CODE_X1;
	if (v[0] > v[3])	code |= CLP_CODE_X2;

	if (v[1] < nw)		code |= CLP_CODE_Y1;
	if (v[1] > v[3])	code |= CLP_CODE_Y2;

	if (v[2] < nw)		code |= CLP_CODE_Z1;
/*
long	nw = v[3] ^ 0x80000000;

	if ( float_cmp(v[0],nw) < 0 )	code |= CLP_CODE_X1;
	if ( float_cmp(v[0],v[3]) > 0 )	code |= CLP_CODE_X2;

	if ( float_cmp(v[1],nw) < 0 )	code |= CLP_CODE_Y1;
	if ( float_cmp(v[1],v[3]) > 0 )	code |= CLP_CODE_Y2;

	if ( float_cmp(v[2],nw) < 0 )	code |= CLP_CODE_Z1;
*/
	//if (nw > v[2])		code |= CLP_CODE_Z1;
	//if (v[2] < v[3]/2.)	code |= CLP_CODE_Z2;

	return code;
}

//=======================================
#define ZCOE	(float)(1<<30)
#define ZCOEI	(1<<30)

void _glPOEVertsToScreen( POE_Vert_t *vertsP, long cnt )
{
float	hwdf = (_vpWD-.5) * 8. / ZCOE, hhef = (_vpHE-.5) * 8. / ZCOE;
long	hwd = _vpWD << 3, hhe = _vpHE << 3;
float	w;

	for(; cnt > 0; --cnt, ++vertsP)
	{
		if ( (w = vertsP->eye[3]) <= 0. )
			w = (ZCOE-1.);
		else
			w = (ZCOE-1.) / w;

		vertsP->screen[0] = float2int(hwdf*w * vertsP->eye[0]) + hwd;
		vertsP->screen[1] = hhe - float2int(hhef*w * vertsP->eye[1]);

		vertsP->sz = ZCOEI-1 - float2int( w * vertsP->eye[2]);
		if ( vertsP->sz < 0 )
			vertsP->sz = 0;
		if ( vertsP->sz > ZCOEI-1 )
			vertsP->sz = ZCOEI-1;
	}
}

//=======================================
void _glTransformO3DtoPOE( O3D_VertBase_t *bvertP, POE_Vert_t *pvertP, long cnt )
{
	if (cnt<=0)	return;
	_glCreateTransMat();

#define M		_glTransMat.mf

	float	hwdf = (_vpWD-.5) * 8. / ZCOE, hhef = (_vpHE-.5) * 8. / ZCOE;
	long	hwd = _vpWD << 3, hhe = _vpHE << 3;

	for (; cnt > 0; --cnt, ++bvertP, ++pvertP)
	{
	float	x, y, z, w;

		if NOT( bvertP->doTrans )	continue;
		bvertP->doTrans = 0;

		x = bvertP->vert[0];
		y = bvertP->vert[1];
		z = bvertP->vert[2];
		float	*e=pvertP->eye;
		e[0] = M[0][0] * x + M[0][1] * y + M[0][2] * z + M[0][3];
		e[1] = M[1][0] * x + M[1][1] * y + M[1][2] * z + M[1][3];
		e[2] = M[2][0] * x + M[2][1] * y + M[2][2] * z + M[2][3];
		e[3] = M[3][0] * x + M[3][1] * y + M[3][2] * z + M[3][3];
		w = e[3];
		if ( !(*((long *)&w)*2) || (*((long *)&w) & 0x80000000) )
			w = (ZCOE-1.);
		else
			w = (ZCOE-1.) / w;

		if NOT( pvertP->clipCode = CLP_Code4( e ) )
		{
			pvertP->screen[0] = float2int(hwdf*w * e[0]) + hwd;
			pvertP->screen[1] = hhe - float2int(hhef*w * e[1]);
		}

		pvertP->sz = ZCOEI-1 - float2int( w * e[2] );
		if ( pvertP->sz < 0 )
			pvertP->sz = 0;
		if ( pvertP->sz > ZCOEI-1 )
			pvertP->sz = ZCOEI-1;
	}
#undef M
}

//=======================================
void _glTransformO3DtoPOENormals( O3D_VertBase_t *bvertP, POE_Vert_t *pvertP, long cnt )
{
	if (cnt<=0)	return;
	_glCreateTransMat();

#define M		_glTransMat.mf

	float	hwdf = (_vpWD-.5) * 8. / ZCOE, hhef = (_vpHE-.5) * 8. / ZCOE;
	long	hwd = _vpWD << 3, hhe = _vpHE << 3;

	for (; cnt > 0; --cnt, ++bvertP, ++pvertP)
	{
	float	x, y, z, w;

		if NOT( bvertP->doTrans )	continue;
		bvertP->doTrans = 0;

		x = bvertP->nor[0];
		y = bvertP->nor[1];
		z = bvertP->nor[2];
 		pvertP->nor[0] = M[0][0] * x + M[0][1] * y + M[0][2] * z;
		pvertP->nor[1] = M[1][0] * x + M[1][1] * y + M[1][2] * z;
		pvertP->nor[2] = M[2][0] * x + M[2][1] * y + M[2][2] * z;
		vec3_normalize( pvertP->nor );

		x = bvertP->vert[0];
		y = bvertP->vert[1];
		z = bvertP->vert[2];
		float	*e=pvertP->eye;
		e[0] = M[0][0] * x + M[0][1] * y + M[0][2] * z + M[0][3];
		e[1] = M[1][0] * x + M[1][1] * y + M[1][2] * z + M[1][3];
		e[2] = M[2][0] * x + M[2][1] * y + M[2][2] * z + M[2][3];
		e[3] = M[3][0] * x + M[3][1] * y + M[3][2] * z + M[3][3];
		w = e[3];
		if ( !(*((long *)&w)*2) || (*((long *)&w) & 0x80000000) )
			w = (ZCOE-1.);
		else
			w = (ZCOE-1.) / w;

		if NOT( pvertP->clipCode = CLP_Code4( e ) )
		{
			pvertP->screen[0] = float2int(hwdf*w * e[0]) + hwd;
			pvertP->screen[1] = hhe - float2int(hhef*w * e[1]);
		}

		pvertP->sz = ZCOEI-1 - float2int( w * e[2] );
		if ( pvertP->sz < 0 )
			pvertP->sz = 0;
		if ( pvertP->sz > ZCOEI-1 )
			pvertP->sz = ZCOEI-1;
	}
#undef M
}

//=======================================
void glTransformv3f( const float *srcp, float *desp, long cnt )
{
	if (cnt<=0)	return;
	_glCreateTransMat();

#define M		_glTransMat.mf

	for (; cnt > 0; --cnt, ++srcp, ++desp)
	{
	float	x, y, z;

		x = srcp[0];
		y = srcp[1];
		z = srcp[2];
		desp[0] = M[0][0] * x + M[0][1] * y + M[0][2] * z + M[0][3];
		desp[1] = M[1][0] * x + M[1][1] * y + M[1][2] * z + M[1][3];
		desp[2] = M[2][0] * x + M[2][1] * y + M[2][2] * z + M[2][3];
		//e[3] = M[3][0] * x + M[3][1] * y + M[3][2] * z + M[3][3];
	}
#undef M
}

/*
	float	m00_01 = m00 * m01,
			m10_11 = m10 * m11,
			m20_21 = m20 * m21,
			m30_31 = m30 * m31;

		xy = x * y;

		v[3+0] = (x + m01) * (y + m00) - xy - m00_01 + z*m02 + m03;
		v[3+1] = (x + m11) * (y + m10) - xy - m10_11 + z*m12 + m13;
		v[3+2] = (x + m21) * (y + m20) - xy - m20_21 + z*m22 + m23;
		v[3+3] = (x + m31) * (y + m30) - xy - m30_31 + z*m32 + m33;
*/
/*
//=======================================
extern "C" void __glTransformNormal3iv( long *vP, const long *sgP, long cnt )
{
//extern glmat_t	*ACT;
glmat_t			*m;
float			srcx,srcy,srcz;
float	m00,m01,m02;
float	m10,m11,m12;
float	m20,m21,m22;

	if (cnt<=0)	return;

	__glIMatrixUpdate();

	m = ACT;
	m00 = m->mf[0][0];  m01 = m->mf[0][1];  m02 = m->mf[0][2];
	m10 = m->mf[1][0];  m11 = m->mf[1][1];  m12 = m->mf[1][2];
	m20 = m->mf[2][0];  m21 = m->mf[2][1];  m22 = m->mf[2][2];

	while( cnt-- )
	{
		srcx = *sgP++;
		srcy = *sgP++;
		srcz = *sgP++;
		*vP++ = m00 * srcx + m10 * srcy + m20 * srcz;
		*vP++ = m01 * srcx + m11 * srcy + m21 * srcz;
		*vP++ = m02 * srcx + m12 * srcy + m22 * srcz;
	}
}

//=======================================
extern "C" void __glTransform3iv( long *vP, const long *sgP, long cnt )
{
//extern glmat_t	*ACT;
glmat_t			*m;
float			srcx,srcy,srcz;
float	m00,m01,m02;
float	m10,m11,m12;
float	m20,m21,m22;
float	m30,m31,m32;


	if (cnt<=0)	return;

	__glIMatrixUpdate();

	m = ACT;
	m00 = m->mf[0][0];  m01 = m->mf[0][1];  m02 = m->mf[0][2];
	m10 = m->mf[1][0];  m11 = m->mf[1][1];  m12 = m->mf[1][2];
	m20 = m->mf[2][0];  m21 = m->mf[2][1];  m22 = m->mf[2][2];
	m30 = m->mf[3][0];  m31 = m->mf[3][1];  m32 = m->mf[3][2];

	while( cnt-- )
	{
		srcx = *sgP++;
		srcy = *sgP++;
		srcz = *sgP++;
		*vP++ = m00 * srcx + m10 * srcy + m20 * srcz + m30;
		*vP++ = m01 * srcx + m11 * srcy + m21 * srcz + m31;
		*vP++ = m02 * srcx + m12 * srcy + m22 * srcz + m32;
	}
}
*/
