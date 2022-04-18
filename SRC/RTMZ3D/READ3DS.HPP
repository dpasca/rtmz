//==============================================
// READ3DS.HPP -
// Copyright (C) Davide Pasca 1995
//==============================================

#ifndef __READ3DS_HPP__
#define __READ3DS_HPP__

#include "OBJ3D.HPP"

#define LIST_FIELDS  \
	char name[80];   \
	void *next;

typedef struct {
	LIST_FIELDS
} List;

long O3D_Load3DS(const char *fnameP);

#endif
