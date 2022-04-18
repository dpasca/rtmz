//==============================================
// QUA.CPP - quaternions stuff
// Copyright (C) by Davide Pasca 1995
//==============================================

#include "CVEC3.HPP"

//=======================================
void QUA_QuatFromAxis(float q[4], float phi, const float a[3])
{
	vec3_equ( q, a );
    vec3_normalize( q );
	phi /= 2.0;
    vec3_mul(q, q, sin(phi));
    q[3] = cos(phi);
}

//=======================================
void QUA_Normalize(float q[4])
{
float mag;

	mag = 100000000.;
    if ( mag = q[0]*q[0] + q[1]*q[1] + q[2]*q[2] + q[3]*q[3] )
		mag = 1./sqrt(mag);
    q[0] *= mag; q[1] *= mag; q[2] *= mag; q[3] *= mag;
}

//=======================================
#define RENORMCOUNT 0
void QUA_Mul(float dest[4], const float q1[4], const float q2[4])
{
//static long	count;
float		t1[3], t2[3];

	vec3_mul(t1,q1,q2[3]);
	vec3_mul(t2,q2,q1[3]);
	vec3_xprd(dest,q2,q1);
	vec3_add(dest,dest,t1);
	vec3_add(dest,dest,t2);
    dest[3] = q1[3] * q2[3] - vec3_dot(q1,q2);

	QUA_Normalize(dest);
    /*if (++count > RENORMCOUNT)
	{
        count = 0;
        QUA_Normalize(dest);
    }*/
}

//=======================================
void QUA_QuatToMat(const float q[4], float m[4][4])
{
float	q0s=q[0]*q[0], q1s=q[1]*q[1], q2s=q[2]*q[2];
float	q01=q[0]*q[1], q23=q[2]*q[3], q03=q[0]*q[3];
float	q12=q[1]*q[2], q02=q[0]*q[2], q13=q[1]*q[3];

    m[0][0] = 1.0 - 2.0 * (q1s + q2s);
    m[1][0] = 2.0 * (q01 - q23);
    m[2][0] = 2.0 * (q02 + q13);
    m[3][0] = 0.0;

    m[0][1] = 2.0 * (q01 + q23);
    m[1][1] = 1.0 - 2.0 * (q2s + q0s);
    m[2][1] = 2.0 * (q12 - q03);
    m[3][1] = 0.0;

    m[0][2] = 2.0 * (q02 - q13);
    m[1][2] = 2.0 * (q12 + q03);
    m[2][2] = 1.0 - 2.0 * (q1s + q0s);
    m[3][2] = 0.0;

    m[0][3] = 0.0;    m[1][3] = 0.0;
    m[2][3] = 0.0;    m[3][3] = 1.0;
}


