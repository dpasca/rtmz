//==============================================
// TRIG.CPP - Am I still using this ??
// Copyright (C) Davide Pasca 1995-97
//
// TABS=4
//==============================================
#include <math.h>
#include "TRIG.HPP"

long  _tr_isin11[TR_GIR], _tr_icos11[TR_GIR];
float _tr_fsin11[TR_GIR], _tr_fcos11[TR_GIR];
long	_tr_sqr[1<<0/*8*/], *_tr_sqrP;
//, _tr_bsr[1<<17];
unsigned char	_tr_sqrt[1<<8];

extern long bsright(long a);
#pragma aux bsright = \
		"bsr eax,ebx"\
        parm caller [ebx] \
        value [eax]       \
        modify [eax ebx];

//=============================
void tr_init( void )
{
long	i;
float	ang, angStep, fact;

	angStep = PI*2.0 / (float)TR_GIR;
	fact = (float)(1L << TR_ISHIFT);
	for (i=0; i < TR_GIR; ++i)
	{
		ang = angStep * i;
		_tr_isin11[i] = (_tr_fsin11[i] = sin(ang)) * fact;
		_tr_icos11[i] = (_tr_fcos11[i] = cos(ang)) * fact;
	}

	/*_tr_sqrP = _tr_sqr+(1<<17);
	for (i=-(1<<17); i < (1<<17); ++i)
		_tr_sqrP[i] = i*i;
	_tr_sqrP[0] = 1;*/

//	for(i=0; i < (1<<17); ++i)
//		_tr_bsr[i] = bsright(i);

	for(i=0; i < (1<<8); ++i)
		_tr_sqrt[i] = sqrt( i );
}

