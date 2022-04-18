//==============================================
// MNU.CPP - menu stuff
// Copyright (C) by Davide Pasca 1995
//==============================================

#include <stdlib.h>
#include <string.h>
#include "PASCALIB.HPP"
#include "BMP.HPP"
#include "MOU.HPP"
#include "MNU.HPP"
#include "TMR.HPP"
#include "EVT.HPP"

#define NOT_VISIBLE			0
#define VISIBLE				1
#define JUST_NOT_VISIBLE	2

struct MNU_Menu_t;
typedef struct MNU_Item_t
{
	char				name[MNU_ITM_MAXLEN];
	UL					id;
	UL					flags;
	US					y1,y2;
	struct MNU_Menu_t	*subMenuP;
	MNU_CallBack_t		callBack;
} MNU_Item_t;

typedef struct MNU_Menu_t
{
	struct MNU_Menu_t       *prevVisP;
	struct MNU_Menu_t       *nextVisP;
	struct MNU_Menu_t       *curMenuP;

	long            mouseX, mouseY;
	long            fontID;
	UB				fontAuto;
	BitMap          map;

	UB				txCol, bkCol, ltCol, dkCol;
	float			scaleWd, scaleHe;

	UB				visible, doRebuild;
	long            z;
	long            staTicks;
	long            px,py;
	long            curItemIdx;
	US				nItems;
	MNU_Item_t      *itemsP;
	long            userLong;

	UB				*specialPointerP;
	long			spointWd, spointHe;
} MNU_Menu_t;

static MNU_Menu_t       *_curMenuP;
static long                     _menuAutoFontID;
#define ACT     _curMenuP


//==============================================
static void inbound(MNU_Menu_t *curP, long *x, long *y)
{
long    wd, he;

	wd = (long)curP->map.wd << 4;
	he = (long)curP->map.he << 4;
	if ( *x < curP->px )                    curP->px = *x;
	else
	if ( *x >= curP->px + wd )      curP->px = *x-wd+1;

	if ( *y < curP->py )                    curP->py = *y;
	else
	if ( *y >= curP->py + he )      curP->py = *y-he+1;
}

//==============================================
static void inbound_mouse(MNU_Menu_t *curP)
{
long    wd, he;

	wd = curP->map.wd << 4;
	he = curP->map.he << 4;
	if ( curP->mouseX < curP->px )            curP->px = curP->mouseX;
	else
	if ( curP->mouseX >= curP->px + wd )      curP->px = curP->mouseX - wd+1;

	if ( curP->mouseY < curP->py )
		curP->py = curP->mouseY;
	else
	if ( curP->mouseY >= curP->py + he )
		curP->py = curP->mouseY-he+1;
}

//==============================================
long MNU_Set(long id){long t=(long)ACT; ACT = (MNU_Menu_t *)id; return t; }
long MNU_Get(void){ return (long)ACT; }

//==============================================
long MNU_UserLongGet( long menuID )
{
MNU_Menu_t      *menuP=(MNU_Menu_t *)menuID;

	if ( menuP )
		return menuP->userLong;

	return 0;
}
//----------------------------------------------
void MNU_UserLongSet( long menuID, long val )
{
MNU_Menu_t      *menuP=(MNU_Menu_t *)menuID;

	if ( menuP )
		menuP->userLong = val;
}

//==============================================
void MNU_ItemNameGet( long menuID, long itm, char *nameP )
{
	if ( menuID )
		strcpy( nameP, ((MNU_Menu_t *)menuID)->itemsP[ itm ].name );
}

//==============================================
void MNU_Dispose( long menuID, BO disposeSubs )
{
MNU_Menu_t      *menuP;

	if NOT( menuP = (MNU_Menu_t *)menuID )
		return;

	if ( menuP == ACT )
		ACT = 0;

	if ( disposeSubs )
	{
	long		i;
	MNU_Menu_t	*subP;

		for(i=0; i < menuP->nItems; ++i)
			if ( subP = menuP->itemsP[i].subMenuP )
			{
				MNU_Dispose( (long)subP, 1 );
				menuP->itemsP[i].subMenuP = 0;
			}
	}

	FREEIF( menuP->specialPointerP );
	FREEIF( menuP->itemsP );
	FREEIF( menuP );
}

//==============================================
long MNU_New(long fontID, UB txCol, UB bkCol, UB ltCol, UB dkCol )
{
MNU_Menu_t      *menuP;

	if NOT( menuP = (MNU_Menu_t *)calloc( sizeof(MNU_Menu_t),1 ) )
		return 0;

	memset(menuP, 0, sizeof(MNU_Menu_t));

	if NOT( fontID )
	{
		if NOT( fontID = BMP_TFGet() )
		{
			if NOT( _menuAutoFontID )
				_menuAutoFontID = BMP_TFLoad( "font4x7.pcx" );
			fontID = _menuAutoFontID;
			menuP->fontAuto = 1;
		}
	}
	menuP->fontID = fontID;
	menuP->txCol = txCol;
	menuP->bkCol = bkCol;
	menuP->ltCol = ltCol;
	menuP->dkCol = dkCol;
	menuP->curMenuP = menuP;
	menuP->scaleWd = 1.;
	menuP->scaleHe = 1.;

	return (long)menuP;
}

//==============================================
void MNU_ColorsSet( long menuID, BO subs, UB txCol, UB bkCol, UB ltCol, UB dkCol )
{
MNU_Menu_t      *menuP=(MNU_Menu_t *)menuID;

	if NOT( menuP )
		return;

	if ( subs )
	{
	long		i;
	MNU_Menu_t	*subP;

		for(i=0; i < menuP->nItems; ++i)
			if ( subP = menuP->itemsP[i].subMenuP )
				MNU_ColorsSet( (long)subP, 1, txCol, bkCol, ltCol, dkCol );
	}

	menuP->txCol = txCol;
	menuP->bkCol = bkCol;
	menuP->ltCol = ltCol;
	menuP->dkCol = dkCol;
}

//==============================================
void MNU_FontSet( long menuID, BO subs, long fontID )
{
MNU_Menu_t      *menuP=(MNU_Menu_t *)menuID;

	if NOT( menuP )
		return;

	if ( subs )
	{
	long		i;
	MNU_Menu_t	*subP;

		for(i=0; i < menuP->nItems; ++i)
			if ( subP = menuP->itemsP[i].subMenuP )
				MNU_FontSet( (long)subP, 1, fontID );
	}

	menuP->fontID = fontID;
}

//==============================================
static UB itmIsDivis(const MNU_Item_t *i)
{
	if ( i->name[0] == '-' && i->name[1] == 0 )     return 1;
	return 0;
}
//==============================================
#define CORNICE 1
static void itemDraw(MNU_Menu_t *m, MNU_Item_t *itmP, short cwd, short che, UB backg, UB hilite)
{
char    *tP = itmP->name;

	UB      tx = m->txCol, bk = m->bkCol, lt = m->ltCol, dk = m->dkCol;
	UB      isDivis = itmIsDivis( itmP );
	short   y1 = itmP->y1, y2 = itmP->y2;
	short   wd = m->map.wd;
	short   x1 = wd - cwd-3;
	short   itmHe = y2 - y1 + 1;
	UL		active = itmP->flags & MNU_FLG_IACTIVE;

	if ( hilite )
	{
		if NOT( isDivis )
		{
		UB	col;

			if NOT( itmP->flags & MNU_FLG_IACTIVE )	col = dk;
			else									col = tx;

			if ( backg )
				BMP_RectFill( CORNICE+1, y1, wd-CORNICE-2, y2, dk );

			if ( itmP->subMenuP )
				BMP_CharWriteCP( BMP_TF_SUBCH, x1+1, y1+2, col );

			if ( itmP->flags & MNU_FLG_ICHECKED )
				BMP_CharWriteCP( BMP_TF_CHKCH, x1+1, y1+2, col );

			BMP_TextWriteCP( tP, CORNICE*2+2, y1+2, col );
		}
	}
	else
		if ( isDivis )
		{
		US      ny=y1+itmHe/4;
			BMP_LineH( 1, wd-3, ny, lt );
			BMP_LineH( 2, wd-2, ny-1, dk );
			BMP_PixelPut( 1, ny-1, bk );
		}
		else
		{
			if ( backg )
				BMP_RectFill( CORNICE+1, y1, wd-CORNICE-2, y2, bk );
			if ( itmP->subMenuP )
			{
				BMP_CharWriteCP( BMP_TF_SUBCH, x1+1, y1+2, dk );
				if ( active ) BMP_CharWriteCP( BMP_TF_SUBCH, x1,   y1+1, tx );
			}

			if ( itmP->flags & MNU_FLG_ICHECKED )
			{
				BMP_CharWriteCP( BMP_TF_CHKCH, x1+1, y1+2, dk );
				if ( active ) BMP_CharWriteCP( BMP_TF_CHKCH, x1,   y1+1, tx );
			}

			BMP_TextWriteCP( tP, CORNICE*2+2, y1+2, dk );
			if ( active ) BMP_TextWriteCP( tP, CORNICE*2+1, y1+1, tx );
		}
}

//==============================================
static void _MNU_ItemRedraw( MNU_Menu_t *m, US i, UB hilite )
{

	BMP_PushSet( &m->map );
		{long oldFont = BMP_TFSet( m->fontID );
		itemDraw( m, m->itemsP+i, BMP_TextWidth("A"), BMP_TFHeight(), 1, hilite );
		BMP_TFSet( oldFont );
		}
	BMP_Pop();
}

//==============================================
static long _MNU_PrepareBitMap(MNU_Menu_t *m)
{
US                      nItems;
MNU_Item_t      *itemsP;
US                      wd,he,itmHe, cwd, che;
long            oldFont, i;
UB                      lt, bk, dk;
BitMap		tmap;

	nItems = m->nItems;
	itemsP = m->itemsP;
	if ( !nItems || !itemsP )       return -1;

	oldFont = BMP_TFSet( m->fontID ); // set font
	cwd = BMP_TextWidth("A"); che = BMP_TFHeight();

	itmHe = che + 4;
	he = CORNICE*4;
	wd = 0;
	for(i=nItems; i > 0; --i, ++itemsP)
	{
	US      t=strlen(itemsP->name);
		wd = MAX(wd,t);
		if ( itmIsDivis(itemsP) )       he += itmHe / 2;
		else                                            he += itmHe;
	}
	wd = (wd+2) * cwd + CORNICE*2 + 5;

	
	if ( BMP_Alloc(&m->map,wd,he,8,0) )
		return -1;

	lt=m->ltCol; bk=m->bkCol; dk=m->dkCol;
	BMP_PushSet( &m->map );
		{US     x2=wd-1, y2=he-1;
		US      yy;
		BMP_RectFill( 0, 0, x2, y2, bk );
		//BMP_LineV( x2-1, 1, y2, dk );
		BMP_LineV( x2, 1, y2, dk );
		//BMP_LineH( 1, x2, y2-1, dk );
		BMP_LineH( 1, x2, y2, dk );
		//BMP_PixelPut( 0 , 0, lt );

		//BMP_LineV( 1, 1, y2-1, lt );
		//BMP_LineH( 1, x2-1, 1, lt );
		BMP_LineV( 0, 0, y2-1, lt );
		BMP_LineH( 0, x2-1, 0, lt );
	
		yy = CORNICE*2;
		for(i=nItems, itemsP = m->itemsP; i > 0; --i, ++itemsP)
		{
		char    *tP = itemsP->name;
			itemsP->y1 = yy+1;
			if (tP[0]=='-') yy += itmHe/2;
			else                    yy += itmHe;
			itemsP->y2 = yy-1;
			itemDraw( m, itemsP, cwd, che, 0, 0 );
		}
		}
	BMP_Pop();


	BMP_TFSet( oldFont );
	return 0;
}

//==============================================
/*      if ( x < ACT->x+CONRICE || x >= ACT->x+ACT->map.wd-CORNICE )
	{
	}
	long oldFont = BMP_TFSet( ACT->fontID ); // set font
	US      itmHe = BMP_TFHeight()+4;
	*/
static void _MNU_FindItem(MNU_Menu_t *m, long x, long y)
{
US                      nItems, i;
MNU_Item_t      *itemsP;

	if NOT( m->map.memP )   return;

	y -= m->py;
	x >>= 4;
	y >>= 4;

	nItems = m->nItems;
	itemsP = m->itemsP;
	for(i=0; i < nItems; ++i, ++itemsP)
		if ( y >= itemsP->y1 && y <= itemsP->y2 )
		{
			if ( i != m->curItemIdx )
			{
				if ( m->curItemIdx >= 0 )
					_MNU_ItemRedraw( m, m->curItemIdx, 0 );

				if ( itemsP->name[0] == '-' && itemsP->name[1] == 0 )
					m->curItemIdx = -1;
				else
				{
					_MNU_ItemRedraw( m, i, 1 );
					m->curItemIdx = i;
				}
			}
			return;
		}

	if ( m->curItemIdx >= 0 )
	{
		_MNU_ItemRedraw( m, m->curItemIdx, 0 );
		m->curItemIdx = -1;
	}
}

//==============================================
static long _MNU_MenuCreate( MNU_Menu_t *newP, MNU_Menu_t *prevP, long x, long y )
{
BitMap  *bmp=BMP_Get();

	if NOT( _MNU_PrepareBitMap(newP) )
	{
		prevP->nextVisP = newP;
		newP->prevVisP = prevP;
		ACT->curMenuP = newP;
		newP->visible = VISIBLE;

		if ( newP->map.wd < bmp->wd )
		{
			if ( x + newP->map.wd*16L > bmp->wd*16L )
				x = bmp->wd*16L - newP->map.wd*16L;
		}
		newP->px = x;
		newP->py += y;
		newP->curItemIdx = -1;
		_MNU_FindItem(newP,x,y);
		return 0;
	}
	return -1;
}

//==============================================
static void _MNU_MenuClose( MNU_Menu_t *menuP, long x, long y, UB posTrack )
{
	menuP->prevVisP->nextVisP = 0;
	menuP->px -= x;  menuP->py -= y;
	BMP_Free( &menuP->map );
	menuP->visible = JUST_NOT_VISIBLE;
	if ( menuP->prevVisP )
	{
		ACT->curMenuP = menuP->prevVisP;
		if ( posTrack )
		{
			inbound( ACT->curMenuP, &x, &y );
			_MNU_FindItem(ACT->curMenuP,x,y);
		}
	}
	else
		ACT->curMenuP = ACT;
}

//==============================================
static long MNU_Rebuild( MNU_Menu_t     *menuP )
{
	if ( menuP->doRebuild && menuP->visible == VISIBLE )
	{
	long            x,y, px;
	MNU_Menu_t      *prevP, *curP;

		curP = ACT->curMenuP;

		x = ACT->mouseX;
		y = ACT->mouseY;

		prevP = menuP->prevVisP;
		px = menuP->px;
		_MNU_MenuClose( menuP, x, y, 0 );

		if ( _MNU_MenuCreate( menuP, prevP, x, y ) )
			return -1;

		menuP->px = px;

		{
		long	hitmsize=0;

			if ( menuP->nItems && menuP->itemsP )
				hitmsize = menuP->itemsP[0].y2 - menuP->itemsP[0].y1 << 3;

			if ( menuP->py < y )
				menuP->py = y - hitmsize;

			if ( y > menuP->py+(menuP->map.he << 4) )
				menuP->py = menuP->py + (menuP->map.he << 4) - 1 + hitmsize;

			_MNU_FindItem( menuP, x, y );
		}

		menuP->doRebuild = 0;

		ACT->curMenuP = curP;
	}

	return 0;
}

//==============================================
void MNU_ItemsReset( long menuID )
{
MNU_Menu_t      *menuP=(MNU_Menu_t *)menuID;

	if ( menuP->nItems )
	{
		SAFE_FREE( menuP->itemsP );
		menuP->nItems = 0;
		menuP->curItemIdx = -1;
		if ( menuP->visible == VISIBLE )
			menuP->doRebuild = 1;
	}
}

//==============================================
void MNU_ItemCheckAll( long menuID, UB check )
{
MNU_Menu_t	*menuP=(MNU_Menu_t *)menuID;
long		i;
UB			changed=0;

	if ( i = menuP->nItems )
	{
	MNU_Item_t      *itemsP = menuP->itemsP;

		if ( check )
			for (; i; --i, ++itemsP)
			{
			UL      old = itemsP->flags;
				changed |= (old != (itemsP->flags |= MNU_FLG_ICHECKED));
			}
		else
			for (; i; --i, ++itemsP)
			{
			UL      old = itemsP->flags;
				changed |= (old != (itemsP->flags &= ~MNU_FLG_ICHECKED));
			}
	}

	if ( changed && menuP->visible == VISIBLE )
		menuP->doRebuild = 1;
}

//==============================================
void MNU_ItemCheck( long menuID, long itm, UB check )
{
MNU_Menu_t      *menuP=(MNU_Menu_t *)menuID;
UB                      changed=0;

	if ( itm >= 0 && itm < menuP->nItems )
	{
	UL      old = menuP->itemsP[ itm ].flags;

		if ( check )
			menuP->itemsP[ itm ].flags |= MNU_FLG_ICHECKED;
		else
			menuP->itemsP[ itm ].flags &= ~MNU_FLG_ICHECKED;

		if ( old != menuP->itemsP[ itm ].flags && menuP->visible == VISIBLE )
			menuP->doRebuild = 1;
	}
}

//==============================================
UL MNU_ItemFlagsGet( long menuID, long itm )
{
MNU_Menu_t	*menuP=(MNU_Menu_t *)menuID;

	if ( itm >= 0 && itm < menuP->nItems )
		return menuP->itemsP[ itm ].flags;

	return 0;
}

//==============================================
void MNU_ItemFlagsSet( long menuID, long itm, UL flags )
{
MNU_Menu_t	*menuP=(MNU_Menu_t *)menuID;

	if ( itm >= 0 && itm < menuP->nItems )
	{
		if ( menuP->itemsP[ itm ].flags != flags )
		{
			menuP->itemsP[ itm ].flags = flags;
			if ( menuP->visible == VISIBLE )
				menuP->doRebuild = 1;
		}
	}
}

//==============================================
void MNU_ItemAddID( long menuID, const char *textP, long subMenuID, UL flags )
{
MNU_Menu_t      *menuP=(MNU_Menu_t *)menuID;
MNU_Item_t      *listP;

	if NOT( menuP->itemsP =
			(MNU_Item_t *)realloc( menuP->itemsP, sizeof(MNU_Item_t)*(menuP->nItems+1) ) )
		return;

	listP = menuP->itemsP + menuP->nItems++;
	strcpymaxsize( listP->name, textP, MNU_ITM_MAXLEN );
	listP->subMenuP = (MNU_Menu_t *)subMenuID;
	listP->flags = flags;
	if ( strcmp(textP,"-") )
		listP->flags |= MNU_FLG_IACTIVE;
	listP->callBack = 0;

	if ( menuP->visible == VISIBLE )
		menuP->doRebuild = 1;
}

//==============================================
void MNU_ItemAddCB( long menuID, const char *textP, MNU_CallBack_t callBack, UL flags )
{
MNU_Menu_t      *menuP=(MNU_Menu_t *)menuID;
MNU_Item_t      *listP;

	if NOT( menuP->itemsP =
			(MNU_Item_t *)realloc( menuP->itemsP, sizeof(MNU_Item_t)*(menuP->nItems+1) ) )
		return;

	listP = menuP->itemsP + menuP->nItems++;
	strcpymaxsize( listP->name, textP, MNU_ITM_MAXLEN );
	listP->subMenuP = 0;
	listP->flags = flags;
	if ( strcmp(textP,"-") )
		listP->flags |= MNU_FLG_IACTIVE;
	listP->callBack = callBack;

	if ( menuP->visible == VISIBLE )
		menuP->doRebuild = 1;
}

//==============================================
UB MNU_EventMouseMove( short xs, short ys )
{
MNU_Menu_t      *curP;
long            x,y;

	if NOT( ACT )   return 0;

	x=(long)xs<<4; y=(long)ys<<4;
	ACT->mouseX = x;
	ACT->mouseY = y;
	curP=ACT->curMenuP;
	if ( curP->visible == VISIBLE && !curP->doRebuild )
	{
		//MNU_Rebuild( curP );
		inbound( curP, &x, &y );
		_MNU_FindItem( curP,x,y );
		return 1;
	}
	return 0;
}
//==============================================
UB MNU_EventMouseClick( UB but, UB state, short xs, short ys )
{
MNU_Menu_t      *curP;
long            x, y;

	if NOT( ACT )   return 0;
	MNU_Rebuild(ACT);

	curP=ACT->curMenuP;
	x=(long)xs<<4; y=(long)ys<<4;

	if ( but == MOU_BUT_RGT )
	{
		if ( state & MOU_BUT_DOWN )
		{
			if NOT( ACT->visible == VISIBLE )  // only for main menu
			{
				if ( ACT->specialPointerP )
					EVT_PointerSet( ACT->specialPointerP, ACT->spointWd, ACT->spointHe );
					
				if NOT ( _MNU_PrepareBitMap(ACT) )
				{
					ACT->z = 0;
					ACT->staTicks = TMR_TicksGet();
					ACT->visible = VISIBLE;
					ACT->curItemIdx = -1;
					ACT->px = x - ACT->map.wd*8;  ACT->py += y;
					_MNU_FindItem(ACT,x,y);
					return 1;
				}
			}

			if ( curP->visible == VISIBLE )    // for main menu and submenus
			{
				_MNU_MenuClose( curP, x, y, 1 );
				if ( ACT->specialPointerP && curP == ACT )
					EVT_PointerSet( 0, 0, 0 );
				return 1;
			}
		}
	}
	else
	if (but == MOU_BUT_LFT )
	{
		if ( state & MOU_BUT_DOWN )
		if ( curP->visible == VISIBLE && !curP->doRebuild )
		{
			_MNU_FindItem(curP,x,y);
			if ( curP->curItemIdx >= 0 )
			{
			MNU_Item_t      *itmP = &curP->itemsP[ curP->curItemIdx ];
			MNU_Menu_t      *sm;

				if ( itmP->flags & MNU_FLG_IACTIVE )
				{
					if ( itmP->callBack )
						itmP->callBack( (long)curP, curP->curItemIdx );
					else
					if ( sm = itmP->subMenuP )
						_MNU_MenuCreate( sm, curP, x, y );
				}
			}
			return 1;
		}
	}
	return 0;
}

//==============================================
static UB doscroll( long *menuCo, long mouseCo, long range, long scrDim, long menuDim )
{
	if ( mouseCo < range )
	{
		if ( *menuCo < 0 )
		{
			if ( mouseCo <= 0 )
				*menuCo += range - mouseCo >> 2;
			else
				*menuCo += (range - mouseCo) / 10;

			if ( *menuCo > 0 )      *menuCo = 0;
			return 1;
		}
	}
	else
	{
	long    startPos = scrDim-range;
	
		if ( mouseCo >= startPos )
		{
			if ( *menuCo+menuDim >= scrDim )
			{
				if ( mouseCo >= scrDim-16 )
					*menuCo -= mouseCo - startPos >> 2;
				else
					*menuCo -= (mouseCo - startPos) / 10;

				if ( *menuCo+menuDim < scrDim )
					*menuCo = scrDim - menuDim;
				return 1;
			}
		}
	}

	return 0;
}

//==============================================
static void _MNU_CheckScroll(MNU_Menu_t *m, long mx, long my, const BitMap *bmP)
{
long    wd, he;

	wd = bmP->wd<<4;
	he = bmP->he<<4;

	if ( doscroll( &m->px, mx, wd/5, wd, m->map.wd<<4 ) )
		_MNU_FindItem( m,mx,my );

	if ( doscroll( &m->py, my, he/4, he, m->map.he<<4 ) )
		_MNU_FindItem( m,mx,my );
}

//==============================================
BO _MNU_CurMenuIsVisible(void)
{
	return ACT && ACT->visible;
}

//==============================================
UB MNU_Render(BitMap *bmP)
{
	if ( ACT && ACT->visible )
	{
	MNU_Menu_t      *m;

		if ( ACT->visible == JUST_NOT_VISIBLE )
		{
			ACT->visible = NOT_VISIBLE;
			return 1;
		}

		
		MNU_Rebuild( ACT->curMenuP );
		_MNU_CheckScroll( ACT->curMenuP, ACT->mouseX, ACT->mouseY, bmP );
		BMP_PushSet( bmP );

			m = ACT;
			while ( m )
			{
				MNU_Rebuild( m );

				if ( TMR_TicksPerSecGet() )
					ACT->z = 10 + (TMR_TicksGet() - ACT->staTicks) * 10 * 120 / TMR_TicksPerSecGet();
				else
					ACT->z += 10;

				if ( ACT->z >= 256 )
				{
					ACT->z = 256;
					BMP_BitMapPutCP( &m->map, m->px>>4, m->py>>4 );
				}
				else
				{
					short nwd = 256 * m->map.wd / ACT->z;
					short nhe = 256 * m->map.he / ACT->z;
					BMP_BitMapStretchCP( &m->map,
										(m->px>>4) /*+ nwd/2*/, (m->py>>4) /*+ nhe/2*/,
										nwd, nhe );
				}
				m = m->nextVisP;
			}
		BMP_Pop();

		return 1;
	}

	return 0;
}
// BMP_RectFillCP(x+1,y+2, x+ACT->map.wd, y+1+ACT->map.he, ACT->dkCol );

//==============================================
void MNU_ScaleSet( long menuID, BO doSubs, float scaleWd, float scaleHe )
{
MNU_Menu_t      *menuP;

	if NOT( menuP = (MNU_Menu_t *)menuID )
		return;

	if ( doSubs )
	{
	long		i;
	MNU_Menu_t	*subP;

		for(i=0; i < menuP->nItems; ++i)
			if ( subP = menuP->itemsP[i].subMenuP )
				MNU_ScaleSet( (long)subP, 1, scaleWd, scaleHe );
	}
	menuP->scaleWd = scaleWd;
	menuP->scaleHe = scaleHe;
}

//==============================================
void MNU_PointerSet( long menuID, UB *dataP, US wd, US he )
{
MNU_Menu_t	*menuP=(MNU_Menu_t *)menuID;

	SAFE_FREE( menuP->specialPointerP );
	if ( dataP )
		if ( NEW(menuP->specialPointerP,wd*he) )
		{
			memcpy( menuP->specialPointerP, dataP, wd*he );
			menuP->spointWd = wd;
			menuP->spointHe = he;
		}
}


