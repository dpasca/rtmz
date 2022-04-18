//==============================================
// EXTTYPES.HPP -
// Copyright (C) Davide Pasca 1995
//==============================================

#ifndef __EXTTYPES_HPP__
#define __EXTTYPES_HPP__

#include <malloc.h>

typedef unsigned long	UL;
typedef unsigned short	US;
typedef unsigned char	UB;
typedef unsigned char	BO;
typedef struct
{
	US	wd,he;
	UB	dp;
	UB	Flags;
	UB	*memP;
	UB	**offsPP;
}BitMap;
typedef struct	{	long	x,y;	}VEC2_t;
typedef struct	{	short	x,y;	}Point2D;

#define MAKE_ID(A,B,C,D)	(((UL)A) | ((UL)(B)<<8) | ((UL)(C)<<16) | ((UL)(D)<<24))
#define NOT(X)			((X)==0)
#define ABS(X)			((X) < 0 ? -(X) : X)
#define MIN(A,B)		((A) < (B) ? (A) : (B))
#define MAX(A,B)		((A) > (B) ? (A) : (B))
#define CLAMP(X,MI,MA)	{if(X<MI)X=MI;if(X>MA)X=MA;}

inline void *ext_new(UL *ptr,UL siz){ return (void *)(*ptr = (UL)malloc(siz)); }
#define NEW(PTR,N)		(ext_new((UL *)&PTR,sizeof(*PTR)*(N)))

#define FREEIF(X)		{if (X) free(X);}
#define SAFE_FREE(X)	{if (X){free(X); X=0;}}

#define FREAD(B,S,F)	(fread(B,1,S,F)!=(S))
#define FWRITE(B,S,F)	(fwrite(B,1,S,F)!=(S))
#define SWAP(A,B,T)		{T=A;A=B;B=T;}
inline void swap(long &a, long &b){long t=a; a=b; b=t;}
inline void swap(UL &a, UL &b){UL t=a; a=b; b=t;}
inline void swap(short &a, short &b){short t=a; a=b; b=t;}
inline void swap(US &a, US &b){US t=a; a=b; b=t;}
inline long MulAndDiv(long a, long b, long c){return (float)a*(float)b/(float)c;}
//==================================
extern void memcpytiny(void *dP, const void *sP, long size );
#pragma aux memcpytiny =   \
"	cld"\
"	rep		movsb"\
parm caller [edi][esi][ecx]\
modify [edi esi ecx];

#define TAG_END	-1
#endif

