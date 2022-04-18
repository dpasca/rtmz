//==============================================
// RTMZ_SUP.CPP
// Copyright (C) Davide Pasca 1995-97
//
// TABS=4
//==============================================
#ifndef __RTMZ_SUP_H__
#define __RTMZ_SUP_H__

#include "EXTTYPES.HPP"
#include "TMR.HPP"

//===========================
class ZoomTask
{
long			Z1,Z2,Z,StpZ;
long			Pau;
long			StaTick;
UB				Status;
long			Flags, Steps;
const BitMap	*SBmP, *DBmP;

public:
	void Start(const BitMap *dbmP, const BitMap *sbmP, long z1, long z2, short n, UB zoomRot, long cent);
	void Idle(void);
	ZoomTask(){	Status = 0; }
};

//======================
/*class TickSampler
{
US		MaxCnt;
US		Cnt;
long	TickSum,TickSumRun;

public:
	void Reset( US max )
	{
		TickSum = 0;
		MaxCnt = max;
		Cnt = 0;
	}
	void Start(void)
	{
		if ( Cnt == 0 )
			TickSumRun = -TMR_TicksGet();
		else
			TickSumRun -= TMR_TicksGet();

		if ( ++Cnt == MaxCnt )
			Cnt = 0;
	}
	void Stop(void)
	{
		TickSumRun += TMR_TicksGet();
		if ( Cnt == 0 )
			TickSum = TickSumRun;
	}
	UB ReadyToGet(void)
	{
		return Cnt==0;
	}
	UL TMR_TicksGetum(void)
	{
		return TickSum;
	}
	long GetTickMed( long n )
	{
		n = n * TMR_TicksPerSecond() * MaxCnt;

		if ( TickSum )
			return  n / TickSum;
		else
			return n;
	}
};
*/
#endif
