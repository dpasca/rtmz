//==============================================
// KBD.CPP - keyboard handler
// Copyright (C) Davide Pasca 1995
//==============================================

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <conio.h>
#include <dos.h>
#include <i86.h>

#include "KBD.HPP"

static UB	(*_keyFuncP)(US,UB);
static BO	_kbd_ingnoreRepeat, _handKeepOld;

static UB	_evt_enabled=1, _atexit_filled;
static long	_evt_i=0;
static US	_evt_k[KBD_EVT_MAX];
static US	_keyLUT[256]={
0, 27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', KBD_BACKSPACE, '\t',
'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n', KBD_LCTRL, 'a', 's',
'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`', KBD_LSHIFT, '!','z', 'x', 'c', 'v',
'b', 'n', 'm', ',', '.', '/', KBD_RSHIFT, '*'|KBD_FLG_KPAD, KBD_LALT, ' ', KBD_CAPSLOCK, KBD_F1,KBD_F2,KBD_F3,KBD_F4,KBD_F5,
KBD_F6,KBD_F7,KBD_F8,KBD_F9,KBD_F10,
KBD_NUMLOCK|KBD_FLG_KPAD, KBD_SCRLLOCK,
'7'|KBD_FLG_KPAD, '8'|KBD_FLG_KPAD, '9'|KBD_FLG_KPAD, '-'|KBD_FLG_KPAD, '4'|KBD_FLG_KPAD, '5'|KBD_FLG_KPAD,
'6'|KBD_FLG_KPAD, '+'|KBD_FLG_KPAD, '1'|KBD_FLG_KPAD, '2'|KBD_FLG_KPAD, '3'|KBD_FLG_KPAD, '0'|KBD_FLG_KPAD,
'.'|KBD_FLG_KPAD, 84,85,86,
KBD_F11, KBD_F12,
89,
90,91,92,93,94,95,96,97,98,99,
100,101,102,103,104,105,106,107,108,109,
110,111,112,113,114,115,116,117,118,119,
120,121,122,123,124,125,126,127,128, 129,
130,131,132,133,134,135,136,137,138,139,
140,141,142,143,144,145,146,147,148,149,
150,151,152,153,154,155,
'\n'|KBD_FLG_KPAD, KBD_RCTRL,128+30,
128+31,128+32,128+33,128+34,128+35,128+36,128+37,128+38,128+39,128+40,128+41,
KBD_PRTSCR,
128+43,128+44,128+45,128+46,128+47,128+48,128+49,128+50,128+51,128+52,
'/'|KBD_FLG_KPAD,128+54,128+55,
KBD_RALT,128+57,128+58,128+59,
128+60,128+61,128+62,128+63,128+64,128+65,128+66,128+67,128+68,128+69,
128+70,
KBD_HOME,KBD_UP,KBD_PGUP,128+74,KBD_LEFT,128+76,KBD_RIGHT,128+78,KBD_END,
KBD_DOWN,KBD_PGDOWN,KBD_INSERT,KBD_DELETE,
128+84,128+85,128+86,128+87,128+88,128+89,
128+90,128+91,128+92,128+93,128+94,128+95,128+96,128+97,128+98,128+99,
128+100,128+101,128+102,128+103,128+104,128+105,128+106,128+107,128+108,128+109,
128+110,KBD_MACRO
};
static UB	_kbd_pressTable[2048];

//==============================
UB KBD_IsPressed( US c )
{
	return _kbd_pressTable[c];
}

//==============================
void KBD_ResetPressTable(void)
{
	memset( _kbd_pressTable, 0, sizeof(_kbd_pressTable) );
}

//==============================
static void (interrupt far *_oldHandler)(void);
static UB	_alreadyInstalled, _expectingExtendedKey;

#pragma off (check_stack)
static void interrupt far __theHandler(void)
{
UB	k;

	_disable();
	k = inp( 0x60 );
	if ( _evt_enabled )
	{
		if ( _evt_i < KBD_EVT_MAX )
		{
			if ( _expectingExtendedKey )
			{
				_evt_k[ _evt_i++ ] = 2048 | (US)k;
				--_expectingExtendedKey;
			}
			else
			{
				if ( k == 0xE0 )
				{
					_expectingExtendedKey = 1;
				}
				else
					_evt_k[ _evt_i++ ] = k;
			}
		}
	}
	//else
	//	_expectingExtendedKey = 0;

	if ( _handKeepOld )
		_oldHandler();
	else
		outp( 0x20, 0x20 );

	_enable();
}

//==============================
void KBD_RepeatIgnore(BO yesNo){ _kbd_ingnoreRepeat = yesNo; }
//==============================
void KBD_HandlerKeepOld(BO yesNo){ _handKeepOld = yesNo; }

//==============================
void KBD_EventsDisable(void){ _evt_enabled = 0; }
void KBD_EventsEnable(void){ _evt_enabled = 1; }

//==============================
UL KBD_EventsListGet( US *listP )
{
UL	n;

	KBD_EventsDisable();
	memcpytiny( listP, _evt_k, sizeof(US)*_evt_i );
	n = _evt_i; // occhio deve essere nel blocco disable-enable
	KBD_EventsEnable();

	return n;
}

//==============================
void KBD_EventsFlush(long n, const US *evtsP)
{
US			e[KBD_EVT_MAX];
const US	*kp;

	if ( evtsP )
	{
		_evt_i = n;
		kp = evtsP;
	}
	else
	{
		KBD_EventsDisable();
			n = _evt_i;
			_evt_i = 0;
			memcpy( e, _evt_k, n*sizeof(US) );
			kp = e;
		KBD_EventsEnable();
	}

	if ( _keyFuncP )
	{
	long	i;

		for(i=n; i > 0; --i, ++kp)
		{
		US	key = (US)*kp & 0xff7f;
		UB	ispress = !(*kp & 0x0080);

			if ( key & 2048 ) // extended ?
				key = (key & 0x007f) + 128;

			if ( (_kbd_ingnoreRepeat && !(_kbd_pressTable[ _keyLUT[key] ] && ispress)) ||
				  !_kbd_ingnoreRepeat )
			{
				_kbd_pressTable[ _keyLUT[key] ] = ispress;
				_keyFuncP( _keyLUT[key], ispress ? KBD_PRESS : KBD_RELEASE );
			}
		}
	}
}

//==============================
void KBD_Close(void)
{
	if ( _alreadyInstalled )
	{
		_disable();
			_dos_setvect( 0x09, _oldHandler );
		_enable();
		_alreadyInstalled = 0;
	}
}
//==============================
long KBD_Open(void)
{
	if NOT( _alreadyInstalled )
	{
		KBD_ResetPressTable();
		_disable();
			_oldHandler = _dos_getvect( 0x09 );
			_dos_setvect( 0x09, __theHandler );
		_enable();
		_alreadyInstalled = 1;

		if NOT( _atexit_filled )
		{
			atexit( KBD_Close );
			_atexit_filled = 1;
		}
	}

	return 0;
}

//==============================
void KBD_FuncSet( UB (*keyFuncP)(US,UB) )
{
	_keyFuncP = keyFuncP;
}

