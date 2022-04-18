//==============================================
// VGAPACK.CPP
// Copyright (C) Davide Pasca 1995-97
//
// TABS=4
//==============================================
#include <stdlib.h>
#include <stdio.h>
#include <dos.h>
#include <mem.h>

//#include "TIMER.HPP"
#include "PASCALIB.HPP"
#include "BMP.HPP"
#include "VGAPACK.HPP"
#include "VESA.HPP"

short VGA_GetActScanLine(void)
{
return (US)inp( 0x3da ) & 0x00ff;
}

#ifdef MODEX
void VGAX_SetActivePage(short page)
{
	VGAX_SetActiveStart((UL)page * (UL)_vgax_bpr * _vgax_he);
	//_vga_lastFrame = GetTicks();
}
void VGAX_SetVisiblePage(short page)
{
	VGAX_SetVisibleStart((UL)page * (UL)_vgax_bpr * _vgax_he);
}
void VGAX_PutPixel(US x, US y, UB color)
{
	outpw(0x3c4, (0x0100 << (x & 3)) | 0x0002);
	*(_VLOOffs[y]+ (x >> 2) + _vgax_actStart) = color;
}
UB VGAX_GetPixel(US x, US y)
{
	US	xx;
	
	xx = x & 3;
	outpw(0x3c4, (0x0100 << xx) | 0x0002);
	outpw(0x3ce, (xx<<8) | 0x0004 );
	return *(_VLOOffs[y]+ (x >> 2) + _vgax_actStart);
}
#endif

//===========================================
UB *_VLOOffs[600];
BitMap	_VLOMap = { 320,200,8,0,(UB*)0xA0000,_VLOOffs };

//===========================================
short	_vgax_wd;
short	_vgax_he;
US		_vgax_bpr;
UL		_vgax_actStart, _vgax_visStart;
UB		*_vgaBaseP=(UB*)0x0a0000;
UL		_vga_lastFrame;
UL		_vga_curstype=0xdeadbeef;

#define STATUS_ADDR     0x3DA
#ifdef MODEX
#define ATTRCON_ADDR    0x3C0
#define MISC_ADDR       0x3C2
#define VGAENABLE_ADDR  0x3C3
#define SEQU_ADDR       0x3C4
#define GRACON_ADDR     0x3CE
#define CRTC_ADDR       0x3D4

//---------------------------------
static void SS_InitScreen(short wd, short he)
{
UL	i;

	_VLOMap.wd = _vgax_wd = wd;
	_VLOMap.he = _vgax_he = he;
	//_VLOMap.Flags = BM_MODE_X_FLG;
	_vgax_bpr = _vgax_wd >> 2;
	_vgax_actStart = _vgax_visStart = 0;

	for(i=0; i < _vgax_he; ++i)
		_VLOMap.offsPP[i] = (UB*)_vgaBaseP + i * _vgax_bpr;
}

static US _ModeX_320x480regs[75] =
{
    0x3c2, 0x00, 0xe7,
    0x3d4, 0x00, 0x5f,
    0x3d4, 0x01, 0x4f,
    0x3d4, 0x02, 0x50,
    0x3d4, 0x03, 0x82,
    0x3d4, 0x04, 0x54,
    0x3d4, 0x05, 0x80,
    0x3d4, 0x06, 0x0d,
    0x3d4, 0x07, 0x3e,
    0x3d4, 0x08, 0x00,
    0x3d4, 0x09, 0x40,
    0x3d4, 0x10, 0xea,
    0x3d4, 0x11, 0xac,
    0x3d4, 0x12, 0xdf,
    0x3d4, 0x13, 0x28,
    0x3d4, 0x14, 0x00,
    0x3d4, 0x15, 0xe7,
    0x3d4, 0x16, 0x06,
    0x3d4, 0x17, 0xe3,
    0x3c4, 0x01, 0x01,
    0x3c4, 0x04, 0x06,
    0x3ce, 0x05, 0x40,
    0x3ce, 0x06, 0x05,
    0x3c0, 0x10, 0x41,
    0x3c0, 0x13, 0x00
};

static US _ModeX_400x300regs[78] =
{
    0x3c2, 0x00, 0xa7,
    0x3d4, 0x00, 0x71,
    0x3d4, 0x01, 0x63,
    0x3d4, 0x02, 0x64,
    0x3d4, 0x03, 0x92,
    0x3d4, 0x04, 0x65,
    0x3d4, 0x05, 0x82,
    0x3d4, 0x06, 0x46,
    0x3d4, 0x07, 0x1f,
    0x3d4, 0x08, 0x00,
    0x3d4, 0x09, 0x40,
    0x3d4, 0x10, 0x31,
    0x3d4, 0x11, 0x80,
    0x3d4, 0x12, 0x2b,
    0x3d4, 0x13, 0x32,
    0x3d4, 0x14, 0x00,
    0x3d4, 0x15, 0x2f,
    0x3d4, 0x16, 0x44,
    0x3d4, 0x17, 0xe3,
    0x3c4, 0x01, 0x01,
    0x3c4, 0x02, 0x0f,
    0x3c4, 0x04, 0x06,
    0x3ce, 0x05, 0x40,
    0x3ce, 0x06, 0x05,
    0x3c0, 0x10, 0x41,
    0x3c0, 0x13, 0x00
};

static US _ModeX_400x600regs[78] =
{
    0x3c2, 0x00, 0xe7,
    0x3d4, 0x00, 0x70,
    0x3d4, 0x01, 0x63,
    0x3d4, 0x02, 0x64,
    0x3d4, 0x03, 0x92,
    0x3d4, 0x04, 0x65,
    0x3d4, 0x05, 0x82,
    0x3d4, 0x06, 0x70,
    0x3d4, 0x07, 0xf0,
    0x3d4, 0x08, 0x00,
    0x3d4, 0x09, 0x60,
    0x3d4, 0x10, 0x5b,
    0x3d4, 0x11, 0x8c,
    0x3d4, 0x12, 0x57,
    0x3d4, 0x13, 0x32,
    0x3d4, 0x14, 0x00,
    0x3d4, 0x15, 0x58,
    0x3d4, 0x16, 0x70,
    0x3d4, 0x17, 0xe3,
    0x3c4, 0x01, 0x01,
    0x3c4, 0x02, 0x0f,
    0x3c4, 0x04, 0x06,
    0x3ce, 0x05, 0x40,
    0x3ce, 0x06, 0x05,
    0x3c0, 0x10, 0x41,
    0x3c0, 0x13, 0x00
};

//---------------------------------
static void setBaseXMode(void)
{
long		temp;

    outp(0x3D4, 0x11);
    temp = inp(0x3D5) & 0x7F;
    outp(0x3D4, 0x11);
    outp(0x3D5, temp);
}

//================================================
static void outReg(US *r)
{
    switch (r[0]) {
        // First handle special cases:

		case ATTRCON_ADDR:
            // reset read/write flip-flop
            inp(STATUS_ADDR);
            outp(ATTRCON_ADDR, r[1] | 0x20);
            // ensure VGA output is enabled
            outp(ATTRCON_ADDR, r[2]);
			break;

		case MISC_ADDR:
		case VGAENABLE_ADDR:
            // Copy directly to port
            outp(r[0], r[2]);
			break;

        case SEQU_ADDR:
		case GRACON_ADDR:
		case CRTC_ADDR:
        default:
            // Index to port
            outp(r[0], r[1]);
            // Value to port+1
            outp(r[0] + 1, r[2]);
			break;
    }
}
//================================================
static void outRegArray(US *r, int n)
{
    while (n--) {
        outReg(r);
        r += 3;
    }
}

//------------------------------
static void SS_Set320x200x256_X(void)
{
	outpw(SEQU_ADDR, 0x0604);
	outpw(CRTC_ADDR, 0x0014);
	outpw(CRTC_ADDR, 0xE317);
	outpw(SEQU_ADDR, 0x0F02);
	memset(_vgaBaseP, 0, 65536);
	SS_InitScreen( 320, 200 );
}

//============================
static void SS_Set320x240x256_X( void )
{
	SS_Set320x200x256_X();
	outp(0x3C2, 0xE3);
	outpw(0x3D4, 0x0616);         /* end vertical blanking */
	outpw(0x3D4, 0xE715);         /* start vertical blanking */
	outpw(0x3D4, 0xDF12);         /* vertical display enable end */
	outpw(0x3D4, 0x2A11);//2c11         /* turn off write protect */
	outpw(0x3D4, 0xEA10);         /* vertical retrace start */
	outpw(0x3D4, 0x3E07);         /* overflow register */
	outpw(0x3D4, 0x0D06);         /* vertical total */
//	outpw(0x3D4, 0xAC11);         /* vertical retrace end AND wr.prot */
	SS_InitScreen( 320, 240 );
}
//================================================
static void SS_Set320x400x256_X(void)
{
	outpw(SEQU_ADDR, 0x0604);
	outpw(CRTC_ADDR, 0x4009);
	outpw(CRTC_ADDR, 0x0014);
	outpw(CRTC_ADDR, 0xE317);
	outpw(SEQU_ADDR, 0x0F02);
	memset(_vgaBaseP, 0, 0x10000);

	SS_InitScreen( 320, 400 );
}
//================================================
static void SS_Set320x480x256_X(void)
{
	setBaseXMode();
    outRegArray(_ModeX_320x480regs, 25);
	outpw(SEQU_ADDR, 0x0F02);
	memset(_vgaBaseP, 0, 0x10000);

	SS_InitScreen( 320, 480 );
}
//================================================
static void SS_Set400x300x256_X(void)
{
	setBaseXMode();
    outRegArray(_ModeX_400x300regs, 26);
	outpw(SEQU_ADDR, 0x0F02);
	memset(_vgaBaseP, 0, 0x10000);
	SS_InitScreen( 400, 300 );
}
//================================================
static void SS_Set400x600x256_X(void)
{
	setBaseXMode();
    outRegArray(_ModeX_400x600regs, 26);
	outpw(SEQU_ADDR, 0x0F02);
	memset(_vgaBaseP, 0, 0x10000);
	SS_InitScreen( 400, 600 );
}
#endif

//====================================
void VGA_ModeSet( US smode )
{
union REGS	inregs, outregs;

	inregs.w.ax = smode;
	int386(0x10, &inregs, &outregs);
}

//====================================
static void set_vesa( long w, long h, long mode )
{
	if NOT( vesa_get_info() )	return;
	if NOT( vesa_set_mode(mode) )
	{
		_VLOMap.wd = w;
		_VLOMap.he = h;
		_VLOMap.dp = 8;
		_VLOMap.Flags = 0;
		BMP_OffsPPCreate( &_VLOMap );
		vesa_free_info();
		return;
	}
	vesa_free_info();

	_VLOMap.wd = w;	_VLOMap.he = h;	_VLOMap.dp = 8;	_VLOMap.Flags = 0;
	BMP_OffsPPCreate( &_VLOMap );
}

//====================================
UB VGA_SetScreenType( UL stype, UB keepPalette )
{
union REGS	inregs, outregs;
UB			oldMode;
UB			pal[768];

	inregs.w.ax = 0x0f00;
	int386(0x10, &inregs, &outregs);
	oldMode = outregs.h.al;

	switch(stype & ~VGA_INTERLINE)
	{
	case VGA_SCRTYP_320X200:	break;
#ifdef MODEX
	case VGA_SCRTYP_320X240_X:	break;
	case VGA_SCRTYP_320X400_X:	break;
	case VGA_SCRTYP_320X480_X:	break;
#endif
	case VGA_SCRTYP_640X480:	break;
	case VGA_SCRTYP_800X600:	break;
	case VGA_SCRTYP_80X24_TXT:	break;
	default:	return oldMode;
	}

	_vga_curstype = stype;
	if ( keepPalette )	VGA_PaletteGet( pal, 0, 256 );

	if ( stype == VGA_SCRTYP_80X24_TXT )
	{
		inregs.w.ax = 3;
		int386(0x10, &inregs, &outregs);
	}
	else
	{
		switch( stype )
		{
		case VGA_SCRTYP_640X400: set_vesa( 640, 400, 0x100 ); break;
		case VGA_SCRTYP_640X480: set_vesa( 640, 480, 0x101 ); break;
		case VGA_SCRTYP_800X600: set_vesa( 800, 600, 0x103 ); break;
			
		default:
			inregs.w.ax = 0x13;
			int386(0x10, &inregs, &outregs);
			//_VLOMap.Flags &= ~BM_MODE_X_FLG;
			switch(stype & ~VGA_INTERLINE)
			{
			case VGA_SCRTYP_320X200:
				_VLOMap.wd = 320;	_VLOMap.he = 200;	_VLOMap.dp = 8;	_VLOMap.Flags = 0;
				BMP_OffsPPCreate( &_VLOMap );
				break;
	
#ifdef MODEX
			case VGA_SCRTYP_320X240_X:	SS_Set320x240x256_X();	break;
			case VGA_SCRTYP_320X400_X:	SS_Set320x400x256_X();	break;
			case VGA_SCRTYP_320X480_X:	SS_Set320x480x256_X();	break;
			case VGA_SCRTYP_400X300_X:	SS_Set400x300x256_X();	break;
			case VGA_SCRTYP_400X600_X:	SS_Set400x600x256_X();	break;
#endif	
			case VGA_SCRTYP_80X24_TXT:	break;
			default:	return oldMode;
			}
			break;
		}
	}

	if ( keepPalette )	VGA_PaletteSet( pal, 0, 256 );

	return oldMode;
}

//==============================================
#ifdef MODEX
void VGAX_SetActiveStart(UL offset)
{
	_vgax_actStart = offset;
}

//==================================================
void VGAX_SetVisibleStart(UL offset)
{
	_vgax_visStart = offset;
	outpw(CRTC_ADDR, 0x0C);               /* set high byte */
	outpw(CRTC_ADDR+1, _vgax_visStart >> 8);
	outpw(CRTC_ADDR, 0x0D);               /* set low byte */
	outpw(CRTC_ADDR+1, _vgax_visStart & 0xff);
}

//------------------------------
void VGAX_ClearScreen(void)
{
	outpw(SEQU_ADDR, 0x0F02);
	memset(_vgaBaseP+_vgax_actStart,0, _vgax_bpr * _vgax_he );
}

//=====================================================
static US _vgax_LMask[4]={0x0f02,0x0e02,0x0c02,0x0802}, //{0x0f02,0x0702,0x0302,0x0102}
		  _vgax_RMask[4]={0x0102,0x0302,0x0702,0x0f02},
		  _vgax_PMask[4]={0x0102,0x0202,0x0402,0x0802};

void VGAX_horizLine(short x1, short x2, short y, UB col)
{
UB	*destP;
US	x14,x24;

	destP = _VLOOffs[y] + _vgax_actStart;
	if (x2-x1 <= 4)
	{
		for(; x1 <= x2; ++x1)
		{
			outpw(0x3c4, _vgax_PMask[x1&3] );
			destP[ x1>>2 ] = col;
		}
		return;
	}

	x14 = x1 >> 2;
	x24 = x2 >> 2;

	outpw(0x3c4, _vgax_LMask[x1&3] );
	destP[ x14 ] = col;

	outpw(0x3c4, _vgax_RMask[x2&3] );
	destP[ x24 ] = col;

	++x14;
	if ( (x24 -= x14) > 0 )
	{
		outpw(0x3c4, 0x0f02);
		memset( destP+x14, col, x24 );
		/*_CX = x24;
		asm{
		les	di,destP
		add	di,x14
		inc	di

		mov	al,col
		cld
		rep	stosb
		} */
	}
}

//=================================
extern void _vgx_copyVLine( UB *d, const UB *s );
#pragma aux _vgx_copyVLine = \
	"mov cx,_vgax_he"\
	"shr cx,1"\
	"xor ebx,ebx"\
	"mov bx,_vgax_bpr"\
	"xor edx,edx"\
	"mov dx,_vgax_wd"\
	"lop: mov al,[esi]"\
		 "add esi,edx"\
		 "mov [edi],al"\
		 "add edi,ebx"\
		 "mov al,[esi]"\
		 "add esi,edx"\
		 "mov [edi],al"\
		 "add edi,ebx"\
		 "dec cx"\
		 "jnz lop"\
	parm [edi][esi]\
	modify [al ebx cx edx edi esi];

//=================================
extern void _vgx_copyBlock( UB *d, const UB *s, UL siz );
#pragma aux _vgx_copyBlock = \
		 "shr ecx,2"\
	"lop: mov ah,[esi+12]"\
		 "mov al,[esi+8]"\
		 "shl eax,16"\
		 "mov ah,[esi+4]"\
		 "mov al,[esi]"\
		 "add esi,16"\
		 "mov [edi],eax"\
		 "add edi,4"\
		 "dec ecx"\
		 "jnz lop"\
	parm [edi][esi][ecx]\
	modify [eax ecx edi esi];
#endif

//=================================
void VGA_DisplayClear(void)
{
	if ( _vga_curstype == VGA_SCRTYP_320X200 )
		memset( (UB *)0xa0000, 0, 320*200 );
	else
	if ( _vga_curstype == VGA_SCRTYP_640X480 )
		vesa_screen_clear( vesa_width, vesa_height, vesa_bits_per_pixel >> 3 );
#ifdef MODEX
	else
		VGAX_ClearScreen();
#endif
}

//=================================
void VGAX_UpdateDisplay( const BitMap *sbmP, UB onepage )
{
UB		*dp, *sp;
long	i, stp;

	switch ( _vga_curstype )
	{
	case VGA_SCRTYP_320X200:
			RefreshClearLoVGAScr( sbmP );
			break;

	case VGA_SCRTYP_640X480:
	case VGA_SCRTYP_800X600:
		vesa_screen_swap_clear( sbmP->memP, 0, vesa_width, vesa_height,
						  vesa_bits_per_pixel >> 3 );
		break;

#ifdef MODEX
	default:
		if ( _vga_curstype & VGA_INTERLINE )	stp = 2;
		else									stp = 1;
	
		sp = sbmP->memP;
		for(long j = 0; j < _vgax_he; ++j, sp += sbmP->wd)
		{
			dp = _VLOMap.offsPP[j] + _vgax_actStart;
			outpw(0x3c4, 0x0102 );	_vgx_copyBlock( dp, sp+0, _vgax_bpr );
			outpw(0x3c4, 0x0202 );	_vgx_copyBlock( dp, sp+1, _vgax_bpr );
			outpw(0x3c4, 0x0402 );	_vgx_copyBlock( dp, sp+2, _vgax_bpr );
			outpw(0x3c4, 0x0802 );	_vgx_copyBlock( dp, sp+3, _vgax_bpr );
		}
		break;
#endif
	}
}

//=================================
/*void VGAX_PutMap0( const BitMap *sbmP, short dx, short dy )
{
UB		*srcP,*dp;
short	sdx,sdy, ddx,ddy;
	US swd;
	UB	*dp2, *sp2;
	long	bprL;
	short	x;

	sdx = sbmP->wd;
	sdy = sbmP->he;

	ddx = _vgax_wd;
	ddy = _vgax_he;

	srcP = sbmP->memP;

	if ( dy < 0 ) 				// PRIMA LA "Y" PER NON SVALIDARE sdx
		if ( (sdy += dy) <= 0 )
			return;
		else
		{
			srcP -= dy * sdx;
			dy = 0;
		}

	if ( dx < 0 )
		if ( (sdx += dx) <= 0 )
			return;
		else
		{
			srcP -= dx;
			dx = 0;
		}


	if ( dx + sdx >= ddx )
		if ( (sdx = ddx - dx) <= 0 )
			return;

	if ( dy + sdy >= ddy )
		if ( (sdy = ddy - dy) <= 0 )
			return;


	swd = sbmP->wd;

	dp = _VLOMap.offsPP[dy] + _vgax_actStart;
	bprL = _vgax_bpr;

	sdx += dx;
	for(x=dx; x < sdx; ++x)
	{
		dp2 = dp + (x >> 2);
		outpw(0x3c4, (0x0100 << (x&3)) | 0x0002 );

		sp2 = srcP++;

#ifdef _SEGMENTED_MEM
		_CX = sdy;
		asm	les	di,dp2
		asm	lds	si,sp2
lop:
asm{	mov	al,ds:[si]
		add	si,swd
		or	al,al
		jz	noSto
		mov	es:[di],al}
noSto:
asm{	add	di,bprL
		dec	cx
		jnz	lop }
#else
/*		asm mov	ecx,sdy
		asm	mov	edi,dp2
		asm	mov	esi,sp2
lop:
asm{	mov	al,[esi]
		add	esi,swd
		or	al,al
		jz	noSto
		mov	[edi],al}
noSto:
asm{	add	edi,bprL
		dec	ecx
		jnz	lop }
* /
#endif
	}
}

//===========================================
void VGAX_RectFill( short p1x, short p1y, short p2x, short p2y, UB col )
{
	for(; p1y <= p2y; ++p1y)
		VGAX_horizLine(p1x, p2x, p1y, col);
}

//===========================================
void VGAX_RectFillColCP( short p1x, short p1y, short p2x, short p2y, UB col, UB pCol )
{
UB		*pP, *pP2;

	if ( p2x < p1x )	swap( p2x, p1x );
	if ( p2y < p1y )	swap( p2y, p1y );

	short mx = _vgax_wd-1;
	short my = _vgax_he-1;

	if ( p2x < 0 ) return;
	else
	if ( p2x > mx ) p2x=mx;

	if ( p2y < 0 ) return;
	else
	if ( p2y > my ) p2y=my;


	if ( p1x < 0 ) p1x=0;
	else
	if ( p1x > mx ) return;

	if ( p1y < 0 ) p1y=0;
	else
	if ( p1y > my ) return;

	if ( (p2y -= p1y-1) <= 0)	return;
	if ( (p2x -= p1x-1) <= 0)	return;

	pP = _VLOMap.offsPP[p1y] + _vgax_actStart;

	for(; p2x--; ++p1x)
	{
		pP2 = pP + (p1x>>2);
		outpw(0x3c4, 0x0100 << (p1x&3) | 0x0002 );
		outpw(0x3ce, ((p1x&3)<<8) | 0x0004 );
		short i = p2y;
		do{
			if (*pP2==pCol)
				*pP2 = col;

			pP2 += _vgax_bpr;
		}while(--i);
	}
}
*/

//====================================
void WaitTof( void )
{
	while NOT( inp(STATUS_ADDR) & 0x08 );
}

//====================================
void RefreshLoVGAScr( const BitMap *bmP )
{
	memcpy(_VLOMap.memP, bmP->memP, (long)_VLOMap.wd * _VLOMap.he );
}

//====================================
extern void memcpyset2(void *d, const void *s, unsigned long pattern, long size );
#pragma aux memcpyset2 =   \
"	shr		ecx,2" \
"	or		ecx,ecx" \
"	jz		nope" \
"lupa:"\
"	mov		ebx,[esi]"\
"	mov		[esi],eax"\
"	mov		[edi],ebx"\
"	add		esi,4"\
"	add		edi,4"\
"	dec		ecx"\
"	jnz		lupa"\
"nope:"\
parm caller [edi][esi][eax][ecx]\
modify [edi esi ebx ecx edx];

void RefreshClearLoVGAScr( const BitMap *bmP )
{
	memcpyset2(_VLOMap.memP, bmP->memP, 0, (long)_VLOMap.wd * _VLOMap.he );
}

//====================================
void VGA_PaletteGet( UB *p, UB staCol, US cnt )
{
US	i;
	cnt += staCol;
	for(i=staCol; i < cnt; ++i)
	{
		outp( 0x3c7, i );
		*p++ = inp( 0x3c9 );
		*p++ = inp( 0x3c9 );
		*p++ = inp( 0x3c9 );
	}
}

//==================================
extern void outpale(const UB *paleP, long cnt );
#pragma aux outpale =   \
"	mov		dx,3c9h"\
"	cld"\
"	rep		outsb"\
parm caller [esi][ecx]\
modify [esi ecx dx];
//====================================
void VGA_PaletteSet( const UB *p, UB staCol, US cnt )
{
	outp( 0x03c8, staCol );
//	outpale( p, cnt * 3 );
	for(cnt *= 3; cnt; --cnt)
		outp( 0x03c9, *p++ );
}

//====================================
void VGA_Palette8Set( const UB *p, UB staCol, US cnt )
{
	outp( 0x03c8, staCol );
	for(cnt *= 3; cnt; --cnt)
		outp( 0x03c9, *p++ >> 2 );
}

//=====================================
void VGA_PaletteSetOne( UB col, UB r, UB g, UB b )
{
	outp( 0x3c8, col );
	outp( 0x3c9, r );
	outp( 0x3c9, g );
	outp( 0x3c9, b );
}

