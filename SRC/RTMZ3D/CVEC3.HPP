//==============================================
// CVEC3.HPP -
// Copyright (C) Davide Pasca 1995
//==============================================

#ifndef __CVEC3_HPP__
#define __CVEC3_HPP__

#include <math.h>

//================================
#define VEC3_OPRVVV(A,B,OP,C)	{	(A)[0]=(B)[0] OP (C)[0];\
									(A)[1]=(B)[1] OP (C)[1];\
									(A)[2]=(B)[2] OP (C)[2];	}

#define VEC3_OPRVVS(A,B,OP,C)	{	(A)[0]=(B)[0] OP (C);\
									(A)[1]=(B)[1] OP (C);\
									(A)[2]=(B)[2] OP (C);	}

//================================
inline void vec3_div( long *c, const long *b, long a )
{
	VEC3_OPRVVS(c,b,/,a);
}
//================================
inline void vec3_div( float *c, const float *b, float a )
{
	VEC3_OPRVVS(c,b,/,a);
}

//================================
inline void vec3_mul( float *c, const float *b, const float *a ){ VEC3_OPRVVV(c,b,*,a);}
inline void vec3_mul( double *c, const double *b, const double *a ){ VEC3_OPRVVV(c,b,*,a);}
inline void vec3_mul( long *c, const double *b, const double *a ){ VEC3_OPRVVV(c,b,*,a);}
//================================
inline void vec3_mul( float *c, const float *b, float a ){ VEC3_OPRVVS(c,b,*,a); }
inline void vec3_mul( double *c, const double *b, double a ){ VEC3_OPRVVS(c,b,*,a); }
inline void vec3_mul( long *c, const double *b, double a ){ VEC3_OPRVVS(c,b,*,a); }

//================================
inline void vec3_shl( short *c, const short *b, short a ){	VEC3_OPRVVS(c,b,<<,a); }
inline void vec3_shl( long *c, const long *b, long a ){	VEC3_OPRVVS(c,b,<<,a); }
inline void vec3_shr( short *c, const short *b, short a ){	VEC3_OPRVVS(c,b,>>,a); }
inline void vec3_shr( long *c, const long *b, long a ){	VEC3_OPRVVS(c,b,>>,a); }

//================================
inline void vec3_set( long *c, long x,long y,long z){ c[0] = x; c[1] = y; c[2] = z; }
inline void vec3_set( float *c, float x,float y,float z){ c[0] = x; c[1] = y; c[2] = z; }
inline void vec3_set( double *c, double x,double y,double z){ c[0] = x; c[1] = y; c[2] = z; }
//================================
#define VEC3_EQU(D,S)	{D[0]=S[0];D[1]=S[1];D[2]=S[2];}
inline void vec3_equ( short *c, const short *a)	VEC3_EQU(c,a)
inline void vec3_equ( short *c, const long *a)	VEC3_EQU(c,a)
inline void vec3_equ( short *c, const float *a)	VEC3_EQU(c,a)
inline void vec3_equ( short *c, const double *a)	VEC3_EQU(c,a)
inline void vec3_equ( long *c, const long *a)	VEC3_EQU(c,a)
inline void vec3_equ( long *c, const float *a)	VEC3_EQU(c,a)
inline void vec3_equ( long *c, const double *a)	VEC3_EQU(c,a)
inline void vec3_equ( float *c, const short *a)	VEC3_EQU(c,a)
inline void vec3_equ( float *c, const long *a)	VEC3_EQU(c,a)
inline void vec3_equ( float *c, const float *a)	VEC3_EQU(c,a)
inline void vec3_equ( float *c, const double *a)	VEC3_EQU(c,a)
inline void vec3_equ( double *c, const float *a)	VEC3_EQU(c,a)
inline void vec3_equ( double *c, const double *a)	VEC3_EQU(c,a)

//================================
inline void vec3_add( short *c, const short *a, const short *b){ VEC3_OPRVVV(c,a,+,b);}
inline void vec3_add( long *c, const long *a, const long *b){ VEC3_OPRVVV(c,a,+,b);}
inline void vec3_add( float *c, const float *a, const float *b){ VEC3_OPRVVV(c,a,+,b);}

//================================
inline void vec3_sub( long *c, const long *a, const long *b){ VEC3_OPRVVV(c,a,-,b); }
inline void vec3_sub( float *c, const float *a, const float *b){ VEC3_OPRVVV(c,a,-,b); }
inline void vec3_sub( double *c, const double *a, const double *b){ VEC3_OPRVVV(c,a,-,b); }

//---------------------------------
#define	VEC3_XPRD(D,A,B,T)\
	T[0] = A[1] * B[2] - A[2] * B[1]; \
	T[1] = A[2] * B[0] - A[0] * B[2]; \
	T[2] = A[0] * B[1] - A[1] * B[0]; \
	VEC3_EQU(D,T);

inline void vec3_xprd( long *d, const long *a, const long *b )
{
long	t[3];	VEC3_XPRD(d,a,b,t);
}     
//---------------------------------
inline void vec3_xprd( float *d, const float *a, const float *b )
{
float	t[3];	VEC3_XPRD(d,a,b,t);
}
//---------------------------------
inline void vec3_xprd( double *d, const double *a, const double *b )
{
double	t[3];	VEC3_XPRD(d,a,b,t);
}     
//---------------------------------
inline void vec3_xprd( float *d, const short *a, const short *b )
{
float	t[3];	VEC3_XPRD(d,a,b,t);
}     
//---------------------------------
inline void vec3_xprd( float *d, const long *a, const long *b )
{
float	t[3];	VEC3_XPRD(d,a,b,t);
}     

//=================================
inline float vec3_dot( const float *a, const float *b ){return a[0]*b[0]+a[1]*b[1]+a[2]*b[2];}

//=================================
inline long vec3_mag( const long *a ){ return sqrt( (float)(a[0]*a[0] + a[1]*a[1] + a[2]*a[2]) ); }
inline float vec3_mag( const float *a ){ return sqrt( a[0]*a[0] + a[1]*a[1] + a[2]*a[2] ); }
inline double vec3_mag( const double *a ){ return sqrt( a[0]*a[0] + a[1]*a[1] + a[2]*a[2] ); }

//=================================
inline void vec3_normalize( short *a )
{
float	b[3], mag;

	vec3_equ( b, a );

	if ( (mag = vec3_mag( b )) == 0. )
		mag = 1e+20;
	else
		mag = 1024. / mag;

	vec3_mul( b, b, mag );
	vec3_equ( a, b );
}
//---------------------------------
inline void vec3_normalize( long *a )
{
float	b[3], mag;

	vec3_equ( b, a );

	if ( (mag = vec3_mag( b )) == 0. )
		mag = 1e+20;
	else
		mag = 65536. / mag;

	vec3_mul( b, b, mag );
	vec3_equ( a, b );
}
//---------------------------------
#include <stdio.h>
#include <conio.h>
inline void vec3_normalize( float *a )
{
float	mag = vec3_mag( a );

	if ( mag == 0. )
		mag = 1e+10;
	else
		mag = 1. / mag;

	vec3_mul( a, a, mag );
}
//---------------------------------
inline void vec3_normalize( double *a )
{
double	mag = vec3_mag( a );

	if ( mag == 0. )
		mag = 1e+20;
	else
		mag = 1. / mag;

	vec3_mul( a, a, mag );
}
#endif
