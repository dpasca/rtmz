//==============================================
// VESA.HPP
// Copyright (C) Davide Pasca 1995-97
//
// See "readme.txt" for other credits
//
// TABS=4
//==============================================
#ifndef __VESA_HPP__
#define __VESA_HPP__

#include "EXTTYPES.HPP"

BO		vesa_get_info(void);
void	vesa_free_info(void);
BO		vesa_set_mode(US vmode);
BO		vesa_get_mode_info(US vmode);
void	vesa_free_mode_info(void);
void	vesa_set_page(long vpage);
void	vesa_screen_swap(UB * source,long swidth,long sheight,long bpp);
void	vesa_screen_swap_clear(UB * source, UL pattern, long swidth, long sheight, long bpp);
void	vesa_screen_clear(long swidth,long sheight,long bpp);

extern	long	vesa_granularity;
extern	long	vesa_page;
extern	long	vesa_width;
extern	long	vesa_height;
extern	long	vesa_bits_per_pixel;

#endif
