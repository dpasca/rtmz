//==============================================
// DOSUTIL.HPP -
// Copyright (C) Davide Pasca 1995
//==============================================

#ifndef __DOSUTIL_HPP__
#define __DOSUTIL_HPP__

char DOS_DrivesGetList( char *listP, short *totalP );

typedef struct
{
	char	name[14];
	long	size;
}DirElem;

long filesize(const char *fnameP);
void SortDirElemsByName(DirElem *deP, UL n);
long ScanDir(const char *dirPath, const char *pattern, DirElem **dePP, UL *entriesP );

#endif

