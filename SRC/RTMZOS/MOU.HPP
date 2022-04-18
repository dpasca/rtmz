//==============================================
// MOUSE.HPP -
// Copyright (C) Davide Pasca 1995
//==============================================

#ifndef __MOUSE_HPP__
#define	__MOUSE_HPP__

#include "exttypes.hpp"

#define MOU_BUT_LFT		1
#define MOU_BUT_RGT		2
#define MOU_BUT_MID		4
#define	MOU_BUT_UP		1
#define	MOU_BUT_DOWN	2

#define	MOU_EVT_MAX		100
typedef struct
{
	UB		b;
	short	x,y;
}MOU_Event_t;

short MOU_Init(void);
void MOU_FuncSetClick( UB (*clickFuncP)(UB,UB,short,short) );
void MOU_FuncSetMove( UB (*moveFuncP)(short,short) );
void MOU_BoundsSet(short x1, short y1, short x2, short y2);
void MOU_PointerShow(void);
void MOU_PointerHide(void);
void MOU_EventsFlush(long n, const MOU_Event_t *evtsP );
UL MOU_EventsListGet( MOU_Event_t *listP );

US MOU_Buttons(void);
short MOU_GetX(void);
short MOU_GetY(void);
void MOU_GetEvents(UB *bP, short *xP,  short  *yP);

#endif
