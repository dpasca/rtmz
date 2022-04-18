//==============================================
// FRPLASMA.HPP -
// Copyright (C) Davide Pasca 1995
//==============================================

#include <time.h>
#include <stdlib.h>
#include "EXTTYPES.HPP"

//=======================================
class FractPlasma
{
public:
	UB		*_mapYP;
	US		_wd;
	short	_frastaWd;
	long	_max;
	double	_ratio;

public:
	~FractPlasma(){ Free(); }

	void Free( void ){ if (_mapYP){ free(_mapYP); _mapYP=0; } }
	long Alloc( US dim )
	{
		_wd=dim;
		if NOT(_mapYP = (UB *)calloc((UL)_wd*_wd*sizeof(UB),1))
			return -1;
		return 0;
	}
	UB GetY( US x, US z ) { return( _mapYP[((long)z*_wd)+x] ); };
	void  SetY( US x, US z, short col )
	{
		_mapYP[((long)z*_wd)+x] = col;
	};
	void Make(short x1,short y1,short x2,short y2,short frasta,long seed, long max=256);
	void Make(short x1,short y1,short x2,short y2,long seed)
	{
		Make(x1, y1, x2, y2, 6, seed);
	}

private:
	short RandPixel(short x,short y,short x1,short y1,short x2,short y2);
	void DoPlasma(short x1,short y1,short x2,short y2);
};
