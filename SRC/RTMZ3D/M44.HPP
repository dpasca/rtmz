//==============================================
// M34.HPP - matrix handler
// Copyright (C) Davide Pasca 1995
//==============================================

#ifndef __M44_HPP__
#define __M44_HPP__

#include "EXTTYPES.HPP"

void m44_identity( float m[4][4] );
void m44_multiply( float d[4][4], const float s1[4][4], const float s2[4][4] );
void m44_invert( float m[4][4] );

void m44_scale( float m[4][4], float x, float y, float z );
inline void m44_equ( float d[4][4], const float s[4][4] )
{	memcpytiny( d, s, 16*sizeof(float) );
}

#endif
