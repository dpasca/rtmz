//==============================================
// RTMZMENU.CPP - menu stuff for rtmz
// Copyright (C) by Davide Pasca 1995
//==============================================

#include <stdio.h>
#include <conio.h>
#include <string.h>
#include <direct.h>
#include "EXTTYPES.HPP"
#include "PASCALIB.HPP"
#include "DOSUTIL.HPP"
#include "KBD.HPP"
#include "MNU.HPP"

extern void openObjMenu( const char *pathP, const char *fnameP );
extern void openTexMenu( const char *pathP, const char *fnameP );

extern void	controlModeM(long,long);
extern void	renderingM(long,long);
extern void	textureM(long,long);
extern void	chResM(long,long);
extern void	lsrcM(long,long);
extern void	lambM(long,long);
extern void	mspecularM(long,long);
extern void	mshininessM(long,long);
extern void	quitProgramM(long,long);

//==============================================
typedef struct
{
	char	path[512];
	char	extList[16][5];
	void	(*userCallBack)(const char *,const char *);
}MFR_Data_t;

//===========================
static UB isVolumeName(const char *nameP)
{
long	len=strlen(nameP);

	return len >= 2 && nameP[len-2] == ':' && nameP[len-1] == '\\';
}

//==============================================
static void substring( char *sub, const char *orig )
{
long	i,len;

	len = strlen( orig );
	for(i=0; i < len; ++i, ++orig)
		if ( *orig != ' ' && *orig != '>' )
			break;

	for(; i < len; ++i, ++orig)
		if ( *orig != ' ' && *orig != '>' )
			*sub++ = *orig;
		else
			break;
	*sub++ = 0;
}
//----------------------------------------------
static void _MFR_RebuildMenu(long menuID);
static void _MFR_MenuCallBack(long m, long i)
{
MFR_Data_t	*dataP;
char		name[MNU_ITM_MAXLEN], filename[20];
long		len;


	dataP = (MFR_Data_t *)MNU_UserLongGet( m );
	MNU_ItemNameGet( m, i, name );
	substring( filename, name );

	len = strlen(name);
	if ( name[ len-1 ] == '>' )
	{
	long	objpathlen;

		objpathlen = strlen(dataP->path);
		if NOT( strcmp(filename,"..") )
		{
			--objpathlen;
			while ( --objpathlen >= 0 )
				if ( dataP->path[ objpathlen ] == '\\' )
				{
					dataP->path[ objpathlen+1 ] = 0;
					goto switchdir;
				}
			return;
		}
		else
		{
			strcat( filename, "\\" );
			if ( isVolumeName(filename) )
				strcpy( dataP->path, filename );
			else
				strcat( dataP->path, filename );
		}
switchdir:
		_MFR_RebuildMenu( m );
	}
	else
		dataP->userCallBack( dataP->path, filename );
}

//----------------------------------------------------
static void _MFR_RebuildMenu(long menuID)
{
MFR_Data_t	*dataP;
DirElem		*deP, *rundeP;
UL			cnt;
long		i;
char		curVol;

	dataP = (MFR_Data_t *)MNU_UserLongGet( menuID );

	deP = 0;	cnt = 0;
	KBD_Close();
	ScanDir( dataP->path, "*.*", &deP, &cnt );
	KBD_Open();

	SortDirElemsByName( deP, cnt );

	MNU_ItemsReset( menuID );

	i = strlen( dataP->path );
	if ( dataP->path[i-2] == ':' && dataP->path[i-1] == '\\' )
	{
	char	drivelist[32], *dlP;
	short	ndrives;

		curVol = dataP->path[0];
		DOS_DrivesGetList( drivelist, &ndrives );
		for(dlP = drivelist; ndrives > 0; --ndrives, ++dlP)
		{
		char	buf[100];

			if ( curVol == *dlP )
				sprintf( buf, "         >%c: <DIR>", *dlP );
			else
				sprintf( buf, "          %c: <DIR>", *dlP );
			MNU_ItemAddCB( menuID, buf, _MFR_MenuCallBack );
		}
	}
	else
		MNU_ItemAddCB( menuID, "          .. <DIR>", _MFR_MenuCallBack );

	rundeP = deP;
	for(i=cnt; i > 0; --i, ++rundeP)
	{
		if ( rundeP->size < 0 && strcmp(rundeP->name, ".") && strcmp(rundeP->name, "..") )
		{
		char	buf[128];

			sprintf( buf, "%12s <DIR>", rundeP->name );
			MNU_ItemAddCB( menuID, buf, _MFR_MenuCallBack );
		}
	}

	rundeP = deP;
	for(i=cnt; i > 0; --i, ++rundeP)
	{
		if ( rundeP->size >= 0 && strcmp(rundeP->name, ".") && strcmp(rundeP->name, "..") )
		{
		char	buf[128], *extP;
		long	len;

			len = strlen(rundeP->name);
			if ( len >= 5 )
			{
			char	(*extlistp)[5];

				extP = rundeP->name + (len-4);
				extlistp = dataP->extList;
				while( **extlistp )
					if NOT( strcmpi( extP, *extlistp++ ) )
					{
						sprintf( buf, "%12s %4ldK", rundeP->name, rundeP->size+1023>>10 );
						MNU_ItemAddCB( menuID, buf, _MFR_MenuCallBack );
					}
			}
		}
	}

	if ( deP )	free( deP );
}

//----------------------------------------------
long MFR_Create( long menuID, const char *pathP,
				 void (*userCallBack)(const char *,const char *), ... )
{
MFR_Data_t	*dataP;

	if NOT( dataP = (MFR_Data_t *)malloc(sizeof(MFR_Data_t)) )
		return -1;

	strcpymaxsize( dataP->path, pathP, sizeof(dataP->path) );
	dataP->userCallBack = userCallBack;

va_list	arg;
char	*txtp, (*extlistp)[5];

	va_start( arg, userCallBack );
	txtp = va_arg( arg, char * );
	extlistp = dataP->extList;
	while( *txtp )
	{
		strcpy( *extlistp++, txtp );
		txtp = va_arg( arg, char * );
	}
	**extlistp = 0;
	va_end( arg );

	MNU_UserLongSet( menuID, (long)dataP );
	_MFR_RebuildMenu( menuID );

	return 0;
}
//==============================================
void MFR_Dispose( long menuID )
{
long	dataP;

	dataP = MNU_UserLongGet( menuID );
	FREEIF( (void *)dataP );
}

//==============================================
static long	_liteMID, _lsrcMID, _lambMID, _mateMID, _mspeMID, _mshiMID;
static long	_openTxMID;
long		_mainMID = 0, _rendMID=0, _textureMID=0, _openMID=0, _resoMID=0, _cmodeMID=0;

void menuDispose(void)
{
	MNU_Dispose( _mainMID, 1 );
}

//==============================================
long menuCreate(const char *curPathP)
{
long	i;

static char *valueNCol[]=
{ "Low", "Medium", "High", "-",
  "Red", "Green", "Blue", "Yellow", "White" };

#define TXC	7
#define BKC	5
#define LTC	4
#define DKC	6

	_mainMID = MNU_New(0,TXC,BKC,LTC,DKC);
	_openMID = MNU_New(0,TXC,BKC,LTC,DKC);
	_cmodeMID = MNU_New(0,TXC,BKC,LTC,DKC);
	_resoMID = MNU_New(0,TXC,BKC,LTC,DKC);

	MNU_ItemAddID( _mainMID, "Open", _openMID );
	MFR_Create( _openMID, curPathP, openObjMenu,
					".GEO", ".GEM", ".3DS", "" );
	MNU_ItemAddID( _mainMID, "Control Mode", _cmodeMID );
		MNU_ItemAddCB( _cmodeMID, "Trackball  <R>", controlModeM, MNU_FLG_ICHECKED );
		//MNU_ItemAddCB( _cmodeMID, "Flying Skate", controlModeM );
		MNU_ItemAddCB( _cmodeMID, "Move Light <L>", controlModeM );

	MNU_ItemAddID( _mainMID, "Resolution", _resoMID );
		MNU_ItemAddCB( _resoMID, "320 x 200         <1>", chResM, MNU_FLG_ICHECKED );
		MNU_ItemAddCB( _resoMID, "640 x 480         <2>", chResM );
		MNU_ItemAddCB( _resoMID, "800 x 600         <3>", chResM );

	MNU_ItemAddID( _mainMID, "-", 0 );

	_rendMID = MNU_New(0,TXC,BKC,LTC,DKC);
	MNU_ItemAddID( _mainMID, "Rendering", _rendMID );
		MNU_ItemAddCB( _rendMID, "Flat Shading       <F>", renderingM, MNU_FLG_ICHECKED );
		MNU_ItemAddCB( _rendMID, "Gouraud Shading    <G>", renderingM );
		MNU_ItemAddCB( _rendMID, "Forgotten Shading  <P>", renderingM );
		MNU_ItemAddCB( _rendMID, "Texture-Mapping    <T>", renderingM );
		MNU_ItemAddCB( _rendMID, "Z-Buffer           <Z>", renderingM );
		MNU_ItemAddCB( _rendMID, "-", renderingM );
		MNU_ItemFlagsSet( _rendMID, 2, 0 );

		_textureMID = MNU_New(0,TXC,BKC,LTC,DKC);
		MNU_ItemAddID( _rendMID, "Texture Specs", _textureMID );
			_openTxMID = MNU_New(0,TXC,BKC,LTC,DKC);
			MNU_ItemAddID( _textureMID, "Open", _openTxMID );
				MFR_Create( _openTxMID, curPathP, openTexMenu, ".PCX", "" );
			MNU_ItemAddCB( _textureMID, "-", textureM );
			MNU_ItemAddCB( _textureMID, "2D Mapping            <Y>", textureM, MNU_FLG_ICHECKED );
			MNU_ItemAddCB( _textureMID, "3D Mapping            <U>", textureM );
			MNU_ItemAddCB( _textureMID, "Chrome Mapping        <K>", textureM );
			MNU_ItemAddCB( _textureMID, "-", textureM );
			MNU_ItemAddCB( _textureMID, "Sphere Displacement   <S>", textureM, MNU_FLG_ICHECKED );
			MNU_ItemAddCB( _textureMID, "Cylinder Displacement <C>", textureM );

	_liteMID = MNU_New(0,TXC,BKC,LTC,DKC);
	MNU_ItemAddID( _mainMID, "Lights", _liteMID );

		_lsrcMID = MNU_New(0,TXC,BKC,LTC,DKC);
		MNU_ItemAddID( _liteMID, "Source", _lsrcMID );
			for (i=0; i < 9; ++i)
				MNU_ItemAddCB( _lsrcMID, valueNCol[i], lsrcM );

		_lambMID = MNU_New(0,TXC,BKC,LTC,DKC);
		MNU_ItemAddID( _liteMID, "Ambient", _lambMID );
			for (i=0; i < 9; ++i)
				MNU_ItemAddCB( _lambMID, valueNCol[i], lambM );


	_mateMID = MNU_New(0,TXC,BKC,LTC,DKC);
	MNU_ItemAddID( _mainMID, "Materials", _mateMID );
		_mspeMID = MNU_New(0,TXC,BKC,LTC,DKC);
		MNU_ItemAddID( _mateMID, "Specular", _mspeMID );
			for (i=0; i < 9; ++i)
				MNU_ItemAddCB( _mspeMID, valueNCol[i], mspecularM );

		_mshiMID = MNU_New(0,TXC,BKC,LTC,DKC);
		MNU_ItemAddID( _mateMID, "Shininess", _mshiMID );
			MNU_ItemAddCB( _mshiMID, "Low    - paper", mshininessM );
			MNU_ItemAddCB( _mshiMID, "Medium - plastic", mshininessM );
			MNU_ItemAddCB( _mshiMID, "High   - metal", mshininessM );
			MNU_ItemAddCB( _mshiMID, "Super  - enamel", mshininessM );


	MNU_ItemAddID( _mainMID, "-", 0 );
	MNU_ItemAddCB( _mainMID, "Quit", quitProgramM );
	long aboutMID = MNU_New(0,TXC,BKC,LTC,DKC);
	MNU_ItemAddID( _mainMID, "About", aboutMID );
		MNU_ItemAddCB( aboutMID, "RTMZ v3.0                     ", 0 );
		MNU_ItemAddCB( aboutMID, "by Davide Pasca               ", 0 );
		MNU_ItemAddCB( aboutMID, " dpasca@ix.netcom.com         ", 0 );
		MNU_ItemAddCB( aboutMID, " dpasca@val.net               ", 0 );
		MNU_ItemAddCB( aboutMID, " http://www.netcom.com/~dpasca", 0 );
		MNU_ItemAddCB( aboutMID, " http://val.net/~dpasca       ", 0 );
		MNU_ItemAddCB( aboutMID, "", 0 );
		MNU_ItemAddCB( aboutMID, "ciaox", 0 );

	MNU_Set( _mainMID );

	return 0;
}
