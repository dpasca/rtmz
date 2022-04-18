//==============================================
// RTMZ_SUP.CPP
// Copyright (C) Davide Pasca 1995-97
//
// TABS=4
//==============================================
#include "RTMZ_SUP.HPP"
#include "EXTTYPES.HPP"
#include "CLP.HPP"
#include "TRIG.HPP"

//=================================================
static inline rotate_vec2( long *v, long co, long si )
{
long	t;

	t	= v[0] * co - v[1] * si >> TR_ISHIFT;
	v[1]= v[0] * si + v[1] * co >> TR_ISHIFT;
	v[0] = t;
}

//=================================================
static void Cnk_PutMapRot3D( const BitMap *dbmP, const BitMap *sbmP, long cx, long cy, long z, float ang )
{
long	wd,he, co, si;

	wd = (128L*sbmP->wd/2 / z)*16;
	he = (129L*sbmP->he/2 / z)*16;
long	vec[4][2]={-wd,-he, wd,-he, wd,he, -wd,he};

	wd = sbmP->wd*16-1;
	he = sbmP->he*16-1;
long	tex[4][2]={0,0, wd,0, wd,he, 0,he};

	POE_BeginPolygon();
		co = tr_icosf( ang );
		si = tr_isinf( ang );
		cx *= 16;
		cy *= 16;
		for (long i=0; i < 4; ++i)
		{
			rotate_vec2( vec[i], co, si );
			POE_Vertex2( vec[i][0] + cx,
						 vec[i][1] + cy );
			POE_Texture2v( tex[i] );
		}
		POE_Color1( 3 );
	POE_EndPolygon();
	//CLP_SetLimits2D( 0,0, dbmP->wd*16-1,dbmP->he*16-1 );
	//CLP_DoPoly( CLP_CODE_XY | CLP_MOD_TEXTURE );
	//Cnk_DrawPolyTextured2D16( dbmP, sbmP );
	//Cnk_DrawPoly16( dbmP );
}

//---------------------------
void ZoomTask::Start(const BitMap *dbmP, const BitMap *sbmP, long z1, long z2, short n, UB zoomRot, long cent )
{
	if ( !n || z1 > z2 )	return;
	SBmP = sbmP;
	DBmP = dbmP;
	Z1 = z1;
	Z2 = z2;
	Z = Z1 << 16;
	StpZ = (Z2-Z1 << 16) / n;
	Steps = n;
	Status = 1;
	Pau = cent;
	Flags = zoomRot;
}

//---------------------------
void ZoomTask::Idle(void)
{
long			z;
static float	ang, stpAng;
static long		lastTicks;

	if NOT( Status )	return;

	if (Status == 1)
	{
		//Cnk_PutMap3D( DBmP, SBmP, DBmP->wd-SBmP->wd>>1, DBmP->he-SBmP->he>>1, Z >> 16 );


		Z += StpZ;
		z = Z >> 16;
		if ( z >= Z2 )
		{
			Status = 2;
			StaTick = TMR_TicksGet();
			Z = Z2 << 16;
		}
	}
	else
	if (Status == 2)
	{
		//Cnk_PutMap( DBmP, SBmP, DBmP->wd-SBmP->wd>>1, DBmP->he-SBmP->he>>1 );

		if (TMR_TicksGet()-StaTick >= Pau)
		{
			if ( Flags )
			{
				Status = 3;
				//StpZ *= .5;
				//Steps *= 1./.5;
				ang = 0.;
				stpAng = .5;
				lastTicks = TMR_TicksGet();
			}
			else
				Status = 0;
			return;
		}
	}
	else
	if (Status == 3)
	{
		Cnk_PutMapRot3D( DBmP, SBmP, DBmP->wd>>1, DBmP->he>>1, Z2, ang );
		ang += stpAng;
		stpAng += .1;
		lastTicks = TMR_TicksGet() - lastTicks;
		if ( lastTicks > TMR_TicksPerSecGet()/10 )
		{
			if ( lastTicks > TMR_TicksPerSecGet()/5 )
				Status = 0;
			else
			{
				ang += stpAng;
				stpAng += .14;
			}
		}
		lastTicks = TMR_TicksGet();
		if ( ang >= 359. )	Status = 4;
	}
	else
	if (Status == 4)
	{
		Z -= StpZ;
		z = Z >> 16;
		if ( z > 0 )
			Cnk_PutMapRot3D( DBmP, SBmP, DBmP->wd>>1, DBmP->he>>1, z, ang );

		if ( --Steps <= 0 )
			Status = 0;

		ang += stpAng;
		stpAng += .14;
		lastTicks = TMR_TicksGet() - lastTicks;
		if ( lastTicks > TMR_TicksPerSecGet()/10 )
		{
			if ( lastTicks > TMR_TicksPerSecGet()/5 )
				Status = 0;
			else
			{
				ang += stpAng;
				stpAng += .14;
				--Steps;
			}
		}
		lastTicks = TMR_TicksGet();
	}
}
