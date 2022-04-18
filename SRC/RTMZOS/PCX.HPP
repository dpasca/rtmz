//==============================================
// PCX.HPP -
// Copyright (C) Davide Pasca 1995
//==============================================
#ifndef __PCX_HPP__
#define __PCX_HPP__

#include "exttypes.hpp"

long PCX_Read( const char *fnameP, UB **memPP, US *wdP, US *heP, UB *dpP, UB *palP );
long PCX_ReadPalette( const char *fnameP, UB *palP );
long PCX_ReadBitMap( const char *nameP, BitMap *bmP, UB *palP );
long PCX_ReadBitMaps( const char *fnamep, BitMap *bmlistp, long bobwd, long bobhe, long nframes, UB *palP );
#endif
