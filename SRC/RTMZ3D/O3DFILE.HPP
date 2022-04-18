//==============================================
// O3DFILE.HPP - 3D file objects
// Copyright (C) Davide Pasca 1994-1995
//==============================================

#ifndef __O3DFILE_HPP__
#define __O3DFILE_HPP__

#include "OBJ3D.HPP"

long O3D_LoadGEO( const char *fileName, long *badPolysP );
long O3D_LoadGEOM( const char *fileName, long *badPolysP );

#endif
