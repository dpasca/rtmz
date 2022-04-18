//==============================================
// VGAPACK.HPP -
// Copyright (C) Davide Pasca 1995
//==============================================

#ifndef __VGAPACK_HPP__
#define __VGAPACK_HPP__

#include "exttypes.hpp"
#include <dos.h>
#include <conio.h>
//#include "timer.h"

#define VGA_INTERLINE			2
#define VGA_MODEX				1
#define VGA_SCRTYP_320X200		0
#define VGA_SCRTYP_320X240_X	(16|VGA_MODEX)
#define VGA_SCRTYP_320X400_X	(32|VGA_MODEX)
#define VGA_SCRTYP_320X480_X	(64|VGA_MODEX)
#define VGA_SCRTYP_400X300_X	(128|VGA_MODEX)
#define VGA_SCRTYP_400X600_X	(256|VGA_MODEX)
#define VGA_SCRTYP_640X400		512
#define VGA_SCRTYP_640X480		1024
#define VGA_SCRTYP_800X600		2048
#define VGA_SCRTYP_80X24_TXT	4096

#define VLO_WD		320L
#define VLO_HE		200L
#define VLO_SIZ		(VLO_WD*VLO_HE)


US VGA_ModeGet(void);
void VGA_ModeSet( US smode );
UB VGA_SetScreenType( UL stype, UB keepPalette );

extern BitMap	_VLOMap;
extern UB*		_VLOOffs[600];
extern UL		_vga_lastFrame;

void RefreshLoVGAScr( const BitMap *bmP );
void RefreshClearLoVGAScr( const BitMap *bmP );
void WaitTof( void );
//====================================
short VGA_GetActScanLine(void);
void VGAX_WaitDrawPermission(void);

extern short	_vgax_wd;
extern short	_vgax_he;
extern US		_vgax_bpr;
extern UL		_vgax_actStart, _vgax_visStart;
extern UB		*_vgaBaseP;

void VGAX_SetActiveStart(UL offset);
void VGAX_SetVisibleStart(UL offset);
void VGAX_PagCpy(short to, short from);

//------------------------------
void VGAX_SetActivePage(short page);
//------------------------------
void VGAX_SetVisiblePage(short page);
//------------------------------
void VGAX_PutPixel(US x, US y, UB color);
//------------------------------
UB VGAX_GetPixel(US x, US y);

void VGAX_ClearScreen(void);
void VGAX_horizLine(short x1, short x2, short y, UB col);
void VGAX_RectFill( short p1x, short p1y, short p2x, short p2y, UB col );
void VGAX_RectFillColCP( short p1x, short p1y, short p2x, short p2y, UB col, UB pCol );
//void VGAX_DrawPoly( const short *polyP, char nVerts, UB col );
void VGAX_PutMapStrech( short dx, short dy, const BitMap *sbmP, short wd, short he );
void VGAX_PutMap3D( const BitMap *mMapP, short x, short y, long z);
void VGAX_PutMap( const BitMap *sbmP, short dx, short dy );
void VGAX_PutMap0( const BitMap *sbmP, short dx, short dy );
void VGAX_BitBltMskCol(const BitMap *sbmP, short xs, short ys,
						short xd, short yd, short ddx, short ddy, UB col );
void VGA_DisplayClear(void);
void VGAX_UpdateDisplay( const BitMap *sbmP, UB onepage );

void VGA_PaletteGet( UB *p, UB staCol, US cnt );
void VGA_PaletteSet( const UB *p, UB staCol, US cnt );
void VGA_Palette8Set( const UB *p, UB staCol, US cnt );
void VGA_PaletteSetOne( UB col, UB r, UB g, UB b );
#endif

