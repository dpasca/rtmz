//==============================================
// EVT.CPP - event stuff
// Copyright (C) by Davide Pasca 1995
//==============================================

#include <conio.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include "EXTTYPES.HPP"
#include "MOU.HPP"
#include "KBD.HPP"
#include "MNU.HPP"
#include "VGAPACK.HPP"
#include "BMP.HPP"
#include "TMR.HPP"
#include "EVT.HPP"

static UB		(*_clickFuncP)(UB,UB,short,short), (*_moveFuncP)(short,short);
static void		(*_idleFuncP)(void), (*_redrFuncP)(void);
static BitMap*	(*_rshapeFuncP)(long,long,long);
static UB		(*_keybFuncP)(US,UB);
static BitMap	*_bufmapP, _fasmap;
static UB		_rshapeDo, /*_visiblePage,*/ _isDoubleBuffer, _updateDisplay=1, _rendImgDamaged;
static long		_rshapeWd, _rshapeHe, _rshapeRes;
static long		_oldScreenMode=-1;

static BitMap   _pointBm;
static UB		_pointAvailable, _recording;
static long		_playbackPos=-1;
static short    _redrDo, _breakMainLoop;
static long		_returnValue;

static UB		*_databaseP;
static long		_dbSiz, _dbAlloc;

//==============================
void EVT_DisplayRestoreOld(void)
{
	if ( _oldScreenMode != -1 )
		VGA_ModeSet( _oldScreenMode );
}

//==============================
static long     _funcStack[ 6 * 32 ];
static long     _funcStackIdx;

void EVT_FuncsPush(void)
{
	if ( _funcStackIdx < 6*32  )
	{
	long    *fsp = &_funcStack[ _funcStackIdx ];

		fsp[0] = (long)_clickFuncP;
		fsp[1] = (long)_moveFuncP;
		fsp[2] = (long)_idleFuncP;
		fsp[3] = (long)_redrFuncP;
		fsp[4] = (long)_rshapeFuncP;
		fsp[5] = (long)_keybFuncP;

		_funcStackIdx += 6;
	}
}

//==============================
void EVT_FuncsPop(void)
{
	if ( _funcStackIdx > 0  )
	{
	long    *fsp = &_funcStack[ _funcStackIdx -= 6 ];

		_clickFuncP = (UB (*)(UB,UB,short,short)) fsp[0];
		_moveFuncP = (UB (*)(short,short))fsp[1];
		_idleFuncP = (void (*)(void))fsp[2];
		_redrFuncP = (void (*)(void))fsp[3];
		_rshapeFuncP = (BitMap* (*)(long,long,long))fsp[4];
		_keybFuncP = (UB (*)(US,UB))fsp[5];

		_rshapeDo = 1;
		_rshapeWd = 320;
		_rshapeHe = 200;
		_rshapeRes = VGA_SCRTYP_320X200;
		_redrDo = 1;
	}
}

//==============================
void EVT_FuncsReset(void)
{
	_clickFuncP = 0;
	_moveFuncP = 0;
	_idleFuncP = 0;
	_redrFuncP = 0;
	_rshapeFuncP = 0;
	_keybFuncP = 0;

	_rshapeDo = 1;
	_rshapeWd = 320;
	_rshapeHe = 200;
	_rshapeRes = VGA_SCRTYP_320X200;
	_redrDo = 1;
}

//==============================
void EVT_DisplayUpdateFlag( UB updateDisplay )
{
	_updateDisplay = updateDisplay;
}

//==============================
void EVT_PointerSet( UB *dataP, US wd, US he )
{
	BMP_Free( &_pointBm );
	_pointAvailable = 0;
	if ( dataP )
	{
		if NOT( BMP_Alloc( &_pointBm, wd, he, 8, 0 ) )
		{
			memcpy( _pointBm.memP, dataP, _pointBm.wd*_pointBm.he );
			_pointAvailable = 1;
		}
	}
}

//==============================
void EVT_FuncMouseClick( UB (*clickFuncP)(UB,UB,short,short) ){ _clickFuncP = clickFuncP; }
void EVT_FuncMouseMove( UB (*moveFuncP)(short,short) ){ _moveFuncP = moveFuncP; }
void EVT_FuncKeyb( UB (*keybFuncP)(US,UB) ){ _keybFuncP = keybFuncP; }
void EVT_FuncReshape( BitMap* (*rshapeFuncP)(long,long,long) ){ _rshapeFuncP = rshapeFuncP; }
void EVT_FuncRedraw( void (*redrFuncP)(void) ){ _redrFuncP = redrFuncP; }
void EVT_FuncIdle( void (*idleFuncP)(void) ){ _idleFuncP = idleFuncP; }
void EVT_PostRedraw(void){ _redrDo = 1; }
void EVT_PostReshape(long wd, long he, long res)
{
	_rshapeDo = 1;
	_rshapeWd = wd;
	_rshapeHe = he;
	_rshapeRes = res;
}

//==============================================
static short    _mouseX, _mouseY, _mouseMoved;
static UB _EVT_MouseClickCB(UB but, UB state, short x, short y)
{
	_mouseX = x; _mouseY = y;
	if ( MNU_EventMouseClick(but,state,x,y) )       return 1;
	if ( _clickFuncP )
		if ( _clickFuncP(but,state,x,y) )       return 1;
	return 0;
}
//==============================================
static UB _EVT_MouseMoveCB(short x, short y)
{
	_mouseX = x; _mouseY = y;
	_mouseMoved = 1;
	if NOT( MNU_EventMouseMove(x,y) )
	if ( _moveFuncP )
		if ( _moveFuncP(x,y) )  return 1;
	return 0;
}

//==============================================
extern BO _MNU_CurMenuIsVisible(void);

static UB _EVT_KeyboardCB(US key, UB state)
{
	if ( state == KBD_PRESS )
		switch ( key )
		{
		case KBD_UP:	if ( MNU_EventMouseMove(_mouseX,_mouseY-=_rshapeHe/50) )
							return 1;
						break;

		case KBD_DOWN:	if ( MNU_EventMouseMove(_mouseX,_mouseY+=_rshapeHe/50) )
							return 1;
						break;

		case KBD_LEFT:	if ( MNU_EventMouseMove(_mouseX-=_rshapeWd/50,_mouseY) )
							return 1;
						break;

		case KBD_RIGHT:	if ( MNU_EventMouseMove(_mouseX+=_rshapeWd/50,_mouseY) )
							return 1;
						break;

		case KBD_PGUP:	if ( MNU_EventMouseMove(_mouseX,_mouseY-=_rshapeHe/10) )
							return 1;
						break;

		case KBD_PGDOWN: if ( MNU_EventMouseMove(_mouseX,_mouseY+=_rshapeHe/10) )
							return 1;
						break;

		case '\n':
			if ( _MNU_CurMenuIsVisible() )
			{
				if ( MNU_EventMouseClick(MOU_BUT_LFT, MOU_BUT_DOWN, _mouseX, _mouseY ) )
					return 1;
			}
			break;

		case 27:
			if ( _MNU_CurMenuIsVisible() )
			{
				if ( MNU_EventMouseClick(MOU_BUT_RGT, MOU_BUT_DOWN, _mouseX, _mouseY ) )
					return 1;
			}
			break;
		}

	if ( _keybFuncP )
		if ( _keybFuncP(key,state) )    return 1;
	return 0;
}

//==============================================
static void mysegv(int a)
{
	printf( "BAD BAD BUG ..SEGMENT VIOLATION !!! (%i)\n", a );
	exit(0);
}

//==============================================
static BO _evt_installed = 0;

void EVT_Install(void)
{
	if ( _evt_installed )
		return;

	signal( SIGSEGV, mysegv );

	_evt_installed = 1;
	TMR_Open( 60 );

	KBD_Open();
	KBD_FuncSet( _EVT_KeyboardCB );

	MOU_Init();
	MOU_PointerHide();
	MOU_FuncSetClick( _EVT_MouseClickCB );
	MOU_FuncSetMove( _EVT_MouseMoveCB );
	_mouseX = MOU_GetX();
	_mouseY = MOU_GetY();
	_rshapeDo = 1;
	_rshapeWd = 320;
	_rshapeHe = 200;
	_rshapeRes = VGA_SCRTYP_320X200;
	_redrDo = 1;
	_rendImgDamaged = 1;
}

//==============================================
void EVT_EventsFlush(void)
{
static UL	nullevtcnt;

	if ( _recording || _playbackPos >= 0 )
	{
		if ( _recording )
		{
			if ( _recording == 2 )
			{
				_recording = 1;
				nullevtcnt = 0;
			}
			else
			{
			MOU_Event_t		evtmou[MOU_EVT_MAX];
			US				evtkey[KBD_EVT_MAX];
			UL				nmou, nkey, sizmou, sizkey, siznew;
			UB				*tp;
	
				nmou = MOU_EventsListGet( evtmou );
				nkey = KBD_EventsListGet( evtkey );
				if ( nmou >= 255 )	nmou = 254;	// make sure it isn't 255. 255
				if ( nkey >= 255 )	nkey = 254;	// is reserved for null event cnt
				if ( !nmou && !nkey && nullevtcnt < 65535 )
					++nullevtcnt;
				else
				{
					if ( nullevtcnt )
					{
						siznew = _dbSiz + sizeof(UB) + sizeof(US);
						if ( siznew > _dbAlloc )
						{
							siznew += 1024;
							if ( tp = (UB *)realloc( _databaseP, siznew ) )
							{
								_databaseP = tp;
								_dbAlloc = siznew;
							}
							else
							{
								printf( "OUT OF MEMORY RECORDING !!\n" );
								goto recpanic; // user shuld know he is in the shit
							}
						}
						_databaseP[ _dbSiz++ ] = 255;
						*(US *)&_databaseP[ _dbSiz ] = nullevtcnt;
						_dbSiz += 2;
						nullevtcnt = 0;
					}

					sizmou = nmou * sizeof(MOU_Event_t);
					sizkey = nkey * sizeof(US);
					siznew = _dbSiz + sizmou + 1 + sizkey + 1;
					if ( siznew > _dbAlloc )
					{
						siznew += 1024;
						if ( tp = (UB *)realloc( _databaseP, siznew ) )
						{
							_databaseP = tp;
							_dbAlloc = siznew;
						}
						else
						{
							printf( "OUT OF MEMORY RECORDING !!\n" );
							goto recpanic; // user shuld know he is in the shit
						}
					}

					_databaseP[ _dbSiz++ ] = nmou;
					memcpytiny( _databaseP + _dbSiz, evtmou, sizmou );
					_dbSiz += sizmou;

					_databaseP[ _dbSiz++ ] = nkey;
					memcpytiny( _databaseP + _dbSiz, evtkey, sizkey );
					_dbSiz += sizkey;
				}
			}
recpanic:
			MOU_EventsFlush(0,0);
			KBD_EventsFlush(0,0);
		}

		if ( _playbackPos >= 0 )
		{
		UB	cnt;

			if ( _playbackPos >= _dbSiz )
				_playbackPos = 0;

			if ( _playbackPos == 0 )
				nullevtcnt = 0;

			if ( nullevtcnt > 0 )
			{
				--nullevtcnt;
				return;
			}

			cnt = _databaseP[ _playbackPos ];
			if ( cnt == 255 )
			{
				if NOT( nullevtcnt = *(US *)&_databaseP[ _playbackPos+1 ] )
					printf( "BAD NULL EVENT !!!\n" );
				else
					--nullevtcnt;
					
				_playbackPos += 3;
				return;
			}

			MOU_EventsFlush( cnt,
							 (MOU_Event_t *)&_databaseP[ _playbackPos+1 ] );
			_playbackPos += cnt * sizeof(MOU_Event_t) + 1;

			cnt = _databaseP[ _playbackPos ];
			KBD_EventsFlush( cnt,
							 (US *)&_databaseP[ _playbackPos+1 ] );
			_playbackPos += cnt * sizeof(US) + 1;
		}
	}
	else
	{
		MOU_EventsFlush(0,0);
		KBD_EventsFlush(0,0);
	}
}

//==============================================
void EVT_MainLoopReturn(long retval)
{
	_breakMainLoop = 1;
	_returnValue = retval;
}

//==============================================
void _EVT_DisplayUpdate(void)
{
	VGAX_UpdateDisplay( _bufmapP, 1 );
}

//==============================================
//#include "COLORS.H"
long EVT_MainLoop(void)
{
	while NOT( _breakMainLoop )
	{
		//COL_SetOneVGA64(0,63,0,0);
		if ( _rshapeDo )
		{
			if ( _rshapeFuncP )
				if ( _bufmapP = _rshapeFuncP(_rshapeWd,_rshapeHe,_rshapeRes) )
				{
					if ( _breakMainLoop )   goto puzzavia;
					if ( _oldScreenMode == -1 )
						_oldScreenMode = VGA_SetScreenType( _rshapeRes, 1 );
					else
					{
						VGA_DisplayClear();
						VGA_SetScreenType( _rshapeRes, 1 );
						VGA_DisplayClear();
					}

					if ( _rshapeRes & VGA_MODEX )
					{
						//VGAX_SetActivePage( _visiblePage ? 0 : 1 );
						//VGAX_SetVisiblePage( _visiblePage );
						_isDoubleBuffer = 1;
					}
					else
						_isDoubleBuffer = 0;

					BMP_Set( _bufmapP );
					MOU_BoundsSet(0, 0, _rshapeWd-1, _rshapeHe-1 );
				}
			_rshapeDo = 0;
		}
		EVT_EventsFlush();
		if ( _breakMainLoop )   goto puzzavia;

		//if ( kbhit() )
		//      if ( _keybFuncP )       _keybFuncP(0,getch());
		//COL_SetOneVGA64(0,0,0,0);

		if ( _idleFuncP )
		{
			_idleFuncP();
			if ( _breakMainLoop )   goto puzzavia;
		}

		if ( _redrFuncP )
		{
			if ( _redrDo == 1 )
			{
				_redrFuncP();
				if ( _breakMainLoop )   goto puzzavia;
				_redrDo = 0;
			}
			else
			if ( _redrDo == 0 )
			{
				_redrFuncP();
				if ( _breakMainLoop )   goto puzzavia;
				BMP_Free( &_fasmap );
				if NOT( BMP_Alloc( &_fasmap, _bufmapP->wd, _bufmapP->he, 8, 0 ) )
				{
					memcpy( _fasmap.memP, _bufmapP->memP, _fasmap.wd*_fasmap.he );
					//printf( "fast = buf\n" );
					_redrDo = -1;
				}
				else
					_redrDo = -2;
			}
			else
			if ( _redrDo == -1 )
			{
				memcpy( _bufmapP->memP, _fasmap.memP, _fasmap.wd*_fasmap.he );
				//printf( "buf = fast\n" );
				_redrDo = -2;
			}
		}

		if ( _bufmapP )
		{
			if ( (MNU_Render(_bufmapP) || (_pointAvailable && _mouseMoved))  && _redrDo == -2 )
				_redrDo = -1;

			if ( _pointAvailable )
				BMP_BitMapPut0CP( &_pointBm, _mouseX-_pointBm.wd/2, _mouseY-_pointBm.he/2 );
			//else
			//      BMP_PixelPutCP( _mouseX, _mouseY, 1 );

			/*if ( _isDoubleBuffer )
			{
				//VGAX_SetActivePage( _visiblePage ? 0 : 1 );
				//VGAX_SetVisiblePage( _visiblePage );
				VGAX_UpdateDisplay( _bufmapP, 1 );
				//_visiblePage = _visiblePage ? 0 : 1;
				//VGAX_SetVisiblePage( _visiblePage );
			}*/
			if ( _updateDisplay && _redrDo != -2 )
				VGAX_UpdateDisplay( _bufmapP, 1 );
		}
		//_mouseMoved = 0;
	}
puzzavia:
	_breakMainLoop = 0;
	BMP_Free( &_fasmap );

	return _returnValue;
}

//==============================================
void EVT_RecordingStop(void)
{
	_recording = 0;
}

//==============================================
void EVT_RecordingStart(void)
{
	_dbSiz = 0;
	_dbAlloc = 0;
	_recording = 2;
}

//==============================================
void EVT_PlaybackStart(void){ _playbackPos = 0; }
void EVT_PlaybackStop(void){ _playbackPos = -1; KBD_ResetPressTable(); }
BO	 EVT_PlaybackIsActive(void){ return _playbackPos >= 0; }

//==============================================
#define EREC_ID	MAKE_ID('E','R','E','2')

long EVT_RecordingSave( const char *fnameP )
{
FILE    *fp;
long    err;

	if ( fp = fopen(fnameP, "wb") )
	{
	UL      erecid;

		erecid = EREC_ID;
		err =  FWRITE(&erecid, 4, fp);
		err |= FWRITE(&_dbSiz, sizeof(UL), fp);
		err |= FWRITE(_databaseP, _dbSiz, fp);

		fclose( fp );

		if NOT(err)
			return 0;
	}

	return -1;
}
//==============================================
long EVT_PlaybackLoad( const char *fnameP )
{
FILE    *fp;
UL              erecid;
UB              *tp;

	if NOT( fp = fopen(fnameP, "rb") )      return -1;

	if ( FREAD(&erecid, sizeof(UL), fp) )	goto exitErr;
	if ( erecid != EREC_ID )				goto exitErr;

	if ( FREAD(&_dbSiz, sizeof(UL), fp) )   goto exitErr;

	if NOT( tp = (UB *)realloc( _databaseP, _dbSiz ) )
		goto exitErr;
	_databaseP = tp;
	_dbAlloc = _dbSiz;

	if ( FREAD(_databaseP, _dbSiz, fp) )    goto exitErr;

	fclose( fp );
	return 0;

exitErr:
	if ( _dbSiz )   _dbSiz = 0;
	fclose( fp );
	return -1;
}

