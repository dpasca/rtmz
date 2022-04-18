//==============================================
// RTMZ.CPP
// Copyright (C) Davide Pasca 1995-97
//
// TABS=4
//==============================================
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <conio.h>
#include <mem.h>
#include <time.h>
#include <direct.h>

#include "RTMZOS.HPP"
#include "RTMZ3D.HPP"
#include "QUA.HPP"
#include "DOSUTIL.HPP"
#include "RTMZ_SUP.HPP"
#include "RTMZBAN.HPP"
#include "FRPLASMA.HPP"

#define TRACKBALL	0
#define MOVELIGHT	1
// from RTMZMENU.CPP
extern long menuCreate(const char *curPathP);
extern void menuDispose(void);

extern long	_rendMID, _textureMID, _resoMID, _cmodeMID, _mainMID;

static BitMap		_aboutMap;
static BitMap		*_bufmapP;
static long			_obj;
static float		_objSphere, _fov=50.; //, _ax,_ay,_az,_stpAy,_stpAx;
static float		_heCoe;
static float		_objPos[3], _objCenterOffset[3], _objScale;
static UB			_conMode=TRACKBALL;
//UL					_aboutCSum;
static short		_mouseX, _mouseY;
static UL			_rendFlags;
static UB			_thereIsTheMouse, _ctrl_pressed, _shift_pressed, _forceRefresh;
static UB			_recording;
static char			_recFileName[20];
static BO			_do_not_print_pbk_msg=0;

static UL			_texFlags = POE_FLG_TEXTURE;
static UL			_texDisplacement = O3D_TD_SPHERE;
static BitMap		_texMap;
static UB			_texPal[768];
static FractPlasma	_frTex;

#define GEO     'o'
#define GEM     'm'
#define TDS     '3'
static char			_curOType=GEO;

//===========================
static UB _pointer11[11*11]=
{
1,0,0,0,0,1,0,0,0,0,1,
0,2,0,0,0,0,0,0,0,2,0,
0,0,2,0,0,2,0,0,2,0,0,
0,0,0,3,0,0,0,3,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,
1,0,2,0,0,3,0,0,2,0,1,	// center
0,0,0,0,0,0,0,0,0,0,0,
0,0,0,3,0,0,0,3,0,0,0,
0,0,2,0,0,2,0,0,2,0,0,
0,2,0,0,0,0,0,0,0,2,0,
1,0,0,0,0,1,0,0,0,0,1
};
static UB _pointer22[22*22]=
{
1,1,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,1,1,
1,1,1,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,1,1,1,
0,1,2,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,2,1,0,
0,0,2,2,2,0,0,0,0,0,0,0,0,0,0,0,0,2,2,2,0,0,
0,0,0,2,2,2,0,0,0,0,2,2,0,0,0,0,2,2,2,0,0,0,
0,0,0,0,2,2,2,0,0,0,2,2,0,0,0,2,2,2,0,0,0,0,
0,0,0,0,0,2,3,3,0,0,0,0,0,0,3,3,2,0,0,0,0,0,
0,0,0,0,0,0,3,3,3,0,0,0,0,3,3,3,0,0,0,0,0,0,
0,0,0,0,0,0,0,3,0,0,0,0,0,0,3,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
1,1,0,0,2,2,0,0,0,0,3,3,0,0,0,0,2,2,0,0,1,1,	// center
1,1,0,0,2,2,0,0,0,0,3,3,0,0,0,0,2,2,0,0,1,1,	// center
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,3,0,0,0,0,0,0,3,0,0,0,0,0,0,0,
0,0,0,0,0,0,3,3,3,0,0,0,0,3,3,3,0,0,0,0,0,0,
0,0,0,0,0,2,3,3,0,0,0,0,0,0,3,3,2,0,0,0,0,0,
0,0,0,0,2,2,2,0,0,0,2,2,0,0,0,2,2,2,0,0,0,0,
0,0,0,2,2,2,0,0,0,0,2,2,0,0,0,0,2,2,2,0,0,0,
0,0,2,2,2,0,0,0,0,0,0,0,0,0,0,0,0,2,2,2,0,0,
0,1,2,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,2,1,0,
1,1,1,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,1,1,1,
1,1,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,1,1
};

//=======================================
static UL CalcMapCheckSum( const BitMap *bmP )
{
UL      i, sum;
UB      *mapP;

	i = bmP->wd * bmP->he;
	mapP = bmP->memP;
	sum = 0;
	while (i--)
		sum += *mapP++ * i;

	return sum;
}

//=====================================
static UB sysPalette[9*3]=
{
0,0,0,
0x00>>2,0x73>>2,0x00>>2,
0x00>>2,0xa3>>2,0x00>>2,
0x00>>2,0xff>>2,0x00>>2,
5,34,52,
0,24,38,
0,10,24,
50,50,50,
50,0,0
};

//===========================
static void loadMaterialPalette(void)
{
UB		*palp;
long	sta,n;

	MLB_PaletteCompute( 9 );
	palp = MLB_PaletteGet( &sta, &n );
	VGA_Palette8Set( palp, sta, n );
	VGA_PaletteSet( sysPalette, 0, 9 );
}
//=====================================
static void updateForTexture(void)
{
	if ( _rendFlags & (POE_FLG_TEXTURE|POE_FLG_TEXCHROME) )
		MLB_MaterialSetAll( MLB_TXMAP_N_PALETTE_PTR, &_texMap, _texPal, TAG_END );
	else
		MLB_MaterialSetAll( MLB_TXMAP_N_PALETTE_PTR, 0, 0, TAG_END );

	loadMaterialPalette();
}

//=====================================
static long openObj( const char *fname )
{
long    err;
char    ext[5];
long    nBadPolys;

	if ( !fname[0] )        return( -1 );
	EVT_PostRedraw();
	O3D_Free( _obj );

	MLB_LibraryReset();
	MLB_MaterialDisposeAll();

	nBadPolys = 0;
	strcpy(ext,fname+strlen(fname)-4);
	if NOT( strcmpi(ext,".GEO") )
	{
		if NOT( err = O3D_LoadGEO( fname, &nBadPolys ) )
			_curOType=GEO;
	}
	else
	if NOT( strcmpi(ext,".GEM") )
	{
		if NOT( err = O3D_LoadGEOM( fname, &nBadPolys ) )
			_curOType=GEM;
	}
	else
	if NOT( strcmpi(ext,".3DS") )
	{
		if NOT( err = O3D_Load3DS( fname ) )
			_curOType=TDS;
	}
	if NOT( _shift_pressed )
		O3D_OptimizeTriObject();

	if ( nBadPolys )
		banpost( 5, 2, "%ld bad polygons were split !", nBadPolys );
	else
		banpost( 1, 1, "" );

	{
	long	t;

		O3D_ValueGet( O3D_TOTAL_POLYS, &t );
		banpost( 7, -1, "%ld Polygons", t );
		O3D_ValueGet( O3D_TOTAL_VERTS, &t );
		banpost( 7, -1, "%ld Vertices", t );
	}

	if ( err )
	{
		banpost( 7, -1, "ERROR: Cannot open \"%s\" !", fname );
		return( 1 );
	}

	if ( _rendFlags )
	{
		if ( _rendFlags & POE_FLG_TEXTURE )
		{
			if NOT( O3D_TextureDisplace( _texMap.wd, _texMap.he, _texDisplacement ) )
			{
				MLB_MaterialSetDefault( MLB_TXMAP_N_PALETTE_PTR, &_texMap, _texPal, TAG_END );
				MLB_MaterialSetAll( MLB_TXMAP_N_PALETTE_PTR, &_texMap, _texPal, TAG_END );
				O3D_PolysFlags( O3D_PF_OR, _rendFlags );
			}
		}
		else
			O3D_PolysFlags( O3D_PF_OR, _rendFlags );
	}

	loadMaterialPalette();
	W3D_ObjectCenter( _obj );

	float	dim[3], min[3];
	O3D_ValueGet( O3D_TOTAL_SPHERE_RAD, &_objSphere );

	_objScale = 1.;
	O3D_GetMinMaxDim( min, 0, dim );
	float t = dim[0]+dim[1]+dim[2];
	if ( t < 25. )
		if NOT( _objScale = 25. / t )
			_objScale = .001;

	_objSphere *= _objScale;

	_objPos[0] = _objPos[1] = 0;
	_objPos[2] = _objSphere;
	vec3_equ( _objCenterOffset, dim );
	vec3_div( _objCenterOffset, _objCenterOffset, 2 );
	vec3_add( _objCenterOffset, _objCenterOffset, min );

/*	glPushMatrix();
		glLoadIdentity();
		O3D_MatrixSet();
	glPopMatrix();

	/*if ( _conMode != 0 )
		_ax = _ay = _az = _stpAy = _stpAx = 0;*/

	return( 0 );
}

//=====================================
void openObjMenu( const char *pathP, const char *fnameP )
{
char	pathfile[1024];

	strcpy( pathfile, pathP );
	strcat( pathfile, fnameP );
	openObj( pathfile );
}

//=====================================
static void bmp_remapcolor( BitMap *bmP, UB oldCol, UB newCol )
{
UB		*memP;

	memP = bmP->memP;
	for(long i = bmP->wd * bmP->he; i; --i, ++memP)
		if ( *memP == oldCol )
			*memP = newCol;
}

//=====================================
static void bmp_lutapply( BitMap *bmP, const UB *lut )
{
UB		*memP;

	memP = bmP->memP;
	for(long i = bmP->wd * bmP->he; i; --i, ++memP)
		*memP = lut[ *memP ];
}

//=====================================
static void killcolors( BitMap *bmP, UB *palP, US ntokill )
{
long	hit[256*2], *hitP;
UB		newPal[256*3];
long	i;
UB		*memP;

	if ( ntokill >= 256 )
		return;

	// Reset frequency table.
	hitP = hit;
	for(i=0; i < 256; ++i, hitP += 2)
	{
		hitP[0] = 0;
		hitP[1] = i;
	}

	if NOT( memP = bmP->memP )
		return;

	// Calc frequency table.
	for(i = bmP->wd * bmP->he; i; --i)
		++hit[ *memP++ * 2L ];

	// Sort by frequency (this sort takes the 1st elem as
	// value to sort and the 2nd as "bag").
	// Least frequent values are on top.
	QSortLX( hit, 256 );

	// Create a translation table from the old palette to
	// a new palette sorted by least freqent values.
	// Also create the new palette.
	UB	*d = newPal;
	UB	invlut[256];
	for(i=0; i < 256; ++i, d += 3)
	{
	UB		*s;
	long	palidx;

		palidx = hit[ i * 2 + 1 ];
		s = palP + palidx * 3;
		d[0] = s[0], d[1] = s[1], d[2] = s[2];

		invlut[ palidx ] = i;
	}
	// replace original palette
	memcpy( palP, newPal, 256*3 );

	// remove the ntokill least frequent colors from the palette and the look up table
	for(i=0; i < ntokill; ++i)
	{
	long	colpos;

		colpos = i * 3;
		invlut[ hit[i*2+1] ] = COL_FindBestRGBInPalette( palP[ colpos ], palP[ colpos+1 ], palP[ colpos+2 ],
								palP+ntokill*3, 256-ntokill ) + ntokill;
	}
	bmp_lutapply( bmP, invlut );
}

//=====================================
void openTexMenu( const char *pathP, const char *fnameP )
{
char	pathfile[1024];

	strcpy( pathfile, pathP );
	strcat( pathfile, fnameP );

	BMP_Free( &_texMap );
	if NOT( PCX_ReadBitMap( pathfile, &_texMap, _texPal ) )
	{
		if NOT( O3D_TextureDisplace( _texMap.wd, _texMap.he, _texDisplacement ) )
		{
			killcolors( &_texMap, _texPal, 9 );
			updateForTexture();
			EVT_PostRedraw();
		}
	}
}

//===========================
void CleanExit( const char *strErrP )
{
	VGA_ModeSet( 3 );
	menuDispose();
	BMP_Free( &_aboutMap );
	if ( _recording )
	{
		EVT_RecordingSave( _recFileName );
		EVT_RecordingStop();
	}

	printf("\n%s\n\nRTMZ v3.0 " __DATE__".\nCopyright (C) Davide Pasca 1995-1997\n\n", strErrP);
	//printf("This program is only for demonstration purposes.\n");
	printf("E-MAIL: dpasca@ix.netcom.com\n"
		   "        dpasca@val.net\n"
		   "HOME:   http://www.netcom.com/~dpasca\n"
		   "        http://val.net/~dpasca\n\n" );
	printf("Try -h for the available parameters.\n\n");
	printf("ciaox\n");

	exit( 0 );
}

//=====================================
#define TRACKBALLSIZE  (0.8)
static float tb_project_to_sphere(float r, float x, float y)
{
float d, t, z;

    d = sqrt(x*x + y*y);
    if (d < r * 0.70710678118654752440)	// Inside sphere
        z = sqrt(r*r - d*d);
	else	// On hyperbola
	{
        t = r / 1.41421356237309504880;
        z = t*t / d;
    }
    return z;
}

//=====================================
static void trackball(float q[4], float p1x, float p1y, float p2x, float p2y)
{
float a[3]; /* Axis of rotation */
float phi;  /* how much to rotate about axis */
float p1[3], p2[3], d[3];
float t;

    if (p1x == p2x && p1y == p2y)	// Zero rotation
	{
		vec3_set(q,0.,0.,0.);
		q[3] = 1.0;
		return;
    }

    /* First, figure out z-coordinates for projection of P1 and P2 to
     * deformed sphere  */
    vec3_set(p1,p1x,p1y, tb_project_to_sphere(TRACKBALLSIZE,p1x,p1y));
    vec3_set(p2,p2x,p2y, tb_project_to_sphere(TRACKBALLSIZE,p2x,p2y));

    vec3_xprd(a,p2,p1);	// Now, we want the cross product of P1 and P2
    vec3_sub(d,p1,p2);	// Figure out how much to rotate around that axis.
    t = vec3_mag(d) / (2.0*TRACKBALLSIZE);

	CLAMP(t, -1., 1.);
    phi = 2.0 * asin(t);
    QUA_QuatFromAxis(q,phi,a);
}

//=======================================
static UB		_spinning, _moving, _zooming, _changingfov;
static float	_curquat[4], _lastquat[4];
static UB		_lspinning, _lmoving;
static float	_lcurquat[4], _llastquat[4];

static short	_stClickY;
static float	_stClickObjZ, _stClickFov;

static UB clickHandleTrackball(UB but, UB state, short x, short y)
{
	if ( _shift_pressed )
	{
		_stClickY = y;
		_stClickObjZ = _objPos[2];
		_stClickFov = _fov;
		_changingfov = 1;
		banpost( 3, 1, "Field Of Vision = %4.2f", _fov );
	}
	else
	if ( _ctrl_pressed )
	{
		_stClickY = y;
		_stClickObjZ = _objPos[2];
		_zooming = 1;
		banpost( 3, 1, "Object Distance = %6.4f", _objPos[2]*1.5 );
	}
	else
	{
		if (but == MOU_BUT_LFT && (state & MOU_BUT_DOWN))
		{
			_spinning = 0;
			_moving = 1;
		}
	}

	if (but == MOU_BUT_LFT && (state & MOU_BUT_UP))
	{
		if ( _moving && (_mouseX != x || _mouseY != y) )
			_spinning = 1;
		_moving = 0;
		_zooming = 0;
		_changingfov = 0;
	}
	_mouseX = x;  _mouseY = y;

	return 1;
}
//---------------------------------------
static UB clickHandleLight(UB but, UB state, short x, short y)
{
	if ( _shift_pressed )
	{
		/*_stClickY = y;
		_stClickObjZ = _objPos[2];
		_stClickFov = _fov;
		_changingfov = 1;
		banpost( 3, 1, "Field Of Vision = %4.2f", _fov );*/
	}
	else
	if ( _ctrl_pressed )
	{
		/*_stClickY = y;
		_stClickObjZ = _objPos[2];
		_zooming = 1;
		banpost( 3, 1, "Object Distance = %6.4f", _objPos[2]*1.5 );*/
	}
	else
	{
		if (but == MOU_BUT_LFT && (state & MOU_BUT_DOWN))
		{
			_lspinning = 0;
			_lmoving = 1;
		}
	}

	if (but == MOU_BUT_LFT && (state & MOU_BUT_UP))
	{
		if ( _lmoving && (_mouseX != x || _mouseY != y) )
			_lspinning = 1;
		_lmoving = 0;
	}
	_mouseX = x;  _mouseY = y;

	return 1;
}

//=======================================
static UB moveHandleTrackball(short x, short y)
{
	if ( _changingfov )
	{
		if ( _shift_pressed )
		{
			_fov = _stClickFov + 140. * (float)(_stClickY - y) / _bufmapP->he;
			CLAMP( _fov, 20., 120. );
			float coe = _stClickFov / _fov;

			_objPos[2] = _stClickObjZ * coe;

			glMatrixModel( GL_PROJECTION );
				glLoadIdentity();
				gluPerspective( _fov, _heCoe, 1., 100000. );
			glMatrixModel( GL_MODELVIEW );
			EVT_PostRedraw();
		}
	}
	else
	if ( _zooming )
	{
		if ( _ctrl_pressed )
		{
		float	he=_bufmapP->he;

			_objPos[2] = _stClickObjZ + _objSphere * (float)(_stClickY - y)*6. / he;
			if ( _objPos[2] < 0. )
				_objPos[2] = 0.;
			EVT_PostRedraw();
		}
	}
	else
	if (_moving)
	{
	float	wd=_bufmapP->wd, he=_bufmapP->he;

		trackball(_lastquat, _mouseX / wd - .5, 1.-2.0*_mouseY / he,
							 x / wd - .5,		1.-2.0*y / he );
		QUA_Mul( _curquat, _lastquat, _curquat );
		EVT_PostRedraw();
	}
	_mouseX = x;  _mouseY = y;

	return 1;
}
//---------------------------------------
static UB moveHandleLight(short x, short y)
{
	if ( _changingfov )
	{
		if ( _shift_pressed )
		{
			//EVT_PostRedraw();
		}
	}
	else
	if ( _zooming )
	{
		if ( _ctrl_pressed )
		{
		/*float	he=_bufmapP->he;

			_objPos[2] = _stClickObjZ + _objSphere * (float)(_stClickY - y)*6. / he;
			if ( _objPos[2] < 0. )
				_objPos[2] = 0.;
			EVT_PostRedraw();*/
		}
	}
	else
	if (_lmoving)
	{
	float	wd=_bufmapP->wd, he=_bufmapP->he;
	float	m[4][4];

		trackball(_llastquat, -_mouseX / wd + .5,	1.-2.0*_mouseY / he,
							  -x / wd + .5,			1.-2.0*y / he );

		QUA_Mul( _lcurquat, _llastquat, _lcurquat );
		QUA_QuatToMat( _lcurquat, m );

		W3D_LightSetPos( m[0][2], -m[1][2], m[2][2] );

		EVT_PostRedraw();
	}
	_mouseX = x;  _mouseY = y;

	return 1;
}

//=====================================
void controlModeM(long menuID, long item)
{
	MNU_ItemCheckAll( menuID, 0 );
	MNU_ItemCheck( menuID, item, 1 );

	switch( item )
	{
	case 0:	_conMode=TRACKBALL;
			EVT_FuncMouseClick( clickHandleTrackball );
			EVT_FuncMouseMove( moveHandleTrackball );
			break;

	case 1: _conMode=MOVELIGHT;
			EVT_FuncMouseClick( clickHandleLight );
			EVT_FuncMouseMove( moveHandleLight );
			break;
	}
}
//=====================================
void quitProgramM(long menu, long item)
{
	CleanExit( "Menu Quit" );
}

//===========================
long	_font4x7;

static void OpenAll( void )
{
	tr_init();      // non alloca
	//_thereIsTheMouse = MOU_Init() == 0 ? 1 : 0;

	if NOT( _obj = O3D_New() )
		CleanExit( "ERROR, NO MEM FOR THE OBJECT !!!" );
	O3D_Set( _obj );

	if NOT( _font4x7 = BMP_TFLoad( "font4x7.pcx" ) )
		CleanExit( "ERROR OPENING FONTS: \"font4x7.pcx\" EXPECTED." );
	BMP_TFSet( _font4x7 );

	W3D_Initialize();

char	curpath[1024];
	getcwd( curpath, sizeof(curpath)-1 );
	strcat( curpath, "\\" );
	if ( menuCreate(curpath) )
		CleanExit( "ERROR CREATING MENUS !!!" );

	trackball(_curquat, 0.0, 0.0, 0.0, 0.0);
	trackball(_lcurquat, 0.0, 0.0, 0.0, 0.0);
	VGA_PaletteSet( sysPalette, 0, 9 );

	// ===== PREPARE TEXTURE ======
	_texPal[0] = _texPal[1] = _texPal[2] = 0;
	COL_CreateRangeRgb64( _texPal, 1,   128, 0x0000405c, 0x00009cff );
	COL_CreateRangeRgb64( _texPal, 128, 128, 0x0000405c, 0x0000041c );
	/*COL_CreateRangeRgb64( _texPal, 128+32, 32, 0x00002237, 0x00062410 );
	COL_CreateRangeRgb64( _texPal, 128+64,    40, 0x00102414, 0x002c1408 );
	COL_CreateRangeRgb64( _texPal, 128+64+40, 24, 0x002c1408, 0x00332013 );*/

	if ( _frTex.Alloc( 128 ) )
		CleanExit("I think I deserve more 32K.. don't you ??");

	if ( BMP_Alloc( &_texMap, 128, 128, 8, 0 ) )
		CleanExit("C'mon.. more RAM !!!!");

	_frTex.Make( 0, 0, 128-1, 128-1, 5, 101/*clock()*/, 128 );
	memcpy( _texMap.memP, _frTex._mapYP, 128*128 );
	_frTex.Free();
	killcolors( &_texMap, _texPal, 9 );

	/*for (long y=0; y < 256; ++y)
		for (long x=0; x < 256; ++x)
			_texMap.memP[ y*256 + x ] = 10 + x/64 + y/2;
			*/
	//killcolors( &_texMap, _texPal, 9 );
	// ============================
}

//===========================
static void handleCoeffSubMenu( short menuSubID, float *a, float l, float m, float h, float big, float lit )
{
	switch ( menuSubID )
	{
	case 0: a[3] = l;	break;
	case 1: a[3] = m;	break;
	case 2: a[3] = h;	break;
	case 4: vec3_set( a, big, lit, lit );	break;
	case 5: vec3_set( a, lit, big, lit );	break;
	case 6: vec3_set( a, lit, lit, big );	break;
	case 7: vec3_set( a, big, big, lit );	break;
	case 8: vec3_set( a, big, big, big );	break;
	}
}

//===========================
void chResM(long menuID, long which)
{
	MNU_ItemCheckAll( menuID, 0 );
	MNU_ItemCheck( menuID, which, 1 );

	switch (which)
	{
	case 0:	EVT_PostReshape(320,200,VGA_SCRTYP_320X200);	break;
	case 1:	EVT_PostReshape(640,480,VGA_SCRTYP_640X480);	break;
	case 2:	EVT_PostReshape(800,600,VGA_SCRTYP_800X600);	break;
	}
	EVT_PostRedraw();
}

//===========================
void renderingM(long menuID, long which)
{
	if ( which >= 0 && which <= 4 )
	{
		if ( which != 4 )
		{
			MNU_ItemCheck( menuID, 0, 0 );
			MNU_ItemCheck( menuID, 1, 0 );
			//MNU_ItemCheck( menuID, 2, 0 );
			MNU_ItemCheck( menuID, 3, 0 );
			MNU_ItemCheck( menuID, which, 1 );
		}

		O3D_PolysFlags( O3D_PF_AND, ~(POE_FLG_GSHADE | POE_FLG_PSHADE |
									  POE_FLG_TEXTURE | POE_FLG_TEXIS3D |
									  POE_FLG_TEXCHROME) );
		switch ( which )
		{
		case 0: _rendFlags = 0;						break;
		case 1: _rendFlags = POE_FLG_GSHADE;		break;
		//case 2: _rendFlags = POE_FLG_PSHADE;		break;
		case 3: if ( _texMap.memP )
					if NOT( O3D_TextureDisplace( _texMap.wd, _texMap.he, _texDisplacement ) )
						_rendFlags = _texFlags;			break;

		case 4:	{
				long	tmpL, expected;

					W3D_ValueGet( W3D_ZBUFFER_FLG, &tmpL );
					expected = tmpL = tmpL ? 0 : 1;

					W3D_ValueSet( W3D_ZBUFFER_FLG, &tmpL );
					W3D_ValueGet( W3D_ZBUFFER_FLG, &tmpL ); // get di ckeck !!
					MNU_ItemCheck( menuID, which, tmpL );
					if ( expected != tmpL )
						banpost( 6, -1, "WARNING: Not enought memory for the Z-Buffer" );
				}
				break;
		}
		updateForTexture();
		O3D_PolysFlags( O3D_PF_OR, _rendFlags );

		if ( _rendFlags == _texFlags )
		{
			MNU_ItemFlagsSet( _mainMID, 5, MNU_ItemFlagsGet( _mainMID, 5 ) & ~MNU_FLG_IACTIVE );
			MNU_ItemFlagsSet( _mainMID, 6, MNU_ItemFlagsGet( _mainMID, 6 ) & ~MNU_FLG_IACTIVE );
		}
		else
		{
			MNU_ItemFlagsSet( _mainMID, 5, MNU_ItemFlagsGet( _mainMID, 5 ) | MNU_FLG_IACTIVE );
			MNU_ItemFlagsSet( _mainMID, 6, MNU_ItemFlagsGet( _mainMID, 6 ) | MNU_FLG_IACTIVE );
		}
	}
	/*else
		if ( which == 6 )
		{
		}*/

	EVT_PostRedraw();
}

//===========================
void textureM(long menuID, long which)
{
	MNU_ItemCheck( menuID, which, 1 );
	switch ( which )
	{
	case 2: _texFlags = POE_FLG_TEXTURE;
			MNU_ItemCheck( menuID, 3, 0 );
			MNU_ItemCheck( menuID, 4, 0 );
			break;
	case 3: _texFlags = POE_FLG_TEXTURE | POE_FLG_TEXIS3D;
			MNU_ItemCheck( menuID, 2, 0 );
			MNU_ItemCheck( menuID, 4, 0 );
			break;
	case 4: _texFlags = POE_FLG_TEXTURE | POE_FLG_TEXCHROME;
			MNU_ItemCheck( menuID, 2, 0 );
			MNU_ItemCheck( menuID, 3, 0 );
			break;

	case 6: O3D_TextureDisplace( _texMap.wd, _texMap.he, _texDisplacement=O3D_TD_SPHERE );
			MNU_ItemCheck( menuID, 7, 0 );
			break;
	case 7: O3D_TextureDisplace( _texMap.wd, _texMap.he, _texDisplacement=O3D_TD_CYLINDER );
			MNU_ItemCheck( menuID, 6, 0 );
			break;
	}

	if ( (which == 2 || which == 3 || which == 4) && (_rendFlags & POE_FLG_TEXTURE) ) // currently rendering a texture
	{
		_rendFlags = _texFlags;
		O3D_PolysFlags( O3D_PF_AND, ~(POE_FLG_TEXTURE|POE_FLG_TEXIS3D|POE_FLG_TEXCHROME) );
		O3D_PolysFlags( O3D_PF_OR, _rendFlags );
	}

	EVT_PostRedraw();
}

//===========================
static void valncol_setvalues( short item, float *a, float l, float m, float h, float big, float lit )
{
	switch ( item )
	{
	case 0: a[3] = l;	break;
	case 1: a[3] = m;	break;
	case 2: a[3] = h;	break;
	case 4: vec3_set( a, big, lit, lit );	break;
	case 5: vec3_set( a, lit, big, lit );	break;
	case 6: vec3_set( a, lit, lit, big );	break;
	case 7: vec3_set( a, big, big, lit );	break;
	case 8: vec3_set( a, big, big, big );	break;
	}
}

//===========================
static inline vec4_equ(float *d, float *s){d[0] = s[0]; d[1] = s[1]; d[2] = s[2]; d[3] = s[3];}
static inline vec4_add(float *d, float *s){d[0] += s[0]; d[1] += s[1]; d[2] += s[2]; d[3] += s[3];}
static inline vec4_sub(float *d, const float *s1, const float *s2)
{d[0] = s1[0]-s2[0]; d[1] = s1[1]-s2[1]; d[2] = s1[2]-s2[2]; d[3] = s1[3]-s2[3];}
static inline vec4_mul(float *d, float t){d[0] *= t; d[1] *= t; d[2] *= t; d[3] *= t;}

#define	ASMA_NFRAMES	10
static UB		_asma_morphing, _asma_frame;
static long		_asma_matid;
static float	_asma_sta[4], _asma_stp[4];

//----------------------------------------------------
static void anim_reach_end(void)
{
	if ( _asma_morphing == 1 )
	{
		vec4_mul( _asma_stp, ASMA_NFRAMES - _asma_frame );
		vec4_add( _asma_sta, _asma_stp );
		MLB_MaterialSetDefault( _asma_matid, &_asma_sta, TAG_END );
		MLB_MaterialSetAll( _asma_matid, &_asma_sta, TAG_END );
	}
	else
	if ( _asma_morphing == 2 )
	{
		_asma_sta[0] += _asma_stp[0] * (ASMA_NFRAMES - _asma_frame);
		MLB_MaterialSetDefault( _asma_matid, _asma_sta[0], TAG_END );
		MLB_MaterialSetAll( _asma_matid, _asma_sta[0], TAG_END );
	}
}

//----------------------------------------------------
static void anim_setmat_query( long id, float *sta, float *end )
{
	anim_reach_end();
	vec4_equ( _asma_sta, sta );
	vec4_sub( _asma_stp, end, sta );
	vec4_mul( _asma_stp, 1. / ASMA_NFRAMES );
	_asma_morphing = 1;
	_asma_frame = 0;
	_asma_matid = id;
}
//----------------------------------------------------
static void anim_setmat_query( long id, float sta, float end )
{
	anim_reach_end();
	_asma_sta[0] = sta;
	_asma_stp[0] = (end - sta) * (1. / ASMA_NFRAMES);
	_asma_morphing = 2;
	_asma_frame = 0;
	_asma_matid = id;
}
//----------------------------------------------------
static void anim_setmat_idle(void)
{
	if ( _asma_morphing )
	{
		if ( _asma_morphing == 1 )
		{
			vec4_add( _asma_sta, _asma_stp );
			MLB_MaterialSetDefault( _asma_matid, &_asma_sta, TAG_END );
			MLB_MaterialSetAll( _asma_matid, &_asma_sta, TAG_END );
		}
		else
		if ( _asma_morphing == 2 )
		{
			_asma_sta[0] += _asma_stp[0];
			MLB_MaterialSetDefault( _asma_matid, &_asma_sta[0], TAG_END );
			MLB_MaterialSetAll( _asma_matid, &_asma_sta[0], TAG_END );
		}

		loadMaterialPalette();
		if ( ++_asma_frame >= ASMA_NFRAMES )
		{
			_asma_morphing = 0;
			_asma_frame = 0;
		}
	}
}

//===========================
static void valncol_setmaterials( long item, long id, float l, float m, float h, float big, float lit )
{
float	sta[4], end[4];

	MLB_MaterialGetDefault( id, &sta, TAG_END );
	vec4_equ( end, sta );
	valncol_setvalues( item, end, l, m, h, big, lit );

	anim_setmat_query( id, sta, end );
	loadMaterialPalette();
}
//===========================
void lsrcM(long menuID, long which)
{
	valncol_setmaterials( which, MLB_LIGHT_PTR, .4, .6, 1., 1.,.7 );
}
//---------------------------
void lambM(long menuID, long which)
{
	valncol_setmaterials( which, MLB_AMBIENT_PTR, .1, .2, .3, .8,.1 );
}
//---------------------------
void mspecularM(long menuID, long which)
{
	valncol_setmaterials( which, MLB_SPECULAR_PTR, .1, .6, 1.4, .95, .05 );
}
//---------------------------
void mshininessM(long menuID, long which)
{
float	shininess, end;

	MLB_MaterialGetDefault( MLB_SHININESS_PTR, &shininess, TAG_END );

	switch ( which )
	{
	case 0: end = 4.;	break;
	case 1:	end = 20.;	break;
	case 2: end = 50.;	break;
	case 3: end = 80.;	break;
	}

	anim_setmat_query( MLB_SHININESS_PTR, shininess, end );
	loadMaterialPalette();

	MLB_MaterialSetDefault( MLB_SHININESS_PTR, &shininess, TAG_END );
	MLB_MaterialSetAll( MLB_SHININESS_PTR, &shininess, TAG_END );
	loadMaterialPalette();
}

//=======================================
UB _pollo=0;
static UB keybFunc( US ch, UB state )
{
	if ( state & KBD_PRESS )
	{
		ch = toupper(ch);
		switch( ch )
		{
		case 27:	CleanExit("Well done\n ciaox");

		case 'R':	controlModeM( _cmodeMID, 0 );	break;
		case 'L':	controlModeM( _cmodeMID, 1 );	break;

		case 'F':	renderingM( _rendMID, 0 );	break;
		case 'G':	renderingM( _rendMID, 1 );	break;
		case 'P':	renderingM( _rendMID, 2 );	break;
		case 'T':	renderingM( _rendMID, 3 );	break;
		case 'Z':	renderingM( _rendMID, 4 );	break;

		case 'Y':	textureM( _textureMID, 2 );	break;
		case 'U':	textureM( _textureMID, 3 );	break;
		case 'K':	textureM( _textureMID, 4 );	break;
		case 'S':	textureM( _textureMID, 6 );	break;
		case 'C':	textureM( _textureMID, 7 );	break;

		case '1':	chResM( _resoMID, 0 );	break;
		case '2':	chResM( _resoMID, 1 );	break;
		case '3':	chResM( _resoMID, 2 );	break;
		case '7':	chResM( _resoMID, 2 );	break;	/* backward compatibility */

		case 'B':	_forceRefresh ^= 1;		break;
		case 'A':	_pollo ^= 1;			break;

		case KBD_LCTRL:		_ctrl_pressed = 1;	break;
		case KBD_LSHIFT:	_shift_pressed = 1;	break;
		}
	}
	else
	if ( state & KBD_RELEASE )
		switch( ch )
		{
		case KBD_LCTRL:		_ctrl_pressed = 0;	break;
		case KBD_LSHIFT:	_shift_pressed = 0;	break;
		}

	return 1;
}
//=======================================
static BitMap *reshape(long wd, long he, long res)	// the user-prog must provide for
{													// the output bitmap..
	switch (res)
	{
	case VGA_SCRTYP_320X200:	_heCoe = 1.4;	EVT_PointerSet( _pointer11, 11, 11 );	break;
	case VGA_SCRTYP_640X480:	_heCoe = 1.37;	EVT_PointerSet( _pointer22, 22, 22 );	break;
	case VGA_SCRTYP_800X600:	_heCoe = 1.37;	EVT_PointerSet( _pointer22, 22, 22 );	break;
	}

	glViewPort( 0, 0, wd, he );
	if ( W3D_ViewPortSet( wd, he ) )
		CleanExit("ERROR SETTING NEW VIEWPORT !!!");

	glMatrixModel( GL_PROJECTION );
		glLoadIdentity();
		gluPerspective( _fov, _heCoe, 1., 100000. );
	glMatrixModel( GL_MODELVIEW );

	BitMap	*bmP;
	W3D_ValueGet( W3D_BITMAP_PTR, (long *)&bmP );

	_bufmapP = bmP;

	return bmP;
}
//=======================================
static long		_frame, _oldclock;
static float	_fps;

static void image(void)
{
float	m[4][4];

	glLoadIdentity();
	glTranslate ( 0., 0., - _objPos[2]*1.5 );
		QUA_QuatToMat( _curquat, m );
		glMultMatrix( (float *)m );
		glTranslate ( -_objCenterOffset[0],
					  -_objCenterOffset[1],
					  -_objCenterOffset[2] );
	glScale( _objScale, _objScale, _objScale );
	W3D_ObjectDraw( _obj, (const float *)glGetCurMatrix()->mf );

	W3D_FrameCalculate();

	W3D_FrameClear();
//extern long	_vpos;
//	_vpos = 20;
	//WaitTof();
	//COL_SetOneVGA64( 0, 63, 0, 0 );
	W3D_FrameRend();
	//COL_SetOneVGA64( 0, 0, 0, 0 );

	banidle();
	banprint(3, 0);

	if ( ++_frame == 20 )
	{
	long	t;
	//extern long	mi,ma;

		t = TMR_TicksGet(); //clock();
		_fps = TMR_TicksPerSecGet()*20. / (t - _oldclock);
		_frame = 0;
		_oldclock = t;
		banpost( 40, 0, "%3.2f", _fps );
	}
	//banpost( 40, 2, "%8ld %8ld", mi, ma );

	if ( _recording )
		BMP_TextWriteCP( "REC", _bufmapP->wd - 18, 0, 8 );
	else
	if ( EVT_PlaybackIsActive() && !_do_not_print_pbk_msg )
	{
	static long	lastticks;
	long		curticks;
	static char	*textp = "REPLAY";

		curticks = TMR_TicksGet();
		if ( curticks - lastticks > TMR_TicksPerSecGet() )
		{
		static long	i;

			switch ( i++ & 3 )
			{
			case 0:
			case 1:
			case 2:	textp = "REPLAY";				break;
			case 3:	textp = "Use mouse to stop";	break;
			}
			lastticks = curticks;
		}
		BMP_TextWriteCP( textp, _bufmapP->wd - strlen(textp) * 5, 0, 8 );
	}
}

//=======================================
static void animate(void)
{
	if ( _spinning )
	{
		QUA_Mul( _curquat, _lastquat, _curquat );
		EVT_PostRedraw();
	}
	if ( _lspinning )
	{
	float	m[4][4];

		QUA_Mul( _lcurquat, _llastquat, _lcurquat );
		QUA_QuatToMat( _lcurquat, m );
		W3D_LightSetPos( m[0][2], -m[1][2], m[2][2] );
		EVT_PostRedraw();
	}

	if ( _forceRefresh )
		EVT_PostRedraw();

	if ( _changingfov )
		banpost( 3, 1, "Field Of Vision = %4.2f", _fov );
	if ( _zooming )
		banpost( 3, 1, "Object Distance = %6.4f", _objPos[2]*1.5 );

	if ( EVT_PlaybackIsActive() && MOU_Buttons() )
	{
		EVT_PlaybackStop();
		EVT_PostRedraw();
	}
			
	anim_setmat_idle();
}

//===========================
static void parseArgs(int argc, char *argv[])
{
UB	stddemo=1;

	for(int i=1; i <= argc; ++i)
	{
		if NOT( strcmpi("-h", argv[i]) )
		{
			CleanExit(
"USAGE: RTMZ [-switches]\n"
"       -r <rec filename>    : record a session\n"
"       -p <rec filename>    : playback a session\n"
"       -o <object filename> : start loading an object\n"
"       -norpmsg             : do not print the replay message\n"
"       -calloldkbd          : keep old keyboard handler (for TSR screen grab)\n"
"       -h                   : display this help message\n"
 );
		}
		else
		if NOT( strcmpi("-norpmsg", argv[i]) )
		{
			_do_not_print_pbk_msg = 1;
		}
		else
		if NOT( strcmpi("-calloldkbd", argv[i]) )
		{
			KBD_HandlerKeepOld( 1 );
		}
		else
		if NOT( strcmpi("-r", argv[i]) )
		{
			if ( i+1 < argc )
			{
				strcpymaxsize( _recFileName, argv[i+1], sizeof(_recFileName) );
				EVT_RecordingStart();
				_recording = 1;
				stddemo = 0;
			}
			else
				CleanExit( "ERROR: expecting filename" );
		}
		else
		if NOT( strcmpi("-p", argv[i]) )
		{
			if ( i+1 < argc )
			{
				if NOT( EVT_PlaybackLoad( argv[i+1] ) )
				{
					EVT_PlaybackStart();
					stddemo = 0;
				}
			}
			else
				CleanExit( "ERROR: expecting filename" );
		}
		else
		if NOT( strcmpi("-o", argv[i]) )
		{
			if ( i+1 < argc )
			{
				openObj( argv[i+1] );
				stddemo = 0;
			}
			else
				CleanExit( "ERROR: expecting filename" );
		}
	}

	if ( stddemo )
		if NOT( EVT_PlaybackLoad( "DEMO.REC" ) )
			EVT_PlaybackStart();
}

//===========================
void main(int argc, char *argv[])
{
	EVT_Install();
	EVT_FuncMouseClick( clickHandleTrackball );
	EVT_FuncMouseMove( moveHandleTrackball );
	EVT_FuncKeyb( keybFunc );
	EVT_FuncReshape( reshape );
	EVT_FuncRedraw( image );
	EVT_FuncIdle( animate );

	OpenAll();
	parseArgs( argc, argv );

	EVT_MainLoop();
}
