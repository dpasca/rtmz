//==============================================
// TMR.HPP - timer handler
// Copyright (C) Davide Pasca 1995
//==============================================

#ifndef __TMR_HPP__
#define __TMR_HPP__

#include "EXTTYPES.HPP"

extern volatile UL	_TMR_Ticks;
extern UL			_TMR_TksPerSec;
extern short _soundEnabled;

long TMR_Open( UL freq );
void TMR_Close(void);
long TMR_FuncAdd( void (__interrupt *handlerp)() );
UL TMR_TicksGet(void);
UL TMR_TicksPerSecGet(void);
void TMR_TicksWait( UL howmany );

#endif

