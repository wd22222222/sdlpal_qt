#pragma once
#ifndef CPALBASEIO_H
#define CPALBASEIO_H

#include <functional>
#include <memory>
#include "cscreen.h"
#include "command.h"
#include "palsurface.h"

#define MENUITEM_VALUE_CANCELLED		   0xFFFF
//使用线程池
#define USE_THREADPOOL					1

typedef std::function<void(WORD)> LPITEMCHANGED_CALLBACK;


typedef enum tagLOADRESFLAG
{
	kLoadScene = (1 << 0),    // load a scene
	kLoadPlayerSprite = (1 << 1),    // load player sprites
} LOADRESFLAG, * LPLOADRESFLAG;

typedef struct tagPALMAP
{
	DWORD          MapTiles[128][64][2]{};
	ByteArray      pTileSprite{};
	INT            iMapNum{ -1 };

} PALMAP, * LPPALMAP;

typedef const PALMAP* LPCPALMAP;

typedef struct tagMENUITEM
{
	INT32          wValue{};
	INT32          wNumWord{};
	BOOL           fEnabled{};
	PAL_POS        pos{};
	PalSize		   size{};
} MENUITEM, * LPMENUITEM;
//typedef DWORD PAL_POS;

typedef struct tagBOX
{
	PAL_POS        pos{};
	WORD           wWidth{}, wHeight{};
	PalTexture	   lpSavedArea{};
	PalSize  getSize() { return PalSize(wWidth, wHeight); };
	PAL_Rect getRect() { return PAL_Rect(pos, getSize()); };
} BOX, * LPBOX;

#define MENUITEM_VALUE_CANCELLED      0xFFFF


typedef enum class  tagBATTLEUISTATE
{
	kBattleUIWait,
	kBattleUISelectMove,
	kBattleUISelectTargetEnemy,
	kBattleUISelectTargetPlayer,
	kBattleUISelectTargetEnemyAll,
	kBattleUISelectTargetPlayerAll,
} BATTLEUISTATE;

typedef enum class tagBATTLEMENUSTATE
{
	kBattleMenuMain,
	kBattleMenuMagicSelect,
	kBattleMenuUseItemSelect,
	kBattleMenuThrowItemSelect,
	kBattleMenuMisc,
	kBattleMenuMiscItemSubMenu,
} BATTLEMENUSTATE;

typedef enum class tagBATTLEUIACTION
{
	kBattleUIActionAttack,
	kBattleUIActionMagic,
	kBattleUIActionCoopMagic,
	kBattleUIActionMisc,
} BATTLEUIACTION;

#define SPRITENUM_BATTLEICON_ATTACK      40
#define SPRITENUM_BATTLEICON_MAGIC       41
#define SPRITENUM_BATTLEICON_COOPMAGIC   42
#define SPRITENUM_BATTLEICON_MISCMENU    43

#define SPRITENUM_BATTLE_ARROW_CURRENTPLAYER           69
#define SPRITENUM_BATTLE_ARROW_CURRENTPLAYER_RED       68

#define SPRITENUM_BATTLE_ARROW_SELECTEDPLAYER          67
#define SPRITENUM_BATTLE_ARROW_SELECTEDPLAYER_RED      66

#define BATTLEUI_LABEL_ITEM              5


#define BATTLEUI_LABEL_DEFEND            58
#define BATTLEUI_LABEL_AUTO              56
#define BATTLEUI_LABEL_INVENTORY         57
#define BATTLEUI_LABEL_FLEE              59
#define BATTLEUI_LABEL_STATUS            60

#define BATTLEUI_LABEL_USEITEM           23
#define BATTLEUI_LABEL_THROWITEM         24

#define TIMEMETER_COLOR_DEFAULT          0x1B
#define TIMEMETER_COLOR_SLOW             0x5B
#define TIMEMETER_COLOR_HASTE            0x2A

typedef struct tagSHOWNUM
{
	WORD             wNum;
	PAL_POS          pos;
	DWORD            dwTime;
	NUMCOLOR         color;
} SHOWNUM;

typedef enum tagBATTLERESULT
{
	kBattleResultWon = 3,			// player won the battle
	kBattleResultLost = 1,			// player lost the battle
	kBattleResultStop = 2,			// player stop the battle
	kBattleResultEnemyFleed = 0x7FFF,//The enemy is fleeing.
	kBattleResultFleed = 0xFFFF,	// player fleed from the battle
	kBattleResultTerminated = 0,	// battle terminated with scripts
	kBattleResultOnGoing = 1000,	// the battle is ongoing
	kBattleResultPreBattle = 1001,	// running pre-battle scripts
	kBattleResultPause = 1002,		// battle pause

} BATTLERESULT;


typedef enum class tagFIGHTERSTATE
{
	kFighterWait,		// waiting time 
	kFighterCharging,	//Charging...
	kFighterCom,		// accepting command
	kFighterAct,		// doing the actual move
	kFighterEnd,	    //end the actual move
} FIGHTERSTATE;

typedef enum class tagBATTLEACTIONTYPE
{
	kBattleActionPass,          // do nothing						无动作
	kBattleActionDefend,        // defend							防御
	kBattleActionAttack,        // physical attack					物攻
	kBattleActionMagic,         // use magic						使用法术
	kBattleActionCoopMagic,     // use cooperative magic			使用合体法术
	kBattleActionFlee,          // flee from the battle				逃跑
	kBattleActionThrowItem,     // throw item onto enemy			投掷物品
	kBattleActionUseItem,       // use item							使用物品	
	kBattleActionAttackMate,    // attack teammate (confused only)	攻击队友（疯魔状态下）
} BATTLEACTIONTYPE;

typedef struct tagBATTLEUI
{
	BATTLEUISTATE    state;
	BATTLEMENUSTATE  MenuState;

	CHAR             szMsg[256];           // message to be shown on the screen
	CHAR             szNextMsg[256];       // next message to be shown on the screen
	WORD             wNextMsgDuration;     // duration of the next message
	DWORD            dwMsgShowTime;        // the end time of showing the message

	WORD             wCurPlayerIndex;      // index of the current player
	WORD             wSelectedAction;      // current selected action
	WORD             wSelectedIndex;       // current selected index of player or enemy
	WORD             wPrevEnemyTarget;     // previous enemy target

	BATTLEACTIONTYPE wActionType;          // type of action to be performed
	WORD             wObjectID;            // object ID of the item or magic to use

	BOOL             fAutoAttack;          // TRUE if auto attack

	SHOWNUM          rgShowNum[BATTLEUI_MAX_SHOWNUM];
} BATTLEUI;


typedef struct tagBATTLEACTION
{
	BATTLEACTIONTYPE   ActionType;
	WORD               wActionID;   // item/magic to use
	SHORT              sTarget;     // -1 for everyone
} BATTLEACTION;

typedef struct tagBATTLEENEMY
{
	WORD               wObjectID{};					// Object ID of this enemy
	ENEMY              e{};							// detailed data of this enemy
	WORD               rgwStatus[kStatusAll]{};		// status effects
	POISONSTATUS       rgPoisons[MAX_POISONS]{};	// poisons
	PAL_POS            pos{};						// current position on the screen
	PAL_POS            posOriginal{};				// original position on the screen
	WORD               wCurrentFrame{};				// current frame number
	FIGHTERSTATE       state{};						// state of this enemy
	WORD               wScriptOnTurnStart{};		//Pre-war scripts, typically include pre-war dialogues
	WORD               wScriptOnBattleEnd{};		//Post-war scripts, such as obtaining certain items
	WORD               wScriptOnReady{};			//wartime scripts, generally referring to the enemy’s moves and strategies
	DWORD              wPrevHP{};					// HP value prior to action（DWORD）
	INT                iColorShift{};

	DWORD              dwActualHealth{};
	DWORD              dwMaxHealth{};
	DWORD			   dwExp{};//
	
	//以下设置与即时战斗相关

	int				   mMoveTimes{};				//The number of attacks the enemy makes per turn
	int				   mCurrentMove{};				//The actual second attack by the enemy
	FLOAT			   flTimeChargingSpeed{};		//Calculate the time charging speed.		
	FLOAT			   flTimeMeter{};				//time-charging meter (0 = empty, 100 = full).

	ByteArray          lpSprite{};					//
	void clear()
	{
		memset(this, 0, sizeof(tagBATTLEENEMY) - sizeof(ByteArray));
		lpSprite.clear();
	};
} BATTLEENEMY;

// We only put some data used in battle here; other data can be accessed in the global data.
typedef struct tagBATTLEPLAYER
{
	INT                iColorShift;
	PAL_POS            pos;                  // current position on the screen
	PAL_POS            posOriginal;          // original position on the screen
	WORD               wHidingTime;          // remaining hiding time
	WORD               wCurrentFrame;        // current frame number
	FIGHTERSTATE       state;                // state of this player
	BATTLEACTION       action;               // action to perform
	BOOL               fDefending;           // TRUE if player is defending
	INT				   fDefenAction;		 // The player action is defend 
	INT				   fNotKillImmediately;  // Not Kill the player immediately
	INT				   fBadStatus;			 //Set the bad status for player
	WORD               wPrevHP;              // HP value prior to action
	WORD               wPrevMP;              // MP value prior to action
	DWORD			   dwExpGained;			 //Experience points for individual players
	//
	FLOAT			   flTimeChargingSpeed;	 //Calculate the time charging speed.		
	FLOAT			   flTimeMeter;			 //time-charging meter (0 = empty, 100 = full).

	ByteArray          lpSprite;			 //Player character

	void clear()
	{
		memset(this, 0, sizeof(tagBATTLEPLAYER) - sizeof(ByteArray));
		lpSprite.clear();
	};

} BATTLEPLAYER;



typedef enum class tabBATTLEPHASE
{
	kBattlePhaseSelectAction,
	kBattlePhasePerformAction
} BATTLEPHASE;


typedef struct tagACTIONQUEUE
{
	BOOL		fIsEmpty;
	BOOL		fIsEnemy;
	DWORD       wDexterity;
	int 		wIndex;
} ACTIONQUEUE;

#define MAX_ACTIONQUEUE_ITEMS (MAX_PLAYERS_IN_PARTY + MAX_ENEMIES_IN_TEAM)


typedef struct tagBATTLE
{

	WORD             wMaxEnemyIndex;


	SHORT            sBackgroundColorShift;

	PAL_POS          posSummon;
	INT              iSummonFrame;         // current frame of the summoned god

	INT              iExpGained;           // total experience value gained
	INT              iCashGained;          // total cash gained

	BOOL             fIsBoss;              // TRUE if boss fight
	BOOL             fEnemyCleared;        // TRUE if enemies are cleared
	BATTLERESULT     BattleResult;

	BATTLEUI         UI;

	
	BOOL             fEnemyMoving;         // TRUE if enemy is moving

	BOOL             fPlayerMoving;         // TRUE if player is moving


	WORD             wMovingPlayerIndex;   // current moving player index

	WORD             wMovingEnemyIndex;   // new, current moving Enemy index

	int              iBlow;


	BATTLEPHASE      Phase;
	BOOL             fRepeat;              // TRUE if player pressed Repeat
	BOOL             fForce;               // TRUE if player pressed Force
	BOOL             fFlee;                // TRUE if player pressed Flee

	BOOL			 g_ifUseMenshe[MAX_PLAYER_ROLES];	//使用梦蛇后，各项属性增加

	FLOAT            flTimeChargingUnit{};					// the base waiting time unit
	
	
	//以下结构含不能直接通过memset清零

#define  BattleMemsetSize  (sizeof(struct tagBATTLE) -		\
		sizeof(BATTLEPLAYER) * MAX_PLAYERS_IN_PARTY -		\
		sizeof(BATTLEENEMY) * MAX_ENEMIES_IN_TEAM -			\
		sizeof(PalTexture) * 2 -							\
		sizeof(ByteArray) * 3)


	BATTLEPLAYER     rgPlayer[MAX_PLAYERS_IN_PARTY]{};		//

	BATTLEENEMY      rgEnemy[MAX_ENEMIES_IN_TEAM]{};		//

	PalTexture		 lpSceneBuf{};							//战斗画面的图层，包括角色、敌人

	PalTexture		 lpBackground{};						//战斗中地图画面的图层

	ByteArray        lpEffectSprite{};					    //战斗效果精灵图。

	std::vector<PalTexture> spriteList{};					//进度条精灵列表
	
	ByteArray        lpSummonSprite{};						// sprite of summoned god

	void clear()											//初始化结构
	{
		memset(this, 0, BattleMemsetSize);
		clearBATTLEPLAYER();
		clearrBATTLEENEMY();
		lpSceneBuf.clear();
		lpBackground.clear();
		spriteList.clear();
		lpSummonSprite.clear();
	}

	void clearBATTLEPLAYER()								//初始化 BATTLEPLAYER 结构
	{
		for (int i = 0; i < MAX_PLAYERS_IN_PARTY; i++)
			rgPlayer[i].clear();
	}
	void clearrBATTLEENEMY()								//初始化 BATTLEENEMY  结构
	{
		for (int i = 0; i < MAX_ENEMIES_IN_TEAM; i++)
			rgEnemy[i].clear();
	}
} 
BATTLE;

typedef struct tagRESOURCES
{
	int              bLoadFlags{};
	int              nEventObject{};										// number of event objects
	std::vector<ByteArray>	 lppEventObjectSprites{};						// event object sprites
	ByteArray        rglpPlayerSprite[MAX_PLAYERS_IN_PARTY + 1]{};			// player sprites
	std::unique_ptr<PALMAP>    lpMap{};                                     // current loaded map
} RESOURCES;


//本类是游戏底层与上层之间的中间层
class CPalBaseIO : public CScreen
{
public:
    struct MAGICITEM
    {
        WORD         wMagic;
        WORD         wMP;
        BOOL         fEnabled;
    } rgMagicItem[MAX_PLAYER_MAGICS];
	BATTLE g_Battle;
	int     g_iCurrentItem = 0;
	int     g_iNumMagic = 0;
	WORD	g_wCurEffectSprite = 0;
	BOOL    g_fScriptSuccess = TRUE;
	WORD    g_wPlayerMP = 0;
	BOOL	g_fUpdatedInBattle = 0;

public:
    CPalBaseIO();

	~CPalBaseIO();

	
	VOID PAL_RNGPlay(INT iNumRNG, INT iStartFrame, INT iEndFrame, INT iSpeed);
    INT PAL_RLEBlitMonoColor(LPCBITMAPRLE lpBitmapRLE, 
		PalTexture& lpDstSurface, PAL_POS pos, BYTE bColor, INT iColorShift);
	VOID PAL_StartDialog(BYTE bDialogLocation, BYTE bFontColor, INT iNumCharFace, BOOL fPlayingRNG);
	VOID PAL_DialogWaitForKey(VOID);
	VOID PAL_ShowDialogText(std::string lpszText);
	VOID PAL_ShowWideDialogText(std::wstring lpszText);
	VOID PAL_ClearDialog(BOOL fWaitForKey);
	VOID PAL_EndDialog(VOID);
	BOOL PAL_IsInDialog(VOID);
	BOOL PAL_DialogIsPlayingRNG(VOID);
	INT	 PAL_ApplyWave(PalTexture& lpSurface);
	VOID PAL_ApplyWave(SDL_Surface* lpSurface);
	VOID PAL_DialogSetDelayTime(INT iDelayTime);


	//cmenu.cpp
	INT PAL_OpeningMenu(VOID);
	VOID PAL_DrawOpeningMenuBackground(VOID);
	WORD PAL_ReadMenu(LPMENUITEM rgMenuItem, INT nMenuItem, WORD wDefaultItem,
		BYTE bLabelColor, LPITEMCHANGED_CALLBACK lpfnMenuItemChanged);

	INT PAL_SaveSlotMenu(WORD wDefaultSlot);
	LPBOX PAL_CreateBox(PAL_POS pos, INT nRows, INT nColumns, INT iStyle, BOOL fSaveScreen);
	VOID PAL_DeleteBox(LPBOX  lpBox);
	BOOL PAL_ConfirmMenu(LPCSTR text = NULL);
	LPBOX PAL_CreateSingleLineBox(PAL_POS pos, INT nLen, BOOL fSaveScreen);

	//cres.cpp
public:
	std::unique_ptr<RESOURCES> gpResources{};

public:
	//cres.cpp
	VOID PAL_SetLoadFlags(BYTE bFlags);
	VOID PAL_LoadResources(VOID);
	LPPALMAP PAL_GetCurrentMap()const;
	LPCSPRITE PAL_GetPlayerSprite(BYTE bPlayerIndex)const;
	LPCSPRITE PAL_GetEventObjectSprite(WORD wEventObjectID)const;
	WORD PAL_SpriteGetNumFrames(LPCSPRITE lpSprite)const;
	WORD PAL_SpriteGetNumFrames(ByteArray& const lpSprite)const
	{
		return PAL_SpriteGetNumFrames(lpSprite.data());
	};
private:
	VOID PAL_InitResources(VOID);
	VOID PAL_FreeResources(VOID);

public:
	//VOID PAL_FreeEventObjectSprites(LPRESOURCES  const p)const;
	//VOID PAL_FreePlayerSprites(LPRESOURCES  const p)const;
	//cplayer.cpp
	BOOL PAL_AddItemToInventory(WORD wObjectID, INT iNum);
	const  INT  PAL_GetItemAmount(WORD wItem)const;
	BOOL PAL_IncreaseHPMP(WORD wPlayerRole, SHORT sHP, SHORT sMP);
	VOID PAL_RemoveEquipmentEffect(WORD wPlayerRole, WORD wEquipPart);
	VOID PAL_AddPoisonForPlayer(WORD wPlayerRole, WORD wPoisonID);
	VOID PAL_CurePoisonByKind(WORD wPlayerRole, WORD wPoisonID);
	VOID PAL_CurePoisonByLevel(WORD wPlayerRole, WORD wMaxLevel);
	BOOL PAL_New_IsAnItemInArray(WORD item, const WORD items[]);
	BOOL PAL_IsPlayerPoisonedByLevel(WORD wPlayerRole, WORD wMinLevel);
	VOID PAL_New_SortPoisonsForPlayerByLevel(WORD wPlayerRole);
	const  INT  PAL_New_GetPoisonIndexForPlayer(WORD wPlayerRole, WORD wPoisonID)const ;
	const  BOOL PAL_New_IsPlayerPoisoned(WORD wPlayerRole)const;
	const  WORD PAL_GetPlayerAttackStrength(WORD wPlayerRole)const;
	const  WORD PAL_GetPlayerMagicStrength(WORD wPlayerRole)const;
	const  WORD PAL_GetPlayerDefense(WORD wPlayerRole) const;
	const  WORD PAL_GetPlayerDexterity(WORD wPlayerRole)const ;
	const  WORD PAL_GetPlayerFleeRate(WORD wPlayerRole) const;
	const  WORD PAL_GetPlayerPoisonResistance(WORD wPlayerRole)const;
	const  WORD PAL_New_GetPlayerSorceryResistance(WORD wPlayerRole)const;
	const  WORD PAL_New_GetPlayerSorceryStrength(WORD wPlayerRole)const;
	const  WORD PAL_GetPlayerElementalResistance(WORD wPlayerRole, INT iAttrib)const;
	const  WORD PAL_New_GetPlayerPhysicalResistance(WORD wPlayerRole)const;
	const  WORD PAL_GetPlayerBattleSprite(WORD wPlayerRole)const;
	const  WORD PAL_GetPlayerCooperativeMagic(WORD wPlayerRole)const;
	const  INT PAL_New_GetPlayerIndex(WORD wPlayerRole)const;
	const  WORD PAL_New_GetPlayerID(WORD wPlayerIndex)const;
	const  INT PAL_New_GetPlayerIndexByPara(PlayerPara pp, BOOL fGetLowest)const;
	const  INT PAL_New_GetPlayerIndexByHealth(BOOL fGetLowest)const;
	const  BOOL PAL_New_IfPlayerCanMove(WORD wPlayerRole)const;
	BOOL PAL_PlayerCanAttackAll(WORD wPlayerRole);
	BOOL PAL_AddMagic(WORD wPlayerRole, WORD wMagic);
	VOID PAL_RemoveMagic(WORD wPlayerRole, WORD wMagic);
	VOID PAL_SetPlayerStatus(WORD wPlayerRole, WORD wStatusID, WORD wNumRound);
	VOID PAL_RemovePlayerStatus(WORD wPlayerRole, WORD wStatusID);
	VOID PAL_New_RemovePlayerAllStatus(WORD wPlayerRole);
	VOID PAL_ClearAllPlayerStatus(VOID);
	//VOID PAL_PlayerLevelUp(WORD wPlayerRole, WORD wNumLevel);
	BOOL PAL_New_GetTrueByPercentage(WORD wPercentage)const;
	DWORD PAL_New_GetLevelUpBaseExp(WORD wLevel)const;
	DWORD PAL_New_GetLevelUpExp(WORD wLevel)const;
	//hidingtime
	INT PAL_GetHidingTime()const;
	VOID PAL_SetHidingTime(int hide);
	VOID PAL_UpdateHidingTime(int play);

	//cmap.cpp
	std::unique_ptr <PALMAP> PAL_LoadMap(INT iMapNum, const ByteArray& fpMapMKF, const ByteArray& fpGopMKF)const;
	VOID PAL_FreeMap(LPPALMAP &lpMap)const;
	LPCBITMAPRLE PAL_MapGetTileBitmap(BYTE x, BYTE y, BYTE h, BYTE ucLayer, LPCPALMAP lpMap)const;
	BOOL PAL_MapTileIsBlocked(BYTE x, BYTE y, BYTE h, LPCPALMAP lpMap)const;
	BYTE PAL_MapGetTileHeight(BYTE x, BYTE y, BYTE h, BYTE ucLayer, LPCPALMAP lpMap)const;
	VOID PAL_MapBlitToSurface(LPCPALMAP lpMap, SDL_Surface* lpSurface, const SDL_Rect* lpSrcRect, BYTE ucLayer)const;
	VOID PAL_MapBlitToSurface(LPCPALMAP lpMap, PalTexture& lpSurface, const SDL_Rect* lpSrcRect, BYTE ucLayer)const;
};

#endif // CPALBASEIO_H
