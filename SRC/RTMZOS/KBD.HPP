//==============================================
// KBD.HPP - keyboard handler
// Copyright (C) Davide Pasca 1995
//==============================================

#ifndef __KBD_HPP__
#define	__KBD_HPP__

#include "exttypes.hpp"

#define KBD_EXT			(128)
#define KBD_FLG_KPAD	(1024)
#define	KBD_NOASC		(512)
#define KBD_BACKSPACE	(14)
//#define KBD_KP_ENTER	(KBD_EXT+28)	// ext 156
#define KBD_LCTRL		(KBD_NOASC+1)
#define KBD_RCTRL		(KBD_EXT+29)	// ext
#define KBD_PRTSCR		(KBD_EXT+42)	// ext

#define KBD_LSHIFT		(KBD_NOASC+2)
//#define KBD_KP_SLASH	(KBD_EXT+53)	// ext
#define KBD_RSHIFT		(KBD_NOASC+3)

#define KBD_RALT		(KBD_EXT+56)	// ext
#define KBD_LALT		(KBD_NOASC+4)
#define KBD_CAPSLOCK	(KBD_NOASC+5)
#define KBD_F1			(KBD_NOASC+6)
#define KBD_F2			(KBD_NOASC+7)
#define KBD_F3			(KBD_NOASC+8)
#define KBD_F4			(KBD_NOASC+9)
#define KBD_F5			(KBD_NOASC+10)
#define KBD_F6			(KBD_NOASC+11)
#define KBD_F7			(KBD_NOASC+12)
#define KBD_F8			(KBD_NOASC+13)
#define KBD_F9			(KBD_NOASC+14)
#define KBD_F10			(KBD_NOASC+15)
#define KBD_F11			(KBD_EXT+87)	// ext
#define KBD_F12			(KBD_EXT+88)	// ext
#define KBD_NUMLOCK		(KBD_NOASC+16)
#define KBD_SCRLLOCK	(KBD_NOASC+17)

#define KBD_HOME		(KBD_EXT+71)	// ext

#define KBD_UP			(KBD_EXT+72)	// ext
#define KBD_PGUP		(KBD_EXT+73)	// ext
#define KBD_LEFT		(KBD_EXT+75)	// ext
#define KBD_RIGHT		(KBD_EXT+77)	// ext
#define KBD_END			(KBD_EXT+79)	// ext
#define KBD_DOWN		(KBD_EXT+80)	// ext
#define KBD_PGDOWN		(KBD_EXT+81)	// ext
#define KBD_INSERT		(KBD_EXT+82)	// ext
#define KBD_DELETE		(KBD_EXT+83)	// ext
#define KBD_MACRO		(KBD_EXT+111)	// ext

#define KBD_PRESS		1
#define KBD_RELEASE		2

#define	KBD_EVT_MAX		64

long KBD_Open(void);
void KBD_Close(void);
void KBD_FuncSet( UB (*keyFuncP)(US,UB) );
void KBD_EventsDisable(void);
void KBD_EventsEnable(void);
UL KBD_EventsListGet( US *listP );
void KBD_EventsFlush(long n, const US *evtsP);
UB KBD_IsPressed( US c );
void KBD_ResetPressTable(void);

void KBD_RepeatIgnore(BO yesNo);
void KBD_HandlerKeepOld(BO yesNo);

#endif
