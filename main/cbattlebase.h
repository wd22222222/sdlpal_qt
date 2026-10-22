#pragma once
#ifndef CBATTLEBASE_H
#define CBATTLEBASE_H

#include <vector>
#include "cenemy.h"
#include "palgpgl.h"
#include "paltexture.h"

//本类定义关于战斗的基本方法和属性方法，
//继承于CPalBaseIO，被其他类继承

const WORD g_rgPlayerPos[5][5][2] =
{
		{{240, 170}},													// one player
		{{200, 176}, {256, 152}},										// two players
		{{180, 180}, {234, 170}, {270, 146}},							// three players
		{{160, 180}, {217, 175}, {255, 155}, {285, 135}},				// 4 players
		{{160, 180}, {210, 175}, {240, 160}, {265, 145}, {285, 125}},	// 5 players
};

const INT ExpTypeToPlayerPara[] =
{
	0, Para_MaxHP, Para_MaxMP, Para_AttackStrength, Para_MagicStrength, Para_Defense, Para_Dexterity, Para_FleeRate
};

const INT ExpTypeToLabel[] =
{
	0, STATUS_LABEL_HP, STATUS_LABEL_MP, STATUS_LABEL_ATTACKPOWER, STATUS_LABEL_MAGICPOWER,
	STATUS_LABEL_RESISTANCE, STATUS_LABEL_DEXTERITY, STATUS_LABEL_FLEERATE
};

enum   B_NEWWORD
{
	e_NewWord_XinXi = PAL_ADDITIONAL_WORD_SECOND,    //信息
	e_NewWord_QinBao,   //情报
	e_NewWord_LinFang = PAL_ADDITIONAL_WORD_SECOND + 2,
	e_NewWord_DuFang,
	e_NewWord_TouQie,
	e_NewWord_WuKang,
	e_NewWord_LiangCiGongJi,
	e_NewWord_WuGongFuJia,
	e_NewWord_MuoFa,
	e_NewWord_WuLiKang,
};

using sprite = PalTexture;//缩微纹理


class CBattleBase: public CEnemy
{
public:
	int g_iCurMiscMenuItem = 0;
	int g_iCurSubMenuItem = 0;
	size_t g_Loop = 0;
	std::vector <WORD> g_ScriptOnBattleEnd;
public:    
	CBattleBase();
    VOID PAL_BattleMakeScene();
    VOID PAL_BattleBackupScene();
	VOID PAL_BattleUpdateFighters(VOID);
	BOOL PAL_IsPlayerDying(WORD wPlayerRole)const;
	INT PAL_BattleSelectAutoTarget(VOID);
	VOID PAL_LoadBattleBackground(VOID);
	INT PAL_New_GetAliveEnemyNum();
	VOID PAL_LoadBattleSprites(VOID);
	VOID PAL_FreeBattleSprites(VOID);
	VOID PAL_BattleBackupStat(VOID);
	BOOL PAL_New_IfEnemyCanMove(WORD wEnemyIndex);
	WORD PAL_NEW_CheckAndGetLegalPlayerTarget(WORD wPlayerRole);
	WORD PAL_NEW_CheckAndGetLegalEnemyTarget(WORD wEnemyIndex);
	INT PAL_New_EnemyNotMoved(WORD wEnemyIndex);

};

#endif // CBATTLEBASE_H
