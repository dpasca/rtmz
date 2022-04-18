//==============================================
// VESA.CPP
// Copyright (C) Davide Pasca 1995-97
//
// See "readme.txt" for other credits
//
// TABS=4
//==============================================

#include	<i86.h>
#include	<string.h>
#include	"vesa.hpp"

//============================================================
typedef struct
{
	US			Segment;	//The real mode segment (offset is 0).
	US			Selector;	//In protected mode, you need to chuck this dude into a segment register and use an offset 0.
} RealPointer;

typedef struct
{
	char		Signature[4];
	US		Version;
	US		OEMNameOffset;
	US		OEMNameSegment;			//Pointer to OEM name?
	UB		Capabilities[4];
	US		SupportedModesOffset;
	US		SupportedModesSegment;		//Pointer to list of supported VESA and OEM modes (terminated with 0xffff).
	UB		Reserved[238];
} VesaInfo;

typedef struct
{
	US		ModeAttributes;
	UB		WindowAAttributes;
	UB		WindowBAttributes;
	US		WindowGranularity;
	US		WindowSize;
	US		StartSegmentWindowA;
	US		StartSegmentWindowB;
	void		(*WindowPositioningFunction)(long page);
	US		BytesPerScanLine;

	//Remainder of this structure is optional for VESA modes in v1.0/1.1, needed for OEM modes.

	US		PixelWidth;
	US		PixelHeight;
	UB		CharacterCellPixelWidth;
	UB		CharacterCellPixelHeight;
	UB		NumberOfMemoryPlanes;
	UB		BitsPerPixel;
	UB		NumberOfBanks;
	UB		MemoryModelType;
	UB		SizeOfBank;
	UB		NumberOfImagePages;
	UB		Reserved1;

	//VBE v1.2+

	UB		RedMaskSize;
	UB		RedFieldPosition;
	UB		GreenMaskSize;
	UB		GreenFieldPosition;
	UB		BlueMaskSize;
	UB		BlueFieldPosition;
	UB		ReservedMaskSize;
	UB		ReservedFieldPosition;
	UB		DirectColourModeInfo;
	UB		Reserved2[216];
} VesaModeData;

typedef struct
{
	UL		edi;
	UL		esi;
	UL		ebp;
	UL		reserved;
	UL		ebx;
	UL		edx;
	UL		ecx;
	UL		eax;
	US		flags;
	US		es,ds,fs,gs,ip,cs,sp,ss;
} RMREGS;

/****************************************************************************/
/* Private function prototypes.												*/
/****************************************************************************/
static void far * allocate_dos_memory(RealPointer * rp,UL bytes_to_allocate);
static void free_dos_memory(RealPointer * rp);

/****************************************************************************/
/* Private data.															*/
/****************************************************************************/
RealPointer	vesa_info_rp;
RealPointer	vesa_mode_data_rp;

/****************************************************************************/
/* Global data.																*/
/****************************************************************************/

VesaInfo		far	*vesa_info =0;
VesaModeData	far	*vesa_mode_data =0;
long				vesa_granularity;
long				vesa_page;
long				vesa_width;
long					vesa_height;
long					vesa_bits_per_pixel;

/****************************************************************************/
/* Get information about the VESA driver.									*/
/*																			*/
/* Returns:																	*/
/*	1		-	VESA driver present.  vesa_info set to point to a		*/
/*					a structure containing capability info etc.				*/
/*	0		-	No VESA driver.											*/
/*																			*/
/****************************************************************************/
BO vesa_get_info(void)
{
	union		REGS	regs;
	struct		SREGS	sregs;
	RMREGS				rmregs;
	BO				ok			= 0;

	if((vesa_info = (VesaInfo far *)allocate_dos_memory(&vesa_info_rp,sizeof(VesaInfo))) != NULL)
	{
		memset(&rmregs,0,sizeof(rmregs));
		memset(&regs,0,sizeof(regs));
		memset(&sregs,0,sizeof(sregs));
		segread(&sregs);
		rmregs.eax = 0x4f00;
		rmregs.es = vesa_info_rp.Segment;
		rmregs.ds = vesa_info_rp.Segment;
		regs.x.eax = 0x300;
		regs.x.ebx = 0x10;
		regs.x.ecx = 0;
		sregs.es = FP_SEG(&rmregs);
		regs.x.edi = FP_OFF(&rmregs);
		int386x(0x31,&regs,&regs,&sregs);	//Get vesa info.
		if(rmregs.eax == 0x4f)
		{
			ok = 1;
		}
	}
	return(ok);
}

/****************************************************************************/
/* Free the information allocated with vesa_get_info()						*/
/****************************************************************************/
void vesa_free_info(void)
{
	free_dos_memory(&vesa_info_rp);
}

/****************************************************************************/
/* Set VESA video mode.  You MUST have previously called vesa_get_info().	*/
/*																			*/
/* Inputs:																	*/
/*	vmode	-	The VESA mode that you want. e.g. 0x101 is 640x480 256		*/
/*				colours.													*/
/*																			*/
/* Returns:																	*/
/*	1	-	The mode has been set.										*/
/*	0	-	The mode has not been set, probably not supported.			*/
/*																			*/
/****************************************************************************/

BO vesa_set_mode(US vmode)
{
union REGS	regs;

	memset(&regs,0,sizeof(regs));
	regs.w.ax = 0x4f02;
	regs.w.bx = vmode;
	int386(0x10,&regs,&regs);
	if (regs.w.ax == 0x004f)
	{
		vesa_get_mode_info(vmode);
		vesa_set_page(0);			//Probably not needed, but here for completeness.
		return 1;
	}

	return 0;
}

/****************************************************************************/
/* Get VESA mode information.  Information is loaded into vesa_mode_data.	*/
/* vesa_get_info() must be called before calling this function.				*/
/* Should error check this really.											*/
/*																			*/
/* Inputs:																	*/
/*	vmode	-	The mode that you wish to get information about.			*/
/*																			*/
/* Returns:																	*/
/*	1	-	vesa_mode_data points to a filled VesaModeData structure.	*/
/*	0	-	vesa_mode_data probably invalid.  Mode probably not			*/
/*				supported.													*/
/*																			*/
/****************************************************************************/
BO vesa_get_mode_info(US vmode)
{
union	REGS	regs;
struct	SREGS	sregs;
RMREGS			rmregs;
BO				ok = 0;

	if ( (vesa_mode_data = (VesaModeData far *)allocate_dos_memory(&vesa_mode_data_rp,sizeof(VesaModeData))) )
	{
		memset(&rmregs,0,sizeof(rmregs));
		rmregs.es = vesa_mode_data_rp.Segment;
		rmregs.ds = vesa_mode_data_rp.Segment;
		rmregs.edi = 0;
		rmregs.eax = 0x4f01;
		rmregs.ecx = vmode;
		memset(&regs,0,sizeof(regs));
		memset(&sregs,0,sizeof(sregs));
		segread(&sregs);
		regs.x.eax = 0x300;
		regs.x.ebx = 0x10;
		regs.x.edi = (UL)&rmregs;
		int386x(0x31,&regs,&regs,&sregs);
		if (regs.h.al == 0)
		{
			//cache a few important items in protected mode memory area.
			vesa_granularity	= vesa_mode_data->WindowGranularity;
			vesa_width			= vesa_mode_data->PixelWidth;
			vesa_height			= vesa_mode_data->PixelHeight;
			vesa_bits_per_pixel	= vesa_mode_data->BitsPerPixel;
			return 1;
		}
	}

	return 0;
}

/****************************************************************************/
/* Free the info previously allocated with vesa_get_info()					*/
/****************************************************************************/
void vesa_free_mode_info(void)
{
	free_dos_memory(&vesa_mode_data_rp);
}

/****************************************************************************/
/* Set the current vesa screen page.										*/
/*																			*/
/* Inputs:																	*/
/*	vpage	-	Page number to set.											*/
/*																			*/
/****************************************************************************/
void vesa_set_page(long vpage)
{
	union	REGS	regs;

	vesa_page = vpage;
	regs.w.ax = 0x4f05;
	regs.w.bx = 0;
	regs.w.cx = 0;
	regs.w.dx = (US)(vpage == 0 ? 0 : (US)((vpage * 64) / vesa_granularity));
	int386(0x10,&regs,&regs);
/*
	regs.w.ax = 0x4f05;
	regs.w.bx = 1;
	regs.w.cx = 0;
	regs.w.dx = (US)(vpage == 0 ? 0 : (US)((vpage * 64) / vesa_granularity));
	int386(0x10,&regs,&regs);
*/
}

//=======================================================
void vesa_screen_clear(long swidth,long sheight,long bpp)
{
	long		page		= 0;
	long		scr_size	= (swidth * sheight * bpp);	//width * height * bytes per pixel.

	while(scr_size > 0)
	{
		vesa_set_page(page);
		memset((void *)0xa0000,0,(scr_size > 65536 ? 65536 : scr_size));
		scr_size -= 65536;
		page++;
	}

}

//=======================================================
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

void vesa_screen_swap_clear(UB * source, UL pattern, long swidth, long sheight, long bpp)
{
long	page = 0;
long	bps = vesa_mode_data->BytesPerScanLine;

	swidth *= bpp;
	if ( bps == swidth )
	{
	long	scr_size;

		scr_size = swidth * sheight;
		while(scr_size > 0)
		{
			vesa_set_page( page );
			memcpyset2( (void *)0xa0000, source, pattern, (scr_size > 65536 ? 65536 : scr_size) );
			scr_size -= 65536;
			source += 65536;
			page++;
		}
	}
	else
	{
	long	y;
	long	destoff, destoffh, destoffhn;

		vesa_set_page( page );
		destoff = 0;
		destoffhn = destoffh = 0;
		for(y=0; y < sheight; ++y)
		{
			destoffhn = (destoff + swidth) & ~65535;
			if ( destoffh == destoffhn )
			{
				memcpyset2( (void *)(0xa0000 + (destoff & 65535)), source, pattern, swidth );
				if ( destoffh != ((destoff + bps) & ~65535) )
					vesa_set_page( ++page );
			}
			else
			{
			long	len1;

				len1 = destoffhn - destoff;
				if ( len1 <= swidth )
					memcpyset2( (void *)(0xa0000 + (destoff & 65535)), source, pattern, len1 );
					
				vesa_set_page( ++page );

				if ( len1 < swidth )
					memcpyset2( (void *)0xa0000, source + len1, pattern, swidth - len1 );
			}

			source += swidth;
			destoff += bps;
			destoffh = destoff & ~65535;
		}
	}
}

/*
void vesa_screen_swap_clear(UB * source, UL pattern, long swidth, long sheight, long bpp)
{
	void	*	dest;
	long		page		= 0;
	long		scr_size	= (swidth * sheight * bpp);	//width * height * bytes per pixel.

	dest = (void *)source;
	while(scr_size > 0)
	{
		vesa_set_page(page);
		memcpyset2( (void *)0xa0000, dest, pattern, (scr_size > 65536 ? 65536 : scr_size) );
		scr_size -= 65536;
		dest = (void *)((UL)dest + 65536);
		page++;
	}

}
*/

//=============================================================
void vesa_screen_swap(UB * source,long swidth,long sheight,long bpp)
{
void	*dest;
long	page = 0;
long	scr_size = (swidth * sheight * bpp);	//width * height * bytes per pixel.

	dest = (void *)source;
	while(scr_size > 0)
	{
		vesa_set_page(page);
		memcpy((void *)0xa0000,dest,(scr_size > 65536 ? 65536 : scr_size));
		scr_size -= 65536;
		dest = (void *)((UL)dest + 65536);
		page++;
	}

}

/****************************************************************************/
/* Allocate a region of DOS memory.											*/
/****************************************************************************/
static void far * allocate_dos_memory(RealPointer * rp,UL bytes_to_allocate)
{
	void	far	*		ptr		= NULL;
	union	REGS		regs;

	bytes_to_allocate = ((bytes_to_allocate + 15) & 0xfffffff0);	//Round up to nearest paragraph.
	memset(&regs,0,sizeof(regs));
	regs.w.ax = 0x100;
	regs.w.bx = (US)(bytes_to_allocate >> 4);		//Allocate dos memory in pages, so convert bytes to pages.
	int386(0x31,&regs,&regs);
	if(regs.x.cflag == 0)
	{	//Everything OK.
		rp->Segment = regs.w.ax;
		rp->Selector = regs.w.dx;
		ptr = MK_FP(regs.w.dx,0);
	}
	return(ptr);
}

/****************************************************************************/
/* Free an area of DOS memory.												*/
/****************************************************************************/
static void free_dos_memory(RealPointer * rp)
{
	union	REGS		regs;

	regs.w.ax = 0x101;
	regs.w.dx = rp->Selector;
	int386(0x31,&regs,&regs);
}

