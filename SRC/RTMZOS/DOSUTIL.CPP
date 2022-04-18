//==============================================
// DOSUTIL.CPP
// Copyright (C) Davide Pasca 1995-97
//
// TABS=4
//==============================================
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <dos.h>

#include "EXTTYPES.HPP"
#include "DOSUTIL.HPP"

//================================
char DOS_DrivesGetList( char *listP, short *totalP )
{
unsigned	old, total, i, j, n;
char		*runlistP;

	_dos_getdrive( &old );
	_dos_setdrive( old, &total );
	n = total;
	if ( n > 32 )	n=32;
	runlistP = listP;
	for(i=1; i <= n; ++i)
	{
		_dos_setdrive( i, &total );
		_dos_getdrive( &j );
		if ( i == j )
			*runlistP++ = 'A' + i - 1;
	}
	
	*totalP = runlistP - listP;
	_dos_setdrive( old, &total );

	return 'A' + old - 1;
}

//================================
/*long filesize(const char *fnameP)
{
FILE	*stream;
long	curpos, length;

	if NOT( stream = fopen( fnameP, "rb" ) )
		return -1;

	curpos = ftell(stream);
	fseek(stream, 0L, SEEK_END);
	length = ftell(stream);
	fseek(stream, curpos, SEEK_SET);

	fclose( stream );

	return length;
}*/

//================================
static short SDE_sort_function( const DirElem *a, const DirElem *b )
{
   return( strcmp(a->name,b->name) );
}

//--------------------------------
void SortDirElemsByName(DirElem *deP, UL n)
{
	qsort( deP, n, sizeof(DirElem), (int (*)(const void*,const void*))SDE_sort_function );
}

//================================
long ScanDir(const char *dirPath, const char *pattern, DirElem **dePP, UL *entriesP )
{
DirElem			*deP;
char			patDPat[1024];
find_t			fileInfo;
long			cnt, err;

	strcpy( patDPat, dirPath );
	strcat( patDPat, pattern );

	cnt = *entriesP;
	deP = *dePP;

	err = _dos_findfirst(patDPat, _A_NORMAL | _A_SUBDIR, &fileInfo);
	while NOT(err)
	{
		if NOT( deP = (DirElem *)realloc(deP, (cnt+1)*sizeof(DirElem)) )
		{
			*entriesP = cnt;
			return -1;
		}
		strcpy( deP[cnt].name, fileInfo.name );

		if ( fileInfo.attrib & _A_SUBDIR )
			deP[cnt].size = -1;
		else
			deP[cnt].size = fileInfo.size;

		++cnt;
		err = _dos_findnext( &fileInfo );
	}	

	*entriesP = cnt;
	*dePP = deP;

	return 0;
}

