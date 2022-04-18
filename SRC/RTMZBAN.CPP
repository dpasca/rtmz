//==============================================
// RTMZBAN.CPP
// Copyright (C) Davide Pasca 1995-97
//
// TABS=4
//==============================================

#include <stdarg.h>
#include <stdio.h>
#include <time.h>
#include "BMP.HPP"
#include "EVT.HPP"
#include "RTMZBAN.HPP"

//===========================
#define	BAN_MAX	5
typedef struct
{
	char	text[128];
	long	staClock, clockCnt;
	UB		busy;
} banner_t;
static banner_t	_banners[ BAN_MAX ];

//==================================
void banpost( long seconds, long pos, const char *fmtP, ... )
{
banner_t	*b;
va_list		arg;

	if ( pos < 0 )
	{
		b = _banners + BAN_MAX - 1;
		for(long i=BAN_MAX; i; --i, --b)
		{
			if NOT( b->busy )
			{
				va_start( arg, fmtP );
				vsprintf( b->text, fmtP, arg );
				va_end( arg );
				b->clockCnt = seconds*CLK_TCK;
				b->staClock = clock();
				b->busy = 1;
				EVT_PostRedraw();
				return;
			}
		}
	}
	else
	{
		CLAMP( pos, 0, BAN_MAX-1 );
		b = _banners + pos;
	}

	va_start( arg, fmtP );
	vsprintf( b->text, fmtP, arg );
	va_end( arg );
	b->clockCnt = seconds*CLK_TCK;
	b->staClock = clock();
	b->busy = 1;
	EVT_PostRedraw();
}

//==================================
void banidle(void)
{
long	clk = clock();

	banner_t *b=_banners;
	for(long i=BAN_MAX; i; --i, ++b)
		if ( b->busy )
			if ( clk - b->staClock > b->clockCnt )
			{
				b->busy = 0;
				EVT_PostRedraw();
			}	
}

//==================================
void banprint( UB col, long y )
{
long	he = BMP_TFHeight()+1;

	banner_t *b=_banners;
	for(long i=BAN_MAX; i; --i, ++b)
		if ( b->busy )
		{
			BMP_TextWriteCP( b->text, 2, y, col );
			y += he;
		}
}

