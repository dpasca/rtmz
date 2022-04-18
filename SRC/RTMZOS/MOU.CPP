//==============================================
// MOUSE.CPP - mouse handler
// Copyright (C) Davide Pasca 1995
//==============================================

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dos.h>
#include <i86.h>

#include "mou.hpp"
#include "vgapack.hpp"

static UB               (*_clickFuncP)(UB,UB,short,short), (*_moveFuncP)(short,short);

//==============================
#define MOVED   ((UB)1<<0)
#define LEFT_D  ((UB)1<<1)
#define LEFT_U  ((UB)1<<2)
#define RITE_D  ((UB)1<<3)
#define RITE_U  ((UB)1<<4)
#define MID_D   ((UB)1<<5)
#define MID_U   ((UB)1<<6)
#define ALLFLG  (MOVED|LEFT_D|LEFT_U|RITE_D|RITE_U|MID_D|MID_U)
static MOU_Event_t      _mou_evt[MOU_EVT_MAX];
static long             _mou_evt_i;
static UB               _mou_evt_enabled=1;

#pragma off (check_stack)
static void _loadds far __MOU_TheHandler(long max, long mcx, long mdx)
{
#pragma aux __MOU_TheHandler parm [EAX] [ECX] [EDX]

	if ( _mou_evt_i < MOU_EVT_MAX && _mou_evt_enabled )
	{
	MOU_Event_t     *e = &_mou_evt[ _mou_evt_i++ ];

		e->b = max;
		e->x = mcx>>1;
		e->y = mdx;
	}
}

//==============================
void MOU_EventsDisable(void){ _mou_evt_enabled = 0; }
void MOU_EventsEnable(void){ _mou_evt_enabled = 1; }

//==============================
UL MOU_EventsListGet( MOU_Event_t *listP )
{
UL      n;

	MOU_EventsDisable();
		memcpytiny( listP, _mou_evt, sizeof(MOU_Event_t)*_mou_evt_i );
		n = _mou_evt_i; // occhio deve essere nel blocco disable-enable
	MOU_EventsEnable();

	return n;
}

//==============================
void MOU_EventsFlush(long n, const MOU_Event_t *evtsP)
{
MOU_Event_t                     e[MOU_EVT_MAX];
const MOU_Event_t       *ep;

	if ( evtsP )
	{
		_mou_evt_i = n;
		ep = evtsP;
	}
	else
	{
		MOU_EventsDisable();
			n = _mou_evt_i;
			_mou_evt_i = 0;
			memcpy( e, _mou_evt, n*sizeof(MOU_Event_t) );
			ep = e;
		MOU_EventsEnable();
	}

	for(; n; --n, ++ep)
	{
	UB              t;
	short   x,y;

		t = ep->b;
		x = ep->x;
		y = ep->y;
		if ( _clickFuncP )
		{
			if (t & LEFT_D) _clickFuncP( MOU_BUT_LFT, MOU_BUT_DOWN, x, y ); else
			if (t & LEFT_U) _clickFuncP( MOU_BUT_LFT, MOU_BUT_UP, x, y );   else
			if (t & RITE_D) _clickFuncP( MOU_BUT_RGT, MOU_BUT_DOWN, x, y ); else
			if (t & RITE_U) _clickFuncP( MOU_BUT_RGT, MOU_BUT_UP, x, y );   else
			if (t & MID_D)  _clickFuncP( MOU_BUT_MID, MOU_BUT_DOWN, x, y ); else
			if (t & MID_U)  _clickFuncP( MOU_BUT_MID, MOU_BUT_UP, x, y );
		}
		if ( _moveFuncP )
			if (t & MOVED)  _moveFuncP( x, y );
	}
}

//==============================
void MOU_FuncSetClick( UB (*clickFuncP)(UB,UB,short,short) ){ _clickFuncP = clickFuncP; }
void MOU_FuncSetMove( UB (*moveFuncP)(short,short) ){ _moveFuncP = moveFuncP; }

//==============================
static void mouDispose(void)
{
union REGS inregs, outregs;
struct SREGS sregs;

	segread(&sregs);
	inregs.w.ax = 0xC;              // install click watcher
	inregs.w.cx = 0;                // no interrupts
	inregs.x.edx = FP_OFF( __MOU_TheHandler );
	sregs.es     = FP_SEG( __MOU_TheHandler );
	int386x( 0x33, &inregs, &outregs, &sregs );
}
//==============================
short MOU_Init(void)
{
union REGS inregs, outregs;

	inregs.w.ax = 0;
	int386(0x33, &inregs, &outregs);

	if (outregs.w.ax == 0xffff)
	{
	struct SREGS sregs;
		segread(&sregs);

		inregs.w.ax = 0xC;              // install click watcher
		inregs.w.cx = ALLFLG;
		inregs.x.edx = FP_OFF( __MOU_TheHandler );
		sregs.es     = FP_SEG( __MOU_TheHandler );
		int386x( 0x33, &inregs, &outregs, &sregs );
		atexit(mouDispose);
		_mou_evt_i = 0;
		return 0;
	}
	else
		return -1;
}

//------------------------------
void MOU_BoundsSet(short x1, short y1, short x2, short y2)
{
union REGS inregs, outregs;

	inregs.w.ax=7;
	inregs.w.cx=(x1+1)*2-1;
	inregs.w.dx=(x2+1)*2-1;
	int386(0x33, &inregs, &outregs);

	inregs.w.ax=8;
	inregs.w.cx=y1;
	inregs.w.dx=y2;
	int386(0x33, &inregs, &outregs);
}

//------------------------------
void MOU_PointerShow(void)
{
union REGS inregs, outregs;

	inregs.w.ax=1;
	int386(0x33, &inregs, &outregs);
}
//------------------------------
void MOU_PointerHide(void)
{
union REGS inregs, outregs;

	inregs.w.ax=2;
	int386(0x33, &inregs, &outregs);
}

//------------------------------
US MOU_Buttons(void)
{
union REGS inregs, outregs;

	inregs.w.ax=3;
	int386(0x33, &inregs, &outregs);

	return outregs.w.bx;
}

//------------------------------
short MOU_GetX(void)
{
union REGS inregs, outregs;

	inregs.w.ax=3;
	int386(0x33, &inregs, &outregs);

	return outregs.w.cx >> 1;
}

//------------------------------
short MOU_GetY(void)
{
union REGS inregs, outregs;

	inregs.w.ax=3;
	int386(0x33, &inregs, &outregs);

	return outregs.w.dx;
}

//------------------------------
void MOU_GetEvents(UB *bP, short *xP, short *yP)
{
union REGS inregs, outregs;

	inregs.w.ax=3;
	int386(0x33, &inregs, &outregs);
	*bP = outregs.w.bx;
	*xP = outregs.w.cx >> 1;
	*yP = outregs.w.dx;
}

