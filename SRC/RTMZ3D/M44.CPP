//==============================================
// M44.HPP - matrix handler
// Copyright (C) Davide Pasca 1995
//==============================================

#include "EXTTYPES.HPP"
#include "M44.HPP"

static float identity[4][4]={
	1.,0.,0.,0.,
	0.,1.,0.,0.,
	0.,0.,1.,0.,
	0.,0.,0.,1.
};

//==================================
void m44_identity( float m[4][4] )
{
	memcpytiny( m, identity, 16*sizeof(float) );
}

//==================================
void m44_scale( float m[4][4], float x, float y, float z )
{
	m[0][0] *= x;  m[1][0] *= x;  m[2][0] *= x;  m[3][0] *= x;
	m[0][1] *= y;  m[1][1] *= y;  m[2][1] *= y;  m[3][1] *= y;
	m[0][2] *= z;  m[1][2] *= z;  m[2][2] *= z;  m[3][2] *= z;
}

//==================================
void m44_multiply( float d[4][4], const float s1[4][4], const float s2[4][4] )
{
short	i;
float	tmp[16], *tmpP;

	tmpP = tmp;
	for(i=0; i < 4; ++i)
	{
	float	xi0 = s1[i][0], xi1 = s1[i][1], xi2 = s1[i][2], xi3 = s1[i][3];

		tmpP[0] = xi0 * s2[0][0] + xi1 * s2[1][0] + xi2 * s2[2][0] + xi3 * s2[3][0];
		tmpP[1] = xi0 * s2[0][1] + xi1 * s2[1][1] + xi2 * s2[2][1] + xi3 * s2[3][1];
		tmpP[2] = xi0 * s2[0][2] + xi1 * s2[1][2] + xi2 * s2[2][2] + xi3 * s2[3][2];
		tmpP[3] = xi0 * s2[0][3] + xi1 * s2[1][3] + xi2 * s2[2][3] + xi3 * s2[3][3];
		tmpP += 4;
	}
	memcpytiny( d, tmp, 16*sizeof(float) );
}

//-------------------------------------
static inline float det2x2(float a, float b, float c, float d){ return a * d - b * c; }
//-------------------------------------
static float det3x3(float a1, float a2, float a3, float b1, float b2, float b3, float c1, float c2, float c3)
{
    return	a1 * det2x2 (b2, b3, c2, c3) - b1 * det2x2 (a2, a3, c2, c3) + c1 * det2x2 (a2, a3, b2, b3);
}
//-------------------------------------
static float det4x4(float mat[4][4])
{
float a1, a2, a3, a4, b1, b2, b3, b4, c1, c2, c3, c4, d1, d2, d3, d4;

    a1 = mat[0][0]; b1 = mat[0][1];
    c1 = mat[0][2]; d1 = mat[0][3];

    a2 = mat[1][0]; b2 = mat[1][1];
    c2 = mat[1][2]; d2 = mat[1][3];

    a3 = mat[2][0]; b3 = mat[2][1];
    c3 = mat[2][2]; d3 = mat[2][3];

    a4 = mat[3][0]; b4 = mat[3][1];
    c4 = mat[3][2]; d4 = mat[3][3];

	return	a1 * det3x3 (b2, b3, b4, c2, c3, c4, d2, d3, d4) -
			b1 * det3x3 (a2, a3, a4, c2, c3, c4, d2, d3, d4) +
			c1 * det3x3 (a2, a3, a4, b2, b3, b4, d2, d3, d4) -
			d1 * det3x3 (a2, a3, a4, b2, b3, b4, c2, c3, c4);
}
//-------------------------------------
static void adjoint(float mat[4][4])
{
float	a1, a2, a3, a4, b1, b2, b3, b4;
float	c1, c2, c3, c4, d1, d2, d3, d4;

    a1 = mat[0][0]; b1 = mat[0][1];
    c1 = mat[0][2]; d1 = mat[0][3];

    a2 = mat[1][0]; b2 = mat[1][1];
    c2 = mat[1][2]; d2 = mat[1][3];

    a3 = mat[2][0]; b3 = mat[2][1];
    c3 = mat[2][2]; d3 = mat[2][3];

    a4 = mat[3][0]; b4 = mat[3][1];
    c4 = mat[3][2]; d4 = mat[3][3];

    /* row column labeling reversed since we transpose rows & columns */
    mat[0][0]  =  det3x3 (b2, b3, b4, c2, c3, c4, d2, d3, d4);
    mat[1][0]  = -det3x3 (a2, a3, a4, c2, c3, c4, d2, d3, d4);
    mat[2][0]  =  det3x3 (a2, a3, a4, b2, b3, b4, d2, d3, d4);
    mat[3][0]  = -det3x3 (a2, a3, a4, b2, b3, b4, c2, c3, c4);

    mat[0][1]  = -det3x3 (b1, b3, b4, c1, c3, c4, d1, d3, d4);
    mat[1][1]  =  det3x3 (a1, a3, a4, c1, c3, c4, d1, d3, d4);
    mat[2][1]  = -det3x3 (a1, a3, a4, b1, b3, b4, d1, d3, d4);
    mat[3][1]  =  det3x3 (a1, a3, a4, b1, b3, b4, c1, c3, c4);

    mat[0][2]  =  det3x3 (b1, b2, b4, c1, c2, c4, d1, d2, d4);
    mat[1][2]  = -det3x3 (a1, a2, a4, c1, c2, c4, d1, d2, d4);
    mat[2][2]  =  det3x3 (a1, a2, a4, b1, b2, b4, d1, d2, d4);
    mat[3][2]  = -det3x3 (a1, a2, a4, b1, b2, b4, c1, c2, c4);

    mat[0][3]  = -det3x3 (b1, b2, b3, c1, c2, c3, d1, d2, d3);
    mat[1][3]  =  det3x3 (a1, a2, a3, c1, c2, c3, d1, d2, d3);
    mat[2][3]  = -det3x3 (a1, a2, a3, b1, b2, b3, d1, d2, d3);
    mat[3][3]  =  det3x3 (a1, a2, a3, b1, b2, b3, c1, c2, c3);
}
//-------------------------------------
void m44_invert( float m[4][4] )
{
float	det;

    det = det4x4(m);
    if ( ABS(det) < .00001 )	return;
	det = 1. / det;

    adjoint(m);

	float	*mP = (float *)m;
	for (long i=16; i; --i)
		*mP++ *= det;
}



