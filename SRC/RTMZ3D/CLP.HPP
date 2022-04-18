//==============================================
// CLP.HPP - clipping routines
// Copyright (C) Davide Pasca 1995
//==============================================

#ifndef __CLP_HPP__
#define __CLP_HPP__

#include "POLYENG.HPP"

#define CLP_BIT_Z2	5
#define CLP_BIT_Z1	4
#define CLP_BIT_Y2	3
#define CLP_BIT_Y1	2
#define CLP_BIT_X2	1
#define CLP_BIT_X1	0

#define CLP_CODE_Z1 (1<<CLP_BIT_Z1)
#define CLP_CODE_Z2 (1<<CLP_BIT_Z2)
#define CLP_CODE_Y1 (1<<CLP_BIT_Y1)
#define CLP_CODE_Y2 (1<<CLP_BIT_Y2)
#define CLP_CODE_X1 (1<<CLP_BIT_X1)
#define CLP_CODE_X2 (1<<CLP_BIT_X2)

const POE_PolyI_t	*CLP_PolyI4D( const POE_PolyI_t *polyP );

#endif
