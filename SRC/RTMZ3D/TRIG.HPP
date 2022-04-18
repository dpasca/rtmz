//==============================================
// TRIG.HPP -
// Copyright (C) Davide Pasca 1995
//==============================================

#ifndef __TRIG_HPP__
#define __TRIG_HPP__

#ifndef PI
#define PI	3.14159265359
#endif

#define	TR_GIR			4096
#define	TR_ISHIFT		15
#define TR_MAXISIN		(1L<<TR_ISHIFT)
#define TR_GR2RAD(__X__)	((__X__)*PI/180.)

extern float	_tr_fsin11[TR_GIR], _tr_fcos11[TR_GIR];
extern long		_tr_isin11[TR_GIR], _tr_icos11[TR_GIR];

inline long	_tr_adjust_ang(float ang)
{
	long	idx = ang * (TR_GIR / 360.);
	if ( idx < 0 )	idx = TR_GIR-1-(-idx & (TR_GIR-1));
	else			idx &= TR_GIR-1;

	return idx;
}

#include "math.h"
inline float tr_fsinf(float ang){ return _tr_fsin11[ _tr_adjust_ang(ang) ]; }
inline float tr_fcosf(float ang){ return _tr_fcos11[ _tr_adjust_ang(ang) ]; }
inline float tr_isinf(float ang){ return _tr_isin11[ _tr_adjust_ang(ang) ]; }
inline float tr_icosf(float ang){ return _tr_icos11[ _tr_adjust_ang(ang) ]; }

void tr_init(void);

#endif

