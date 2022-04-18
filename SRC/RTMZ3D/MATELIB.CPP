//==============================================
// Matelib.cpp - 
// Copyright (C) Davide Pasca 1995
//==============================================

#include <stdlib.h>
#include <string.h>
#include "MATELIB.HPP"
#include "PASCALIB.HPP"
#include "CVEC3.HPP"

static MLB_Library_t	_library;
MLB_Library_t			*__MLB_actLibP=&_library;

#define	ACT	__MLB_actLibP

static inline vec4_set(float *d, float x,float y,float z,float w){d[0]=x; d[1]=y; d[2]=z; d[3]=w;}
static inline vec4_equ(float *d, float *s){d[0] = s[0]; d[1] = s[1]; d[2] = s[2]; d[3] = s[3];}
static inline vec4_equ_dck(float *d, float *s){if (d){ d[0] = s[0]; d[1] = s[1]; d[2] = s[2]; d[3] = s[3]; }}
static inline vec4_equ_sck(float *d, float *s){if (s){ d[0] = s[0]; d[1] = s[1]; d[2] = s[2]; d[3] = s[3]; }}

//==============================================
UB *MLB_PaletteGet(long *stacolp, long *ncolsp)
{
	if ( stacolp )	*stacolp = ACT->shadePal.userColors;
	if ( ncolsp )	*ncolsp = 256 - ACT->shadePal.userColors;

	return (UB *)ACT->shadePal.palette + ACT->shadePal.userColors * 3;
}

//==============================================
void MLB_MaterialDisposeAll( void )
{
US				nMats;
MLB_Material_t	*matP;

	ACT->nTextures = 0;
	for( matP = ACT->materials, nMats = ACT->nMaterials; nMats; ++matP )
	{
		if ( matP->active )
		{
			SAFE_FREE( matP->textureP );
			matP->active = 0;
			--nMats;
		}
	}
}

//==============================================
void MLB_LibraryReset(void)
{
	MLB_MaterialDisposeAll();
	memset(ACT,0,sizeof(MLB_Library_t));
	vec4_set( ACT->deflight, 1., 1., 1., 1. );
	vec4_set( ACT->defambient, .8, .8, .8, .1 );
	vec4_set( ACT->defdiffuse, .75, .75, .75, .9 );
	vec4_set( ACT->defspecular, .5, .5, .5, .4 );
	ACT->defshininess = 5.;
}

//==============================================
long MLB_MaterialFind( const char *nameP )
{
US				nMats;
MLB_Material_t	*matP;

	for( matP = ACT->materials, nMats = ACT->nMaterials; nMats; ++matP )
	{
		if ( matP->active )
		{
			--nMats;
			if NOT( strncmp( matP->name, nameP, sizeof(matP->name) ) )
				return matP - ACT->materials;
		}
	}

	return -1;
}

//==============================================
static void parseMatInfo(va_list arg, long id, char **name,
						 float **light, float **ambient, float **diffuse, float **specular, float **shininess,
						 BitMap **txMapP, UB **palP )
{
	*light = *ambient = *diffuse = *specular = 0;
	*shininess = 0;
	*txMapP = 0;
	*palP = 0;
	*name = 0;
	while ( id != TAG_END )
	{
		switch( id )
		{
		case MLB_NAME_PTR:		*name = va_arg( arg, char * );			break;
		case MLB_LIGHT_PTR:		*light = va_arg( arg, float * );		break;
		case MLB_AMBIENT_PTR:	*ambient = va_arg( arg, float * );		break;
		case MLB_DIFFUSE_PTR:	*diffuse = va_arg( arg, float * );		break;
		case MLB_SPECULAR_PTR:	*specular = va_arg( arg, float * );		break;
		case MLB_SHININESS_PTR:	*shininess = va_arg( arg, float * );	break;
		case MLB_TXMAP_N_PALETTE_PTR:	*txMapP = va_arg( arg, BitMap * );
										*palP = va_arg( arg, UB * );		break;
		}
		id = va_arg( arg, long );
	}
}
//==============================================
static long checkAndCreateTexture( MLB_Texture_t **texturePP, BitMap *txMapP, UB *palP)
{
	if ( txMapP || palP )
	{
	MLB_Texture_t	*tp;

		if ( *texturePP )	tp = *texturePP;
		else
		{
			if NOT( tp = (MLB_Texture_t *)malloc( sizeof(MLB_Texture_t) ) )
				return -1;
			ACT->nTextures = 1;
		}

		*texturePP = tp;
		if ( txMapP )	tp->origMap = *txMapP;
		if ( palP )		memcpy( tp->origPal, palP, 768 );
	}

	return 0;
}

//==============================================
long MLB_MaterialSetAll( long tag, ... )
{
char			*name;
float			*light, *ambient, *diffuse, *specular, *shininess;
BitMap			*txMapP;
UB				*palP;
MLB_Material_t	*matP;
long			i, err;

va_list	arg;
	va_start( arg, tag );
	parseMatInfo( arg, tag, &name, &light, &ambient, &diffuse, &specular, &shininess, &txMapP, &palP );
	va_end( arg );

	if ( txMapP )
		ACT->nTextures = 1;
	else
		ACT->nTextures = 0;

	matP = ACT->materials;
	for(i=ACT->nMaterials; i > 0; ++matP)
	{
		if ( matP->active )
		{
			if ( err = checkAndCreateTexture( &matP->textureP, txMapP, palP ) )
				return err;
			vec4_equ_sck( matP->light, light );
			vec4_equ_sck( matP->ambient, ambient );
			vec4_equ_sck( matP->diffuse, diffuse );
			vec4_equ_sck( matP->specular, specular );
			if ( shininess )	matP->shininess = *shininess;
			--i;
		}
	}
	return 0;
}
//==============================================
void MLB_MaterialGetDefault( long tag, ... )
{
char	*name;
float	*light, *ambient, *diffuse, *specular, *shininess;
BitMap	*txMapP;
UB		*palP;

va_list	arg;
	va_start( arg, tag );
	parseMatInfo( arg, tag, &name, &light, &ambient, &diffuse, &specular, &shininess, &txMapP, &palP );
	va_end( arg );

	vec4_equ_dck( light, ACT->deflight );
	vec4_equ_dck( ambient, ACT->defambient );
	vec4_equ_dck( diffuse, ACT->defdiffuse );
	vec4_equ_dck( specular, ACT->defspecular );
	if ( shininess )	*shininess = ACT->defshininess;
	if ( txMapP )	*txMapP = ACT->deftexture.origMap;
	if ( palP )		memcpy( palP, ACT->deftexture.origPal, 768 );
}
//==============================================
void MLB_MaterialSetDefault( long tag, ... )
{
char	*name;
float	*light, *ambient, *diffuse, *specular, *shininess;
BitMap	*txMapP;
UB		*palP;

va_list	arg;
	va_start( arg, tag );
	parseMatInfo( arg, tag, &name, &light, &ambient, &diffuse, &specular, &shininess, &txMapP, &palP );
	va_end( arg );

	vec4_equ_sck( ACT->deflight, light );
	vec4_equ_sck( ACT->defambient, ambient );
	vec4_equ_sck( ACT->defdiffuse, diffuse );
	vec4_equ_sck( ACT->defspecular, specular );
	if ( shininess )	ACT->defshininess = *shininess;
	if ( txMapP )	ACT->deftexture.origMap = *txMapP;
	if ( palP )		memcpy( ACT->deftexture.origPal, palP, 768 );
}

//==============================================
static long materialAddOverwrite( UB doOverWrite, long tag, va_list	arg )
{
char			*name;
float			*light, *ambient, *diffuse, *specular, *shininess;
BitMap			*txMapP;
UB				*palP;
long			err;
long			matID;
MLB_Material_t	*matP;

	parseMatInfo( arg, tag, &name, &light, &ambient, &diffuse, &specular, &shininess, &txMapP, &palP );

	if ( (matID = MLB_MaterialFind( name )) >= 0 )
	{
		if ( doOverWrite )
		{
		MLB_Material_t	*matP;

			matP = &ACT->materials[matID];
			if ( err = checkAndCreateTexture( &matP->textureP, txMapP, palP ) )
				return err;
			vec4_equ_sck( matP->light, light );
			vec4_equ_sck( matP->ambient, ambient );
			vec4_equ_sck( matP->diffuse, diffuse );
			vec4_equ_sck( matP->specular, specular );
			if ( shininess )	matP->shininess = *shininess;
		}	
		return matID;
	}

	if ( ACT->nMaterials >= MLB_MAXMATERIALS )
		return -1;

	for( matP = ACT->materials; matP->active; ++matP );

	strcpymaxsize( matP->name, name, sizeof(matP->name) );
	matP->active = 1;

	if ( txMapP )
		ACT->nTextures = 1;
	else
		ACT->nTextures = 0;
	if ( err = checkAndCreateTexture( &matP->textureP, txMapP, palP ) )
		return err;

	if ( light )	vec4_equ( matP->light, light );			else	vec4_equ( matP->light, ACT->deflight );
	if ( ambient )	vec4_equ( matP->ambient, ambient );		else	vec4_equ( matP->ambient, ACT->defambient );
	if ( diffuse )	vec4_equ( matP->diffuse, diffuse );		else	vec4_equ( matP->diffuse, ACT->defdiffuse );
	if ( specular )	vec4_equ( matP->specular, specular );	else	vec4_equ( matP->specular, ACT->defspecular );
	if ( shininess )	matP->shininess = *shininess;		else	matP->shininess = ACT->defshininess;
	++ACT->nMaterials;

	return matP - ACT->materials;
}
//----------------------------------------------
long MLB_MaterialAdd( long tag, ... )
{
long	err;
va_list	arg;

	va_start( arg, tag );
	err = materialAddOverwrite( 0, tag, arg );
	va_end( arg );

	return err;
}
//----------------------------------------------
long MLB_MaterialSetOrAdd( long tag, ... )
{
long	err;
va_list	arg;

	va_start( arg, tag );
	err = materialAddOverwrite( 1, tag, arg );
	va_end( arg );

	return err;
}

//==============================================
void MLB_PaletteCompute( US userColors )
{
MLB_Material_t		*matP=ACT->materials;
COL_ShadePalette_t	*spalP=&ACT->shadePal;
UB					shadePalCol;
UB					*paletteP;
long 				i;

	shadePalCol = 0;
	spalP->userColors = userColors;

	SAFE_FREE( ACT->levPalP );
	if ( ACT->nTextures )
	{
	UB	*srcPalP;

		spalP->nShades = 1;
		spalP->nColors = 256-userColors;
		paletteP = (UB *)spalP->palette + userColors*3;
		for(i=ACT->nMaterials; i > 0; --i, ++matP)
			if ( matP->active && matP->textureP )
			{
				memset( matP->textureP->origPal, 0, userColors*3 );
				srcPalP = matP->textureP->origPal + userColors*3;
				for(i = spalP->nColors; i > 0; --i)
				{
					*paletteP++ = *srcPalP++;
					*paletteP++ = *srcPalP++;
					*paletteP++ = *srcPalP++;
				}
				break;
			}
		ACT->levPalP = COL_NewDark( (UB *)spalP->palette, 32, .4, 256 );
	}
	else
	{
	float	stepli;

		spalP->nColors = ACT->nMaterials;
		if ( spalP->nColors )
			if ( spalP->nShades = (256-userColors) / spalP->nColors )
				stepli = (1.-.3) / spalP->nShades;
	
		paletteP = (UB *)spalP->palette + userColors*3;
		for(i=ACT->nMaterials; i > 0; --i, ++matP)
		{
			if ( matP->active )
			{
			float	ia_oa[3],id_od[3], is_os[3], il_ol[3], li, shin;
	
				vec3_equ(il_ol,matP->light);	vec3_mul(il_ol,il_ol,matP->light[3]);
				vec3_equ(ia_oa,matP->ambient);	vec3_mul(ia_oa,ia_oa,matP->ambient[3]);
				vec3_equ(id_od,matP->diffuse);	vec3_mul(id_od,id_od,matP->diffuse[3]);
				vec3_equ(is_os,matP->specular);	vec3_mul(is_os,is_os,matP->specular[3]);
				shin = matP->shininess;
				li = .3;
				for(US j=spalP->nShades; j; --j, li += stepli)
				{
				long	tv[3];
				float	ts;
	
					ts = pow( li, shin );
					tv[0] = (ia_oa[0] + (ts*is_os[0] + il_ol[0]*li*id_od[0])) * 255.;
					tv[1] = (ia_oa[1] + (ts*is_os[1] + il_ol[1]*li*id_od[1])) * 255.;
					tv[2] = (ia_oa[2] + (ts*is_os[2] + il_ol[2]*li*id_od[2])) * 255.;
					CLAMP(tv[0],0,255); CLAMP(tv[1],0,255); CLAMP(tv[2],0,255);
					paletteP[0] = tv[0];
					paletteP[1] = tv[1];
					paletteP[2] = tv[2];
					paletteP += 3;
				}
				++shadePalCol;
			}
		}
	}
}

