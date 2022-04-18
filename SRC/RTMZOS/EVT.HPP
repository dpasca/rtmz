//==============================================
// EVT.HPP - event stuff
// Copyright (C) by Davide Pasca 1995
//==============================================

#ifndef	__EVT_HPP__
#define	__EVT_HPP__

#include "EXTTYPES.HPP"

void EVT_Install();

void EVT_FuncMouseClick( UB (*clickFuncP)(UB,UB,short,short) );
void EVT_FuncMouseMove( UB (*moveFuncP)(short,short) );
void EVT_FuncKeyb( UB (*keybFuncP)(US,UB) );
void EVT_FuncSuperKeyb( UB (*keybFuncP)(US,UB) );
void EVT_FuncReshape( BitMap* (*rshapeFuncP)(long,long,long) );
void EVT_FuncRedraw( void (*redrFuncP)(void) );
void EVT_FuncIdle( void (*idleFuncP)(void) );
void EVT_FuncsPush(void);
void EVT_FuncsPop(void);
void EVT_FuncsReset(void);

void EVT_PostRedraw(void);
void EVT_PostReshape(long wd, long he, long res);
void EVT_EventsFlush(void);
long EVT_MainLoop(void);
void EVT_MainLoopReturn(long retval);
void EVT_PointerSet( UB *dataP, US wd, US he );

void EVT_RecordingStop(void);
void EVT_RecordingStart(void);
long EVT_RecordingSave( const char *fnameP );

long EVT_PlaybackLoad( const char *fnameP );
void EVT_PlaybackStart(void);
void EVT_PlaybackStop(void);
BO	 EVT_PlaybackIsActive(void);

void EVT_DisplayUpdateFlag( UB updateDisplay );
void EVT_DisplayRestoreOld(void);

#endif

