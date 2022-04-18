//==============================================
// FRPLASMA.CPP
// Copyright (C) Davide Pasca 1995-97
//
// TABS=4
//==============================================
#include "FRPLASMA.HPP"
#include "PASCALIB.HPP"

#define RANDOM(x)	(RandShort()%x)

//----------------------------------
short FractPlasma::RandPixel(short x,short y,short x1,short y1,short x2,short y2)
{
short	col;

	short dx,dy;
	dx = x-x1;
	dy = y-y1;
	col = (RANDOM(_max) - _max/2) * ((long)ABS(dx)+ABS(dy)) / _frastaWd;
	col += (long)GetY(x1,y1)+GetY(x2,y2) >> 1;

	if (col < 1) col = 1;
	else
	if (col >= _max) col = _max-1;

	SetY(x,y,col);

	return col;
}
//----------------------------------
void FractPlasma::DoPlasma(short x1,short y1,short x2,short y2)
{
long x,y,s,p;

	if (x2-x1 <= 1 && y2-y1 <= 1)
		return;

	x = x1 + x2 >> 1;
	y = y1 + y2 >> 1;

	if NOT(p = GetY(x, y1))
		p = RandPixel(x,y1,x1,y1,x2,y1);
	s = p;

	if NOT(p = GetY(x2,y))
		p = RandPixel(x2,y,x2,y1,x2,y2);
	s += p;

	if NOT(p = GetY(x,y2))
		p = RandPixel(x,y2,x1,y2,x2,y2);
	s += p;

	if NOT(p = GetY(x1,y))
		p = RandPixel(x1,y,x1,y1,x1,y2);
	s += p;

	if NOT(GetY(x,y))
		SetY(x,y,s >> 2);

	DoPlasma(x1,y1,x,y);
	DoPlasma(x,y1,x2,y);
	DoPlasma(x1,y,x,y2);
	DoPlasma(x,y,x2,y2);
}

//---------------------------------
#define FP_DOPIX(x,y) \
	if ((val = GetY(x1,y1)) > 0) \
	{                             \
		av += val;                 \
		num++;                      \
	}

void FractPlasma::Make(short x1,short y1,short x2,short y2, short frasta, long seed, long max)
{
long	av = 0;
long	val;
short	num = 0;

	RandomizeShort(seed);
	if NOT( frasta )
		_frastaWd = _wd;
	else
		_frastaWd = _wd / frasta;

	FP_DOPIX(x1,y1);
	FP_DOPIX(x2,y1);
	FP_DOPIX(x2,y2);
	FP_DOPIX(x1,y2);

	_max = max;
	if (!av || RANDOM(max/8) == 0)
		av = RANDOM(max);
	else
		if (num)
			av /= num;

long	m1=max/3, m2=max/6;

	if NOT( GetY(x1,y1) )	SetY(x1,y1, av + RANDOM(m1)-m2);
	if NOT( GetY(x2,y1) )	SetY(x2,y1, av + RANDOM(m1)-m2);
	if NOT( GetY(x2,y2) )	SetY(x2,y2, av + RANDOM(m1)-m2);
	if NOT( GetY(x1,y2) )	SetY(x1,y2, av + RANDOM(m1)-m2);

	DoPlasma(x1,y1,x2,y2);
}

