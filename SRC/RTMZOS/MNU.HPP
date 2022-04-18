//==============================================
// MNU.HPP - menu stuff
// Copyright (C) by Davide Pasca 1995
//==============================================

#ifndef __MNU_HPP__
#define __MNU_HPP__

#include "EXTTYPES.HPP"

#define MNU_ITM_MAXLEN  40

#define MNU_FLG_IACTIVE         1
#define MNU_FLG_ICHECKED        2

typedef void (*MNU_CallBack_t)(long,long);

long MNU_New(long fontID, UB txCol, UB bkCol, UB ltCol, UB dkCol );
void MNU_ColorsSet( long menuID, BO subs, UB txCol, UB bkCol, UB ltCol, UB dkCol );
void MNU_FontSet( long menuID, BO subs, long fontID );
void MNU_Dispose( long menuID, BO disposeSubs );
void MNU_ItemsReset( long menuID );
void MNU_ItemCheckAll( long menuID, UB check );
void MNU_ItemCheckRange( long menuID, long staitm, long enditm, UB check );
UL   MNU_ItemFlagsGet( long menuID, long itm );
void MNU_ItemFlagsSet( long menuID, long itm, UL flags );
void MNU_ItemCheck( long menuID, long itm, UB check );
void MNU_ItemAddCB( long menuID, const char *textP, MNU_CallBack_t callBack, UL flags=0 );
void MNU_ItemAddID( long menuID, const char *textP, long subMenu, UL flags=0 );
void MNU_ItemNameGet( long menuID, long itm, char *nameP );
long MNU_Set(long menuID);
long MNU_Get(void);
long MNU_UserLongGet( long menuID );
void MNU_UserLongSet( long menuID, long val );
void MNU_PointerSet( long menuID, UB *dataP, US wd, US he );
void MNU_ScaleSet( long menuID, BO doSubs, float scaleWd, float scaleHe );

UB MNU_EventMouseMove( short x, short y);
UB MNU_EventMouseClick( UB but, UB state, short x, short y);
UB MNU_Render(BitMap *bmP);

#endif
