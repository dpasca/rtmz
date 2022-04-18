//==============================================
// RTMZBAN.CPP
// Copyright (C) Davide Pasca 1995-97
//
// TABS=4
//==============================================
#ifndef __RTMZBAN_H__
#define __RTMZBAN_H__

#include "EXTTYPES.HPP"

void banpost( long seconds, long pos, const char *fmtP, ... );
void banidle( void );
void banprint( UB col, long y );

#endif
