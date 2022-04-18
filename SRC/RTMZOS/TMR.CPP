//==============================================
// TMR.C - timer handler
// Copyright (C) Davide Pasca 1995
//==============================================

#include <stdlib.h>
#include <signal.h>
#include <dos.h>
#include <conio.h>
#include <time.h>

#include "TMR.HPP"

#define	INTR			0x08
#define PIC_CMD			0x20
#define PIC_NONSPEC_EOI	0x20


static void	(__interrupt __far *_oldHandler)();
static void (__interrupt __far *_userHandler)();

volatile UL 	_TMR_Ticks;
UL				_TMR_TksPerSec=0;
static volatile UL	_counter, _counterStp;

extern void disint(void);
#pragma aux disint =   \
"	cli";

extern void enint(void);
#pragma aux enint =   \
"	sti";

//-------------------------------
static void /*_loadds*/ __interrupt __far handler()
{
//static long	rgb;

	//disint();
	_disable();
		//VGA_PaletteSetOne( 0, rgb >> 8, rgb >>6, rgb );
		//rgb += 8;
		++_TMR_Ticks;
		_counter += _counterStp;
		if ( _counter >= 0x10000 )
		{
			_counter -= 0x10000;
			_chain_intr( _oldHandler );
		}
		else
			outp(PIC_CMD, PIC_NONSPEC_EOI);

		//if ( _userHandler )
		//	_userHandler();

	//enint();
	_enable();
}

//========================================================
void TMR_Close(void)
{
	if ( _oldHandler )
	{
		_disable();
			_dos_setvect(INTR, _oldHandler);
			outp( 0x43, 0x34 );
			outp( 0x40, 0 );
			outp( 0x40, 0 );
	
			signal(SIGABRT, SIG_DFL);
			//signal(SIGFPE, SIG_DFL);
			signal(SIGINT, SIG_DFL);
	
			_oldHandler = 0;
			_userHandler = 0;
		_enable();
	}
}

//========================================================
long TMR_Open( UL freq )
{
UL counter;

	if ( _oldHandler )      return -1;

	atexit( TMR_Close ); /* set traps for error, ctrl c, */
	signal( SIGABRT, (void (*)(int))TMR_Close );
	//signal( SIGFPE, (void (*)(int))TMR_Close );
	signal( SIGINT, (void (*)(int))TMR_Close );

	_disable();

		_TMR_Ticks = 0;
		_TMR_TksPerSec = freq;
		_oldHandler = _dos_getvect( INTR );
		_dos_setvect( INTR, handler );
	
		counter = 0x1234DD;
		if ( freq )
			counter /= freq;
	
		outp( 0x43, 0x34 );
		outp( 0x40, counter & 255 );
		outp( 0x40, counter >> 8 );
	
		_counter = 0;
		_counterStp = counter;
	
	_enable();

	return 0;
}

//========================================================
long TMR_FuncAdd( void (__interrupt __far *handlerp)() )
{
	_userHandler = handlerp;

	return 0;
}

//========================================================
UL TMR_TicksGet(void)
{
	return _TMR_Ticks;
}

//========================================================
UL TMR_TicksPerSecGet(void)
{
	return _TMR_TksPerSec;
}

//========================================================
void TMR_TicksWait( UL howmany )
{
UL	sta = _TMR_Ticks;

	while (_TMR_Ticks-sta < howmany);
}


