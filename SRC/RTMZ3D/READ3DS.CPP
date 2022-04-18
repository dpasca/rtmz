//==============================================
// READ3DS.CPP
// Copyright (C) Davide Pasca 1995-97
//
// See "readme.txt" for other credits
//
// TABS=4
//==============================================
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <ctype.h>

#include "exttypes.hpp"
#include "OBJ3D.HPP"
#include "PASCALIB.HPP"
#include "MATELIB.HPP"

#define M3DMAGIC         (0x4D4D)
#define COLOR_F          (0x0010)
#define COLOR_24         (0x0011)
#define INT_PERCENTAGE   (0x0030)
#define FLOAT_PERCENTAGE (0x0031)
#define MDATA            (0x3D3D)
#define AMBIENT_LIGHT    (0x2100)
#define SOLID_BGND       (0x1200)
#define USE_SOLID_BGND   (0x1201)
#define FOG              (0x2200)
#define FOG_BGND         (0x2210)
#define USE_FOG          (0x2201)
#define DEFAULT_VIEW     (0x3000)
#define VIEW_CAMERA      (0x3080)
#define MAT_ENTRY        (0xAFFF)
#define NAMED_OBJECT     (0x4000)
#define N_TRI_OBJECT     (0x4100)
#define N_DIRECT_LIGHT   (0x4600)
#define N_CAMERA         (0x4700)
#define OBJ_HIDDEN       (0x4010)
#define OBJ_DOESNT_CAST  (0x4012)
#define POINT_ARRAY      (0x4110)
#define FACE_ARRAY       (0x4120)
#define MSH_MAT_GROUP    (0x4130)
#define SMOOTH_GROUP     (0x4150)
#define MESH_MATRIX      (0x4160)
#define DL_OFF           (0x4620)
#define DL_SPOTLIGHT     (0x4610)
#define DL_SHADOWED      (0x4630)
#define MAT_NAME         (0xA000)
#define MAT_AMBIENT      (0xA010)
#define MAT_DIFFUSE      (0xA020)
#define MAT_SPECULAR     (0xA030)
#define MAT_SHININESS    (0xA040)
#define MAT_TRANSPARENCY (0xA050)

/* A generic list type */
#define LIST_INSERT(root, node) list_insert ((List **)&root, (List *)node)
#define LIST_FIND(root, name)   list_find   ((List **)&root, name)
#define LIST_DELETE(root, node) list_delete ((List **)&root, (List *)node)
#define LIST_KILL(root)         list_kill   ((List **)&root)

typedef unsigned char  byte;
typedef unsigned short word;
typedef unsigned long  dword;


typedef struct {
	dword start;
	dword end;
	dword length;
	word  tag;
} Chunk;

#define	ACT	__O3D_actObjP
//==============================
static O3D_Object_t	*_meshRootP;
static char			_obj_name[81]="";
static FILE			*in;

//==============================
static void list_insert(List **root, List *new_node)
{
	new_node->next = *root;
	*root = new_node;
}

//==============================
static byte		read_byte(){ byte data;	data = fgetc(in); return data; }
static word		read_word(){ word data;	fread(&data, 2, 1, in); return data; }
static dword	read_dword(){ dword data; fread (&data, 4, 1, in);	return data; }
static float	read_float(){ float data;	fread (&data, 4, 1, in); return data; }

//==============================
static void start_chunk (Chunk *chunk)
{
	chunk->start  = ftell(in);
	chunk->tag    = read_word();
	chunk->length = read_dword();
	chunk->end    = chunk->start + chunk->length;
}
//------------------------------
static void end_chunk (Chunk *chunk)
{
	fseek (in, chunk->end, SEEK_SET);
}

//------------------------------
static void read_point(float *v)
{
	v[0] =  read_float();
	v[2] =  read_float();
	v[1] =  read_float();
}

//------------------------------
static char *read_string(void)
{
static char string[80];
long		i;

	for (i = 0; i < 80; ++i)
	{
		string[i] = read_byte();
		if (string[i] == '\0')	break;
	}
	if (i==80)
		string[79] = '\0';
	return string;
}

//-------------------------------
static long parse_colour(float *col)
{
Chunk		chunk;
long		err=0;

	start_chunk (&chunk);
		switch (chunk.tag)
		{
		case COLOR_F:	col[0] = read_float();
						col[1] = read_float();
						col[2] = read_float();
						break;
	
		case COLOR_24:	col[0] = read_byte()/255.;
						col[1] = read_byte()/255.;
						col[2] = read_byte()/255.;
						break;
	
		default:		err=-1;		break;
		}
	end_chunk (&chunk);

	return err;
}

//-----------------------------------------
static long parse_point_array(void)
{
	if ( O3D_AllocVerts( read_word()) )				return -1;

	ACT->curVertNum = ACT->NVerts;
	for (long i = 0; i < ACT->curVertNum; ++i)
		read_point( ACT->baseVertsP[ i ].vert );

	return 0;
}

//-----------------------------------------
static long parse_msh_mat_group( void )
{
char	name[80];
long	matID;
float	diffuse[]={.0,.0,1.,.95};

	strcpy( name, read_string() );
	if ( (matID = MLB_MaterialAdd( MLB_NAME_PTR,name, MLB_DIFFUSE_PTR,diffuse, TAG_END )) < 0 )
		return -1;

	for (short i=read_word(); i; --i)
	{
	short	face = read_word();
		CLAMP( face, 0, ACT->curPolyNum );
		ACT->PolysP[ face ].materialID = matID;
    }

	return 0;
}

//-----------------------------------------
static long parse_face_array(Chunk *mainchunk)
{
long		err;

	if ( O3D_AllocPolys(read_word()) ) 	return -1;

	for (long i=ACT->NPolys; i; --i)
	{
		O3D_PolyBegin( 0, 0 );
			_O3D_PolyVertNew( read_word() );
			_O3D_PolyVertNew( read_word() );
			_O3D_PolyVertNew( read_word() );
		O3D_PolyEnd();
		read_word();
	}

	Chunk	chunk;
	do{
		start_chunk (&chunk);
		if (chunk.end <= mainchunk->end)
		{
			switch (chunk.tag)
			{
			case MSH_MAT_GROUP: if ( err = parse_msh_mat_group() )	return err;	break;
			//case SMOOTH_GROUP:  parse_smooth_group();	break;
			}
		}
		end_chunk (&chunk);
	}while (chunk.end < mainchunk->end);

	return 0;
}

//-----------------------------------------
static void parse_mesh_matrix(void)
{
	/*for (short i = 0; i < 4; i++)
		for (short j = 0; j < 3; j++)
			ACT->mat.mf[j][i] = read_float();*/
}

//-----------------------------------------
static long parse_n_tri_object(Chunk *mainchunk)
{
Chunk			chunk;
long			err=0;
O3D_Object_t	*objP;

	if NOT( objP = (O3D_Object_t *)O3D_New(_obj_name) )
		return -1;

long	old=O3D_Set((long)objP);
	do{
		start_chunk(&chunk);
		if (chunk.end <= mainchunk->end)
		{
			switch (chunk.tag)
			{
			case POINT_ARRAY:	if ( err = parse_point_array() ) goto exitErr;		break;
			case FACE_ARRAY:	if ( err = parse_face_array(&chunk)) goto exitErr;	break;
			case MESH_MATRIX:	parse_mesh_matrix();  								break;
			}
		}
		end_chunk(&chunk);
	}while (chunk.end < mainchunk->end);

	LIST_INSERT( _meshRootP, objP );
exitErr:

	O3D_Set( old );
	return err;
}

//-----------------------------------------
static long parse_named_object(Chunk *mainchunk)
{
Chunk	chunk;
long	err;

	strcpymaxsize(_obj_name, read_string(), 80);
	do{
		start_chunk (&chunk);
		if (chunk.end <= mainchunk->end)
		{
			switch (chunk.tag)
			{
			case N_TRI_OBJECT:		if ( err = parse_n_tri_object(&chunk) )	return err;	break;
			//case OBJ_HIDDEN:		if (_meshP)	_meshP->hidden = 1;	break;
			//case OBJ_DOESNT_CAST:	if (_meshP)	_meshP->shadow = 0;	break;
			//case N_DIRECT_LIGHT:	parse_n_direct_light(&chunk);	break;
			//case N_CAMERA:			parse_n_camera();				break;
			}
		}
		end_chunk (&chunk);
	}while (chunk.end < mainchunk->end);

	return 0;
}

//-------------------------------
static short parse_int_percentage(void){US	percent = read_word();	return percent; }
static float parse_float_percentage(void){float	percent = read_float();	return percent; }
//-------------------------------
static float parse_percentage(void)
{
Chunk	chunk;
float	percent = 0.0;

    start_chunk (&chunk);
    switch (chunk.tag) {
	case 0x0030: percent = parse_int_percentage()/100.0;	     break;
	case 0x0031: percent = parse_float_percentage();		     break;
    }
    end_chunk (&chunk);
    return percent;
}

//-------------------------------
static long parse_mat_entry (Chunk *mainchunk)
{
Chunk			chunk;
char			name[80];
static float	ambient[]={.2,.2,.2,.05}, diffuse[]={.9,.1,.1,.95}, specular[]={.2,.2,.2,.2};
float			shininess=10., reflection=.2;

    do{
		start_chunk (&chunk);
	
		if (chunk.end <= mainchunk->end)
		{
			switch (chunk.tag)
			{
			case 0xA000: strcpy(name, read_string());			break;
			case 0xA010: parse_colour( ambient );				break;
			case 0xA020: parse_colour( diffuse );				break;
			case 0xA030: parse_colour( specular );				break;
			case 0xA040: shininess = 100.0*parse_percentage();	break;
	/*		case 0xA050: mprop->transparency = parse_percentage();		break;
			case 0xA080: mprop->self_illum = TRUE;						break; */
			case 0xA220: reflection = parse_percentage();	//(void)parse_mapname (&chunk);
						 break;
			case 0xA310: if (reflection == 0.0)	reflection = 1.0;	break;
	/*		case 0xA200: mprop->tex_strength = parse_percentage();
						 strcpy (mprop->tex_map, parse_mapname (&chunk));	 break;
	
			case 0xA230: mprop->bump_strength = parse_percentage();
						 strcpy (mprop->bump_map, parse_mapname (&chunk));	 break;
	*/	    }
		}
		end_chunk (&chunk);
    } while (chunk.end < mainchunk->end);

	specular[3] = reflection;
	MLB_MaterialSetOrAdd(	MLB_NAME_PTR,name,	MLB_AMBIENT_PTR,ambient, MLB_DIFFUSE_PTR,diffuse,
							MLB_SPECULAR_PTR,specular,	MLB_SHININESS_PTR,&shininess, TAG_END );

	return 0;
}

//-------------------------------
static long parse_mdata(Chunk *mainchunk)
{
Chunk	chunk;
long	err;
float	global_amb[]={1.,1.,1.,.1};

	do{
		start_chunk (&chunk);
		if (chunk.end <= mainchunk->end)
		{
			switch (chunk.tag)
			{
			case 0x2100: parse_colour( global_amb );
						 MLB_MaterialSetDefault( MLB_AMBIENT_PTR, global_amb, TAG_END );
						 MLB_MaterialSetAll( MLB_AMBIENT_PTR, global_amb, TAG_END );
						 break;
			//case SOLID_BGND:     if ( err = parse_color(&bgnd_color) )
			//						return err;
			//					 break;

			//case USE_SOLID_BGND: write_bgsolid (out, bgnd_color);		 break;
			//case FOG:            parse_fog (&chunk);             		 break;
			//case FOG_BGND:       parse_fog_bgnd();               		 break;
			//case USE_FOG:        write_fog (out, fog_color, fog_distance); break;
			case MAT_ENTRY:		if ( err = parse_mat_entry(&chunk) )	return err; break;
			case NAMED_OBJECT:	if ( err = parse_named_object(&chunk) )	return err; break;
			}
		}
		end_chunk (&chunk);
	}while (chunk.end < mainchunk->end);

	return 0;
}

//===============================
static long parse_3ds(Chunk *mainchunk)
{
Chunk	chunk;
long	err;

	do{
		start_chunk (&chunk);
		if (chunk.end <= mainchunk->end)
		{
			switch (chunk.tag)
			{
			case MDATA:	if ( err = parse_mdata(&chunk) ) return err;	break;
			}
		}
		end_chunk (&chunk);
	}while (chunk.end < mainchunk->end);

	return 0;
}

//-------------------------------
static long parse_file(void)
{
Chunk	chunk;
long	err;

	start_chunk(&chunk);
	if (chunk.tag == M3DMAGIC)
		err = parse_3ds(&chunk);
	else
		err = -1;
	end_chunk(&chunk);

	return err;
}

//-------------------------------
long O3D_Load3DS(const char *fnameP)
{
long	err=0;

	if NOT( in = fopen( fnameP, "rb" ) )
		return -1;

	_meshRootP = 0;
	if NOT( err = parse_file() )
	{
		*ACT = *_meshRootP;
		POE_PolyI_t *polyP = ACT->PolysP;
		for(long i=ACT->curPolyNum; i; --i, ++polyP)
		{
			polyP->vertH = &ACT->vertsP;
			polyP->userLong = (long)ACT;
		}

		free( _meshRootP );
		O3D_Update();
		if ( __MLB_actLibP->nMaterials == 0 )
			MLB_MaterialAdd( MLB_NAME_PTR, "NULL", TAG_END );
	}
	fclose( in );

	if ( err )
	{
		*ACT = *_meshRootP;
		O3D_Free( (long)ACT );
	}

	return err;
}

