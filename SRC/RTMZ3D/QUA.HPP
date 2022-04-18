//==============================================
// QUA.HPP - quaternions stuff
// Copyright (C) by Davide Pasca 1995
//==============================================

#ifndef	__QUA_HPP__
#define __QUA_HPP__

#include "EXTTYPES.HPP"

void QUA_QuatFromAxis(float q[4], float phi, const float a[3]);
void QUA_Normalize(float q[4]);
void QUA_Mul(float dest[4], const float q1[4], const float q2[4]);
void QUA_QuatToMat(const float q[4], float m[4][4]);

#endif
