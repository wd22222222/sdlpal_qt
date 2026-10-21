/* -*- mode: c; tab-width: 4; c-basic-offset: 3; c-file-style: "linux" -*- */
//
// Copyright (c) 2009, Wei Mingzhi <whistler_wmz@users.sf.net>.
// All rights reserved.
//
// This file is part of SDLPAL.
//
// SDLPAL is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//
#include "cfight.h"
#include "cscript.h"
#include "cpaldata.h"

#define NOMINMAX
#include <assert.h>
#include <algorithm>

#define PAL_RunTriggerScript  pCScript->PAL_RunTriggerScript
#define PAL_RunAutoScript     pCScript->PAL_RunAutoScript

static int s_iFrame;//0 or 1

CFight::CFight()
{

}

VOID CFight::PAL_BattleDelay(
	WORD       wDuration,
	WORD       wObjectID,
	BOOL       fUpdateGesture
)
/*++
  功能：    Delay a while during battle.
  当战斗时延迟一段时间
  参数：    [IN]  wDuration - Number of frames of the delay.
  [IN]  wObjectID - The object ID to be displayed during the delay.
  [IN]  fUpdateGesture - TRUE if update the gesture for enemies, FALSE if not.
  延迟帧数，延迟时需要显示的对象id，更新敌人动作（更新为真，否则为假）
  返回值：    None.
  --*/
{
	int    i, j;
	DWORD  dwTime = SDL_GetTicks_New() + cfgBtlFrameMs;

	for (i = 0; i < wDuration; i++)
	{
		if (fUpdateGesture)
		{
			//
			// Update the gesture of enemies.
			// 更新敌人动作
			for (j = 0; j <= g_Battle.wMaxEnemyIndex; j++)
			{
				if (g_Battle.rgEnemy[j].wObjectID == 0 ||
					g_Battle.rgEnemy[j].rgwStatus[kStatusSleep] != 0 ||
					g_Battle.rgEnemy[j].rgwStatus[kStatusParalyzed] != 0)
				{
					continue;
				}

				if (--g_Battle.rgEnemy[j].e.wIdleAnimSpeed == 0)
				{
					g_Battle.rgEnemy[j].wCurrentFrame++;
					g_Battle.rgEnemy[j].e.wIdleAnimSpeed =
						gpGlobals->g.lprgEnemy[gpGlobals->g.rgObject[g_Battle.rgEnemy[j].wObjectID].enemy.wEnemyID].wIdleAnimSpeed;
				}

				if (g_Battle.rgEnemy[j].wCurrentFrame >= g_Battle.rgEnemy[j].e.wIdleFrames)
				{
					g_Battle.rgEnemy[j].wCurrentFrame = 0;
				}
			}
		}

		//
		// Wait for the time of one frame. Accept input here.
		// 等待一帧的时间，在此接受输入
		PAL_ProcessEvent();
		while (SDL_GetTicks_New() <= dwTime)
		{
			PAL_ProcessEvent();
			SDL_Delay(1);
		}

		//
		// Set the time of the next frame.
		// 设置下一帧的时间
		dwTime = SDL_GetTicks_New() + cfgBtlFrameMs;

		PAL_BattleMakeScene();
		RenderBlendCopy(gpTextureReal, g_Battle.lpSceneBuf);;

		PAL_BattleUIUpdate();

		if (wObjectID != 0)
		{
			if (wObjectID == BATTLE_LABEL_ESCAPEFAIL) // HACKHACK
			{
				PAL_DrawText(PAL_GetWord(wObjectID), PAL_XY(130, 75),
					15, TRUE, FALSE);
			}
			else if ((SHORT)wObjectID < 0)
			{
				PAL_DrawText(PAL_GetWord(-((SHORT)wObjectID)), PAL_XY(170, 45),
					DESCTEXT_COLOR, TRUE, FALSE);
			}
			else
			{
				PAL_DrawText(PAL_GetWord(wObjectID), PAL_XY(210, 50),
					15, TRUE, FALSE);
			}
		}

		VIDEO_UpdateScreen(gpTextureReal);
	}
}


BOOL CFight::PAL_BattleDisplayStatChange()
/*++
  功能：    Display the HP and MP changes of all players and enemies.
  展示角色和敌人的hp和mp改变值
  参数：    None.
  返回值：  TRUE if there are any number displayed, FALSE if not.
  如果有显示数字返回真，否则返回假
  --*/
{
	int      i, x, y;
	INT      sDamage;
	WORD     wPlayerRole;
	BOOL     f = FALSE;

	for (i = 0; i <= g_Battle.wMaxEnemyIndex; i++)
	{
		if (g_Battle.rgEnemy[i].wObjectID == 0)
		{
			continue;
		}

		if (g_Battle.rgEnemy[i].wPrevHP != g_Battle.rgEnemy[i].dwActualHealth)
		{
			//
			// Show the number of damage
			//
			sDamage = g_Battle.rgEnemy[i].dwActualHealth - g_Battle.rgEnemy[i].wPrevHP;

			x = PAL_X(g_Battle.rgEnemy[i].pos) - 9;
			y = PAL_Y(g_Battle.rgEnemy[i].pos) - 115;
			y = std::max(10, (int)y);

			if (sDamage > 0)
			{
				PAL_BattleUIShowNum((WORD)(sDamage), PAL_XY(x, y), kNumColorYellow);
			}
			else
			{
				PAL_BattleUIShowNum((WORD)(-sDamage), PAL_XY(x, y), kNumColorBlue);
			}
			f = TRUE;
		}
	}

	for (i = 0; i <= gpGlobals->wMaxPartyMemberIndex; i++)
	{
		wPlayerRole = gpGlobals->rgParty[i].wPlayerRole;

		if (g_Battle.rgPlayer[i].wPrevHP != gpGlobals->g.PlayerRoles.rgwHP[wPlayerRole])
		{
			sDamage = gpGlobals->g.PlayerRoles.rgwHP[wPlayerRole] - g_Battle.rgPlayer[i].wPrevHP;

			x = PAL_X(g_Battle.rgPlayer[i].pos) - 9;
			y = PAL_Y(g_Battle.rgPlayer[i].pos) - 75;
			y = std::max(10, (int)y);

			if (sDamage > 0)
			{
				PAL_BattleUIShowNum((WORD)(sDamage), PAL_XY(x, y), kNumColorYellow);
			}
			else
			{
				PAL_BattleUIShowNum((WORD)(-sDamage), PAL_XY(x, y), kNumColorBlue);
			}
			f = TRUE;
		}

		if (g_Battle.rgPlayer[i].wPrevMP != gpGlobals->g.PlayerRoles.rgwMP[wPlayerRole])
		{
			sDamage = gpGlobals->g.PlayerRoles.rgwMP[wPlayerRole] - g_Battle.rgPlayer[i].wPrevMP;

			x = PAL_X(g_Battle.rgPlayer[i].pos) - 9;
			y = PAL_Y(g_Battle.rgPlayer[i].pos) - 67;
			y = std::max(10, (int)y);

			// Only show MP increasing
			if (sDamage > 0)
			{
				PAL_BattleUIShowNum((WORD)(sDamage), PAL_XY(x, y), kNumColorCyan);
			}
			else
				if (cfgShowMPDrop)
				{
					PAL_BattleUIShowNum((WORD)(-sDamage), PAL_XY(x, y), kNumColorBlue);
				}
			f = TRUE;
		}
	}

	return f;
}

void CFight::PAL_BattlePostActionCheck(
	BOOL fCheckPlayers
)
/*++
  功能：    Essential checks after an action is executed.
  参数：    [IN]  fCheckPlayers - TRUE if check for players, FALSE if not.
  返回值：    None.
  --*/
{
	BOOL     fFade{};
	BOOL     fEnemyRemaining{};

	for (int i = 0; i <= g_Battle.wMaxEnemyIndex; i++)
	{
		if (g_Battle.rgEnemy[i].wObjectID == 0)
		{
			continue;
		}

		if ((g_Battle.rgEnemy[i].dwActualHealth) == 0)
		{
			// This enemy is KO'ed
			g_Battle.iExpGained += g_Battle.rgEnemy[i].dwExp;

			if (cfgPostBtlScript)
				g_ScriptOnBattleEnd.push_back(g_Battle.rgEnemy[i].wScriptOnBattleEnd);

			if (cfgExpFormula)
				for (int j = 0; j <= gpGlobals->wMaxPartyMemberIndex; j++)
				{
					WORD w = gpGlobals->rgParty[i].wPlayerRole;
					if (gpGlobals->g.PlayerRoles.rgwHP[w])
					{
						g_Battle.rgPlayer[j].dwExpGained += g_Battle.rgEnemy[i].dwExp;
					}
					else
					{
						g_Battle.rgPlayer[j].dwExpGained += g_Battle.rgEnemy[i].dwExp / 10;
					}
				}
			g_Battle.iCashGained += g_Battle.rgEnemy[i].e.wCash;

			SOUND_Play(g_Battle.rgEnemy[i].e.wDeathSound);
			g_Battle.rgEnemy[i].wObjectID = 0;
			fFade = TRUE;

			continue;
		}

		fEnemyRemaining = TRUE;
	}

	if (!fEnemyRemaining)
	{
		g_Battle.fEnemyCleared = TRUE;
		g_Battle.UI.state = BATTLEUISTATE::kBattleUIWait;
	}

	if (fCheckPlayers && !gpGlobals->fAutoBattle)
	{
		for (int i = 0; i <= gpGlobals->wMaxPartyMemberIndex; i++)
		{
			WORD w = gpGlobals->rgParty[i].wPlayerRole;

			if (gpGlobals->g.PlayerRoles.rgwHP[w] < g_Battle.rgPlayer[i].wPrevHP &&
				gpGlobals->g.PlayerRoles.rgwHP[w] == 0)
			{	//角色死亡，掩护的角色会愤怒。。。
				w = gpGlobals->g.PlayerRoles.rgwCoveredBy[w];

				if (PAL_New_GetPlayerIndex(w) == -1)
				{
					continue;
				}

				if (gpGlobals->g.PlayerRoles.rgwHP[w] > 0 && PAL_New_IfPlayerCanMove(w))
				{
					WORD wName = gpGlobals->g.PlayerRoles.rgwName[w];

					if (gpGlobals->g.rgObject[wName].player.wScriptOnFriendDeath != 0)
					{
						PAL_BattleDelay(10, 0, TRUE);

						PAL_BattleMakeScene();
						RenderBlendCopy(gpTextureReal, g_Battle.lpSceneBuf);;

						VIDEO_UpdateScreen(gpTextureReal);

						g_Battle.BattleResult = kBattleResultPause;

						gpGlobals->g.rgObject[wName].player.wScriptOnFriendDeath =
							PAL_RunTriggerScript(gpGlobals->g.rgObject[wName].player.wScriptOnFriendDeath, w);

						g_Battle.BattleResult = kBattleResultOnGoing;

						PAL_ClearKeyAll();
						goto end;
					}
				}
			}
		}

		for (int i = 0; i <= gpGlobals->wMaxPartyMemberIndex; i++)
		{
			WORD w = gpGlobals->rgParty[i].wPlayerRole;

			if (!PAL_New_IfPlayerCanMove(w))
			{
				continue;
			}

			if (gpGlobals->g.PlayerRoles.rgwHP[w] < g_Battle.rgPlayer[i].wPrevHP)
			{
				//从正常状态到濒死状态，会有相应的脚本
				if (gpGlobals->g.PlayerRoles.rgwHP[w] > 0 && PAL_IsPlayerDying(w) &&
					g_Battle.rgPlayer[i].wPrevHP >= gpGlobals->g.PlayerRoles.rgwMaxHP[w] / 5)
				{
					SOUND_Play(gpGlobals->g.PlayerRoles.rgwDyingSound[w]);

					WORD wCover = gpGlobals->g.PlayerRoles.rgwCoveredBy[w];
					if (PAL_New_GetPlayerIndex(wCover) == -1	//掩护角色不在队伍中
						|| gpGlobals->g.PlayerRoles.rgwHP[wCover] == 0
						|| !PAL_New_IfPlayerCanMove(wCover))	//掩护角色不能行动
					{
						continue;
					}

					WORD wName = gpGlobals->g.PlayerRoles.rgwName[w];
					if (gpGlobals->g.rgObject[wName].player.wScriptOnDying != 0)
					{
						PAL_BattleDelay(10, 0, TRUE);
						PAL_BattleMakeScene();
						RenderBlendCopy(gpTextureReal, g_Battle.lpSceneBuf);;

						VIDEO_UpdateScreen(gpTextureReal);

						g_Battle.BattleResult = kBattleResultPause;

						gpGlobals->g.rgObject[wName].player.wScriptOnDying =
							PAL_RunTriggerScript(gpGlobals->g.rgObject[wName].player.wScriptOnDying, w);

						g_Battle.BattleResult = kBattleResultOnGoing;
						PAL_ClearKeyAll();
					}

					goto end;
				}
			}
		}
	}

end:
	if (fFade)
	{
		PAL_BattleBackupScene();
		PAL_BattleMakeScene();
		RenderBlendCopy(gpTextureReal, g_Battle.lpSceneBuf);;
		PAL_BattleFadeScene();
	}

	//
	// Fade out the summoned god
	//
	if (g_Battle.lpSummonSprite.size())
	{
		PAL_BattleUpdateFighters();
		PAL_BattleDelay(1, 0, FALSE);

		g_Battle.lpSummonSprite.clear();

		g_Battle.sBackgroundColorShift = 0;

		PAL_BattleBackupScene();
		PAL_BattleMakeScene();
		RenderBlendCopy(gpTextureReal, g_Battle.lpSceneBuf);;
		PAL_BattleFadeScene();
	}
}



void CFight::PAL_BattleStartFrame()
/*++
  功能：    Called once per video frame in battle.战斗画面一帧，调用一次
  参数：    None.
  返回值：    None.
  --*/
{
	//检查战斗准备情况，返回1 战斗结束
	if (PAL_BattleStartFramePrepare())
		return;

	//classic
	if (isNoClassic())
	{
		//
	}
	if (g_Battle.Phase == BATTLEPHASE::kBattlePhaseSelectAction)
	{
		// The R and F keys and Fleeing should affect all players
		if (g_Battle.UI.MenuState == BATTLEMENUSTATE::kBattleMenuMain &&
			g_Battle.UI.state == BATTLEUISTATE::kBattleUISelectMove)
		{
			if (GetKeyPress() & kKeyRepeat)
			{
				g_Battle.fRepeat = TRUE;
			}
			else if (GetKeyPress() & kKeyForce)
			{
				g_Battle.fForce = TRUE;
			}
		}

		if (g_Battle.fRepeat)
		{
			SetKeyPress(kKeyRepeat);
		}
		else if (g_Battle.fForce)
		{
			SetKeyPress(kKeyForce);
		}
		else if (g_Battle.fFlee)
		{
			SetKeyPress(kKeyFlee);
		}

		if (g_Battle.UI.state == BATTLEUISTATE::kBattleUIWait)
		{
			auto x = PAL_BattlePlayerCanOperation();
			if (x < 0)
			{
				//No players are available to control the game.
				PAL_BattlePlayerSelected();
				//
				PAL_BattleAddTimeMeter();
			}
			else if(g_Battle.rgPlayer[x].state == FIGHTERSTATE::kFighterWait)
			{
				g_Battle.wMovingPlayerIndex = x;
				g_Battle.rgPlayer[x].state = FIGHTERSTATE::kFighterCom;
				PAL_BattleUIPlayerReady(x);
			}
		}
	}
	else
	{
		//
		// Are all actions finished?
		//
		int i = PAL_BattleGetNextDoing();
		if(i < 0)
		{
			//本轮没有战斗动作或已经全部结束

			PAL_BattleBackupStat();

			for (int i = 0; i <= gpGlobals->wMaxPartyMemberIndex; i++)
			{
				//如动作结束，执行
				if (g_Battle.rgPlayer[i].state == FIGHTERSTATE::kFighterEnd)
				{
					PAL_BattleProcessPlayerAction(i);
					g_Battle.rgPlayer[i].state = FIGHTERSTATE::kFighterWait;
				}
			}

			for (int i = 0; i <= g_Battle.wMaxEnemyIndex; i++)
			{
				//如果动作结束，执行
				if (g_Battle.rgEnemy[i].state == FIGHTERSTATE::kFighterEnd)
				{
					PAL_BattleProcessEnemyAction(i);
					g_Battle.rgEnemy[i].state == FIGHTERSTATE::kFighterWait;
				}
			}
			//战斗回合结束时执行
			PAL_BattlePostActionCheck(FALSE);
			if (PAL_BattleDisplayStatChange())
			{
				PAL_BattleDelay(8, 0, TRUE);
			}

			//if (g_Battle.iHidingTime > 0)
			if (PAL_GetHidingTime() > 0)
			{
				int x = PAL_GetHidingTime();
				PAL_SetHidingTime(--x);
				if (x == 0)
				{
					PAL_BattleBackupScene();
					PAL_BattleMakeScene();
					RenderBlendCopy(gpTextureReal, g_Battle.lpSceneBuf);
					PAL_BattleFadeScene();
				}
			}

			//if (g_Battle.iHidingTime == 0)
			if (PAL_GetHidingTime() == 0)
			{
				for (int i = 0; i <= g_Battle.wMaxEnemyIndex; i++)
				{
					if (g_Battle.rgEnemy[i].wObjectID == 0)
					{
						continue;
					}
					//Each round is executed only once.
					g_Battle.rgEnemy[i].wScriptOnTurnStart =
						PAL_RunTriggerScript(g_Battle.rgEnemy[i].wScriptOnTurnStart, i);
				}
			}

			// Clear all item-using records
			for (int i = 0; i < MAX_INVENTORY; i++)
			{
				gpGlobals->rgInventory[i].nAmountInUse = 0;
			}

			// Proceed to next turn...
			g_Battle.Phase = BATTLEPHASE::kBattlePhaseSelectAction;
			g_Loop++;//战斗进入下一轮
		}
		else
		{
			//以下进行战斗动作
			{

				if (i >= MAX_PLAYERS_IN_PARTY)
				{
					int j = i - MAX_PLAYERS_IN_PARTY;
					//if (g_Battle.iHidingTime == 0 && !fOnlyPuppet &&
					if (PAL_GetHidingTime() == 0 && !fOnlyPuppet &&
						g_Battle.rgEnemy[j].wObjectID != 0)
					{
						//敌方动作
						PAL_BattleEnemyDoAction(j);

						if (g_Battle.rgEnemy[j].mMoveTimes > ++g_Battle.rgEnemy[j].mCurrentMove)
						{
							g_Battle.rgEnemy[j].flTimeMeter *= 0.75;
						}
						else
						{
							g_Battle.rgEnemy[j].mCurrentMove = g_Battle.rgEnemy[j].mMoveTimes;
							g_Battle.rgEnemy[j].flTimeMeter = 0;
							g_Battle.rgEnemy[j].state = FIGHTERSTATE::kFighterEnd;
						}
					}
					else
					{
						g_Battle.rgEnemy[j].flTimeMeter = 0.0f;
						g_Battle.rgEnemy[j].state = FIGHTERSTATE::kFighterEnd;
					}
				}
				else
				{
					if (g_Battle.rgPlayer[i].state == FIGHTERSTATE::kFighterAct)//玩家动作
					{
						PAL_BattlePlayerDoAction(i);
						g_Battle.rgPlayer[i].flTimeMeter = 0.0f;
						g_Battle.rgPlayer[i].state = FIGHTERSTATE::kFighterEnd;
					}
				}
			}
		}
	}

	// Update the battle UI
	PAL_BattleUIUpdate();
}

VOID CFight::PAL_BattleCommitAction(BOOL fRepeat)
/*++
  功能：
  执行玩家决定的动作。
  Commit the action which the player decided.
  //动作对象为 g_Battle.UI.wCurPlayerIndex

  参数：

  [IN]  fRepeat - TRUE if repeat the last action.

  返回值：

  None.

  --*/
{
	WORD      w;
	auto const index = g_Battle.UI.wCurPlayerIndex;
	if (!fRepeat)
	{
		g_Battle.rgPlayer[index].action.ActionType =
			g_Battle.UI.wActionType;
		g_Battle.rgPlayer[index].action.sTarget =
			(SHORT)g_Battle.UI.wSelectedIndex;
		g_Battle.rgPlayer[index].action.wActionID =
			g_Battle.UI.wObjectID;
	}
	else if (g_Battle.rgPlayer[index].action.ActionType == BATTLEACTIONTYPE::kBattleActionPass)
	{
		//缺省动作为物攻，一但有攻击能力时执行
		g_Battle.rgPlayer[index].action.ActionType = BATTLEACTIONTYPE::kBattleActionAttack;
		g_Battle.rgPlayer[index].action.sTarget = -1;
	}

	//
	// Check if the action is valid
	//
	switch (g_Battle.rgPlayer[index].action.ActionType)
	{
	case BATTLEACTIONTYPE::kBattleActionMagic:
		w = g_Battle.rgPlayer[index].action.wActionID;
		w = gpGlobals->g.lprgMagic[gpGlobals->g.rgObject[w].magic.wMagicNumber].wCostMP;

		if (gpGlobals->g.PlayerRoles.rgwMP[gpGlobals->rgParty[index].wPlayerRole] < w)
		{
			w = g_Battle.rgPlayer[index].action.wActionID;
			w = gpGlobals->g.lprgMagic[gpGlobals->g.rgObject[w].magic.wMagicNumber].wType;
			if (w == kMagicTypeApplyToPlayer || w == kMagicTypeApplyToParty ||
				w == kMagicTypeTrance)
			{
				g_Battle.rgPlayer[index].action.ActionType = BATTLEACTIONTYPE::kBattleActionDefend;
			}
			else
			{
				g_Battle.rgPlayer[index].action.ActionType = BATTLEACTIONTYPE::kBattleActionAttack;
				if (g_Battle.rgPlayer[index].action.sTarget == -1)
				{
					g_Battle.rgPlayer[index].action.sTarget = 0;
				}
			}
		}
		break;

	case BATTLEACTIONTYPE::kBattleActionUseItem:
		if ((gpGlobals->g.rgObject[g_Battle.rgPlayer[index].action.wActionID].item.wFlags & kItemFlagConsuming) == 0)
		{
			break;
		}

	case BATTLEACTIONTYPE::kBattleActionThrowItem:
		for (w = 0; w < MAX_INVENTORY; w++)
		{
			if (gpGlobals->rgInventory[w].wItem == g_Battle.rgPlayer[index].action.wActionID)
			{
				gpGlobals->rgInventory[w].nAmountInUse++;
				break;
			}
		}
		break;

	default:
		break;
	}

	if (g_Battle.UI.wActionType == BATTLEACTIONTYPE::kBattleActionFlee)
	{
		g_Battle.fFlee = TRUE;
	}

	///???
	g_Battle.rgPlayer[index].state = FIGHTERSTATE::kFighterAct;
	g_Battle.UI.state = BATTLEUISTATE::kBattleUIWait;

}

VOID CFight::PAL_BattleShowPlayerAttackAnim(
	WORD		wPlayerIndex,
	BOOL		fCritical
)
/*++
  功能：  Show the physical attack effect for player.
  参数：  [IN]  wPlayerIndex - the index of the player.
  [IN]		fCritical - TRUE if this is a critical hit.
  返回值：  None.
  --*/
{
	WORD wPlayerRole = gpGlobals->rgParty[wPlayerIndex].wPlayerRole;
	SHORT sTarget = g_Battle.rgPlayer[wPlayerIndex].action.sTarget;

	int index, i, j;
	int enemy_x = 0, enemy_y = 0, enemy_h = 0, x, y, dist = 0;

	DWORD dwTime;

	if (sTarget != -1)
	{
		enemy_x = PAL_X(g_Battle.rgEnemy[sTarget].pos);
		enemy_y = PAL_Y(g_Battle.rgEnemy[sTarget].pos);

		enemy_h = PAL_RLEGetHeight(PAL_SpriteGetFrame(g_Battle.rgEnemy[sTarget].lpSprite, g_Battle.rgEnemy[sTarget].wCurrentFrame));

		if (sTarget >= 3)
		{
			dist = (sTarget - wPlayerIndex) * 8;
		}
	}
	else
	{
		enemy_x = 150;
		enemy_y = 100;
	}

	index = gpGlobals->g.rgwBattleEffectIndex[PAL_GetPlayerBattleSprite(wPlayerRole)][1];
	index *= 3;

	//
	// Play the attack voice
	//
	if (gpGlobals->g.PlayerRoles.rgwHP[wPlayerRole] > 0)
	{
		if (!fCritical)
		{
			SOUND_Play(gpGlobals->g.PlayerRoles.rgwAttackSound[wPlayerRole]);
		}
		else
		{
			SOUND_Play(gpGlobals->g.PlayerRoles.rgwCriticalSound[wPlayerRole]);
		}
	}

	//
	// Show the animation
	//
	x = enemy_x - dist + 64;
	y = enemy_y + dist + 20;

	g_Battle.rgPlayer[wPlayerIndex].wCurrentFrame = 8;
	g_Battle.rgPlayer[wPlayerIndex].pos = PAL_XY(x, y);

	PAL_BattleDelay(2, 0, TRUE);

	x -= 10;
	y -= 2;
	g_Battle.rgPlayer[wPlayerIndex].pos = PAL_XY(x, y);

	PAL_BattleDelay(1, 0, TRUE);

	g_Battle.rgPlayer[wPlayerIndex].wCurrentFrame = 9;
	x -= 16;
	y -= 4;

	SOUND_Play(gpGlobals->g.PlayerRoles.rgwWeaponSound[wPlayerRole]);

	x = enemy_x;
	y = enemy_y - enemy_h / 3 + 10;

	dwTime = SDL_GetTicks_New() + cfgBtlFrameMs;

	for (i = 0; i < 3; i++)
	{
		LPCBITMAPRLE b = PAL_SpriteGetFrame(g_Battle.lpEffectSprite, index++);

		//
		// Wait for the time of one frame. Accept input here.
		//
		PAL_ProcessEvent();
		while (SDL_GetTicks_New() <= dwTime)
		{
			PAL_ProcessEvent();
			SDL_Delay(1);
		}

		//
		// Set the time of the next frame.
		//
		dwTime = SDL_GetTicks_New() + cfgBtlFrameMs;

		//
		// Update the gesture of enemies.
		//
		for (j = 0; j <= g_Battle.wMaxEnemyIndex; j++)
		{
			if (g_Battle.rgEnemy[j].wObjectID == 0 ||
				g_Battle.rgEnemy[j].rgwStatus[kStatusSleep] > 0 ||
				g_Battle.rgEnemy[j].rgwStatus[kStatusParalyzed] > 0)
			{
				continue;
			}

			if (--g_Battle.rgEnemy[j].e.wIdleAnimSpeed == 0)
			{
				g_Battle.rgEnemy[j].wCurrentFrame++;
				g_Battle.rgEnemy[j].e.wIdleAnimSpeed =
					gpGlobals->g.lprgEnemy[gpGlobals->g.rgObject[
						g_Battle.rgEnemy[j].wObjectID].enemy.wEnemyID].wIdleAnimSpeed;
			}

			if (g_Battle.rgEnemy[j].wCurrentFrame >= g_Battle.rgEnemy[j].e.wIdleFrames)
			{
				g_Battle.rgEnemy[j].wCurrentFrame = 0;
			}
		}

		PAL_BattleMakeScene();
		RenderBlendCopy(gpTextureReal, g_Battle.lpSceneBuf);;

		PAL_RLEBlitToSurface(b, gpTextureReal, PAL_XY(x - PAL_RLEGetWidth(b) / 2, y - PAL_RLEGetHeight(b)));
		x -= 16;
		y += 16;

		PAL_BattleUIUpdate();

		if (i == 0)
		{
			if (sTarget == -1)
			{
				for (j = 0; j <= g_Battle.wMaxEnemyIndex; j++)
				{
					g_Battle.rgEnemy[j].iColorShift = 6;
				}
			}
			else
			{
				g_Battle.rgEnemy[sTarget].iColorShift = 6;
			}

			PAL_BattleDisplayStatChange();
			PAL_BattleBackupStat();
		}

		VIDEO_UpdateScreen(gpTextureReal);

		if (i == 1)
		{
			g_Battle.rgPlayer[wPlayerIndex].pos =
				PAL_XY(PAL_X(g_Battle.rgPlayer[wPlayerIndex].pos) + 2,
					PAL_Y(g_Battle.rgPlayer[wPlayerIndex].pos) + 1);
		}
	}

	dist = 8;

	for (i = 0; i <= g_Battle.wMaxEnemyIndex; i++)
	{
		g_Battle.rgEnemy[i].iColorShift = 0;
	}

	if (sTarget == -1)
	{
		for (i = 0; i < 3; i++)
		{
			for (j = 0; j <= g_Battle.wMaxEnemyIndex; j++)
			{
				x = PAL_X(g_Battle.rgEnemy[j].pos);
				y = PAL_Y(g_Battle.rgEnemy[j].pos);

				x -= dist;
				y -= dist / 2;
				g_Battle.rgEnemy[j].pos = PAL_XY(x, y);
			}

			PAL_BattleDelay(1, 0, TRUE);
			dist /= -2;
		}
	}
	else
	{
		x = PAL_X(g_Battle.rgEnemy[sTarget].pos);
		y = PAL_Y(g_Battle.rgEnemy[sTarget].pos);

		for (i = 0; i < 3; i++)
		{
			x -= dist;
			dist /= -2;
			y += dist;
			g_Battle.rgEnemy[sTarget].pos = PAL_XY(x, y);

			PAL_BattleDelay(1, 0, TRUE);
		}
	}
}

VOID CFight::PAL_BattleShowPlayerUseItemAnim(
	WORD         wPlayerIndex,
	WORD         wObjectID,
	SHORT        sTarget
)
/*++
  功能：

  Show the "use item" effect for player.

  参数：

  [IN]  wPlayerIndex - the index of the player.

  [IN]  wObjectID - the object ID of the item to be used.

  [IN]  sTarget - the target player of the action.

  返回值：

  None.

  --*/
{
	int i, j;

	PAL_BattleDelay(4, 0, TRUE);

	g_Battle.rgPlayer[wPlayerIndex].pos =
		PAL_XY(PAL_X(g_Battle.rgPlayer[wPlayerIndex].pos) - 15,
			PAL_Y(g_Battle.rgPlayer[wPlayerIndex].pos) - 7);

	g_Battle.rgPlayer[wPlayerIndex].wCurrentFrame = 5;

	SOUND_Play(28);

	for (i = 0; i <= 6; i++)
	{
		if (sTarget == -1)
		{
			for (j = 0; j <= gpGlobals->wMaxPartyMemberIndex; j++)
			{
				g_Battle.rgPlayer[j].iColorShift = i;
			}
		}
		else
		{
			g_Battle.rgPlayer[sTarget].iColorShift = i;
		}

		PAL_BattleDelay(1, wObjectID, TRUE);
	}

	for (i = 5; i >= 0; i--)
	{
		if (sTarget == -1)
		{
			for (j = 0; j <= gpGlobals->wMaxPartyMemberIndex; j++)
			{
				g_Battle.rgPlayer[j].iColorShift = i;
			}
		}
		else
		{
			g_Battle.rgPlayer[sTarget].iColorShift = i;
		}

		PAL_BattleDelay(1, wObjectID, TRUE);
	}
}


void CFight::PAL_BattlePlayerSelected()
{

	//
	// actions for all players are decided. fill in the action queue.
	//
	int  wMoveTimes{};
	g_Battle.fRepeat = FALSE;
	g_Battle.fForce = FALSE;
	g_Battle.fFlee = FALSE;


	//
	// Put all enemies into action queue
	//
	for (int i = 0; i <= g_Battle.wMaxEnemyIndex; i++)
	{
		if (g_Battle.rgEnemy[i].wObjectID == 0)
		{
			continue;
		}
		// Instant combat, if you are in charge or action, skip
		if (g_Battle.rgEnemy[i].state != FIGHTERSTATE::kFighterWait)
		{
			continue;
		}
		wMoveTimes = g_Battle.rgEnemy[i].e.wDualMove + 2;
		int iHealthyPlayerNum = PAL_New_GetHealthyPlayerNum();

		if (gpGlobals->wMaxPartyMemberIndex > 2)
		{
			wMoveTimes += std::min(2, std::max((iHealthyPlayerNum - 1) / 2, 0));
		}

#ifdef FINISH_GAME_MORE_ONE_TIME
		if (gpGlobals->bFinishGameTime > 0)
		{
			wMoveTimes += std::min(2, std::max(iHealthyPlayerNum - 1, 0));
			wMoveTimes += std::min(1, std::max(gpGlobals->bFinishGameTime - 1, 0));
		}
#endif 										
		if (wMoveTimes % 2 == 1 && (PAL_New_GetAliveEnemyNum() < 2) || RandomLong(0, 9) < 5)
		{
			wMoveTimes++;
		}
		wMoveTimes /= 2;

		WORD wExtraMoveTimes = 0;

#ifdef FINISH_GAME_MORE_ONE_TIME
		wExtraMoveTimes = std::min(1, std::max(gpGlobals->bFinishGameTime - 1, 0));
#endif 						
		wMoveTimes = std::min((int)wMoveTimes, cfgFoeMaxActions + wExtraMoveTimes);

		int fDexterityIncrease = FALSE;
		//
		if ((iHealthyPlayerNum == 3 && (g_Loop & 1)) || iHealthyPlayerNum > 3)
		{
			fDexterityIncrease = TRUE;
		}
		g_Battle.rgEnemy[i].mMoveTimes = wMoveTimes;

		g_Battle.rgEnemy[i].flTimeChargingSpeed = PAL_GetTimeChargingSpeed(PAL_GetEnemyActualDexterity(i)*RandomFloat(0.9,1.1));
	}

	//
	// Put all players into action queue
	//
	for (int i = 0; i <= gpGlobals->wMaxPartyMemberIndex; i++)
	{
		auto wPlayerRole = gpGlobals->rgParty[i].wPlayerRole;

		if (gpGlobals->g.PlayerRoles.rgwHP[wPlayerRole] == 0 ||
			gpGlobals->rgPlayerStatus[wPlayerRole][kStatusSleep] > 0 ||
			gpGlobals->rgPlayerStatus[wPlayerRole][kStatusParalyzed] > 0)
		{
			//
			// players who are unable to move should attack physically if recovered
			// in the same turn
			//

			g_Battle.rgPlayer[i].action.ActionType = BATTLEACTIONTYPE::kBattleActionAttack;
			g_Battle.rgPlayer[i].state = FIGHTERSTATE::kFighterAct;
			///???
			auto wDexterity = PAL_GetPlayerActualDexterity(i);
			wDexterity *= RandomFloat(0.9, 1.1);
			g_Battle.rgPlayer[i].flTimeChargingSpeed = PAL_GetTimeChargingSpeed(wDexterity);
		}
		else
		{
			auto wDexterity = PAL_GetPlayerActualDexterity(i);
			wDexterity *= RandomFloat(0.9, 1.1);
			g_Battle.rgPlayer[i].flTimeChargingSpeed = PAL_GetTimeChargingSpeed(wDexterity);
			if (gpGlobals->rgPlayerStatus[wPlayerRole][kStatusConfused] > 0)
			{
				g_Battle.rgPlayer[i].action.ActionType = BATTLEACTIONTYPE::kBattleActionAttack;
				g_Battle.rgPlayer[i].state = FIGHTERSTATE::kFighterAct;
			}
		}
	}

	//
	// Perform the actions
	//
	g_Battle.Phase = BATTLEPHASE::kBattlePhasePerformAction;

}

//检查是否还有玩家可以操作，无返回-1 有返回玩家在队伍中的序号
//Check if there are still players who can operate it, no return -1
// has the serial number of the returning player in the team
const int CFight::PAL_BattlePlayerCanOperation()const
{
	for (int i = 0; i <= gpGlobals->wMaxPartyMemberIndex; i++)
	{
		auto wPlayerRole = gpGlobals->rgParty[i].wPlayerRole;

		//
		// Don't select action for this player if player is KO'ed,
		// sleeped, confused or paralyzed
		//
		if (gpGlobals->g.PlayerRoles.rgwHP[wPlayerRole] == 0 || !PAL_New_IfPlayerCanMove(wPlayerRole))
		{
			continue;
		}

		//
		// Start the menu for the first player whose action is not
		// yet selected
		//
		if (g_Battle.rgPlayer[i].state == FIGHTERSTATE::kFighterWait)
		{
			return i;
		}
		else if (g_Battle.rgPlayer[i].action.ActionType == BATTLEACTIONTYPE::kBattleActionCoopMagic)
		{
			//
			// Skip other players if someone selected coopmagic
			//
			break;
		}
	}
	return -1;
}

void CFight::PAL_BattlePlayerDoAction(int iPlayer)
{

	//玩家动作
	auto i = iPlayer;
	auto wPlayerRole = gpGlobals->rgParty[i].wPlayerRole;
	g_Battle.rgPlayer[i].fDefending = FALSE;
	if (gpGlobals->g.PlayerRoles.rgwHP[wPlayerRole] == 0)
	{
		if (gpGlobals->rgPlayerStatus[wPlayerRole][kStatusPuppet] == 0)
		{
			g_Battle.rgPlayer[i].action.ActionType = BATTLEACTIONTYPE::kBattleActionPass;
		}
	}
	else if (gpGlobals->rgPlayerStatus[wPlayerRole][kStatusSleep] > 0 ||
		gpGlobals->rgPlayerStatus[wPlayerRole][kStatusParalyzed] > 0)
	{
		g_Battle.rgPlayer[i].action.ActionType = BATTLEACTIONTYPE::kBattleActionPass;
	}
	else if (gpGlobals->rgPlayerStatus[wPlayerRole][kStatusConfused] > 0)
	{
		g_Battle.rgPlayer[i].action.ActionType = BATTLEACTIONTYPE::kBattleActionAttackMate;
	}

	//
	// Perform the action for this player.
	//
	g_Battle.fPlayerMoving = TRUE;
	g_Battle.wMovingPlayerIndex = i;
	PAL_BattlePlayerPerformAction(i);
	g_Battle.fPlayerMoving = FALSE;

}

void CFight::PAL_BattleEnemyDoAction(int i)
{
	//敌方动作
	g_Battle.rgEnemy[i].wScriptOnReady =
		PAL_RunTriggerScript(g_Battle.rgEnemy[i].wScriptOnReady, i);

	g_Battle.fEnemyMoving = TRUE;
	g_Battle.wMovingEnemyIndex = i;
	PAL_BattleEnemyPerformAction(i);
	g_Battle.fEnemyMoving = FALSE;
}

//每轮战斗循环之前的检测
//返回0 正常，返回1 战斗结束
const int CFight::PAL_BattleStartFramePrepare()
{
	fOnlyPuppet = TRUE;
	if (!g_Battle.fEnemyCleared)
	{
		PAL_BattleUpdateFighters();
	}

	// Update the scene
	PAL_BattleMakeScene();
	RenderBlendCopy(gpTextureReal, g_Battle.lpSceneBuf);;
	//
	// Check if the battle is over
	//
	if (g_Battle.fEnemyCleared)
	{
		//
		// All enemies are cleared. Won the battle.
		//
		g_Battle.BattleResult = kBattleResultWon;
		SOUND_Play(-1);
		return 1;
	}
	else
	{
		BOOL fEnded = TRUE;
		//检查我方存活情况
		for (int i = 0; i <= gpGlobals->wMaxPartyMemberIndex; i++)
		{
			auto wPlayerRole = gpGlobals->rgParty[i].wPlayerRole;
			if (gpGlobals->g.PlayerRoles.rgwHP[wPlayerRole] != 0)
			{
				fOnlyPuppet = FALSE;
				fEnded = FALSE;
				break;
			}
			else if (gpGlobals->rgPlayerStatus[wPlayerRole][kStatusPuppet] != 0)
			{
				fEnded = FALSE;
			}
		}

		if (fEnded)
		{
			// All players are dead. Lost the battle.
			g_Battle.BattleResult = kBattleResultLost;
			return 1;
		}
	}

	if (cfgDreamSnakeUp)//使用梦蛇后，各项属性增加
		for (int i = 0; i <= gpGlobals->wMaxPartyMemberIndex; i++)
		{
			auto wPlayerRole = gpGlobals->rgParty[i].wPlayerRole;
			if (gpGlobals->rgEquipmentEffect[kBodyPartExtra].rgwSpriteNumInBattle[wPlayerRole] == 5)
				g_Battle.g_ifUseMenshe[wPlayerRole] = 1;
		}

	return 0;
}

//动作队列成员的一帧，调用一次 ???
int CFight::PAL_BattleStartFrameOne(int actionAueue_Item)
{
	return 0;
}

VOID CFight::PAL_BattleUIShowNum(WORD wNum, PAL_POS pos, NUMCOLOR color)
/*++
Purpose:    Show a number on battle screen (indicates HP/MP change).
在战斗屏幕上显示数字（表示生命值和魔法值的变化）。
Parameters:    [IN]  wNum - number to be shown.
要显示的数字。
[IN]  pos - position of the number on the screen.
数字在屏幕上的位置。
[IN]  color - color of the number.
数字的颜色。
Return value:    None.
--*/
{
	int     i;

	for (i = 0; i < BATTLEUI_MAX_SHOWNUM; i++)
	{
		if (g_Battle.UI.rgShowNum[i].wNum == 0)
		{
			g_Battle.UI.rgShowNum[i].wNum = wNum;
			g_Battle.UI.rgShowNum[i].pos = PAL_XY(PAL_X(pos) - 15, PAL_Y(pos));
			g_Battle.UI.rgShowNum[i].color = color;
			g_Battle.UI.rgShowNum[i].dwTime = SDL_GetTicks_New();

			break;
		}
	}
}

VOID CFight::PAL_BattleShowPlayerPreMagicAnim(
	WORD         wPlayerIndex,
	BOOL         fSummon
)
/*++
  功能：    Show the effect for player before using a magic.
  参数：    [IN]  wPlayerIndex - the index of the player.
  [IN]  fSummon - TRUE if player is using a summon magic.
  返回值：    None.
  --*/
{
	int   i, j;
	DWORD dwTime = SDL_GetTicks_New();
	WORD  wPlayerRole = gpGlobals->rgParty[wPlayerIndex].wPlayerRole;

	for (i = 0; i < 4; i++)
	{
		g_Battle.rgPlayer[wPlayerIndex].pos =
			PAL_XY(PAL_X(g_Battle.rgPlayer[wPlayerIndex].pos) - (4 - i),
				PAL_Y(g_Battle.rgPlayer[wPlayerIndex].pos) - (4 - i) / 2);

		PAL_BattleDelay(1, 0, TRUE);
	}

	PAL_BattleDelay(2, 0, TRUE);

	g_Battle.rgPlayer[wPlayerIndex].wCurrentFrame = 5;
	if (!cfgIsWin95)
	{
		SOUND_Play(gpGlobals->g.PlayerRoles.rgwMagicSound[wPlayerRole]);
	}

	if (!fSummon)
	{
		int x, y, index;

		x = PAL_X(g_Battle.rgPlayer[wPlayerIndex].pos);
		y = PAL_Y(g_Battle.rgPlayer[wPlayerIndex].pos);

		index = gpGlobals->g.rgwBattleEffectIndex[PAL_GetPlayerBattleSprite(wPlayerRole)][0];
		index *= 10;
		index += 15;
		//#ifdef PAL_WIN95
		if (cfgIsWin95)
			SOUND_Play(gpGlobals->g.PlayerRoles.rgwMagicSound[wPlayerRole]);
		//#endif
		for (i = 0; i < 10; i++)
		{
			LPCBITMAPRLE b = PAL_SpriteGetFrame(g_Battle.lpEffectSprite, index++);

			//
			// Wait for the time of one frame. Accept input here.
			//
			PAL_ProcessEvent();
			while (SDL_GetTicks_New() <= dwTime)
			{
				PAL_ProcessEvent();
				SDL_Delay(1);
			}

			//
			// Set the time of the next frame.
			//
			dwTime = SDL_GetTicks_New() + cfgBtlFrameMs;

			//
			// Update the gesture of enemies.
			//
			for (j = 0; j <= g_Battle.wMaxEnemyIndex; j++)
			{
				if (g_Battle.rgEnemy[j].wObjectID == 0 ||
					g_Battle.rgEnemy[j].rgwStatus[kStatusSleep] != 0 ||
					g_Battle.rgEnemy[j].rgwStatus[kStatusParalyzed] != 0)
				{
					continue;
				}

				if (--g_Battle.rgEnemy[j].e.wIdleAnimSpeed == 0)
				{
					g_Battle.rgEnemy[j].wCurrentFrame++;
					g_Battle.rgEnemy[j].e.wIdleAnimSpeed =
						gpGlobals->g.lprgEnemy[gpGlobals->g.rgObject[g_Battle.rgEnemy[j].wObjectID].enemy.wEnemyID].wIdleAnimSpeed;
				}

				if (g_Battle.rgEnemy[j].wCurrentFrame >= g_Battle.rgEnemy[j].e.wIdleFrames)
				{
					g_Battle.rgEnemy[j].wCurrentFrame = 0;
				}
			}

			PAL_BattleMakeScene();
			RenderBlendCopy(gpTextureReal, g_Battle.lpSceneBuf);;

			PAL_RLEBlitToSurface(b, gpTextureReal, PAL_XY(x - PAL_RLEGetWidth(b) / 2, y - PAL_RLEGetHeight(b)));

			PAL_BattleUIUpdate();

			VIDEO_UpdateScreen(gpTextureReal);
		}
	}

	PAL_BattleDelay(1, 0, TRUE);
}

VOID CFight::PAL_BattleShowPlayerDefMagicAnim(
	WORD         wPlayerIndex,
	WORD         wObjectID,
	SHORT        sTarget
)
/*++
  功能：    Show the defensive magic effect for player.

  参数：    [IN]  wPlayerIndex - the index of the player.
  [IN]  wObjectID - the object ID of the magic to be used.
  [IN]  sTarget - the target player of the action.
  返回值：    None.
  --*/
{
	//LPSPRITE   lpSpriteEffect;
	int        l, iMagicNum, iEffectNum, n, i, j, x, y;
	DWORD      dwTime = SDL_GetTicks_New();

	iMagicNum = gpGlobals->g.rgObject[wObjectID].magic.wMagicNumber;
	iEffectNum = gpGlobals->g.lprgMagic[iMagicNum].wEffect;

	l = PAL_MKFGetDecompressedSize(iEffectNum, gpGlobals->f.fpFIRE);
	if (l <= 0)
	{
		return;
	}

	auto lpSpriteEffect = PAL_MKFDecompressChunk(iEffectNum, gpGlobals->f.fpFIRE);

	n = PAL_SpriteGetNumFrames(lpSpriteEffect);

	g_Battle.rgPlayer[wPlayerIndex].wCurrentFrame = 6;//???

	PAL_BattleDelay(1, 0, TRUE);

	for (i = 0; i < n; i++)
	{
		LPCBITMAPRLE b = PAL_SpriteGetFrame(lpSpriteEffect, i);

		if (i == (cfgIsWin95 ? 0 : gpGlobals->g.lprgMagic[iMagicNum].wSoundDelay))//
		{
			SOUND_Play(gpGlobals->g.lprgMagic[iMagicNum].wSound);
		}

		//
		// Wait for the time of one frame. Accept input here.
		//
		PAL_ProcessEvent();
		while (SDL_GetTicks_New() <= dwTime)
		{
			PAL_ProcessEvent();
			SDL_Delay(1);
		}

		//
		// Set the time of the next frame.
		//
		dwTime = SDL_GetTicks_New() +
			((SHORT)(gpGlobals->g.lprgMagic[iMagicNum].wSpeed) + 5) * 10;

		PAL_BattleMakeScene();
		RenderBlendCopy(gpTextureReal, g_Battle.lpSceneBuf);;

		if (gpGlobals->g.lprgMagic[iMagicNum].wType == kMagicTypeApplyToParty)
		{
			assert(sTarget == -1);

			for (l = 0; l <= gpGlobals->wMaxPartyMemberIndex; l++)
			{
				x = PAL_X(g_Battle.rgPlayer[l].pos);
				y = PAL_Y(g_Battle.rgPlayer[l].pos);

				x += (SHORT)gpGlobals->g.lprgMagic[iMagicNum].wXOffset;
				y += (SHORT)gpGlobals->g.lprgMagic[iMagicNum].wYOffset;

				PAL_RLEBlitToSurface(b, gpTextureReal,
					PAL_XY(x - PAL_RLEGetWidth(b) / 2, y - PAL_RLEGetHeight(b)));
			}
		}
		else if (gpGlobals->g.lprgMagic[iMagicNum].wType == kMagicTypeApplyToPlayer)
		{
			assert(sTarget != -1);

			x = PAL_X(g_Battle.rgPlayer[sTarget].pos);
			y = PAL_Y(g_Battle.rgPlayer[sTarget].pos);

			x += (SHORT)gpGlobals->g.lprgMagic[iMagicNum].wXOffset;
			y += (SHORT)gpGlobals->g.lprgMagic[iMagicNum].wYOffset;

			PAL_RLEBlitToSurface(b, gpTextureReal,
				PAL_XY(x - PAL_RLEGetWidth(b) / 2, y - PAL_RLEGetHeight(b)));

			//
			// Repaint the previous player
			//
			//if (sTarget > 0 && g_Battle.iHidingTime  == 0)
			if (sTarget > 0 && PAL_GetHidingTime() == 0)
			{
				if (gpGlobals->rgPlayerStatus[gpGlobals->rgParty[sTarget - 1].wPlayerRole][kStatusConfused] == 0)
				{
					LPCBITMAPRLE p = PAL_SpriteGetFrame(g_Battle.rgPlayer[sTarget - 1].lpSprite, g_Battle.rgPlayer[sTarget - 1].wCurrentFrame);

					x = PAL_X(g_Battle.rgPlayer[sTarget - 1].pos);
					y = PAL_Y(g_Battle.rgPlayer[sTarget - 1].pos);

					x -= PAL_RLEGetWidth(p) / 2;
					y -= PAL_RLEGetHeight(p);

					PAL_RLEBlitToSurface(p, gpTextureReal, PAL_XY(x, y));
				}
			}
		}
		else
		{
			assert(FALSE);
		}

		PAL_BattleUIUpdate();

		VIDEO_UpdateScreen(gpTextureReal);
	}

	//free(lpSpriteEffect);

	for (i = 0; i < 6; i++)
	{
		if (gpGlobals->g.lprgMagic[iMagicNum].wType == kMagicTypeApplyToParty)
		{
			for (j = 0; j <= gpGlobals->wMaxPartyMemberIndex; j++)
			{
				g_Battle.rgPlayer[j].iColorShift = i;
			}
		}
		else
		{
			g_Battle.rgPlayer[sTarget].iColorShift = i;
		}

		PAL_BattleDelay(1, 0, TRUE);
	}

	for (i = 6; i >= 0; i--)
	{
		if (gpGlobals->g.lprgMagic[iMagicNum].wType == kMagicTypeApplyToParty)
		{
			for (j = 0; j <= gpGlobals->wMaxPartyMemberIndex; j++)
			{
				g_Battle.rgPlayer[j].iColorShift = i;
			}
		}
		else
		{
			g_Battle.rgPlayer[sTarget].iColorShift = i;
		}

		PAL_BattleDelay(1, 0, TRUE);
	}
}

VOID CFight::PAL_BattleShowPlayerOffMagicAnim(
	WORD         wPlayerIndex,
	WORD         wObjectID,
	SHORT        sTarget,
	BOOL         fSummon
)
/*++
  功能：    Show the offensive magic animation for player.
  参数：    [IN]  wPlayerIndex - the index of the player.
  [IN]  wObjectID - the object ID of the magic to be used.
  [IN]  sTarget - the target enemy of the action.
  返回值：    None.
  --*/
{
	//LPSPRITE   lpSpriteEffect;
	int        l, iMagicNum, iEffectNum, n, i, k, x, y, wave, blow;
	DWORD      dwTime = SDL_GetTicks_New();

#ifdef ROTATE_SOME_MAGIC_ANIM
	BOOL		bRotateMagicAnim = FALSE;
	CONST WORD	wRotateMagicObjectID[20] =
	{//0x015e, //灭绝一击
		//0x0167, //御剑伏魔
		//0x0175, //大咒蛇
		0x0185,	//血魔神功
		0x0188, //火龙掌
		0x018a, //魔掌天下
		0x018d, //群魔乱舞
	};
#endif

	iMagicNum = gpGlobals->g.rgObject[wObjectID].magic.wMagicNumber;
	iEffectNum = gpGlobals->g.lprgMagic[iMagicNum].wEffect;

#ifdef ROTATE_SOME_MAGIC_ANIM
	for (i = 0; i < 20; i++)
	{
		if (wObjectID == 0)
		{
			break;
		}
		else if (wObjectID == wRotateMagicObjectID[i])
		{
			bRotateMagicAnim = TRUE;
			break;
		}
	}
#endif

	l = PAL_MKFGetDecompressedSize(iEffectNum, gpGlobals->f.fpFIRE);
	if (l <= 0)
	{
		return;
	}

	//lpSpriteEffect = (LPSPRITE)malloc(l);

	ByteArray lpSpriteEffect = PAL_MKFDecompressChunk(iEffectNum, gpGlobals->f.fpFIRE);

	n = PAL_SpriteGetNumFrames(lpSpriteEffect);

	PAL_BattleDelay(1, 0, TRUE);

	l = n - gpGlobals->g.lprgMagic[iMagicNum].wSoundDelay;
	l *= (SHORT)gpGlobals->g.lprgMagic[iMagicNum].wEffectTimes;
	l += n;
	l += gpGlobals->g.lprgMagic[iMagicNum].wShake;

	wave = gpGlobals->wScreenWave;
	gpGlobals->wScreenWave += gpGlobals->g.lprgMagic[iMagicNum].wWave;

	if (cfgIsWin95 && !fSummon && gpGlobals->g.lprgMagic[iMagicNum].wSound != 0)
	{
		//AUDIO_PlaySound(gpGlobals->g.lprgMagic[iMagicNum].wSound);
		SOUND_Play(gpGlobals->g.lprgMagic[iMagicNum].wSound);
	}


	for (i = 0; i < l; i++)
	{
		LPCBITMAPRLE b;
		if (i == gpGlobals->g.lprgMagic[iMagicNum].wSoundDelay && wPlayerIndex != (WORD)-1)
		{
			g_Battle.rgPlayer[wPlayerIndex].wCurrentFrame = 6;
		}
		blow = ((g_Battle.iBlow > 0) ? RandomLong(0, g_Battle.iBlow) : RandomLong(g_Battle.iBlow, 0));

		for (k = 0; k <= g_Battle.wMaxEnemyIndex; k++)
		{
			if (g_Battle.rgEnemy[k].wObjectID == 0)
			{
				continue;
			}

			x = PAL_X(g_Battle.rgEnemy[k].pos) + blow;
			y = PAL_Y(g_Battle.rgEnemy[k].pos) + blow / 2;

			g_Battle.rgEnemy[k].pos = PAL_XY(x, y);
		}

		if (l - i > gpGlobals->g.lprgMagic[iMagicNum].wShake)
		{
			if (i < n)
			{
				k = i;
			}
			else
			{
				k = i - gpGlobals->g.lprgMagic[iMagicNum].wSoundDelay;
				k %= n - gpGlobals->g.lprgMagic[iMagicNum].wSoundDelay;
				k += gpGlobals->g.lprgMagic[iMagicNum].wSoundDelay;
			}

			b = PAL_SpriteGetFrame(lpSpriteEffect, k);

			if (!cfgIsWin95 && (i - gpGlobals->g.lprgMagic[iMagicNum].wSoundDelay) % n == 0)
			{
				SOUND_Play(gpGlobals->g.lprgMagic[iMagicNum].wSound);
			}
		}
		else
		{
			VIDEO_ShakeScreen(i, 3);
			b = PAL_SpriteGetFrame(lpSpriteEffect, (l - gpGlobals->g.lprgMagic[iMagicNum].wShake - 1) % n);
		}

		//
		// Wait for the time of one frame. Accept input here.
		//
		PAL_ProcessEvent();
		while (SDL_GetTicks_New() <= dwTime)
		{
			PAL_ProcessEvent();
			SDL_Delay(1);
		}

		//
		// Set the time of the next frame.
		//
		dwTime = SDL_GetTicks_New() +
			((SHORT)(gpGlobals->g.lprgMagic[iMagicNum].wSpeed) + 5) * 10;

		PAL_BattleMakeScene();

		RenderBlendCopy(gpTextureReal, g_Battle.lpSceneBuf);;

		if (gpGlobals->g.lprgMagic[iMagicNum].wType == kMagicTypeNormal)
		{//普通仙术
			assert(sTarget != -1);

			x = PAL_X(g_Battle.rgEnemy[sTarget].pos);
			y = PAL_Y(g_Battle.rgEnemy[sTarget].pos);

			x += (SHORT)gpGlobals->g.lprgMagic[iMagicNum].wXOffset;
			y += (SHORT)gpGlobals->g.lprgMagic[iMagicNum].wYOffset;

			PAL_RLEBlitToSurface(b, gpTextureReal, PAL_XY(x - PAL_RLEGetWidth(b) / 2, y - PAL_RLEGetHeight(b)));

			if (i == l - 1 && gpGlobals->wScreenWave < 9 &&
				gpGlobals->g.lprgMagic[iMagicNum].wKeepEffect == 0xFFFF)
			{

				PAL_RLEBlitToSurface(b, g_Battle.lpBackground,
					PAL_XY(x - PAL_RLEGetWidth(b) / 2, y - PAL_RLEGetHeight(b)));

			}
		}
		else if (gpGlobals->g.lprgMagic[iMagicNum].wType == kMagicTypeAttackAll)
		{
			const int effectpos[3][2] = { {70, 140}, {100, 110}, {160, 100} };

			assert(sTarget == -1);

			for (k = 0; k < 3; k++)
			{
				x = effectpos[k][0];
				y = effectpos[k][1];

				x += (SHORT)gpGlobals->g.lprgMagic[iMagicNum].wXOffset;
				y += (SHORT)gpGlobals->g.lprgMagic[iMagicNum].wYOffset;

				PAL_RLEBlitToSurface(b, gpTextureReal, PAL_XY(x - PAL_RLEGetWidth(b) / 2, y - PAL_RLEGetHeight(b)));

				if (i == l - 1 && gpGlobals->wScreenWave < 9 &&
					gpGlobals->g.lprgMagic[iMagicNum].wKeepEffect == 0xFFFF)
				{

					PAL_RLEBlitToSurface(b, g_Battle.lpBackground,
						PAL_XY(x - PAL_RLEGetWidth(b) / 2, y - PAL_RLEGetHeight(b)));
				}
			}
		}
		else if (gpGlobals->g.lprgMagic[iMagicNum].wType == kMagicTypeAttackWhole ||
			gpGlobals->g.lprgMagic[iMagicNum].wType == kMagicTypeAttackField)
		{
			assert(sTarget == -1);

			if (gpGlobals->g.lprgMagic[iMagicNum].wType == kMagicTypeAttackWhole)
			{
				x = 120;
				y = 100;
			}
			else
			{
				x = 160;
				y = 200;
			}

			x += (SHORT)gpGlobals->g.lprgMagic[iMagicNum].wXOffset;
			y += (SHORT)gpGlobals->g.lprgMagic[iMagicNum].wYOffset;

			PAL_RLEBlitToSurface(b, gpTextureReal, PAL_XY(x - PAL_RLEGetWidth(b) / 2, y - PAL_RLEGetHeight(b)));

			if (i == l - 1 && gpGlobals->wScreenWave < 9 &&
				gpGlobals->g.lprgMagic[iMagicNum].wKeepEffect == 0xFFFF)
			{

				PAL_RLEBlitToSurface(b, g_Battle.lpBackground,
					PAL_XY(x - PAL_RLEGetWidth(b) / 2, y - PAL_RLEGetHeight(b)));

			}
		}
		else
		{
			assert(FALSE);
		}

		PAL_BattleUIUpdate();

		VIDEO_UpdateScreen(gpTextureReal);
	}

	gpGlobals->wScreenWave = wave;
	VIDEO_ShakeScreen(0, 0);

	//free(lpSpriteEffect);

	for (i = 0; i <= g_Battle.wMaxEnemyIndex; i++)
	{
		g_Battle.rgEnemy[i].pos = g_Battle.rgEnemy[i].posOriginal;
	}
}


VOID CFight::PAL_BattleShowEnemyMagicAnim(WORD wObjectID, SHORT sTarget)
/*++
  功能：    Show the offensive magic animation for enemy.
  参数：    [IN]  wObjectID - the object ID of the magic to be used.
  [IN]  sTarget - the target player index of the action.
  返回值：    None.
  --*/
{
	//LPSPRITE   lpSpriteEffect;
	int        l, iMagicNum, iEffectNum, n, i, k, x, y, wave, blow;
	DWORD      dwTime = SDL_GetTicks_New();

	iMagicNum = gpGlobals->g.rgObject[wObjectID].magic.wMagicNumber;
	iEffectNum = gpGlobals->g.lprgMagic[iMagicNum].wEffect;

	l = PAL_MKFGetDecompressedSize(iEffectNum, gpGlobals->f.fpFIRE);
	if (l <= 0)
	{
		return;
	}

	ByteArray lpSpriteEffect = PAL_MKFDecompressChunk(iEffectNum, gpGlobals->f.fpFIRE);

	n = PAL_SpriteGetNumFrames(lpSpriteEffect);

	l = n - gpGlobals->g.lprgMagic[iMagicNum].wSoundDelay;
	l *= (SHORT)gpGlobals->g.lprgMagic[iMagicNum].wEffectTimes;
	l += n;
	l += gpGlobals->g.lprgMagic[iMagicNum].wShake;

	wave = gpGlobals->wScreenWave;
	gpGlobals->wScreenWave += gpGlobals->g.lprgMagic[iMagicNum].wWave;

	for (i = 0; i < l; i++)
	{
		LPCBITMAPRLE b;

		blow = ((g_Battle.iBlow > 0) ? RandomLong(0, g_Battle.iBlow) : RandomLong(g_Battle.iBlow, 0));

		for (k = 0; k <= gpGlobals->wMaxPartyMemberIndex; k++)
		{
			x = PAL_X(g_Battle.rgPlayer[k].pos) + blow;
			y = PAL_Y(g_Battle.rgPlayer[k].pos) + blow / 2;

			g_Battle.rgPlayer[k].pos = PAL_XY(x, y);
		}

		if (l - i > gpGlobals->g.lprgMagic[iMagicNum].wShake)
		{
			if (i < n)
			{
				k = i;
			}
			else
			{
				k = i - gpGlobals->g.lprgMagic[iMagicNum].wSoundDelay;
				k %= n - gpGlobals->g.lprgMagic[iMagicNum].wSoundDelay;
				k += gpGlobals->g.lprgMagic[iMagicNum].wSoundDelay;
			}

			b = PAL_SpriteGetFrame(lpSpriteEffect, k);

			//if (i == gpGlobals->g.lprgMagic[iMagicNum].wSoundDelay)
			if (i == (cfgIsWin95 ? 0 : gpGlobals->g.lprgMagic[iMagicNum].wSoundDelay))
			{
				SOUND_Play(gpGlobals->g.lprgMagic[iMagicNum].wSound);
			}
		}
		else
		{
			VIDEO_ShakeScreen(i, 3);
			b = PAL_SpriteGetFrame(lpSpriteEffect, (l - gpGlobals->g.lprgMagic[iMagicNum].wShake - 1) % n);
		}

		//
		// Wait for the time of one frame. Accept input here.
		//
		PAL_ProcessEvent();
		while (SDL_GetTicks_New() <= dwTime)
		{
			PAL_ProcessEvent();
			SDL_Delay(1);
		}

		//
		// Set the time of the next frame.
		//
		dwTime = SDL_GetTicks_New() +
			((SHORT)(gpGlobals->g.lprgMagic[iMagicNum].wSpeed) + 5) * 10;

		PAL_BattleMakeScene();
		RenderBlendCopy(gpTextureReal, g_Battle.lpSceneBuf);;

		if (gpGlobals->g.lprgMagic[iMagicNum].wType == kMagicTypeNormal)
		{
			assert(sTarget != -1);

			x = PAL_X(g_Battle.rgPlayer[sTarget].pos);
			y = PAL_Y(g_Battle.rgPlayer[sTarget].pos);

			x += (SHORT)gpGlobals->g.lprgMagic[iMagicNum].wXOffset;
			y += (SHORT)gpGlobals->g.lprgMagic[iMagicNum].wYOffset;

			PAL_RLEBlitToSurface(b, gpTextureReal, PAL_XY(x - PAL_RLEGetWidth(b) / 2, y - PAL_RLEGetHeight(b)));

			if (i == l - 1 && gpGlobals->wScreenWave < 9 &&
				gpGlobals->g.lprgMagic[iMagicNum].wKeepEffect == 0xFFFF)
			{
				PAL_RLEBlitToSurface(b, g_Battle.lpBackground,
					PAL_XY(x - PAL_RLEGetWidth(b) / 2, y - PAL_RLEGetHeight(b)));
			}
		}
		else if (gpGlobals->g.lprgMagic[iMagicNum].wType == kMagicTypeAttackAll)
		{
			const int effectpos[3][2] = { {180, 180}, {234, 170}, {270, 146} };

			assert(sTarget == -1);

			for (k = 0; k < 3; k++)
			{
				x = effectpos[k][0];
				y = effectpos[k][1];

				x += (SHORT)gpGlobals->g.lprgMagic[iMagicNum].wXOffset;
				y += (SHORT)gpGlobals->g.lprgMagic[iMagicNum].wYOffset;

				PAL_RLEBlitToSurface(b, gpTextureReal, PAL_XY(x - PAL_RLEGetWidth(b) / 2, y - PAL_RLEGetHeight(b)));

				if (i == l - 1 && gpGlobals->wScreenWave < 9 &&
					gpGlobals->g.lprgMagic[iMagicNum].wKeepEffect == 0xFFFF)
				{
					PAL_RLEBlitToSurface(b, g_Battle.lpBackground,
						PAL_XY(x - PAL_RLEGetWidth(b) / 2, y - PAL_RLEGetHeight(b)));
				}
			}
		}
		else if (gpGlobals->g.lprgMagic[iMagicNum].wType == kMagicTypeAttackWhole ||
			gpGlobals->g.lprgMagic[iMagicNum].wType == kMagicTypeAttackField)
		{
			assert(sTarget == -1);

			if (gpGlobals->g.lprgMagic[iMagicNum].wType == kMagicTypeAttackWhole)
			{
				x = 240;
				y = 150;
			}
			else
			{
				x = 160;
				y = 200;
			}

			x += (SHORT)gpGlobals->g.lprgMagic[iMagicNum].wXOffset;
			y += (SHORT)gpGlobals->g.lprgMagic[iMagicNum].wYOffset;

			PAL_RLEBlitToSurface(b, gpTextureReal, PAL_XY(x - PAL_RLEGetWidth(b) / 2, y - PAL_RLEGetHeight(b)));

			if (i == l - 1 && gpGlobals->wScreenWave < 9 &&
				gpGlobals->g.lprgMagic[iMagicNum].wKeepEffect == 0xFFFF)
			{
				PAL_RLEBlitToSurface(b, g_Battle.lpBackground,
					PAL_XY(x - PAL_RLEGetWidth(b) / 2, y - PAL_RLEGetHeight(b)));
			}
		}
		else
		{
			assert(FALSE);
		}

		PAL_BattleUIUpdate();

		VIDEO_UpdateScreen(gpTextureReal);
	}

	gpGlobals->wScreenWave = wave;
	VIDEO_ShakeScreen(0, 0);

	//free(lpSpriteEffect);

	for (i = 0; i <= gpGlobals->wMaxPartyMemberIndex; i++)
	{
		g_Battle.rgPlayer[i].pos = g_Battle.rgPlayer[i].posOriginal;
	}
}

VOID CFight::PAL_BattleShowPlayerSummonMagicAnim(WORD wPlayerIndex, WORD wObjectID)
/*++
  功能：    Show the summon magic animation for player.
  参数：    [IN]  wPlayerIndex - the index of the player.
  [IN]  wObjectID - the object ID of the magic to be used.
  返回值：    None.
  --*/
{
	int           i, j;
	WORD          wMagicNum = gpGlobals->g.rgObject[wObjectID].magic.wMagicNumber;
	WORD          wEffectMagicID = 0;
	DWORD         dwTime = SDL_GetTicks_New();

	for (wEffectMagicID = 0; wEffectMagicID < MAX_OBJECTS; wEffectMagicID++)
	{
		if (gpGlobals->g.rgObject[wEffectMagicID].magic.wMagicNumber ==
			gpGlobals->g.lprgMagic[wMagicNum].wEffect)
		{
			break;
		}
	}

	assert(wEffectMagicID < MAX_OBJECTS);

	//
	// Brighten the players
	//
	for (i = 1; i <= 10; i++)
	{
		for (j = 0; j <= gpGlobals->wMaxPartyMemberIndex; j++)
		{
			g_Battle.rgPlayer[j].iColorShift = i;
		}

		PAL_BattleDelay(1, wObjectID, TRUE);
	}

	PAL_BattleBackupScene();

	//#ifdef PAL_WIN95
	if (cfgIsWin95)
		SOUND_Play(gpGlobals->g.lprgMagic[wMagicNum].wSound);
	//#endif

		//
		// Load the sprite of the summoned god
		//
	j = gpGlobals->g.lprgMagic[wMagicNum].wSummonEffect + 10;
	//i = PAL_MKFGetDecompressedSize(j, gpGlobals->f.fpF);

	g_Battle.lpSummonSprite = PAL_MKFDecompressChunk(j, gpGlobals->f.fpF);

	g_Battle.iSummonFrame = 0;
	g_Battle.posSummon = PAL_XY(230 + (SHORT)(gpGlobals->g.lprgMagic[wMagicNum].wXOffset),
		155 + (SHORT)(gpGlobals->g.lprgMagic[wMagicNum].wYOffset));
	g_Battle.sBackgroundColorShift = (SHORT)(gpGlobals->g.lprgMagic[wMagicNum].wEffectTimes);

	//
	// Fade in the summoned god
	//
	PAL_BattleMakeScene();
	RenderBlendCopy(gpTextureReal, g_Battle.lpSceneBuf);;
	PAL_BattleFadeScene();

	//
	// Show the animation of the summoned god
	// TODO: There is still something missing here compared to the original game.
	//
	while (g_Battle.iSummonFrame < PAL_SpriteGetNumFrames(g_Battle.lpSummonSprite.data()) - 1)
	{
		//
		// Wait for the time of one frame. Accept input here.
		//
		PAL_ProcessEvent();
		while (SDL_GetTicks_New() <= dwTime)
		{
			PAL_ProcessEvent();
			SDL_Delay(1);
		}

		//
		// Set the time of the next frame.
		//
		dwTime = SDL_GetTicks_New() +
			((SHORT)(gpGlobals->g.lprgMagic[wMagicNum].wSpeed) + 5) * 10;

		PAL_BattleMakeScene();
		RenderBlendCopy(gpTextureReal, g_Battle.lpSceneBuf);;

		PAL_BattleUIUpdate();

		VIDEO_UpdateScreen(gpTextureReal);

		g_Battle.iSummonFrame++;
	}

	//
	// Show the actual magic effect
	//
	PAL_BattleShowPlayerOffMagicAnim((WORD)-1, wEffectMagicID, -1, TRUE);
}

VOID CFight::PAL_BattleShowPostMagicAnim(VOID)
/*++
  功能：    Show the post-magic animation.
  参数：    None
  返回值：    None.

  --*/
{
	int         i, j, x, y, dist = 8;
	PAL_POS     rgEnemyPosBak[MAX_ENEMIES_IN_TEAM] = { 0 };

	for (i = 0; i < MAX_ENEMIES_IN_TEAM; i++)
	{
		rgEnemyPosBak[i] = g_Battle.rgEnemy[i].pos;
	}

	for (i = 0; i < 3; i++)
	{
		for (j = 0; j <= g_Battle.wMaxEnemyIndex; j++)
		{
			if (g_Battle.rgEnemy[j].dwActualHealth == g_Battle.rgEnemy[j].wPrevHP)
			{
				continue;
			}

			x = PAL_X(g_Battle.rgEnemy[j].pos);
			y = PAL_Y(g_Battle.rgEnemy[j].pos);

			x -= dist;
			y -= dist / 2;

			g_Battle.rgEnemy[j].pos = PAL_XY(x, y);

			g_Battle.rgEnemy[j].iColorShift = ((i == 1) ? 6 : 0);
		}

		PAL_BattleDelay(1, 0, TRUE);
		dist /= -2;
	}

	for (i = 0; i < MAX_ENEMIES_IN_TEAM; i++)
	{
		g_Battle.rgEnemy[i].pos = rgEnemyPosBak[i];
	}

	PAL_BattleDelay(1, 0, TRUE);
}

VOID CFight::PAL_BattlePlayerValidateAction(WORD wPlayerIndex)
/*++
  功能：    Validate player's action, fallback to other action when needed.
  参数：    [IN]  wPlayerIndex - the index of the player.
  返回值：    None.
  --*/
{
	const WORD   wPlayerRole = gpGlobals->rgParty[wPlayerIndex].wPlayerRole;
	const WORD   wObjectID = g_Battle.rgPlayer[wPlayerIndex].action.wActionID;
	const SHORT  sTarget = g_Battle.rgPlayer[wPlayerIndex].action.sTarget;
	BOOL         fValid = TRUE, fToEnemy = FALSE;
	WORD         w;
	int          i;

	switch (g_Battle.rgPlayer[wPlayerIndex].action.ActionType)
	{
	case BATTLEACTIONTYPE::kBattleActionAttack:
		fToEnemy = TRUE;
		break;

	case BATTLEACTIONTYPE::kBattleActionPass:
		break;

	case BATTLEACTIONTYPE::kBattleActionDefend:
		break;

	case BATTLEACTIONTYPE::kBattleActionMagic:
	{
		//
		// Make sure player actually has the magic to be used
		//
		for (i = 0; i < MAX_PLAYER_MAGICS; i++)
		{
			if (gpGlobals->g.PlayerRoles.rgwMagic[i][wPlayerRole] == wObjectID)
			{
				break; // player has this magic
			}
		}

		if (i >= MAX_PLAYER_MAGICS)
		{
			fValid = FALSE;
		}

		w = gpGlobals->g.rgObject[wObjectID].magic.wMagicNumber;

		if (gpGlobals->rgPlayerStatus[wPlayerRole][kStatusSilence] > 0)
		{
			//
			// Player is silenced
			//
			fValid = FALSE;
		}

		if (gpGlobals->g.PlayerRoles.rgwMP[wPlayerRole] <
			gpGlobals->g.lprgMagic[w].wCostMP)
		{
			//
			// No enough MP
			//
			fValid = FALSE;
		}

		//
		// Fallback to physical attack if player is using an offensive magic,
		// defend if player is using a defensive or healing magic
		//
		if (gpGlobals->g.rgObject[wObjectID].magic.wFlags & kMagicFlagUsableToEnemy)
		{
			if (!fValid)
			{
				g_Battle.rgPlayer[wPlayerIndex].action.ActionType = BATTLEACTIONTYPE::kBattleActionAttack;
			}
			else if (gpGlobals->g.rgObject[wObjectID].magic.wFlags & kMagicFlagApplyToAll)
			{
				g_Battle.rgPlayer[wPlayerIndex].action.sTarget = -1;
			}
			else if (sTarget == -1)
			{
				g_Battle.rgPlayer[wPlayerIndex].action.sTarget = PAL_BattleSelectAutoTarget();
			}

			fToEnemy = TRUE;
		}
		else
		{
			if (!fValid)
			{
				g_Battle.rgPlayer[wPlayerIndex].action.ActionType = BATTLEACTIONTYPE::kBattleActionDefend;
			}
			else if (gpGlobals->g.rgObject[wObjectID].magic.wFlags & kMagicFlagApplyToAll)
			{
				g_Battle.rgPlayer[wPlayerIndex].action.sTarget = -1;
			}
			else if (g_Battle.rgPlayer[wPlayerIndex].action.sTarget == -1)
			{
				g_Battle.rgPlayer[wPlayerIndex].action.sTarget = wPlayerIndex;
			}
		}
		break;
	}

	case BATTLEACTIONTYPE::kBattleActionCoopMagic:
	{
		fToEnemy = TRUE;

		for (i = 0; i <= gpGlobals->wMaxPartyMemberIndex; i++)
		{
			w = gpGlobals->rgParty[i].wPlayerRole;

			if (PAL_IsPlayerDying(w) || !PAL_New_IfPlayerCanMove(w))
			{
				g_Battle.rgPlayer[wPlayerIndex].action.ActionType = BATTLEACTIONTYPE::kBattleActionAttack;
				break;
			}
		}

		if (g_Battle.rgPlayer[wPlayerIndex].action.ActionType == BATTLEACTIONTYPE::kBattleActionCoopMagic)
		{
			if (gpGlobals->g.rgObject[wObjectID].magic.wFlags & kMagicFlagApplyToAll)
			{
				g_Battle.rgPlayer[wPlayerIndex].action.sTarget = -1;
			}
			else if (sTarget == -1)
			{
				g_Battle.rgPlayer[wPlayerIndex].action.sTarget = PAL_BattleSelectAutoTarget();
			}
		}
		break;
	}

	case BATTLEACTIONTYPE::kBattleActionFlee:
		break;

	case BATTLEACTIONTYPE::kBattleActionThrowItem:
	{
		fToEnemy = TRUE;

		if (PAL_GetItemAmount(wObjectID) == 0)
		{
			g_Battle.rgPlayer[wPlayerIndex].action.ActionType = BATTLEACTIONTYPE::kBattleActionAttack;
		}
		else if (gpGlobals->g.rgObject[wObjectID].item.wFlags & kItemFlagApplyToAll)
		{
			g_Battle.rgPlayer[wPlayerIndex].action.sTarget = -1;
		}
		else if (g_Battle.rgPlayer[wPlayerIndex].action.sTarget == -1)
		{
			g_Battle.rgPlayer[wPlayerIndex].action.sTarget = PAL_BattleSelectAutoTarget();
		}
		break;
	}

	case BATTLEACTIONTYPE::kBattleActionUseItem:
	{
		if (PAL_GetItemAmount(wObjectID) == 0)
		{
			g_Battle.rgPlayer[wPlayerIndex].action.ActionType = BATTLEACTIONTYPE::kBattleActionDefend;
		}
		else if (gpGlobals->g.rgObject[wObjectID].item.wFlags & kItemFlagApplyToAll)
		{
			g_Battle.rgPlayer[wPlayerIndex].action.sTarget = -1;
		}
		else if (g_Battle.rgPlayer[wPlayerIndex].action.sTarget == -1)
		{
			g_Battle.rgPlayer[wPlayerIndex].action.sTarget = wPlayerIndex;
		}
		break;
	}

	case BATTLEACTIONTYPE::kBattleActionAttackMate:
	{
		if (gpGlobals->rgPlayerStatus[wPlayerRole][kStatusConfused] == 0)
		{
			//
			// Attack enemies instead if player is not confused
			//
			fToEnemy = TRUE;
			g_Battle.rgPlayer[wPlayerIndex].action.ActionType = BATTLEACTIONTYPE::kBattleActionAttack;
		}
		else
		{
			for (i = 0; i <= gpGlobals->wMaxPartyMemberIndex; i++)
			{
				if (i != wPlayerIndex &&
					gpGlobals->g.PlayerRoles.rgwHP[gpGlobals->rgParty[i].wPlayerRole] != 0)
				{
					break;
				}
			}

			if (i > gpGlobals->wMaxPartyMemberIndex)
			{
				//
				// Attack enemies if no one else is alive
				//
				fToEnemy = TRUE;
				g_Battle.rgPlayer[wPlayerIndex].action.ActionType = BATTLEACTIONTYPE::kBattleActionAttack;
			}
		}
		break;
	}
	}

	//
	// Check if player can attack all enemies at once, or attack one enemy
	//
	if (g_Battle.rgPlayer[wPlayerIndex].action.ActionType == BATTLEACTIONTYPE::kBattleActionAttack)
	{
		if (sTarget == -1)
		{
			if (!PAL_PlayerCanAttackAll(wPlayerRole))
			{
				g_Battle.rgPlayer[wPlayerIndex].action.sTarget = PAL_BattleSelectAutoTarget();
			}
		}
		else if (PAL_PlayerCanAttackAll(wPlayerRole))
		{
			g_Battle.rgPlayer[wPlayerIndex].action.sTarget = -1;
		}
	}

	if (fToEnemy && g_Battle.rgPlayer[wPlayerIndex].action.sTarget >= 0)
	{
		if (g_Battle.rgEnemy[g_Battle.rgPlayer[wPlayerIndex].action.sTarget].wObjectID == 0)
		{
			g_Battle.rgPlayer[wPlayerIndex].action.sTarget = PAL_BattleSelectAutoTarget();
			assert(g_Battle.rgPlayer[wPlayerIndex].action.sTarget >= 0);
		}
	}
}

VOID CFight::PAL_BattlePlayerPerformAction(WORD wPlayerIndex)
/*++
  功能：    Perform the selected action for a player.
  我方角色施展动作
  参数：    [IN]  wPlayerIndex - the index of the player.
  角色的序号
  返回值：    None.
  --*/
{
	DWORD    iDamage;
	WORD     wPlayerRole = gpGlobals->rgParty[wPlayerIndex].wPlayerRole;
	SHORT    sTarget;
	int      x, y;
	int      i, j, t;
	WORD     str, def, res, wObject, wMagicNum;
	BOOL     fCritical;
	WORD     rgwCoopPos[3][2] = { {208, 157}, {234, 170}, {260, 183} };

	g_Battle.wMovingPlayerIndex = wPlayerIndex;
	g_Battle.iBlow = 0;

	PAL_BattlePlayerValidateAction(wPlayerIndex);
	PAL_BattleBackupStat();

	sTarget = g_Battle.rgPlayer[wPlayerIndex].action.sTarget;


	switch (g_Battle.rgPlayer[wPlayerIndex].action.ActionType)
	{
	case BATTLEACTIONTYPE::kBattleActionAttack:
	{
		WORD wTimes = 1;
		if (sTarget != -1)
		{
			//
			// Attack one enemy
			//
			for (t = 0; t < (gpGlobals->rgPlayerStatus[wPlayerRole][kStatusDualAttack] ? 2 : 1); t++)
			{
				str = PAL_GetPlayerAttackStrength(wPlayerRole);
				def = PAL_New_GetEnemyDefense(sTarget);
				res = PAL_New_GetEnemyPhysicalResistance(+sTarget);
				fCritical = FALSE;

				iDamage = PAL_CalcPhysicalAttackDamage(str, def, res) + RandomLong(1, 2);

				if (PAL_New_GetTrueByPercentage(17) ||
					gpGlobals->rgPlayerStatus[wPlayerRole][kStatusBravery] > 0)
				{	// Critical Hit	// 普通暴击
					iDamage *= 3;
					fCritical = TRUE;
					gpGlobals->Exp.rgHealthExp[wPlayerRole].wCount += RandomLong(1, 2);
					gpGlobals->Exp.rgAttackExp[wPlayerRole].wCount += RandomLong(3, 5);
					gpGlobals->Exp.rgAttackExp[wPlayerRole].wExp += RandomLong(2, 4)
						* pow(gpGlobals->g.PlayerRoles.rgwLevel[wPlayerRole], 2);
				}

				if (wPlayerRole == RoleID_LiXiaoYao && PAL_New_GetTrueByPercentage(8))
				{	// Bonus hit for Li Xiaoyao	//李逍遥，会心一击
					iDamage *= 2;
					fCritical = TRUE;
					gpGlobals->Exp.rgHealthExp[wPlayerRole].wCount += RandomLong(1, 2);
					gpGlobals->Exp.rgAttackExp[wPlayerRole].wCount += RandomLong(3, 5);
					gpGlobals->Exp.rgAttackExp[wPlayerRole].wExp += RandomLong(2, 4)
						* pow(gpGlobals->g.PlayerRoles.rgwLevel[wPlayerRole], 2);
				}

				iDamage = (INT)(iDamage * RandomFloat(1, 1.125));
				iDamage = std::max(1, (int)iDamage);
				iDamage = std::min(iDamage, g_Battle.rgEnemy[sTarget].dwActualHealth);
				g_Battle.rgEnemy[sTarget].dwActualHealth -= iDamage;

				if (t == 0)
				{
					g_Battle.rgPlayer[wPlayerIndex].wCurrentFrame = 7;
					PAL_BattleDelay(4, 0, TRUE);
				}

				PAL_BattleShowPlayerAttackAnim(wPlayerIndex, fCritical);
			}
		}
		else
		{
			// Attack all enemies
			for (t = 0; t < (gpGlobals->rgPlayerStatus[wPlayerRole][kStatusDualAttack] ? 2 : 1); t++)
			{
				int division = 1;
				const int index[MAX_ENEMIES_IN_TEAM] = { 2, 1, 0, 4, 3 };

				wTimes = 1;
				fCritical = FALSE;
				if (PAL_New_GetTrueByPercentage(17) ||
					gpGlobals->rgPlayerStatus[wPlayerRole][kStatusBravery] > 0)
				{	// Critical Hit	// 普通暴击
					wTimes *= 3;
					fCritical = TRUE;
					gpGlobals->Exp.rgHealthExp[wPlayerRole].wCount += RandomLong(0, 1);
					gpGlobals->Exp.rgAttackExp[wPlayerRole].wCount += RandomLong(1, 3);
					gpGlobals->Exp.rgAttackExp[wPlayerRole].wExp += RandomLong(2, 4)
						* pow(gpGlobals->g.PlayerRoles.rgwLevel[wPlayerRole], 2);
				}

				if (wPlayerRole == RoleID_LiXiaoYao && PAL_New_GetTrueByPercentage(8))
				{	// Bonus hit for Li Xiaoyao	//李逍遥，会心一击
					wTimes *= 2;
					fCritical = TRUE;
					gpGlobals->Exp.rgHealthExp[wPlayerRole].wCount += RandomLong(0, 2);
					gpGlobals->Exp.rgAttackExp[wPlayerRole].wCount += RandomLong(1, 2);
					gpGlobals->Exp.rgAttackExp[wPlayerRole].wExp += RandomLong(2, 4)
						* pow(gpGlobals->g.PlayerRoles.rgwLevel[wPlayerRole], 2);
				}

				if (t == 0)
				{
					g_Battle.rgPlayer[wPlayerIndex].wCurrentFrame = 7;
					PAL_BattleDelay(4, 0, TRUE);
				}

				for (i = 0; i < MAX_ENEMIES_IN_TEAM; i++)
				{
					if (g_Battle.rgEnemy[index[i]].wObjectID == 0 ||
						index[i] > g_Battle.wMaxEnemyIndex)
					{
						continue;
					}

					str = PAL_GetPlayerAttackStrength(wPlayerRole);
					def = PAL_New_GetEnemyDefense(index[i]);
					res = PAL_New_GetEnemyPhysicalResistance(index[i]);

					iDamage = PAL_CalcPhysicalAttackDamage(str, def, res) + RandomLong(1, 2);
					iDamage = (INT)(iDamage * RandomFloat(1, 1.125));
					iDamage *= wTimes;
					iDamage /= division;
					iDamage = std::max(1, (int)iDamage);
					iDamage = std::min(iDamage, g_Battle.rgEnemy[index[i]].dwActualHealth);
					g_Battle.rgEnemy[index[i]].dwActualHealth -= iDamage;

					division++;
					division = std::min(division, 3);
				}

				PAL_BattleShowPlayerAttackAnim(wPlayerIndex, fCritical);
			}
		}

		PAL_BattleUpdateFighters();
		PAL_BattleMakeScene();
		RenderBlendCopy(gpTextureReal, g_Battle.lpSceneBuf);;
		PAL_BattleDelay(3, 0, TRUE);

		gpGlobals->Exp.rgAttackExp[wPlayerRole].wCount++;
		gpGlobals->Exp.rgHealthExp[wPlayerRole].wCount += RandomLong(2, 3);

		break;
	}

	case BATTLEACTIONTYPE::kBattleActionAttackMate:
	{

		if (PAL_IsPlayerDying(wPlayerRole))
		{
			break;
		}

		//
		// Check if there is someone else who is alive
		//
		for (i = 0; i <= gpGlobals->wMaxPartyMemberIndex; i++)
		{
			if (i == wPlayerIndex)
			{
				continue;
			}

			if (gpGlobals->g.PlayerRoles.rgwHP[gpGlobals->rgParty[i].wPlayerRole] > 0)
			{
				break;
			}
		}

		if (i <= gpGlobals->wMaxPartyMemberIndex)
		{
			//
			// Pick a target randomly
			//
			do
			{
				sTarget = RandomLong(0, gpGlobals->wMaxPartyMemberIndex);
			} while (sTarget == wPlayerIndex || gpGlobals->g.PlayerRoles.rgwHP[gpGlobals->rgParty[sTarget].wPlayerRole] == 0);

			for (j = 0; j < 2; j++)
			{
				g_Battle.rgPlayer[wPlayerIndex].wCurrentFrame = 8;
				PAL_BattleDelay(1, 0, TRUE);

				g_Battle.rgPlayer[wPlayerIndex].wCurrentFrame = 0;
				PAL_BattleDelay(1, 0, TRUE);
			}

			PAL_BattleDelay(2, 0, TRUE);

			x = PAL_X(g_Battle.rgPlayer[sTarget].pos) + 30;
			y = PAL_Y(g_Battle.rgPlayer[sTarget].pos) + 12;

			g_Battle.rgPlayer[wPlayerIndex].pos = PAL_XY(x, y);
			g_Battle.rgPlayer[wPlayerIndex].wCurrentFrame = 8;
			PAL_BattleDelay(5, 0, TRUE);

			g_Battle.rgPlayer[wPlayerIndex].wCurrentFrame = 9;
			SOUND_Play(gpGlobals->g.PlayerRoles.rgwWeaponSound[wPlayerRole]);

			str = PAL_GetPlayerAttackStrength(wPlayerRole);
			def = PAL_GetPlayerDefense(gpGlobals->rgParty[sTarget].wPlayerRole);
			if (g_Battle.rgPlayer[sTarget].fDefending)
			{
				def *= 2;
			}

			iDamage = PAL_CalcPhysicalAttackDamage(str, def, 20 + 0.8 * PAL_New_GetPlayerPhysicalResistance(wPlayerRole));
			if (gpGlobals->rgPlayerStatus[gpGlobals->rgParty[sTarget].wPlayerRole][kStatusProtect] > 0)
			{
				iDamage /= 2;
			}

			iDamage = std::max((int)iDamage, 1);

#ifdef LIMIT_DAMAGE_WHEN_ATTACK_MATE
			iDamage = min(iDamage, 500);
#endif
			if (iDamage > (SHORT)gpGlobals->g.PlayerRoles.rgwHP[gpGlobals->rgParty[sTarget].wPlayerRole])
			{
				iDamage = gpGlobals->g.PlayerRoles.rgwHP[gpGlobals->rgParty[sTarget].wPlayerRole];
			}

			gpGlobals->g.PlayerRoles.rgwHP[gpGlobals->rgParty[sTarget].wPlayerRole] -= iDamage;

			g_Battle.rgPlayer[sTarget].pos =
				PAL_XY(PAL_X(g_Battle.rgPlayer[sTarget].pos) - 12,
					PAL_Y(g_Battle.rgPlayer[sTarget].pos) - 6);
			PAL_BattleDelay(1, 0, TRUE);

			g_Battle.rgPlayer[sTarget].iColorShift = 6;
			PAL_BattleDelay(1, 0, TRUE);

			PAL_BattleDisplayStatChange();

			g_Battle.rgPlayer[sTarget].iColorShift = 0;
			PAL_BattleDelay(4, 0, TRUE);

			PAL_BattleUpdateFighters();
			PAL_BattleDelay(4, 0, TRUE);
		}

		break;
	}

	case BATTLEACTIONTYPE::kBattleActionCoopMagic:
	{

		wObject = PAL_GetPlayerCooperativeMagic(gpGlobals->rgParty[wPlayerIndex].wPlayerRole);
		wMagicNum = gpGlobals->g.rgObject[wObject].magic.wMagicNumber;

		if (gpGlobals->g.lprgMagic[wMagicNum].wType == kMagicTypeSummon)
		{
			PAL_BattleShowPlayerPreMagicAnim(wPlayerIndex, wObject);
			PAL_BattleShowPlayerSummonMagicAnim((WORD)-1, wObject);
		}
		else
		{
			//AUDIO_PlaySound(29);
			SOUND_Play(29);

			for (i = 1; i <= 6; i++)
			{
				//
				// Update the position for the player who invoked the action
				//
				x = PAL_X(g_Battle.rgPlayer[wPlayerIndex].posOriginal) * (6 - i);
				y = PAL_Y(g_Battle.rgPlayer[wPlayerIndex].posOriginal) * (6 - i);

				x += rgwCoopPos[0][0] * i;
				y += rgwCoopPos[0][1] * i;

				x /= 6;
				y /= 6;

				g_Battle.rgPlayer[wPlayerIndex].pos = PAL_XY(x, y);

				//
				// Update the position for other players
				//
				t = 0;

				for (j = 0; j <= gpGlobals->wMaxPartyMemberIndex; j++)
				{
					if ((WORD)j == wPlayerIndex)
					{
						continue;
					}

					t++;

					x = PAL_X(g_Battle.rgPlayer[j].posOriginal) * (6 - i);
					y = PAL_Y(g_Battle.rgPlayer[j].posOriginal) * (6 - i);

					x += rgwCoopPos[t][0] * i;
					y += rgwCoopPos[t][1] * i;

					x /= 6;
					y /= 6;

					g_Battle.rgPlayer[j].pos = PAL_XY(x, y);
				}

				PAL_BattleDelay(1, 0, TRUE);
			}

			for (i = gpGlobals->wMaxPartyMemberIndex; i >= 0; i--)
			{
				if ((WORD)i == wPlayerIndex)
				{
					continue;
				}

				g_Battle.rgPlayer[i].wCurrentFrame = 5;

				PAL_BattleDelay(3, 0, TRUE);
			}

			g_Battle.rgPlayer[wPlayerIndex].iColorShift = 6;
			g_Battle.rgPlayer[wPlayerIndex].wCurrentFrame = 5;
			SOUND_Play(157);
			PAL_BattleDelay(5, 0, TRUE);

			g_Battle.rgPlayer[wPlayerIndex].wCurrentFrame = 6;
			g_Battle.rgPlayer[wPlayerIndex].iColorShift = 0;
			PAL_BattleDelay(3, 0, TRUE);

			//#ifdef PAL_WIN95
			PAL_BattleShowPlayerOffMagicAnim((WORD)-1, wObject, sTarget, FALSE);

		}

		for (i = 0; i <= gpGlobals->wMaxPartyMemberIndex; i++)
		{
			//合体时体力损失
			float mCostMP_coefficient =
				gpGlobals->g.PlayerRoles.rgwLevel[gpGlobals->rgParty[i].wPlayerRole] / 60.0;
			gpGlobals->g.PlayerRoles.rgwHP[gpGlobals->rgParty[i].wPlayerRole] -=
				gpGlobals->g.lprgMagic[wMagicNum].wCostMP * mCostMP_coefficient;

			if ((SHORT)(gpGlobals->g.PlayerRoles.rgwHP[gpGlobals->rgParty[i].wPlayerRole]) <= 0)
			{
				gpGlobals->g.PlayerRoles.rgwHP[gpGlobals->rgParty[i].wPlayerRole] = 1;
			}

			//
			// Reset the time meter for everyone when using coopmagic
			//
			g_Battle.rgPlayer[i].state = FIGHTERSTATE::kFighterWait;
		}

		PAL_BattleBackupStat(); // so that "damages" to players won't be shown

		str = 0;

		for (i = 0; i <= gpGlobals->wMaxPartyMemberIndex; i++)
		{
			str += PAL_GetPlayerAttackStrength(gpGlobals->rgParty[i].wPlayerRole);
			str += PAL_GetPlayerMagicStrength(gpGlobals->rgParty[i].wPlayerRole);
		}

		str /= 3;
		str += 150;

		PAL_New_ApplyMagicDamageToEnemy(sTarget, str, wObject, TRUE);// Inflict damage to enemies
		PAL_BattleDisplayStatChange();
		PAL_BattleShowPostMagicAnim();
		PAL_BattleDelay(5, 0, TRUE);

		if (gpGlobals->g.lprgMagic[wMagicNum].wType != kMagicTypeSummon)
		{
			PAL_BattlePostActionCheck(FALSE);

			//
			// Move all players back to the original position
			//
			for (i = 1; i <= 6; i++)
			{
				//
				// Update the position for the player who invoked the action
				//
				x = PAL_X(g_Battle.rgPlayer[wPlayerIndex].posOriginal) * i;
				y = PAL_Y(g_Battle.rgPlayer[wPlayerIndex].posOriginal) * i;

				x += rgwCoopPos[0][0] * (6 - i);
				y += rgwCoopPos[0][1] * (6 - i);

				x /= 6;
				y /= 6;

				g_Battle.rgPlayer[wPlayerIndex].pos = PAL_XY(x, y);

				//
				// Update the position for other players
				//
				t = 0;

				for (j = 0; j <= gpGlobals->wMaxPartyMemberIndex; j++)
				{
					g_Battle.rgPlayer[j].wCurrentFrame = 0;

					if ((WORD)j == wPlayerIndex)
					{
						continue;
					}

					t++;

					x = PAL_X(g_Battle.rgPlayer[j].posOriginal) * i;
					y = PAL_Y(g_Battle.rgPlayer[j].posOriginal) * i;

					x += rgwCoopPos[t][0] * (6 - i);
					y += rgwCoopPos[t][1] * (6 - i);

					x /= 6;
					y /= 6;

					g_Battle.rgPlayer[j].pos = PAL_XY(x, y);
				}

				PAL_BattleDelay(1, 0, TRUE);
			}
		}
		break;
	}

	case BATTLEACTIONTYPE::kBattleActionDefend:
	{

		g_Battle.rgPlayer[wPlayerIndex].fDefending = TRUE;
		gpGlobals->Exp.rgDefenseExp[wPlayerRole].wCount += 2;
		if (cfgDefBoostActive)
		{
			//本回合，回合数
			g_Battle.rgPlayer[wPlayerRole].fDefenAction = cfgDefBoostActive;
		}
		break;
	}
	case BATTLEACTIONTYPE::kBattleActionFlee:
	{
		str = PAL_GetPlayerFleeRate(wPlayerRole);
		def = 0;

		for (i = 0; i <= g_Battle.wMaxEnemyIndex; i++)
		{
			if (g_Battle.rgEnemy[i].wObjectID == 0)
			{
				continue;
			}

			def += (SHORT)(g_Battle.rgEnemy[i].e.wFleeRate);
			if (cfgDmgFormula)//#ifdef cfgDmgFormula
				def += g_Battle.rgEnemy[i].e.wLevel * 3;
			else
				def += (g_Battle.rgEnemy[i].e.wLevel + 6) * 2;
		}

		if ((SHORT)def < 0)
		{
			def = 0;
		}

		if (RandomLong(0, str) >= RandomLong(0, def) && !g_Battle.fIsBoss)
		{
			//
			// Successful escape
			//
			PAL_BattlePlayerEscape();
		}
		else
		{
			//
			// Failed escape
			//
			g_Battle.rgPlayer[wPlayerIndex].wCurrentFrame = 0;

			for (i = 0; i < 3; i++)
			{
				x = PAL_X(g_Battle.rgPlayer[wPlayerIndex].pos) + 4;
				y = PAL_Y(g_Battle.rgPlayer[wPlayerIndex].pos) + 2;

				g_Battle.rgPlayer[wPlayerIndex].pos = PAL_XY(x, y);

				PAL_BattleDelay(1, 0, TRUE);
			}

			g_Battle.rgPlayer[wPlayerIndex].wCurrentFrame = 1;
			PAL_BattleDelay(8, BATTLE_LABEL_ESCAPEFAIL, TRUE);

			gpGlobals->Exp.rgFleeExp[wPlayerRole].wCount += 2;
		}
		break;
	}

	case BATTLEACTIONTYPE::kBattleActionMagic:
	{

		wObject = g_Battle.rgPlayer[wPlayerIndex].action.wActionID;
		wMagicNum = gpGlobals->g.rgObject[wObject].magic.wMagicNumber;

		//#ifdef PAL_WIN95
		if (cfgIsWin95)
			PAL_BattleShowPlayerPreMagicAnim(wPlayerIndex, wObject);//
		else
			//#else
			PAL_BattleShowPlayerPreMagicAnim(wPlayerIndex,
				(gpGlobals->g.lprgMagic[wMagicNum].wType == kMagicTypeSummon));
		//#endif

		if (!gpGlobals->fAutoBattle)
		{
			gpGlobals->g.PlayerRoles.rgwMP[wPlayerRole] -= gpGlobals->g.lprgMagic[wMagicNum].wCostMP;
			if ((SHORT)(gpGlobals->g.PlayerRoles.rgwMP[wPlayerRole]) < 0)
			{
				gpGlobals->g.PlayerRoles.rgwMP[wPlayerRole] = 0;
			}
		}

		if (gpGlobals->g.lprgMagic[wMagicNum].wType == kMagicTypeApplyToPlayer ||
			gpGlobals->g.lprgMagic[wMagicNum].wType == kMagicTypeApplyToParty ||
			gpGlobals->g.lprgMagic[wMagicNum].wType == kMagicTypeTrance)
		{
			//
			// Using a defensive magic
			//
			WORD w = 0;

			if (g_Battle.rgPlayer[wPlayerIndex].action.sTarget != -1)
			{
				w = gpGlobals->rgParty[g_Battle.rgPlayer[wPlayerIndex].action.sTarget].wPlayerRole;
			}
			else if (gpGlobals->g.lprgMagic[wMagicNum].wType == kMagicTypeTrance)
			{
				w = wPlayerRole;
			}

			gpGlobals->g.rgObject[wObject].magic.wScriptOnUse =
				PAL_RunTriggerScript(gpGlobals->g.rgObject[wObject].magic.wScriptOnUse, wPlayerRole);

			if (g_fScriptSuccess)
			{
				PAL_BattleShowPlayerDefMagicAnim(wPlayerIndex, wObject, sTarget);

				gpGlobals->g.rgObject[wObject].magic.wScriptOnSuccess =
					PAL_RunTriggerScript(gpGlobals->g.rgObject[wObject].magic.wScriptOnSuccess, w);

				if (g_fScriptSuccess)
				{
					if (gpGlobals->g.lprgMagic[wMagicNum].wType == kMagicTypeTrance)
					{
						for (i = 0; i < 6; i++)
						{
							g_Battle.rgPlayer[wPlayerIndex].iColorShift = i * 2;
							PAL_BattleDelay(1, 0, TRUE);
						}

						PAL_BattleBackupScene();
						PAL_LoadBattleSprites();

						g_Battle.rgPlayer[wPlayerIndex].iColorShift = 0;

						PAL_BattleMakeScene();
						RenderBlendCopy(gpTextureReal, g_Battle.lpSceneBuf);;
						PAL_BattleFadeScene();
					}
				}
			}
		}
		else
		{
			//
			// Using an offensive magic
			//
			gpGlobals->g.rgObject[wObject].magic.wScriptOnUse =
				PAL_RunTriggerScript(gpGlobals->g.rgObject[wObject].magic.wScriptOnUse, wPlayerRole);

			if (g_fScriptSuccess)
			{
				if (gpGlobals->g.lprgMagic[wMagicNum].wType == kMagicTypeSummon)
				{
					PAL_BattleShowPlayerSummonMagicAnim(wPlayerIndex, wObject);
				}
				else
				{
					//#ifdef PAL_WIN95
					if (cfgIsWin95)
						PAL_BattleShowPlayerOffMagicAnim(wPlayerIndex, wObject, sTarget, FALSE);
					//#else
					else
						PAL_BattleShowPlayerOffMagicAnim(wPlayerIndex, wObject, sTarget);
					//#endif
				}

				if (sTarget == -1)
				{
#ifndef EDIT_SCRIT_EVENTOBJECTID_WHEN_ITIS_0XFFFF
					gpGlobals->g.rgObject[wObject].magic.wScriptOnSuccess =
						PAL_RunTriggerScript(gpGlobals->g.rgObject[wObject].magic.wScriptOnSuccess, (WORD)sTarget);
#else
					gpGlobals->g.rgObject[wObject].magic.wScriptOnSuccess =
						PAL_RunTriggerScript(gpGlobals->g.rgObject[wObject].magic.wScriptOnSuccess,
							gpGlobals->rgParty[wPlayerIndex].wPlayerRole);
#endif
				}
				else
				{
					gpGlobals->g.rgObject[wObject].magic.wScriptOnSuccess =
						PAL_RunTriggerScript(gpGlobals->g.rgObject[wObject].magic.wScriptOnSuccess, (WORD)sTarget);
				}
				str = PAL_GetPlayerMagicStrength(wPlayerRole);
				PAL_New_ApplyMagicDamageToEnemy(sTarget, str, wObject, TRUE);// Inflict damage to enemies
			}
		}

		PAL_BattleDisplayStatChange();
		PAL_BattleShowPostMagicAnim();
		PAL_BattleDelay(5, 0, TRUE);

		gpGlobals->Exp.rgMagicExp[wPlayerRole].wCount += RandomLong(2, 3);
		gpGlobals->Exp.rgMagicPowerExp[wPlayerRole].wCount++;

		break;
	}

	case BATTLEACTIONTYPE::kBattleActionThrowItem:
	{

		wObject = g_Battle.rgPlayer[wPlayerIndex].action.wActionID;

		for (i = 0; i < 4; i++)
		{
			g_Battle.rgPlayer[wPlayerIndex].pos =
				PAL_XY(PAL_X(g_Battle.rgPlayer[wPlayerIndex].pos) - (4 - i),
					PAL_Y(g_Battle.rgPlayer[wPlayerIndex].pos) - (4 - i) / 2);

			PAL_BattleDelay(1, 0, TRUE);
		}

		PAL_BattleDelay(2, wObject, TRUE);

		g_Battle.rgPlayer[wPlayerIndex].wCurrentFrame = 5;
		SOUND_Play(gpGlobals->g.PlayerRoles.rgwMagicSound[wPlayerRole]);

		PAL_BattleDelay(8, wObject, TRUE);

		g_Battle.rgPlayer[wPlayerIndex].wCurrentFrame = 6;
		PAL_BattleDelay(2, wObject, TRUE);

		//
		// Run the script
		//
		gpGlobals->g.rgObject[wObject].item.wScriptOnThrow =
			PAL_RunTriggerScript(gpGlobals->g.rgObject[wObject].item.wScriptOnThrow, (WORD)sTarget);

		//
		// Remove the thrown item from inventory
		//
		PAL_AddItemToInventory(wObject, -1);

		PAL_BattleDisplayStatChange();
		PAL_BattleDelay(4, 0, TRUE);
		PAL_BattleUpdateFighters();
		PAL_BattleDelay(4, 0, TRUE);

		break;
	}

	case BATTLEACTIONTYPE::kBattleActionUseItem:           //使用物品
	{

		wObject = g_Battle.rgPlayer[wPlayerIndex].action.wActionID;

		PAL_BattleShowPlayerUseItemAnim(wPlayerIndex, wObject, sTarget);

		//
		// Run the script
		//
		gpGlobals->g.rgObject[wObject].item.wScriptOnUse =
#ifndef EDIT_SCRIT_EVENTOBJECTID_WHEN_ITIS_0XFFFF
			PAL_RunTriggerScript(gpGlobals->g.rgObject[wObject].item.wScriptOnUse,
				(sTarget == -1) ? 0xFFFF : gpGlobals->rgParty[sTarget].wPlayerRole);
#else
			PAL_RunTriggerScript(gpGlobals->g.rgObject[wObject].item.wScriptOnUse,
				(sTarget == -1) ? gpGlobals->rgParty[wPlayerIndex].wPlayerRole : gpGlobals->rgParty[sTarget].wPlayerRole);
#endif
		//
		// Remove the item if the item is consuming
		//
		if (gpGlobals->g.rgObject[wObject].item.wFlags & kItemFlagConsuming)
		{
			PAL_AddItemToInventory(wObject, -1);
		}

		//if (g_Battle.iHidingTime < 0)
		if (PAL_GetHidingTime() < 0)
		{
			//g_Battle.iHidingTime = -g_Battle.iHidingTime;
			PAL_SetHidingTime(-PAL_GetHidingTime());
			PAL_BattleBackupScene();
			PAL_BattleMakeScene();
			RenderBlendCopy(gpTextureReal, g_Battle.lpSceneBuf);;
			PAL_BattleFadeScene();
		}

		PAL_BattleUpdateFighters();
		PAL_BattleDisplayStatChange();
		PAL_BattleDelay(8, 0, TRUE);

		break;
	}

	case BATTLEACTIONTYPE::kBattleActionPass:
	{

		break;
	}
	}

	//
	// Revert this player back to waiting state.
	//
	g_Battle.rgPlayer[wPlayerIndex].state = FIGHTERSTATE::kFighterWait;

	PAL_BattlePostActionCheck(FALSE);

}

INT CFight::PAL_BattleEnemySelectTargetIndex(VOID)const
/*++
  功能：    Select a attackable player randomly.
  参数：    None.
  返回值：   目标序号.
  --*/
{
	int i;

	i = RandomLong(0, gpGlobals->wMaxPartyMemberIndex);

	while (gpGlobals->g.PlayerRoles.rgwHP[gpGlobals->rgParty[i].wPlayerRole] == 0)
	{
		i = RandomLong(0, gpGlobals->wMaxPartyMemberIndex);
	}

	return i;
}


BOOL CFight::PAL_New_IfEnemyCanEscape(WORD wEnemyIndex)
{
	WORD percentage = g_Battle.rgEnemy[wEnemyIndex].dwActualHealth * 100 / std::max((int)g_Battle.rgEnemy[wEnemyIndex].dwMaxHealth, 1);

	if (!g_Battle.fIsBoss && PAL_New_IfEnemyCanMove(wEnemyIndex))
	{
		if (PAL_New_GetAliveEnemyNum() < PAL_New_GetHealthyPlayerNum() && percentage < 25)
		{
			return TRUE;
		}
		else if (PAL_New_GetAliveEnemyNum() <= PAL_New_GetHealthyPlayerNum() && percentage <= 13)
		{
			return TRUE;
		}
	}
	return FALSE;
}

VOID CFight::PAL_BattleEnemyPerformAction(
	WORD		wEnemyIndex
)
/*++
  功能：    Perform the selected action for a player.
  参数：    [IN]  wEnemyIndex - the index of the player.
  返回值：    None.
  --*/
{
	int        str, def, i, x, y, ex, ey;
	SHORT      sElementalResistance[NUM_MAGIC_ELEMENTAL] = { 0 };
	SHORT      sPoisonResistance;
	WORD       wPlayerRole, w, wMagic, wMagicNum;
	SHORT      sTarget;
	INT			iDamage;
	BOOL       fAutoDefend = FALSE, rgfMagAutoDefend[MAX_PLAYERS_IN_PARTY]{ 0 };

	g_Battle.iBlow = 0;

	PAL_BattleBackupStat();
#ifdef ENEMY_CAN_ESCAPE
	if (PAL_New_IfEnemyCanEscape(wEnemyIndex))
	{
		str = PAL_New_GetEnemyFleeRate(wEnemyIndex);
		int index = PAL_New_GetPlayerIndexByPara(Para_FleeRate, FALSE);
		wPlayerRole = gpGlobals->rgParty[index].wPlayerRole;
		def = PAL_GetPlayerFleeRate(wPlayerRole);
		def = (def > 2 * str) ? (2 * str) : def;
		if (RandomLong(0, str / max(1, PAL_New_GetAliveEnemyNum())) >= RandomLong(0, def) && PAL_New_GetTrueByPercentage(50))
		{
			PAL_BattleEnemyEscape();				// 默认结果是战斗终止
			g_Battle.BattleResult = kBattleResultEnemyFleed;// 此处改为敌人逃跑
			return;
		}
	}
#endif

	wMagic = g_Battle.rgEnemy[wEnemyIndex].e.wMagic;

	if (cfgSemiRandomTgt == 0)//不完全是随机选择目标
		sTarget = PAL_BattleEnemySelectTargetIndex();
	else //基础值大于0时，较大几率选择生命值最低的角色。不大于0时，较大几率选择生命值最高的角色。
	{
		WORD tMagicNum;
		if (wMagic == 0 || wMagic == 0xFFFF)
		{
			tMagicNum = 0;
		}
		else
		{
			tMagicNum = gpGlobals->g.rgObject[wMagic].magic.wMagicNumber;
		}
		if (wMagic == 0xFFFF)
		{
			sTarget = PAL_BattleEnemySelectTargetIndex();
		}
		else if (wMagic == 0 || (SHORT)gpGlobals->g.lprgMagic[tMagicNum].wBaseDamage >= 0)
		{
			if (RandomLong(0, 9) < 6)
			{
				sTarget = PAL_New_GetPlayerIndexByHealth(TRUE);
			}
			else
			{
				sTarget = PAL_BattleEnemySelectTargetIndex();
			}
		}
		else
		{
			if (RandomLong(0, 9) < 4)
			{
				sTarget = PAL_New_GetPlayerIndexByHealth(FALSE);
			}
			else
			{
				sTarget = PAL_BattleEnemySelectTargetIndex();
			}
		}
	}
	//#endif//不完全是随机选择目标

	wPlayerRole = gpGlobals->rgParty[sTarget].wPlayerRole;

	if (g_Battle.rgEnemy[wEnemyIndex].rgwStatus[kStatusSleep] > 0 ||
		g_Battle.rgEnemy[wEnemyIndex].rgwStatus[kStatusParalyzed] > 0 ||
		//g_Battle.iHidingTime > 0)
		PAL_GetHidingTime() > 0)
	{
		//
		// Do nothing
		//
		goto end;
	}
	if (!g_Battle.rgEnemy[wEnemyIndex].dwActualHealth)
	{
		goto end;
	}
	if (g_Battle.rgEnemy[wEnemyIndex].rgwStatus[kStatusConfused] > 0)
	{
		// TODO
		//敌方混乱
		if (cfgConfuseMode)
		{
			i = 5;
			if (PAL_New_GetAliveEnemyNum() > 1)
			{
				do
				{
					i = RandomLong(0, g_Battle.wMaxEnemyIndex);
				} while (i == wEnemyIndex ||
					g_Battle.rgEnemy[i].wObjectID == 0 ||
					g_Battle.rgEnemy[i].dwActualHealth <= 0);
			}
			if (i <= g_Battle.wMaxEnemyIndex)
			{

				sTarget = i;
				//伤害计算
				iDamage = PAL_CalcPhysicalAttackDamage(PAL_New_GetEnemyAttackStrength(wEnemyIndex) + RandomLong(0, 2),
					PAL_New_GetEnemyDefense(sTarget),
					PAL_New_GetEnemyPhysicalResistance(sTarget));
				iDamage += RandomLong(0, 1);
				g_Battle.rgEnemy[sTarget].dwActualHealth -= std::min(iDamage, (int)g_Battle.rgEnemy[sTarget].dwActualHealth);


				//声音
				SOUND_Play(g_Battle.rgEnemy[wEnemyIndex].e.wAttackSound);
				//位置
				ex = PAL_X(g_Battle.rgEnemy[sTarget].pos) - 24;
				ey = PAL_Y(g_Battle.rgEnemy[sTarget].pos) - 16;

				g_Battle.rgEnemy[wEnemyIndex].pos = PAL_XY(ex, ey);

				//动作
				for (i = 0; i < g_Battle.rgEnemy[wEnemyIndex].e.wMagicFrames; i++)
				{
					g_Battle.rgEnemy[wEnemyIndex].wCurrentFrame =
						g_Battle.rgEnemy[wEnemyIndex].e.wIdleFrames + i;
					PAL_BattleDelay(2, 0, FALSE);
				}

				for (i = 0; i < 3 - g_Battle.rgEnemy[wEnemyIndex].e.wMagicFrames; i++)
				{
					x = PAL_X(g_Battle.rgEnemy[wEnemyIndex].pos) - 2;
					y = PAL_Y(g_Battle.rgEnemy[wEnemyIndex].pos) - 1;
					g_Battle.rgEnemy[wEnemyIndex].pos = PAL_XY(x, y);
					PAL_BattleDelay(1, 0, FALSE);
				}

				//显示结果
				if (PAL_BattleDisplayStatChange())
				{
					PAL_BattleDelay(6, 0, TRUE);
				}
			}
			else//没有队友，攻击我方
			{
				if (cfgConfuseMode == 2 && g_Battle.rgEnemy[wEnemyIndex].dwActualHealth)
				{
					sTarget = PAL_BattleEnemySelectTargetIndex();//重新选择
					PAL_BattleEnemyPhysical(wEnemyIndex, sTarget);
				}
			}
		}
		goto end;
	}
	//
	if (cfgAutoDefRatio == 0)//自动防御值为0，手动防御
		//新增每次行动前。闪动一次//////
	{

		auto oldcolor = g_Battle.rgEnemy[wEnemyIndex].iColorShift;
		g_Battle.rgEnemy[wEnemyIndex].iColorShift = 7;
		PAL_BattleDelay(4, 0, 0);//延迟周期
		g_Battle.rgEnemy[wEnemyIndex].iColorShift = oldcolor;
	}
	//
	if (wMagic != 0 &&
		RandomLong(0, 9) < g_Battle.rgEnemy[wEnemyIndex].e.wMagicRate &&
		g_Battle.rgEnemy[wEnemyIndex].rgwStatus[kStatusSilence] == 0)
	{
		//
		// Magical attack
		//
		if (wMagic == 0xFFFF)
		{
			//
			// Do nothing
			//
			goto end;
		}

		wMagicNum = gpGlobals->g.rgObject[wMagic].magic.wMagicNumber;

		str = PAL_New_GetEnemyMagicStrength(wEnemyIndex);

		ex = PAL_X(g_Battle.rgEnemy[wEnemyIndex].pos);
		ey = PAL_Y(g_Battle.rgEnemy[wEnemyIndex].pos);

		ex += 12;
		ey += 6;

		g_Battle.rgEnemy[wEnemyIndex].pos = PAL_XY(ex, ey);
		PAL_BattleDelay(1, 0, FALSE);

		ex += 4;
		ey += 2;

		g_Battle.rgEnemy[wEnemyIndex].pos = PAL_XY(ex, ey);
		PAL_BattleDelay(1, 0, FALSE);

		SOUND_Play(g_Battle.rgEnemy[wEnemyIndex].e.wMagicSound);

		for (i = 0; i < g_Battle.rgEnemy[wEnemyIndex].e.wMagicFrames; i++)
		{
			g_Battle.rgEnemy[wEnemyIndex].wCurrentFrame =
				g_Battle.rgEnemy[wEnemyIndex].e.wIdleFrames + i;
			PAL_BattleDelay(g_Battle.rgEnemy[wEnemyIndex].e.wActWaitFrames, 0, FALSE);
		}

		if (g_Battle.rgEnemy[wEnemyIndex].e.wMagicFrames == 0)
		{
			PAL_BattleDelay(1, 0, FALSE);
		}

		if (gpGlobals->g.lprgMagic[wMagicNum].wSoundDelay == 0)
		{
			for (i = 0; i <= g_Battle.rgEnemy[wEnemyIndex].e.wAttackFrames; i++)
			{
				g_Battle.rgEnemy[wEnemyIndex].wCurrentFrame =
					i - 1 + g_Battle.rgEnemy[wEnemyIndex].e.wIdleFrames + g_Battle.rgEnemy[wEnemyIndex].e.wMagicFrames;
				PAL_BattleDelay(g_Battle.rgEnemy[wEnemyIndex].e.wActWaitFrames, 0, FALSE);
			}
		}
		else//对于风雪冰天和大咒蛇等法术的处理
		{
			//PAL_BattleDelay(gpGlobals->g.lprgMagic[wMagicNum].wSoundDelay, 0, FALSE);
			for (i = 0; i <= g_Battle.rgEnemy[wEnemyIndex].e.wAttackFrames; i++)
			{
				g_Battle.rgEnemy[wEnemyIndex].wCurrentFrame =
					i - 1 + g_Battle.rgEnemy[wEnemyIndex].e.wIdleFrames + g_Battle.rgEnemy[wEnemyIndex].e.wMagicFrames;
				PAL_BattleDelay(g_Battle.rgEnemy[wEnemyIndex].e.wActWaitFrames, 0, FALSE);
			}
		}


		if (gpGlobals->g.lprgMagic[wMagicNum].wType != kMagicTypeNormal)
		{
			sTarget = -1;
			//对魔法的防御，全体
			for (i = 0; i <= gpGlobals->wMaxPartyMemberIndex; i++)
			{
				w = gpGlobals->rgParty[i].wPlayerRole;

				if (PAL_New_IfPlayerCanMove(w)
					&& (PAL_New_GetTrueByPercentage(cfgAutoDefRatio)
						|| PAL_GetManuallyDefend())
					&& gpGlobals->g.PlayerRoles.rgwHP[w] != 0)
				{
					rgfMagAutoDefend[i] = TRUE;
					g_Battle.rgPlayer[i].wCurrentFrame = 3;
				}
				else
				{
					rgfMagAutoDefend[i] = FALSE;
				}
			}
		}
		else if (PAL_New_IfPlayerCanMove(wPlayerRole) &&
			(PAL_New_GetTrueByPercentage(cfgAutoDefRatio)
				|| PAL_GetManuallyDefend()))
		{	//选择的目标不能是HP等于0的，所以不用在此判断HP不等于0
			fAutoDefend = TRUE;
			g_Battle.rgPlayer[sTarget].wCurrentFrame = 3;
		}


		gpGlobals->g.rgObject[wMagic].magic.wScriptOnUse =
			PAL_RunTriggerScript(gpGlobals->g.rgObject[wMagic].magic.wScriptOnUse, wPlayerRole);

		if (g_fScriptSuccess)
		{
			PAL_BattleShowEnemyMagicAnim(wMagic, sTarget);

			gpGlobals->g.rgObject[wMagic].magic.wScriptOnSuccess =
				PAL_RunTriggerScript(gpGlobals->g.rgObject[wMagic].magic.wScriptOnSuccess, wPlayerRole);
		}

		if ((SHORT)(gpGlobals->g.lprgMagic[wMagicNum].wBaseDamage) > 0)
		{
			if (sTarget == -1)
			{
				//
				// damage all players
				//
				for (i = 0; i <= gpGlobals->wMaxPartyMemberIndex; i++)
				{
					w = gpGlobals->rgParty[i].wPlayerRole;
					if (gpGlobals->g.PlayerRoles.rgwHP[w] == 0)
					{	// skip dead players
						continue;
					}

					def = PAL_GetPlayerDefense(w);
					for (x = 0; x < NUM_MAGIC_ELEMENTAL; x++)
					{
						sElementalResistance[x] = (SHORT)PAL_GetPlayerElementalResistance(w, x);
					}
					sPoisonResistance = (SHORT)PAL_GetPlayerPoisonResistance(w);

					//str *= 0.8;

					iDamage = PAL_CalcMagicDamage(str, def, sElementalResistance, sPoisonResistance, wMagic, w);

					iDamage /= ((g_Battle.rgPlayer[i].fDefending ? 2 : 1) *
						((gpGlobals->rgPlayerStatus[w][kStatusProtect] > 0) ? 2 : 1)) +
						(rgfMagAutoDefend[i] ? 1 : 0);

					//#ifdef GAIN_MORE_HIDDEN_EXP
					if (cfgHiddenExp)//更多的隐藏经验
						if (rgfMagAutoDefend[i] == TRUE)
						{
							gpGlobals->Exp.rgDefenseExp[wPlayerRole].wCount += RandomLong(0, 1);
							gpGlobals->Exp.rgDexterityExp[wPlayerRole].wCount += RandomLong(0, 1);
							gpGlobals->Exp.rgFleeExp[wPlayerRole].wCount += RandomLong(0, 1);
						}
					//#endif

					iDamage = std::max(iDamage, 0);
					iDamage = std::min(iDamage, (int)gpGlobals->g.PlayerRoles.rgwHP[w]);
					if (!debugSwitch[fIsInvincible])
						gpGlobals->g.PlayerRoles.rgwHP[w] -= iDamage;


					if (gpGlobals->g.PlayerRoles.rgwHP[w] == 0)
					{
						SOUND_Play(gpGlobals->g.PlayerRoles.rgwDeathSound[w]);
					}
				}
			}
			else
			{
				//
				// damage one player
				//
				def = PAL_GetPlayerDefense(wPlayerRole);

				for (x = 0; x < NUM_MAGIC_ELEMENTAL; x++)
				{
					sElementalResistance[x] = PAL_GetPlayerElementalResistance(wPlayerRole, x);
				}
				sPoisonResistance = PAL_GetPlayerPoisonResistance(wPlayerRole);

				iDamage = PAL_CalcMagicDamage(str, def, sElementalResistance, sPoisonResistance, wMagic, wPlayerRole);

				iDamage /= ((g_Battle.rgPlayer[sTarget].fDefending ? 2 : 1) *
					((gpGlobals->rgPlayerStatus[wPlayerRole][kStatusProtect] > 0) ? 2 : 1)) +
					(fAutoDefend ? 1 : 0);

				//#ifdef GAIN_MORE_HIDDEN_EXP
				if (cfgHiddenExp)//更多的隐藏经验
					if (fAutoDefend == TRUE)
					{
						gpGlobals->Exp.rgDefenseExp[wPlayerRole].wCount += RandomLong(0, 1);
						gpGlobals->Exp.rgDexterityExp[wPlayerRole].wCount += RandomLong(0, 1);
						gpGlobals->Exp.rgFleeExp[wPlayerRole].wCount += RandomLong(0, 1);
					}
				//#endif

				iDamage = std::max(iDamage, 0);
				iDamage = std::min(iDamage, (int)gpGlobals->g.PlayerRoles.rgwHP[wPlayerRole]);

				//#ifndef INVINCIBLE
				if (!debugSwitch[fIsInvincible])
					gpGlobals->g.PlayerRoles.rgwHP[wPlayerRole] -= iDamage;
				//#endif

				if (gpGlobals->g.PlayerRoles.rgwHP[wPlayerRole] == 0)
				{
					SOUND_Play(gpGlobals->g.PlayerRoles.rgwDeathSound[wPlayerRole]);
				}
			}
		}

		if (!gpGlobals->fAutoBattle)
		{
			PAL_BattleDisplayStatChange();
		}

		for (i = 0; i < 5; i++)
		{
			if (sTarget == -1)
			{
				for (x = 0; x <= gpGlobals->wMaxPartyMemberIndex; x++)
				{
					if (g_Battle.rgPlayer[x].wPrevHP ==
						gpGlobals->g.PlayerRoles.rgwHP[gpGlobals->rgParty[x].wPlayerRole])
					{
						//
						// Skip unaffected players
						//
						continue;
					}

					g_Battle.rgPlayer[x].wCurrentFrame = 4;
					if (i > 0)
					{
						g_Battle.rgPlayer[x].pos =
							PAL_XY(PAL_X(g_Battle.rgPlayer[x].pos) + (8 >> i),
								PAL_Y(g_Battle.rgPlayer[x].pos) + (4 >> i));
					}
					g_Battle.rgPlayer[x].iColorShift = ((i < 3) ? 6 : 0);
				}
			}
			else
			{
				g_Battle.rgPlayer[sTarget].wCurrentFrame = 4;
				if (i > 0)
				{
					g_Battle.rgPlayer[sTarget].pos =
						PAL_XY(PAL_X(g_Battle.rgPlayer[sTarget].pos) + (8 >> i),
							PAL_Y(g_Battle.rgPlayer[sTarget].pos) + (4 >> i));
				}
				g_Battle.rgPlayer[sTarget].iColorShift = ((i < 3) ? 6 : 0);
			}

			PAL_BattleDelay(1, 0, FALSE);
		}

		g_Battle.rgEnemy[wEnemyIndex].wCurrentFrame = 0;
		g_Battle.rgEnemy[wEnemyIndex].pos = g_Battle.rgEnemy[wEnemyIndex].posOriginal;

		PAL_BattleDelay(1, 0, FALSE);
		PAL_BattleUpdateFighters();

		PAL_BattlePostActionCheck(TRUE);
		PAL_BattleDelay(8, 0, TRUE);
	}
	else
	{
		//
		// Physical attack
		//
		PAL_BattleEnemyPhysical(wEnemyIndex, sTarget);
	}
end:
	i = 0; // do nothing

}

VOID CFight::PAL_BattleStealFromEnemy(
	WORD		wTarget,
	WORD		wStealRate
)
/*++
  功能：    Steal from the enemy.
  参数：    [IN]  wTarget - the target enemy index.
  [IN]		wStealRate - the rate of successful theft.
  返回值：    None.
  --*/
{
	int   iPlayerIndex = g_Battle.wMovingPlayerIndex;
	int   offset, x, y, i;
	std::string  s;

	g_Battle.rgPlayer[iPlayerIndex].wCurrentFrame = 10;
	offset = ((INT)wTarget - iPlayerIndex) * 8;

	x = PAL_X(g_Battle.rgEnemy[wTarget].pos) + 64 - offset;
	y = PAL_Y(g_Battle.rgEnemy[wTarget].pos) + 20 - offset / 2;

	g_Battle.rgPlayer[iPlayerIndex].pos = PAL_XY(x, y);

	PAL_BattleDelay(1, 0, TRUE);

	for (i = 0; i < 5; i++)
	{
		x -= i + 8;
		y -= 4;

		g_Battle.rgPlayer[iPlayerIndex].pos = PAL_XY(x, y);

		if (i == 4)
		{
			g_Battle.rgEnemy[wTarget].iColorShift = 6;
		}

		PAL_BattleDelay(1, 0, TRUE);
	}

	g_Battle.rgEnemy[wTarget].iColorShift = 0;
	x--;
	g_Battle.rgPlayer[iPlayerIndex].pos = PAL_XY(x, y);
	PAL_BattleDelay(3, 0, TRUE);

	g_Battle.rgPlayer[iPlayerIndex].state = FIGHTERSTATE::kFighterWait;
	PAL_BattleUpdateFighters();
	PAL_BattleDelay(1, 0, TRUE);

	if (g_Battle.rgEnemy[wTarget].e.nStealItem > 0 &&
		(RandomLong(1, 10) <= wStealRate || wStealRate == 0))
	{
		if (g_Battle.rgEnemy[wTarget].e.wStealItem == 0)
		{
			// stolen coins
			//修改偷敌人金钱每次固定2分之1 不足1000 全部
			//int c = g_Battle.rgEnemy[wTarget].e.nStealItem / RandomLong(2, 3);
			int c = g_Battle.rgEnemy[wTarget].e.nStealItem / 2;
			if (c <= 500)c *= 2;
			g_Battle.rgEnemy[wTarget].e.nStealItem -= c;
			gpGlobals->dwCash += c;

			if (c > 0)
			{
				s = PAL_GetWord(34);
				s += "  ";
				s += va("%d", c);
				s += "  ";
				s += PAL_GetWord(10);
				//strcpy(s, PAL_GetWord(34));
				//strcat(s, " ");
				//strcat(s, va("%d", c));
				//strcat(s, " ");
				//strcat(s, PAL_GetWord(10));
			}
		}
		else
		{
			//
			// stolen item
			//
			g_Battle.rgEnemy[wTarget].e.nStealItem--;
			PAL_AddItemToInventory(g_Battle.rgEnemy[wTarget].e.wStealItem, 1);

			s = PAL_GetWord(34);
			s += PAL_GetWord(g_Battle.rgEnemy[wTarget].e.wStealItem);
		}

		if (!s.empty())
		{
			PAL_StartDialog(kDialogCenterWindow, 0, 0, FALSE);
			PAL_ShowDialogText(s.c_str());
		}
	}
}

VOID CFight::PAL_BattleSimulateMagic(
	SHORT		sTarget,
	WORD		wMagicObjectID,
	WORD		wBaseDamage,
	BOOL		fIncludeMagicDamage
)
/*++
  功能：  Simulate a magic for players. Mostly used in item throwing script.
  参数：
  [IN]  sTarget - the target enemy index. -1 = all enemies.
  [IN]  wMagicObjectID - the object ID of the magic to be simulated.
  [IN]  wBaseDamage - the base damage of the simulation.
  返回值：  None.
  --*/
{
	WORD	wMagicDamage, wMagicNumber;

	if (gpGlobals->g.rgObject[wMagicObjectID].magic.wFlags & kMagicFlagApplyToAll)
	{
		sTarget = -1;
	}
	else if (sTarget == -1)
	{
		sTarget = PAL_BattleSelectAutoTarget();
	}
	//
	// Show the magic animation
	//
//fdef PAL_WIN95
	if (cfgIsWin95)
		PAL_BattleShowPlayerOffMagicAnim(0xFFFF, wMagicObjectID, sTarget, FALSE);
	//lse
	else
		PAL_BattleShowPlayerOffMagicAnim(0xFFFF, wMagicObjectID, sTarget);
	//ndif

	wMagicNumber = gpGlobals->g.rgObject[wMagicObjectID].magic.wMagicNumber;
	wMagicDamage = gpGlobals->g.lprgMagic[wMagicNumber].wBaseDamage;

	if (fIncludeMagicDamage == FALSE ||
		wMagicObjectID == 0x017b ||
		wMagicObjectID == 0x0181)
	{
		gpGlobals->g.lprgMagic[wMagicNumber].wBaseDamage = 0;
	}

	if ((short)wMagicDamage > 0 || wBaseDamage > 0)
	{
		PAL_New_ApplyMagicDamageToEnemy(sTarget, wBaseDamage, wMagicObjectID, FALSE);
	}
	gpGlobals->g.lprgMagic[wMagicNumber].wBaseDamage = wMagicDamage;
}

VOID CFight::PAL_New_ApplyMagicDamageToEnemy(
	SHORT		sTarget,
	WORD		wBaseDamage,
	WORD		wMagicObjectID,
	BOOL		fOneDamageAtLeast
)
{
	INT		iDamage;
	int     i, j;
	WORD	def;
	SHORT	sElemResistance[NUM_MAGIC_ELEMENTAL] = { 0 };
	SHORT	sPoisonResistance;
	WORD	wMagicNumber = gpGlobals->g.rgObject[wMagicObjectID].magic.wMagicNumber;

	if ((SHORT)(gpGlobals->g.lprgMagic[wMagicNumber].wBaseDamage) < 0)
	{
		return;
	}

	if (sTarget == -1)
	{	// Apply to all enemies
		for (i = 0; i <= g_Battle.wMaxEnemyIndex; i++)
		{
			if (g_Battle.rgEnemy[i].wObjectID == 0)
			{
				continue;
			}

			def = PAL_New_GetEnemyDefense(i);

			for (j = 0; j < NUM_MAGIC_ELEMENTAL; j++)
			{
				sElemResistance[j] = (PAL_New_GetEnemyElementalResistance((WORD)i, j) - 50) * 1.5;//old 2
			}
			sPoisonResistance = (PAL_New_GetEnemyPoisonResistance(i) - 50) * 1.5;//old 2

			iDamage = PAL_CalcMagicDamage(wBaseDamage, def, sElemResistance, sPoisonResistance, wMagicObjectID);
			iDamage = fOneDamageAtLeast ? std::max(1, iDamage) : std::max(0, iDamage);
			iDamage = std::min(iDamage, MAX_DAMAGE);
			iDamage = std::min(iDamage, (int)g_Battle.rgEnemy[i].dwActualHealth);

			g_Battle.rgEnemy[i].dwActualHealth -= iDamage;
		}
	}
	else
	{	// Apply to one enemy
		def = PAL_New_GetEnemyDefense(sTarget);

		for (j = 0; j < NUM_MAGIC_ELEMENTAL; j++)
		{
			sElemResistance[j] = (PAL_New_GetEnemyElementalResistance((WORD)sTarget, j) - 50) * 2;
		}
		sPoisonResistance = (PAL_New_GetEnemyPoisonResistance((WORD)sTarget) - 50) * 2;

		iDamage = PAL_CalcMagicDamage(wBaseDamage, (WORD)def, sElemResistance, sPoisonResistance, wMagicObjectID);
		iDamage = fOneDamageAtLeast ? std::max(1, iDamage) : std::max(0, iDamage);
		iDamage = std::min(iDamage, MAX_DAMAGE);
		iDamage = std::min(iDamage, (int)g_Battle.rgEnemy[sTarget].dwActualHealth);

		g_Battle.rgEnemy[sTarget].dwActualHealth -= iDamage;
	}
}

void  CFight::PAL_BattleUIUpdate()
/*++
  Purpose:    Update the status of battle UI.
  更新战斗界面的状态。
  Parameters:    None.
  Return value:      None.
  --*/
{

	auto& gpSpriteUI = gpGlobals->gpSpriteUI;

	//当不在玩家选择阶段，计数增加
	if (g_Battle.Phase != BATTLEPHASE::kBattlePhaseSelectAction)
		m_TimeCount++;
	s_iFrame ^= 1;


	//若攻击方式为自动攻击并且不在自动战斗模式下（即在战斗界面中的杂项菜单中选择了“围攻”）
	if (g_Battle.UI.fAutoAttack && !gpGlobals->fAutoBattle)
	{
		//
		// Draw the "auto attack" message if in the autoattack mode.
		//
		if (GetKeyPress() & kKeyMenu)
		{	// 若在自动攻击模式下按菜单键（ESC），则取消自动攻击
			g_Battle.UI.fAutoAttack = FALSE;
		}
		else
		{	// 如果在自动攻击模式下则显示“围攻”
			PAL_DrawText(PAL_GetWord(BATTLEUI_LABEL_AUTO), PAL_XY(280, 10),
				MENUITEM_COLOR_CONFIRMED, TRUE, FALSE);
		}
	}

	if (gpGlobals->fAutoBattle)
	{
		PAL_AutoBattle();
	}

	if (GetKeyPress() & kKeyAuto)
	{
		g_Battle.UI.fAutoAttack = !g_Battle.UI.fAutoAttack;
		g_Battle.UI.MenuState = BATTLEMENUSTATE::kBattleMenuMain;
	}

	if (g_Battle.Phase == BATTLEPHASE::kBattlePhasePerformAction)
	{
		goto end;
	}

	if (!g_Battle.UI.fAutoAttack)
	{
		//// 画玩家信息栏
		// Draw the player info boxes.
		//
		PAL_BattleDrawPlayerinformationBar();
	}

	if (GetKeyPress() & kKeyStatus)
	{
		PAL_PlayerStatus();
		goto end;
	}

	if (cfgShowBtlData)
	{
		//在战斗中显示相关数据开关
		if (GetKeyPress() & kKeyGetInfo)
		{
			gpGlobals->fShowDataInBattle = PAL_SwitchMenu(gpGlobals->fShowDataInBattle);
			goto end;
		}
		//在战斗中显示相关数据
		if (GetKeyPress() & kKeyEnemyInfo)
		{
			PAL_New_EnemyStatus();
			goto end;
		}
	}

	if (g_Battle.UI.state != BATTLEUISTATE::kBattleUIWait)
	{
		auto wPlayerRole = gpGlobals->rgParty[g_Battle.UI.wCurPlayerIndex].wPlayerRole;

		if (gpGlobals->g.PlayerRoles.rgwHP[wPlayerRole] == 0 &&
			gpGlobals->rgPlayerStatus[wPlayerRole][kStatusPuppet])
		{
			g_Battle.UI.wActionType = BATTLEACTIONTYPE::kBattleActionAttack;

			if (PAL_PlayerCanAttackAll(gpGlobals->rgParty[g_Battle.UI.wCurPlayerIndex].wPlayerRole))
			{
				g_Battle.UI.wSelectedIndex = (WORD)-1;
			}
			else
			{
				g_Battle.UI.wSelectedIndex = PAL_BattleSelectAutoTarget();
			}

			PAL_BattleCommitAction(FALSE);
			goto end; // don't go further
		}

		//
		// Cancel any actions if player is dead or sleeping.
		//如果角色已经死亡或者进入睡眠状态则取消其任何动作。
		if (gpGlobals->g.PlayerRoles.rgwHP[wPlayerRole] == 0 ||
			gpGlobals->rgPlayerStatus[wPlayerRole][kStatusSleep] != 0 ||
			gpGlobals->rgPlayerStatus[wPlayerRole][kStatusParalyzed] != 0)
		{
			g_Battle.UI.wActionType = BATTLEACTIONTYPE::kBattleActionPass;
			PAL_BattleCommitAction(FALSE);
			goto end; // don't go further
		}

		if (gpGlobals->rgPlayerStatus[wPlayerRole][kStatusConfused] != 0)
		{
			g_Battle.UI.wActionType = BATTLEACTIONTYPE::kBattleActionAttackMate;
			PAL_BattleCommitAction(FALSE);
			goto end; // don't go further
		}

		if (g_Battle.UI.fAutoAttack)
		{
			g_Battle.UI.wActionType = BATTLEACTIONTYPE::kBattleActionAttack;

			if (PAL_PlayerCanAttackAll(gpGlobals->rgParty[g_Battle.UI.wCurPlayerIndex].wPlayerRole))
			{
				g_Battle.UI.wSelectedIndex = (WORD)-1;
			}
			else
			{
				g_Battle.UI.wSelectedIndex = PAL_BattleSelectAutoTarget();
			}

			PAL_BattleCommitAction(FALSE);
			goto end; // don't go further
		}

		//
		// Draw the arrow on the player's head.
		// 画玩家头上的箭头
		auto i = SPRITENUM_BATTLE_ARROW_CURRENTPLAYER_RED;
		if (s_iFrame & 1)
		{
			i = SPRITENUM_BATTLE_ARROW_CURRENTPLAYER;
		}

		auto x = g_rgPlayerPos[gpGlobals->wMaxPartyMemberIndex][g_Battle.UI.wCurPlayerIndex][0] - 8;
		auto y = g_rgPlayerPos[gpGlobals->wMaxPartyMemberIndex][g_Battle.UI.wCurPlayerIndex][1] - 74;

		PAL_RLEBlitToSurface(PAL_SpriteGetFrame(gpSpriteUI, i), gpTextureReal, PAL_XY(x, y));
	}

	switch (g_Battle.UI.state)
	{
	case BATTLEUISTATE::kBattleUIWait:
	{
		if (!g_Battle.fEnemyCleared)
		{
			for (int i = 0; i <= gpGlobals->wMaxPartyMemberIndex; i++)
			{
				if (g_Battle.rgPlayer[i].state == FIGHTERSTATE::kFighterCom)
				{
					PAL_BattleUIPlayerReady(i);
					break;
				}
			}
		}
		break;
	}

	case BATTLEUISTATE::kBattleUISelectMove:
	{
		//
		// Draw the icons and selecting
		PAL_BattleUISelectMove();

		break;
	}

	case BATTLEUISTATE::kBattleUISelectTargetEnemy:
	{
		PAL_BattleUISelectTargetEnemy();
		break;
	}

	case BATTLEUISTATE::kBattleUISelectTargetPlayer:
	{
		PAL_BattleUISelectTargetPlayer();
		break;
	}
	case BATTLEUISTATE::kBattleUISelectTargetEnemyAll:
	{
		//
		// Don't bother selecting
		//敌人选择目标为全体时无需选择
		g_Battle.UI.wSelectedIndex = (WORD)-1;
		PAL_BattleCommitAction(FALSE);
		break;
	}

	case BATTLEUISTATE::kBattleUISelectTargetPlayerAll:
	{
		//
		// Don't bother selecting
		g_Battle.UI.wSelectedIndex = (WORD)-1;
		PAL_BattleCommitAction(FALSE);
		break;
	}
	}

end:
	//
	// Show the text message if there is one.
	//
	if (cfgShowBtlData)
		//在战斗中显示敌我数据
		if (gpGlobals->fShowDataInBattle && g_Battle.BattleResult == kBattleResultOnGoing)
		{
			PAL_New_BattleUIShowData();
		}
	//
	// Draw the numbers
	for (int i = 0; i < BATTLEUI_MAX_SHOWNUM; i++)
	{
		if (g_Battle.UI.rgShowNum[i].wNum > 0)
		{
			if ((SDL_GetTicks_New() - g_Battle.UI.rgShowNum[i].dwTime) / cfgBtlFrameMs > 10)
			{
				g_Battle.UI.rgShowNum[i].wNum = 0;
			}
			else
			{
				PAL_DrawNumber(g_Battle.UI.rgShowNum[i].wNum, 5,
					PAL_XY(PAL_X(g_Battle.UI.rgShowNum[i].pos),
						PAL_Y(g_Battle.UI.rgShowNum[i].pos) -
						(SDL_GetTicks_New() - g_Battle.UI.rgShowNum[i].dwTime) / cfgBtlFrameMs),
					g_Battle.UI.rgShowNum[i].color, kNumAlignRight);
			}
		}
	}
	//
	if (isNoClassic())
		PAL_ShowProgressBar();
	//// ???
	PAL_DrawNumber(m_TimeCount, 6, PAL_XY(280, 30), kNumColorYellow, kNumAlignRight);

	PAL_ClearKeyState();
}


VOID CFight::PAL_BattleFadeScene()
/*++
  功能：    Fade in the scene of battle.
  参数：    无。
  返回值：    无。
  --*/
{
	RenderPresent(gpTextureRealBak);
	PAL_BattleUIUpdate();
	for (int i = 0; i < 64; i++)
	{
		RenderBlendCopy(gpRenderTexture, gpTextureRealBak);
		RenderBlendCopy(gpRenderTexture, gpTextureReal, i << 2, RenderMode::rmMode0, 0);
		VIDEO_UpdateScreen(gpRenderTexture);
		PAL_Delay(10);
	}
	VIDEO_UpdateScreen(gpTextureReal);
}


VOID CFight::PAL_BattlePlayerEscape(
	VOID
)
/*++
  目的：    Player flee the battle.
  玩家在战斗中逃跑。
  参数：    无。
  返回值：    无。
  --*/
{
	int         i, j;
	WORD        wPlayerRole;

	SOUND_Play(45);

	PAL_BattleUpdateFighters();

	for (i = 0; i <= gpGlobals->wMaxPartyMemberIndex; i++)
	{
		wPlayerRole = gpGlobals->rgParty[i].wPlayerRole;

		if (gpGlobals->g.PlayerRoles.rgwHP[wPlayerRole] > 0)
		{
			g_Battle.rgPlayer[i].wCurrentFrame = 0;
		}
	}

	for (i = 0; i < 16; i++)
	{
		for (j = 0; j <= gpGlobals->wMaxPartyMemberIndex; j++)
		{
			wPlayerRole = gpGlobals->rgParty[j].wPlayerRole;

			if (gpGlobals->g.PlayerRoles.rgwHP[wPlayerRole] > 0)
			{
				//
				// TODO: This is still not the same as the original game
				//
				switch (j)
				{
				case 0:
					if (gpGlobals->wMaxPartyMemberIndex > 0)
					{
						g_Battle.rgPlayer[j].pos =
							PAL_XY(PAL_X(g_Battle.rgPlayer[j].pos) + 4,
								PAL_Y(g_Battle.rgPlayer[j].pos) + 6);
						break;
					}

				case 1:
					g_Battle.rgPlayer[j].pos =
						PAL_XY(PAL_X(g_Battle.rgPlayer[j].pos) + 4,
							PAL_Y(g_Battle.rgPlayer[j].pos) + 4);
					break;

				case 2:
					g_Battle.rgPlayer[j].pos =
						PAL_XY(PAL_X(g_Battle.rgPlayer[j].pos) + 6,
							PAL_Y(g_Battle.rgPlayer[j].pos) + 3);
					break;

				case 3:
					g_Battle.rgPlayer[j].pos =
						PAL_XY(PAL_X(g_Battle.rgPlayer[j].pos) + 7,
							PAL_Y(g_Battle.rgPlayer[j].pos) + 2);
					break;

				case 4:
					g_Battle.rgPlayer[j].pos =
						PAL_XY(PAL_X(g_Battle.rgPlayer[j].pos) + 8,
							PAL_Y(g_Battle.rgPlayer[j].pos) + 1);
					break;

				default:
					assert(FALSE); // Not possible
					break;
				}
			}
		}

		PAL_BattleDelay(1, 0, FALSE);
	}

	//
	// Remove all players from the screen
	// 从屏幕中移走所有角色
	for (i = 0; i <= gpGlobals->wMaxPartyMemberIndex; i++)
	{
		g_Battle.rgPlayer[i].pos = PAL_XY(9999, 9999);
	}

	PAL_BattleDelay(1, 0, FALSE);

	g_Battle.BattleResult = kBattleResultFleed;
}


INT CFight::PAL_CalcPhysicalAttackDamage(
	DWORD wAttackStrength, DWORD wDefense, DWORD wAttackResistance,
	INT		   wPlayerRole
)
{
	//重载，加入我方主动防御加成
	float dag = 1.0;
	if (cfgDefBoostActive)
		if (wPlayerRole >= 0 && g_Battle.rgPlayer[wPlayerRole].fDefenAction && g_Battle.fEnemyMoving)
		{
			wDefense *= 3;
		}

	return dag * PAL_CalcPhysicalAttackDamageBase(wAttackStrength, wDefense, wAttackResistance);
}

const INT CFight::PAL_CalcMagicDamage(
	WORD			wMagicStrength,
	WORD			wDefense,
	SHORT			sElementalResistance[NUM_MAGIC_ELEMENTAL],
	SHORT			sPoisonResistance,
	WORD			wMagicID,
	INT				wPlayerRole
)const
//加入我方主动防御加成
{
	float  def = 1.0;
	int ret = 0;
	if (cfgDefBoostActive)
		if (wPlayerRole >= 0 && g_Battle.rgPlayer[wPlayerRole].fDefenAction && g_Battle.fEnemyMoving)
		{
			//本轮主动防御
			wDefense *= 2;
			def = 0.6;
		}
	ret = PAL_CalcMagicDamageBase(
		wMagicStrength,
		wDefense,
		sElementalResistance,
		sPoisonResistance,
		wMagicID) * def;
	return ret;
}

const INT CFight::PAL_CalcMagicDamageBase(DWORD wMagicStrength,
	DWORD wDefense, SHORT sElementalResistance[NUM_MAGIC_ELEMENTAL], SHORT sPoisonResistance, WORD wMagicID)const
/*++
功能：     Calculate the damage of magic.
计算法术伤害值
参数：     [IN]  wMagicStrength - magic strength of attacker.
[IN]  wDefense - defense value of inflictor.
[IN]  rgwElementalResistance - inflictor's resistance to the elemental magics.
[IN]  wPoisonResistance - inflictor's resistance to poison.
[IN]  wMagicID - object ID of the magic.
攻击者灵力值，被攻击者防御值，被攻击者对各属性伤害的抗性，被攻击者毒抗，法术id
返回值： The damage value of the magic attack.
法术伤害值
--*/
{
	INT             iDamage;
	WORD            wElem;
	WORD	        wBaseDamage;
	WORD			wMagicNumber;

	wMagicNumber = gpGlobals->g.rgObject[wMagicID].magic.wMagicNumber;
	wBaseDamage = gpGlobals->g.lprgMagic[wMagicNumber].wBaseDamage;

	if ((SHORT)wBaseDamage < 0)
	{
		return 0;
	}

	// Formula courtesy of palxex and shenyanduxing
	wMagicStrength *= RandomFloat(1, 1.1);

	iDamage = PAL_CalcBaseDamage(wMagicStrength, wDefense);

	wElem = gpGlobals->g.lprgMagic[wMagicNumber].wElemental;

	if (cfgDmgFormula) //#ifdef cfgDmgFormula
	{
		if (wElem >= 1 && wElem <= 6)
		{
			iDamage /= 2;
			iDamage += wBaseDamage * 2 / 3;
		}
		else
		{
			iDamage /= 3;
			iDamage += wBaseDamage;
		}
	}
	else//#else
	{
		iDamage /= 4;
		iDamage += wBaseDamage;
	}//#endif	


	switch (wElem)
	{
	case 1:
	case 2:
	case 3:
	case 4:
	case 5:
	{
		iDamage *= (100.0 - static_cast<float> (sElementalResistance[wElem - 1])) / 100.0;
		SHORT rgsMagicEffect = 10 + gpGlobals->g.lprgBattleField[gpGlobals->wNumBattleField].rgsMagicEffect[wElem - 1];
		iDamage *= rgsMagicEffect / 10.0;
		break;
	}

	case 6: //毒系
	{
		iDamage *= (100.0 - static_cast<float>(sPoisonResistance)) / 100.0;
		break;
	}

	case 0:
	default:
		break;
	}
	return iDamage;
}

INT CFight::PAL_CalcPhysicalAttackDamageBase(DWORD wAttackStrength, DWORD wDefense, DWORD wAttackResistance)
/*++
功能：    Calculate the damage value of physical attacking.
计算物理攻击伤害值
参数：    [IN]  wAttackStrength - attack strength of attacker.
[IN]  wDefense - defense value of inflictor.
[IN]  wAttackResistance - inflictor's resistance to physical attack.
攻击者武术值，被攻击者防御值，被攻击者物抗
返回值：    The damage value of the physical attacking.
物理攻击伤害值
--*/
{
	INT             iDamage;

	iDamage = PAL_CalcBaseDamage(wAttackStrength, wDefense);
	//wAttackResistance = min(wAttackResistance, 20);
	if (wAttackResistance != 0)
	{
		//iDamage *= iDamage * 10 / wAttackResistance;
		iDamage *= (110.0 - wAttackResistance) / 100.0;
	}

	return iDamage;
}


void CFight::PAL_BattleEnemyPhysical(WORD wEnemyIndex, WORD wPlayerID)
{

	int str, def, iCoverIndex, i, x, y, ex, ey, iSound, iDamage;

	WORD wPlayerRole = gpGlobals->rgParty[wPlayerID].wPlayerRole;


	auto sTarget = wPlayerID;

	WORD wFrameBak = g_Battle.rgPlayer[sTarget].wCurrentFrame;

	str = PAL_New_GetEnemyAttackStrength(wEnemyIndex);
	def = PAL_GetPlayerDefense(wPlayerRole);

	if (g_Battle.rgPlayer[sTarget].fDefending)
	{
		def *= 2;
	}

	SOUND_Play(g_Battle.rgEnemy[wEnemyIndex].e.wAttackSound);

	iCoverIndex = -1;

	auto fAutoDefend = PAL_New_GetTrueByPercentage(cfgAutoDefRatio)
		|| PAL_GetManuallyDefend();



	//
	// Check if the inflictor should be protected
	//
	if ((PAL_IsPlayerDying(wPlayerRole) || !PAL_New_IfPlayerCanMove(wPlayerRole)) && fAutoDefend)
	{
		auto w = gpGlobals->g.PlayerRoles.rgwCoveredBy[wPlayerRole];
		iCoverIndex = PAL_New_GetPlayerIndex(w);

		if (iCoverIndex != -1)
		{
			if (PAL_IsPlayerDying(gpGlobals->rgParty[iCoverIndex].wPlayerRole)
				|| !PAL_New_IfPlayerCanMove(gpGlobals->rgParty[iCoverIndex].wPlayerRole))
			{
				iCoverIndex = -1;
			}
		}
	}

	//
	// If no one can cover the inflictor and inflictor is in a
	// bad status, don't evade
	//
	if (iCoverIndex == -1 && !PAL_New_IfPlayerCanMove(wPlayerRole))
	{
		fAutoDefend = FALSE;
	}

	for (i = 0; i < g_Battle.rgEnemy[wEnemyIndex].e.wMagicFrames; i++)
	{
		g_Battle.rgEnemy[wEnemyIndex].wCurrentFrame =
			g_Battle.rgEnemy[wEnemyIndex].e.wIdleFrames + i;
		PAL_BattleDelay(2, 0, FALSE);
	}

	for (i = 0; i < 3 - g_Battle.rgEnemy[wEnemyIndex].e.wMagicFrames; i++)
	{
		x = PAL_X(g_Battle.rgEnemy[wEnemyIndex].pos) - 2;
		y = PAL_Y(g_Battle.rgEnemy[wEnemyIndex].pos) - 1;
		g_Battle.rgEnemy[wEnemyIndex].pos = PAL_XY(x, y);
		PAL_BattleDelay(1, 0, FALSE);
	}
	//#ifdef PAL_WIN95
	if (!cfgIsWin95 || g_Battle.rgEnemy[wEnemyIndex].e.wActionSound != 0)
		//#endif
	{
		SOUND_Play(g_Battle.rgEnemy[wEnemyIndex].e.wActionSound);
	}
	PAL_BattleDelay(1, 0, FALSE);

	ex = PAL_X(g_Battle.rgPlayer[sTarget].pos) - 44;
	ey = PAL_Y(g_Battle.rgPlayer[sTarget].pos) - 16;

	iSound = g_Battle.rgEnemy[wEnemyIndex].e.wCallSound;

	//#ifdef GAIN_MORE_HIDDEN_EXP
	if (cfgHiddenExp)//更多的隐藏经验
		if (fAutoDefend == TRUE)
		{
			gpGlobals->Exp.rgDefenseExp[wPlayerRole].wCount += RandomLong(0, 1);
			gpGlobals->Exp.rgDexterityExp[wPlayerRole].wCount += RandomLong(0, 1);
			gpGlobals->Exp.rgFleeExp[wPlayerRole].wCount += RandomLong(0, 1);
		}
	//#endif 

	if (iCoverIndex != -1)
	{
		iSound = gpGlobals->g.PlayerRoles.rgwCoverSound[gpGlobals->rgParty[iCoverIndex].wPlayerRole];

		g_Battle.rgPlayer[iCoverIndex].wCurrentFrame = 3;

		x = PAL_X(g_Battle.rgPlayer[sTarget].pos) - 24;
		y = PAL_Y(g_Battle.rgPlayer[sTarget].pos) - 12;

		g_Battle.rgPlayer[iCoverIndex].pos = PAL_XY(x, y);
	}
	else if (fAutoDefend)
	{
		g_Battle.rgPlayer[sTarget].wCurrentFrame = 3;
		iSound = gpGlobals->g.PlayerRoles.rgwCoverSound[wPlayerRole];
	}

	if (g_Battle.rgEnemy[wEnemyIndex].e.wAttackFrames == 0)
	{
		g_Battle.rgEnemy[wEnemyIndex].wCurrentFrame =
			g_Battle.rgEnemy[wEnemyIndex].e.wIdleFrames - 1;

		g_Battle.rgEnemy[wEnemyIndex].pos = PAL_XY(ex, ey);

		PAL_BattleDelay(2, 0, FALSE);
	}
	else
	{
		for (i = 0; i <= g_Battle.rgEnemy[wEnemyIndex].e.wAttackFrames; i++)
		{
			g_Battle.rgEnemy[wEnemyIndex].wCurrentFrame =
				g_Battle.rgEnemy[wEnemyIndex].e.wIdleFrames +
				g_Battle.rgEnemy[wEnemyIndex].e.wMagicFrames + i - 1;

			g_Battle.rgEnemy[wEnemyIndex].pos = PAL_XY(ex, ey);

			PAL_BattleDelay(g_Battle.rgEnemy[wEnemyIndex].e.wActWaitFrames, 0, FALSE);
		}
	}

	//实际打到目标，
	if (!fAutoDefend)
	{
		g_Battle.rgPlayer[sTarget].wCurrentFrame = 4;

		iDamage = PAL_CalcPhysicalAttackDamage(str + RandomLong(0, 2),

			def, 20 + 0.8 * PAL_New_GetPlayerPhysicalResistance(wPlayerRole), wPlayerRole);
		iDamage += RandomLong(0, 1);

		if (gpGlobals->rgPlayerStatus[wPlayerRole][kStatusProtect])
		{
			iDamage /= 2;
		}
		iDamage = std::max(iDamage, 1);
		iDamage = std::min(iDamage, (int)gpGlobals->g.PlayerRoles.rgwHP[wPlayerRole]);

		//#ifndef INVINCIBLE
		if (!debugSwitch[fIsInvincible])
			gpGlobals->g.PlayerRoles.rgwHP[wPlayerRole] -= iDamage;
		//#endif

		PAL_BattleDisplayStatChange();

		g_Battle.rgPlayer[sTarget].iColorShift = 6;
	}

	//#ifdef PAL_WIN95
			//if (iSound != 0)
	if (!cfgIsWin95 || iSound != 0)
		//#endif
	{
		SOUND_Play(iSound);
	}

	PAL_BattleDelay(1, 0, FALSE);

	g_Battle.rgPlayer[sTarget].iColorShift = 0;

	if (iCoverIndex != -1)
	{
		g_Battle.rgEnemy[wEnemyIndex].pos =
			PAL_XY(PAL_X(g_Battle.rgEnemy[wEnemyIndex].pos) - 10,
				PAL_Y(g_Battle.rgEnemy[wEnemyIndex].pos) - 8);
		g_Battle.rgPlayer[iCoverIndex].pos =
			PAL_XY(PAL_X(g_Battle.rgPlayer[iCoverIndex].pos) + 4,
				PAL_Y(g_Battle.rgPlayer[iCoverIndex].pos) + 2);
	}
	else
	{
		g_Battle.rgPlayer[sTarget].pos =
			PAL_XY(PAL_X(g_Battle.rgPlayer[sTarget].pos) + 8,
				PAL_Y(g_Battle.rgPlayer[sTarget].pos) + 4);
	}

	PAL_BattleDelay(1, 0, FALSE);

	if (gpGlobals->g.PlayerRoles.rgwHP[wPlayerRole] == 0)
	{
		SOUND_Play(gpGlobals->g.PlayerRoles.rgwDeathSound[wPlayerRole]);
		wFrameBak = 2;
	}
	else if (PAL_IsPlayerDying(wPlayerRole))
	{
		wFrameBak = 1;
	}

	if (iCoverIndex == -1)
	{
		g_Battle.rgPlayer[sTarget].pos =
			PAL_XY(PAL_X(g_Battle.rgPlayer[sTarget].pos) + 2,
				PAL_Y(g_Battle.rgPlayer[sTarget].pos) + 1);
	}

	PAL_BattleDelay(3, 0, FALSE);

	g_Battle.rgEnemy[wEnemyIndex].pos = g_Battle.rgEnemy[wEnemyIndex].posOriginal;
	g_Battle.rgEnemy[wEnemyIndex].wCurrentFrame = 0;

	PAL_BattleDelay(1, 0, FALSE);

	g_Battle.rgPlayer[sTarget].wCurrentFrame = wFrameBak;
	PAL_BattleDelay(1, 0, TRUE);

	g_Battle.rgPlayer[sTarget].pos = g_Battle.rgPlayer[sTarget].posOriginal;
	PAL_BattleDelay(4, 0, TRUE);

	PAL_BattleUpdateFighters();

	if (iCoverIndex == -1 && !fAutoDefend &&
		g_Battle.rgEnemy[wEnemyIndex].e.wAttackEquivItemRate >= RandomLong(1, 10))
	{
		i = g_Battle.rgEnemy[wEnemyIndex].e.wAttackEquivItem;
		gpGlobals->g.rgObject[i].item.wScriptOnUse =
			PAL_RunTriggerScript(gpGlobals->g.rgObject[i].item.wScriptOnUse, wPlayerRole);
	}

	PAL_BattlePostActionCheck(TRUE);

}

const INT CFight::PAL_GetManuallyDefend()const
{
	//功能：返回手动防御，成功返回 TRUE ，不成功返回0
	auto k = (cfgAutoDefRatio == 0
		&& (SDL_GetTicks_New() - lastKeyDownTime) < 200);
	return k;
}

void CFight::PAL_AutoBattle()
{
	//战斗自动进行
	// 当角色状态时可以接受命令时
	for (int i = 0; i <= gpGlobals->wMaxPartyMemberIndex; i++)
	{
		if (g_Battle.rgPlayer[i].state == FIGHTERSTATE::kFighterCom)
		{
			break;
		}
	}

	if (g_Battle.UI.state != BATTLEUISTATE::kBattleUIWait)
	{
		auto w = PAL_BattleUIPickAutoMagic(gpGlobals->rgParty[g_Battle.UI.wCurPlayerIndex].wPlayerRole, 9999);

		if (w == 0)
		{
			g_Battle.UI.wActionType = BATTLEACTIONTYPE::kBattleActionAttack;
			g_Battle.UI.wSelectedIndex = PAL_BattleSelectAutoTarget();
		}
		else
		{
			g_Battle.UI.wActionType = BATTLEACTIONTYPE::kBattleActionMagic;
			g_Battle.UI.wObjectID = w;

			if (gpGlobals->g.rgObject[w].magic.wFlags & kMagicFlagApplyToAll)
			{
				g_Battle.UI.wSelectedIndex = (WORD)-1;
			}
			else
			{
				g_Battle.UI.wSelectedIndex = PAL_BattleSelectAutoTarget();
			}
		}

		PAL_BattleCommitAction(FALSE);
	}
}

void CFight::PAL_BattleProcessPlayerAction(int iPlayerRole)
{
	//如果玩家充能未完成，跳过

	auto i = iPlayerRole;
	
	auto wPlayerRole = gpGlobals->rgParty[i].wPlayerRole;

	// Update statuses
	for (int j = 0; j < kStatusAll; j++)
	{
		if (gpGlobals->rgPlayerStatus[wPlayerRole][j] > 0)
		{
			gpGlobals->rgPlayerStatus[wPlayerRole][j]--;
		}
	}
	//主动防御加强
	if (cfgDefBoostActive > 0)
		//每一轮减少
		if (g_Battle.rgPlayer[i].fDefenAction)
			g_Battle.rgPlayer[i].fDefenAction--;
	if (cfgDefBoostActive < 0)
		cfgDefBoostActive = 0;

	//
	// Run poison scripts
	// 执行中毒脚本前、后都需要排序 我方
	PAL_New_SortPoisonsForPlayerByLevel(wPlayerRole);
	for (int j = 0; j < MAX_POISONS; j++)
	{
		if (gpGlobals->rgPoisonStatus[j][i].wPoisonID != 0
			&& gpGlobals->g.PlayerRoles.rgwHP[wPlayerRole] != 0
			)
		{
			if (cfgPoisonUp) //增加毒的烈度
			{
				auto dwPrevHP = gpGlobals->g.PlayerRoles.rgwHP[wPlayerRole];
				auto dwPrevMP = gpGlobals->g.PlayerRoles.rgwMP[wPlayerRole];

				gpGlobals->rgPoisonStatus[j][i].wPoisonScript =
					PAL_RunTriggerScript(gpGlobals->rgPoisonStatus[j][i].wPoisonScript, wPlayerRole);

				auto iExtraPoisonDamage = dwPrevHP - gpGlobals->g.PlayerRoles.rgwHP[wPlayerRole];
				auto iExtraPoisonMPDamage = dwPrevMP - gpGlobals->g.PlayerRoles.rgwMP[wPlayerRole];

				iExtraPoisonDamage = std::max(iExtraPoisonDamage, 0);
				iExtraPoisonMPDamage = std::max(iExtraPoisonMPDamage, 0);
				iExtraPoisonDamage *= gpGlobals->rgPoisonStatus[j][i].wPoisonIntensity;
				iExtraPoisonMPDamage *= gpGlobals->rgPoisonStatus[j][i].wPoisonIntensity;
				iExtraPoisonDamage = std::min(iExtraPoisonDamage, MAX_DAMAGE);
				iExtraPoisonMPDamage = std::min(iExtraPoisonMPDamage, MAX_DAMAGE);
				PAL_IncreaseHPMP(wPlayerRole, -(SHORT)iExtraPoisonDamage, -(SHORT)iExtraPoisonMPDamage);
			}
			else
				gpGlobals->rgPoisonStatus[j][i].wPoisonScript =
				PAL_RunTriggerScript(gpGlobals->rgPoisonStatus[j][i].wPoisonScript, wPlayerRole);
		}
	}
	PAL_New_SortPoisonsForPlayerByLevel(wPlayerRole);

	//如果死亡了并且不是傀儡虫控制状态，则移除所有的状态
	if (gpGlobals->g.PlayerRoles.rgwHP[wPlayerRole] == 0
		&& gpGlobals->rgPlayerStatus[wPlayerRole][kStatusPuppet] == 0)
	{
		PAL_New_RemovePlayerAllStatus(wPlayerRole);
	}
}

void CFight::PAL_BattleProcessEnemyAction(int enemy)
{
	auto i = enemy;
	// Update statuses
	for (int j = 0; j < kStatusAll; j++)
	{
		if (g_Battle.rgEnemy[i].rgwStatus[j] > 0)
		{
			g_Battle.rgEnemy[i].rgwStatus[j]--;
		}
	}
	//
	// Run poison scripts
	// 执行中毒脚本前、后都需要排序  敌方
	PAL_New_SortPoisonsForEnemyByLevel(i);
	for (int j = 0; j < MAX_POISONS; j++)
	{
		WORD wPoisonID = g_Battle.rgEnemy[i].rgPoisons[j].wPoisonID;
		if (wPoisonID != 0)
		{
			if (cfgPoisonUp)
			{
				auto dwPrevHP = g_Battle.rgEnemy[i].dwActualHealth;
				g_Battle.rgEnemy[i].rgPoisons[j].wPoisonScript =
					PAL_RunTriggerScript(g_Battle.rgEnemy[i].rgPoisons[j].wPoisonScript, (WORD)i);
				auto iExtraPoisonDamage = dwPrevHP - g_Battle.rgEnemy[i].dwActualHealth;
				WORD wPoisonIntensity = g_Battle.rgEnemy[i].rgPoisons[j].wPoisonIntensity;
				if (iExtraPoisonDamage > 0 && wPoisonIntensity > 0)
				{
					iExtraPoisonDamage *= wPoisonIntensity;
					iExtraPoisonDamage = std::min((int)iExtraPoisonDamage, (int)g_Battle.rgEnemy[i].dwActualHealth);
					g_Battle.rgEnemy[i].dwActualHealth -= iExtraPoisonDamage;
				}
			}
			else
				//#else
				g_Battle.rgEnemy[i].rgPoisons[j].wPoisonScript =
				PAL_RunTriggerScript(g_Battle.rgEnemy[i].rgPoisons[j].wPoisonScript, (WORD)i);
			//#endif
		}
	}
	PAL_New_SortPoisonsForEnemyByLevel(i);

}

void CFight::PAL_BattleDrawPlayerinformationBar()
{
	//// 画玩家信息栏
	// Draw the player info boxes.
	//
	for (int i = 0; i <= gpGlobals->wMaxPartyMemberIndex; i++)
	{
		auto wPlayerRole = gpGlobals->rgParty[i].wPlayerRole;
		int w = 0;
		auto j = TIMEMETER_COLOR_DEFAULT;

		if (gpGlobals->rgPlayerStatus[wPlayerRole][kStatusSleep] != 0 ||
			gpGlobals->rgPlayerStatus[wPlayerRole][kStatusConfused] != 0 ||
			gpGlobals->rgPlayerStatus[wPlayerRole][kStatusPuppet] != 0)
		{
			w = 0;
		}

		// 更改角色信息栏的位置。在屏幕下方
		if (gpGlobals->wMaxPartyMemberIndex >= 3)
		{
			if (i > 3)
			{
				PAL_PlayerInfoBox(PAL_XY(91 + 77 * 2, 165 - (i - 3) * 38), wPlayerRole, w, j, FALSE);
				continue;
			}

			PAL_PlayerInfoBox(PAL_XY(14 + 77 * i, 165), wPlayerRole, w, j, FALSE);
			continue;
		}

		PAL_PlayerInfoBox(PAL_XY(91 + 77 * i, 165), wPlayerRole,
			w, j, FALSE);
	}
}

void CFight::PAL_BattleDrawIconAndSelect()
{
	//画菜单图标并选择
	//选择结果值为g_Battle.UI.MenuState中的值
	auto& gpSpriteUI = gpGlobals->gpSpriteUI;
	struct
	{
		int               iSpriteNum;
		PAL_POS           pos;
		BATTLEUIACTION    action;
	} rgItems[] =
	{// 4个战斗界面中的菜单
		{SPRITENUM_BATTLEICON_ATTACK, PAL_XY(27, 140), BATTLEUIACTION::kBattleUIActionAttack},
		{SPRITENUM_BATTLEICON_MAGIC, PAL_XY(0, 155), BATTLEUIACTION::kBattleUIActionMagic},
		{SPRITENUM_BATTLEICON_COOPMAGIC, PAL_XY(54, 155), BATTLEUIACTION::kBattleUIActionCoopMagic},
		{SPRITENUM_BATTLEICON_MISCMENU, PAL_XY(27, 170), BATTLEUIACTION::kBattleUIActionMisc}
	};
	std::vector<PAL_Rect> itemRects;
	for (int n = 0; n < 4; n++)
	{
		itemRects.push_back(PAL_Rect(rgItems[n].pos, PalSize(25, 25)));
	}
	//多于4个人的时候调整战斗菜单盘的位置
	if (gpGlobals->wMaxPartyMemberIndex >= 3)
	{
		for (int i = 0; i < 4; i++)
		{
			rgItems[i].pos -= (37 << 16);//向上移动37像素
		}
	}
	// 用鼠标选择战斗菜单
	if (g_InputMouse)
	{
		if (int item = isRectsAtPoint(itemRects, g_InputMouse))
		{
			if (g_Battle.UI.wSelectedAction == item - 1)
			{
				//已经选择 模拟空格键
				SetKeyPress(kKeySearch);
			}
			else {
				g_Battle.UI.wSelectedAction = item - 1;
			}
		}
	}
	// 用上下左右键来选择4个菜单
	else if (g_Battle.UI.MenuState == BATTLEMENUSTATE::kBattleMenuMain)
	{
		if (getDirEction() == kDirNorth)
		{
			g_Battle.UI.wSelectedAction = 0;
		}
		else if (getDirEction() == kDirSouth)
		{
			g_Battle.UI.wSelectedAction = 3;
		}
		else if (getDirEction() == kDirWest)
		{
			if (PAL_BattleUIIsActionValid(BATTLEUIACTION::kBattleUIActionMagic))
			{
				g_Battle.UI.wSelectedAction = 1;
			}
		}
		else if (getDirEction() == kDirEast)
		{
			if (PAL_BattleUIIsActionValid(BATTLEUIACTION::kBattleUIActionCoopMagic))
			{
				g_Battle.UI.wSelectedAction = 2;
			}
		}
	}

	if (!PAL_BattleUIIsActionValid(rgItems[g_Battle.UI.wSelectedAction].action))
	{
		g_Battle.UI.wSelectedAction = 0;
	}

	for (int i = 0; i < 4; i++)
	{
		//
		if (g_Battle.UI.wSelectedAction == i)
		{
			PAL_RLEBlitToSurface(PAL_SpriteGetFrame(gpSpriteUI, rgItems[i].iSpriteNum),
				gpTextureReal, rgItems[i].pos);
		}
		else if (PAL_BattleUIIsActionValid(rgItems[i].action))
		{
			PAL_RLEBlitMonoColor(PAL_SpriteGetFrame(gpSpriteUI, rgItems[i].iSpriteNum),
				gpTextureReal, rgItems[i].pos, 0, -4);
		}
		else
		{
			PAL_RLEBlitMonoColor(PAL_SpriteGetFrame(gpSpriteUI, rgItems[i].iSpriteNum),
				gpTextureReal, rgItems[i].pos, 0x10, -4);
		}
	}
}

void CFight::PAL_BattleUISelectMove()
{
	PAL_BattleDrawIconAndSelect();

	switch (g_Battle.UI.MenuState)
	{
	case BATTLEMENUSTATE::kBattleMenuMain:
	{
		PAL_BattleMenuMain();
		break;
	}

	case BATTLEMENUSTATE::kBattleMenuMagicSelect:
	{
		auto w = PAL_MagicSelectionMenuUpdate();
		if (w == 0xFFFF)
			break;

		g_Battle.UI.MenuState = BATTLEMENUSTATE::kBattleMenuMain;

		//决定魔法目标类型
		if (w != 0)
		{
			g_Battle.UI.wActionType = BATTLEACTIONTYPE::kBattleActionMagic;
			g_Battle.UI.wObjectID = w;

			if (gpGlobals->g.rgObject[w].magic.wFlags & kMagicFlagUsableToEnemy)
			{
				if (gpGlobals->g.rgObject[w].magic.wFlags & kMagicFlagApplyToAll)
				{
					g_Battle.UI.state = BATTLEUISTATE::kBattleUISelectTargetEnemyAll;
				}
				else
				{
					g_Battle.UI.wSelectedIndex = g_Battle.UI.wPrevEnemyTarget;
					g_Battle.UI.state = BATTLEUISTATE::kBattleUISelectTargetEnemy;
				}
			}
			else
			{
				if (gpGlobals->g.rgObject[w].magic.wFlags & kMagicFlagApplyToAll)
				{
					g_Battle.UI.state = BATTLEUISTATE::kBattleUISelectTargetPlayerAll;
				}
				else
				{
					g_Battle.UI.wSelectedIndex = 0;

					g_Battle.UI.state = BATTLEUISTATE::kBattleUISelectTargetPlayer;
				}
			}
		}

		break;
	}

	case BATTLEMENUSTATE::kBattleMenuUseItemSelect:
		PAL_BattleUIUseItem();
		break;

	case BATTLEMENUSTATE::kBattleMenuThrowItemSelect:
		PAL_BattleUIThrowItem();
		break;

	case BATTLEMENUSTATE::kBattleMenuMisc:
	{
		auto w = PAL_BattleUIMiscMenuUpdate();

		if (w != 0xFFFF)
		{
			g_Battle.UI.MenuState = BATTLEMENUSTATE::kBattleMenuMain;

			switch (w)
			{
			case 1: // auto   围攻
				g_Battle.UI.fAutoAttack = TRUE;
				break;

			case 2: // item  物品
				g_Battle.UI.MenuState = BATTLEMENUSTATE::kBattleMenuMiscItemSubMenu;
				g_iCurSubMenuItem = 0;
				break;

			case 3: // defend  防御
				g_Battle.UI.wActionType = BATTLEACTIONTYPE::kBattleActionDefend;
				PAL_BattleCommitAction(FALSE);
				break;

			case 4: // flee    逃跑
				g_Battle.UI.wActionType = BATTLEACTIONTYPE::kBattleActionFlee;
				PAL_BattleCommitAction(FALSE);
				break;

			case 5: // status   状态
				PAL_PlayerStatus();
				break;
			case 6: // 显示数据开关
				if (cfgShowBtlData) {
					gpGlobals->fShowDataInBattle =
						PAL_SwitchMenu(gpGlobals->fShowDataInBattle);
				}
				break;

#ifdef SHOW_ENEMY_STATUS
			case 7: // 显示敌方状态
				PAL_New_EnemyStatus();
				break;
#endif								
			}
		}
		break;
	}


	case BATTLEMENUSTATE::kBattleMenuMiscItemSubMenu:
	{
#ifdef SORT_INVENTORY
		PAL_New_SortInventory();
#endif
		auto w = PAL_BattleUIMiscItemSubMenuUpdate();

		if (w != 0xFFFF)
		{
			g_Battle.UI.MenuState = BATTLEMENUSTATE::kBattleMenuMain;

			switch (w)
			{
			case 1: // use  // 使用（物品）
				g_Battle.UI.MenuState = BATTLEMENUSTATE::kBattleMenuUseItemSelect;
				PAL_ItemSelectMenuInit(kItemFlagUsable);
				break;

			case 2: // throw	// 投掷（物品）
				g_Battle.UI.MenuState = BATTLEMENUSTATE::kBattleMenuThrowItemSelect;
				PAL_ItemSelectMenuInit(kItemFlagThrowable);
				break;
			}
		}
		break;
	}
	}

}

void CFight::PAL_BattleMenuMain()
{
	if (GetKeyPress() & kKeySearch)
	{
		switch (g_Battle.UI.wSelectedAction)
		{
			//选择物攻
		case 0:
		{
			g_Battle.UI.wActionType = BATTLEACTIONTYPE::kBattleActionAttack;
			if (PAL_PlayerCanAttackAll(gpGlobals->rgParty[g_Battle.UI.wCurPlayerIndex].wPlayerRole))
			{
				g_Battle.UI.state = BATTLEUISTATE::kBattleUISelectTargetEnemyAll;
			}
			else
			{
				g_Battle.UI.wSelectedIndex = g_Battle.UI.wPrevEnemyTarget;
				g_Battle.UI.state = BATTLEUISTATE::kBattleUISelectTargetEnemy;
			}
			break;
		}

		//选择仙术
		case 1:
		{
			auto wPlayerRole = gpGlobals->rgParty[
				g_Battle.UI.wCurPlayerIndex].wPlayerRole;

			g_Battle.UI.MenuState = BATTLEMENUSTATE::kBattleMenuMagicSelect;
			PAL_MagicSelectionMenuInit(wPlayerRole, TRUE, 0);
			break;
		}

		//选择合体法术
		case 2:
		{
			auto w = gpGlobals->rgParty[g_Battle.UI.wCurPlayerIndex].wPlayerRole;
			w = PAL_GetPlayerCooperativeMagic(w);

			g_Battle.UI.wActionType = BATTLEACTIONTYPE::kBattleActionCoopMagic;
			g_Battle.UI.wObjectID = w;

			if (gpGlobals->g.rgObject[w].magic.wFlags & kMagicFlagUsableToEnemy)
			{
				if (gpGlobals->g.rgObject[w].magic.wFlags & kMagicFlagApplyToAll)
				{
					g_Battle.UI.state = BATTLEUISTATE::kBattleUISelectTargetEnemyAll;
				}
				else
				{
					g_Battle.UI.wSelectedIndex = g_Battle.UI.wPrevEnemyTarget;
					g_Battle.UI.state = BATTLEUISTATE::kBattleUISelectTargetEnemy;
				}
			}
			else
			{
				if (gpGlobals->g.rgObject[w].magic.wFlags & kMagicFlagApplyToAll)
				{
					g_Battle.UI.state = BATTLEUISTATE::kBattleUISelectTargetPlayerAll;
				}
				else
				{
					g_Battle.UI.wSelectedIndex = 0;
					g_Battle.UI.state = BATTLEUISTATE::kBattleUISelectTargetPlayer;
				}
			}
			break;
		}

		//选择杂项菜单
		case 3:
		{
			g_Battle.UI.MenuState = BATTLEMENUSTATE::kBattleMenuMisc;
			g_iCurMiscMenuItem = 0;
			break;
		}
		}
	}
	else if (GetKeyPress() & kKeyDefend)
	{
		g_Battle.UI.wActionType = BATTLEACTIONTYPE::kBattleActionDefend;
		PAL_BattleCommitAction(FALSE);
	}
	else if (GetKeyPress() & kKeyForce)
	{
		auto w = PAL_BattleUIPickAutoMagic(gpGlobals->rgParty[g_Battle.UI.wCurPlayerIndex].wPlayerRole, 60);

		if (w == 0)
		{
			g_Battle.UI.wActionType = BATTLEACTIONTYPE::kBattleActionAttack;

			if (PAL_PlayerCanAttackAll(gpGlobals->rgParty[g_Battle.UI.wCurPlayerIndex].wPlayerRole))
			{
				g_Battle.UI.wSelectedIndex = (WORD)-1;
			}
			else
			{
				g_Battle.UI.wSelectedIndex = PAL_BattleSelectAutoTarget();
			}
		}
		else
		{
			g_Battle.UI.wActionType = BATTLEACTIONTYPE::kBattleActionMagic;
			g_Battle.UI.wObjectID = w;

			if (gpGlobals->g.rgObject[w].magic.wFlags & kMagicFlagApplyToAll)
			{
				g_Battle.UI.wSelectedIndex = (WORD)-1;
			}
			else
			{
				g_Battle.UI.wSelectedIndex = PAL_BattleSelectAutoTarget();
			}
		}

		PAL_BattleCommitAction(FALSE);
	}
	else if (GetKeyPress() & kKeyFlee)
	{
		if (g_Battle.fIsBoss && cfgBossRetryQ)
		{
			//与BOSS战斗时 Q 键直接重新开始
			if (PAL_ConfirmMenu("从战斗中退出吗？")) {
				PAL_BattleCommitAction(FALSE);
				g_Battle.BattleResult = kBattleResultStop;
				return;
			}

		}
		g_Battle.UI.wActionType = BATTLEACTIONTYPE::kBattleActionFlee;
		PAL_BattleCommitAction(FALSE);
	}
	else if (GetKeyPress() & kKeyUseItem)
	{
		g_Battle.UI.MenuState = BATTLEMENUSTATE::kBattleMenuUseItemSelect;
		PAL_ItemSelectMenuInit(kItemFlagUsable);
	}
	else if (GetKeyPress() & kKeyThrowItem)
	{
		g_Battle.UI.MenuState = BATTLEMENUSTATE::kBattleMenuThrowItemSelect;
		PAL_ItemSelectMenuInit(kItemFlagThrowable);
	}
	else if (GetKeyPress() & kKeyRepeat)
	{
		PAL_BattleCommitAction(TRUE);
	}

	else if (GetKeyPress() & kKeyMenu)
	{
		g_Battle.rgPlayer[g_Battle.UI.wCurPlayerIndex].state = FIGHTERSTATE::kFighterWait;
		g_Battle.UI.state = BATTLEUISTATE::kBattleUIWait;

		if (g_Battle.UI.wCurPlayerIndex > 0)
		{
			//
			// Revert to the previous player
			//恢复到之前的角色
			do
			{
				g_Battle.rgPlayer[--g_Battle.UI.wCurPlayerIndex].state = FIGHTERSTATE::kFighterWait;

				if (g_Battle.rgPlayer[g_Battle.UI.wCurPlayerIndex].action.ActionType == BATTLEACTIONTYPE::kBattleActionThrowItem)
				{
					for (int i = 0; i < MAX_INVENTORY; i++)
					{
						if (gpGlobals->rgInventory[i].wItem ==
							g_Battle.rgPlayer[g_Battle.UI.wCurPlayerIndex].action.wActionID)
						{
							gpGlobals->rgInventory[i].nAmountInUse--;
							break;
						}
					}
				}
				else if (g_Battle.rgPlayer[g_Battle.UI.wCurPlayerIndex].action.ActionType == BATTLEACTIONTYPE::kBattleActionUseItem)
				{
					if (gpGlobals->g.rgObject[g_Battle.rgPlayer[g_Battle.UI.wCurPlayerIndex].action.wActionID].item.wFlags & kItemFlagConsuming)
					{
						for (int i = 0; i < MAX_INVENTORY; i++)
						{
							if (gpGlobals->rgInventory[i].wItem ==
								g_Battle.rgPlayer[g_Battle.UI.wCurPlayerIndex].action.wActionID)
							{
								gpGlobals->rgInventory[i].nAmountInUse--;
								break;
							}
						}
					}
				}
			} while (g_Battle.UI.wCurPlayerIndex > 0 &&
				(gpGlobals->g.PlayerRoles.rgwHP[gpGlobals->rgParty[g_Battle.UI.wCurPlayerIndex].wPlayerRole] == 0 ||
					gpGlobals->rgPlayerStatus[gpGlobals->rgParty[g_Battle.UI.wCurPlayerIndex].wPlayerRole][kStatusConfused] > 0 ||
					gpGlobals->rgPlayerStatus[gpGlobals->rgParty[g_Battle.UI.wCurPlayerIndex].wPlayerRole][kStatusSleep] > 0 ||
					gpGlobals->rgPlayerStatus[gpGlobals->rgParty[g_Battle.UI.wCurPlayerIndex].wPlayerRole][kStatusParalyzed] > 0));
		}
	}
}

VOID CFight::PAL_BattleUIPlayerReady(WORD wPlayerIndex)
/*++
Purpose:

Start the action selection menu of the specified player.
开始选定角色的动作选择菜单。
Parameters:    [IN]  wPlayerIndex - the player index.
角色的序号
Return value:    None.
--*/
{
	//When the real-time battle screen is in the status of a select action
	g_Battle.UI.wCurPlayerIndex = wPlayerIndex;
	g_Battle.UI.state = BATTLEUISTATE::kBattleUISelectMove;
	g_Battle.UI.wSelectedAction = 0;
	g_Battle.UI.MenuState = BATTLEMENUSTATE::kBattleMenuMain;

}

void CFight::PAL_BattleUISelectTargetEnemy()

{
	int x = -1;
	int y = 0;

	for (int i = 0; i <= g_Battle.wMaxEnemyIndex; i++)
	{
		if (g_Battle.rgEnemy[i].wObjectID != 0)
		{
			x = i;
			y++;
		}
	}

	if (x == -1)
	{
		g_Battle.UI.state = BATTLEUISTATE::kBattleUISelectMove;
		return;
	}

	if (g_Battle.UI.wActionType == BATTLEACTIONTYPE::kBattleActionCoopMagic)
	{
		if (!PAL_BattleUIIsActionValid(BATTLEUIACTION::kBattleUIActionCoopMagic))
		{
			g_Battle.UI.state = BATTLEUISTATE::kBattleUISelectMove;
			return;
		}
	}

	//
	// Don't bother selecting when only 1 enemy left
	if (y == 1)
	{
		g_Battle.UI.wPrevEnemyTarget = (WORD)x;
		g_Battle.UI.wSelectedIndex = g_Battle.UI.wPrevEnemyTarget;
		PAL_BattleCommitAction(FALSE);
		return;
	}
	g_Battle.UI.wSelectedIndex = std::min((int)g_Battle.UI.wSelectedIndex, x);

	for (int i = 0; i <= x; i++)
	{
		if (g_Battle.rgEnemy[g_Battle.UI.wSelectedIndex].wObjectID != 0)
		{
			break;
		}
		g_Battle.UI.wSelectedIndex++;
		g_Battle.UI.wSelectedIndex %= x + 1;
	}

	//
	// Highlight the selected enemy
	//  使选中的敌人高亮
	if ((s_iFrame) & 1)
	{
		auto i = g_Battle.UI.wSelectedIndex;

		x = PAL_X(g_Battle.rgEnemy[i].pos);
		y = PAL_Y(g_Battle.rgEnemy[i].pos);

		x -= PAL_RLEGetWidth(PAL_SpriteGetFrame(g_Battle.rgEnemy[i].lpSprite, g_Battle.rgEnemy[i].wCurrentFrame)) / 2;
		y -= PAL_RLEGetHeight(PAL_SpriteGetFrame(g_Battle.rgEnemy[i].lpSprite, g_Battle.rgEnemy[i].wCurrentFrame));

		PAL_RLEBlitWithColorShift(PAL_SpriteGetFrame(g_Battle.rgEnemy[i].lpSprite, g_Battle.rgEnemy[i].wCurrentFrame),
			gpTextureReal, PAL_XY(x, y), 7);
	}

	x = g_Battle.wMaxEnemyIndex;

	std::vector<PAL_Rect> itemRects;
	for (int i = 0; i <= x; i++)
	{
		if (g_Battle.rgEnemy[i].wObjectID == 0)
			continue;
		auto ex = PAL_X(g_Battle.rgEnemy[i].pos);
		auto ey = PAL_Y(g_Battle.rgEnemy[i].pos);
		auto  w = PAL_RLEGetWidth(PAL_SpriteGetFrame(g_Battle.rgEnemy[i].lpSprite, g_Battle.rgEnemy[i].wCurrentFrame));
		auto  h = PAL_RLEGetHeight(PAL_SpriteGetFrame(g_Battle.rgEnemy[i].lpSprite, g_Battle.rgEnemy[i].wCurrentFrame));
		if (w > 60) w = 60;
		if (y > 80) h = 80;
		itemRects.push_back(PAL_Rect(PAL_XY(ex - w / 2, ey - h), PalSize(w, h)));
	}
	// Use mouse to select enemy
	if (g_InputMouse)
	{
		if (int item = isRectsAtPoint(itemRects, g_InputMouse))
		{
			if (g_Battle.UI.wSelectedIndex == item - 1)
			{
				//已经选择 模拟空格键
				SetKeyPress(kKeySearch);
			}
			else {
				g_Battle.UI.wSelectedIndex = item - 1;
			}
		}
	}

	if (GetKeyPress() & kKeyMenu)
	{
		g_Battle.UI.state = BATTLEUISTATE::kBattleUISelectMove;
	}
	else if (GetKeyPress() & kKeySearch)
	{
		g_Battle.UI.wPrevEnemyTarget = g_Battle.UI.wSelectedIndex;
		PAL_BattleCommitAction(FALSE);
	}
	else if (GetKeyPress() & (kKeyLeft | kKeyDown))
	{
		do
		{
			if (g_Battle.UI.wSelectedIndex == 0 || g_Battle.UI.wSelectedIndex > x)
				g_Battle.UI.wSelectedIndex = x;
			else
			{
				g_Battle.UI.wSelectedIndex--;
			}
		} while (g_Battle.rgEnemy[g_Battle.UI.wSelectedIndex].wObjectID == 0);
	}
	else if (GetKeyPress() & (kKeyRight | kKeyUp))
	{
		do
		{
			if (g_Battle.UI.wSelectedIndex == x)
				g_Battle.UI.wSelectedIndex = 0;
			else
				g_Battle.UI.wSelectedIndex++;

		} while (g_Battle.rgEnemy[g_Battle.UI.wSelectedIndex].wObjectID == 0);
	}
}
void CFight::PAL_BattleUISelectTargetPlayer()

{
	//
	// Don't bother selecting when only 1 player is in the party
	if (gpGlobals->wMaxPartyMemberIndex == 0)
	{
		g_Battle.UI.wSelectedIndex = 0;
		PAL_BattleCommitAction(FALSE);
	}

	auto j = SPRITENUM_BATTLE_ARROW_SELECTEDPLAYER;
	if (s_iFrame & 1)
	{
		j = SPRITENUM_BATTLE_ARROW_SELECTEDPLAYER_RED;
	}

	//
	// Draw arrows on the selected player
	//在选中的角色头上画箭头
	auto x = g_rgPlayerPos[gpGlobals->wMaxPartyMemberIndex][g_Battle.UI.wSelectedIndex][0] - 8;
	auto y = g_rgPlayerPos[gpGlobals->wMaxPartyMemberIndex][g_Battle.UI.wSelectedIndex][1] - 67;

	auto& gpSpriteUI = gpGlobals->gpSpriteUI;

	PAL_RLEBlitToSurface(PAL_SpriteGetFrame(gpSpriteUI, j), gpTextureReal, PAL_XY(x, y));

	std::vector<PAL_Rect> itemRects;
	for (int i = gpGlobals->wMaxPartyMemberIndex; i >= 0; i--)
	{
		auto pos = g_Battle.rgPlayer[i].pos;
		auto lBMR = PAL_SpriteGetFrame(g_Battle.rgPlayer[i].lpSprite, g_Battle.rgPlayer[i].wCurrentFrame);
		pos = PAL_XY(PAL_X(pos) - PAL_RLEGetWidth(lBMR) / 2, PAL_Y(pos) - PAL_RLEGetHeight(lBMR));
		PalSize size = PalSize(PAL_RLEGetWidth(lBMR), PAL_RLEGetHeight(lBMR));
		itemRects.push_back(PAL_Rect(pos, size));
	}
	//使用鼠标选择角色
	if (g_InputMouse)
	{
		if (int item = isRectsAtPoint(itemRects, g_InputMouse))
		{
			int selectedIndex = gpGlobals->wMaxPartyMemberIndex - (item - 1);
			if (g_Battle.UI.wSelectedIndex == selectedIndex)
			{
				//已经选择 模拟空格键
				SetKeyPress(kKeySearch);
			}
			else {
				g_Battle.UI.wSelectedIndex = selectedIndex;
			}
		}
	}
	if (GetKeyPress() & kKeyMenu)
	{
		g_Battle.UI.state = BATTLEUISTATE::kBattleUISelectMove;
	}
	else if (GetKeyPress() & kKeySearch)
	{
		PAL_BattleCommitAction(FALSE);
	}
	else if (GetKeyPress() & (kKeyLeft | kKeyDown))
	{
		if (g_Battle.UI.wSelectedIndex != 0)
		{
			g_Battle.UI.wSelectedIndex--;
		}
		else
		{
			g_Battle.UI.wSelectedIndex = gpGlobals->wMaxPartyMemberIndex;
		}
	}
	else if (GetKeyPress() & (kKeyRight | kKeyUp))
	{
		if (g_Battle.UI.wSelectedIndex < gpGlobals->wMaxPartyMemberIndex)
		{
			g_Battle.UI.wSelectedIndex++;
		}
		else
		{
			g_Battle.UI.wSelectedIndex = 0;
		}
	}

	return;
}
WORD CFight::PAL_BattleUIMiscMenuUpdate()
/*++
Purpose:    Update the misc menu.
更新杂项菜单。
Parameters:    None.
Return value:    The selected item number. 0 if cancelled, 0xFFFF if not confirmed.
选中的菜单项序号。如果取消则为0，如果未确定则为0xFFF。
--*/
{
	int iMenuItemMaxIndex = 4;

	if (cfgShowBtlData)////#ifdef cfgShowBtlData
		iMenuItemMaxIndex++;

#ifdef SHOW_ENEMY_STATUS
	iMenuItemMaxIndex++;
#endif

	//
	// Draw the menu
	//
	auto itemRects = PAL_BattleUIDrawMiscMenu(g_iCurMiscMenuItem, FALSE);
	if (g_InputMouse)
	{
		if (int item = isRectsAtPoint(itemRects, g_InputMouse))
		{
			if (g_iCurMiscMenuItem == item - 1)
			{
				//已经选择 模拟空格键
				SetKeyPress(kKeySearch);
			}
			else {
				g_iCurMiscMenuItem = item - 1;
			}
		}
	}

	//
	// Process inputs
	//
	if (GetKeyPress() & (kKeyUp | kKeyLeft))
	{// 按上键和左键时菜单项序号减一
		g_iCurMiscMenuItem--;
		if (g_iCurMiscMenuItem < 0)
		{
			g_iCurMiscMenuItem = iMenuItemMaxIndex;
		}
	}
	else if (GetKeyPress() & (kKeyDown | kKeyRight))
	{  // 按下键和右键时菜单项序号加一
		g_iCurMiscMenuItem++;
		if (g_iCurMiscMenuItem > iMenuItemMaxIndex) // 当菜单项序号最大时，再按下键或者右键其序号变为最最小
		{
			g_iCurMiscMenuItem = 0;
		}
	}
	else if (GetKeyPress() & kKeySearch)
	{ // 按下左CTRL键时相当于回车键
		return g_iCurMiscMenuItem + 1;
	}
	else if (GetKeyPress() & kKeyMenu)
	{ // 按下菜单键退出杂项菜单
		return 0;
	}

	return 0xFFFF;
}

WORD CFight::PAL_BattleUIMiscItemSubMenuUpdate()
/*++
Purpose:    Update the item sub menu of the misc menu.
更新杂项菜单的子菜单。
Parameters:    None.
Return value:    The selected item number. 0 if cancelled, 0xFFFF if not confirmed.
选中的菜单项序号。如果取消则为0，如果未确定则为0xFFFF。
--*/
{
	int             i;
	BYTE            bColor;

	MENUITEM rgMenuItem[] = {
		// value   label                      enabled   position
		{ 0, BATTLEUI_LABEL_USEITEM, TRUE, PAL_XY(44, 62) },
		{ 1, BATTLEUI_LABEL_THROWITEM, TRUE, PAL_XY(44, 80) },
	};

	//
	// Draw the menu
	//
	PAL_BattleUIDrawMiscMenu(1, TRUE);

	PAL_CreateBox(PAL_XY(30, 50), 1, 1, 0, FALSE);

	std::vector<PAL_Rect> itemRects;
	//
	// Draw the menu items
	//
	for (i = 0; i < 2; i++)
	{
		bColor = MENUITEM_COLOR;

		if (i == g_iCurSubMenuItem)
		{
			bColor = MENUITEM_COLOR_SELECTED;
		}

		PAL_DrawText(PAL_GetWord(rgMenuItem[i].wNumWord), rgMenuItem[i].pos, bColor,
			TRUE, FALSE);
		itemRects.push_back(PAL_Rect(rgMenuItem[i].pos, PalSize(32, 16)));
	}

	if (g_InputMouse)
	{
		if (int item = isRectsAtPoint(itemRects, g_InputMouse))
		{
			if (g_iCurSubMenuItem == item - 1)
			{
				//已经选择 模拟空格键
				SetKeyPress(kKeySearch);
			}
			else {
				g_iCurSubMenuItem = item - 1;
			}
		}
	}
	//
	// Process inputs
	//
	if (GetKeyPress() & (kKeyUp | kKeyLeft))
	{
		g_iCurSubMenuItem = 0;
	}
	else if (GetKeyPress() & (kKeyDown | kKeyRight))
	{
		g_iCurSubMenuItem = 1;
	}
	else if (GetKeyPress() & kKeySearch)
	{
		return g_iCurSubMenuItem + 1;
	}
	else if (GetKeyPress() & kKeyMenu)
	{
		return 0;
	}

	return 0xFFFF;
}

const std::vector<PAL_Rect> CFight::PAL_BattleUIDrawMiscMenu(WORD wCurrentItem, BOOL fConfirmed)
/*++
Purpose:    Draw the misc menu.
画出杂项菜单。
Parameters:    [IN]  wCurrentItem - the current selected menu item.
现在选中的菜单项。
[IN]  fConfirmed - TRUE if confirmed, FALSE if not.
如果确定值为TRUE，否则为FALSE。
Return value:    None.
--*/
{
	INT			i, x, y;
	BYTE		bColor;
	BYTE		bItemNum;
	std::vector<PAL_Rect> rects;
	MENUITEM rgMenuItem[10]{ 0 };
	{
		i = 0;
		x = 16;
		y = 14;
		rgMenuItem[i - 1] = { i++, BATTLEUI_LABEL_AUTO, TRUE, PAL_XY(x, y + (i * 18)) };	//围攻
		rgMenuItem[i - 1] = { i++, BATTLEUI_LABEL_INVENTORY, TRUE, PAL_XY(x, y + (i * 18)) };//道具
		rgMenuItem[i - 1] = { i++, BATTLEUI_LABEL_DEFEND, TRUE, PAL_XY(x, y + (i * 18)) };	//防御
		rgMenuItem[i - 1] = { i++, BATTLEUI_LABEL_FLEE, TRUE, PAL_XY(x, y + (i * 18)) };	//逃跑
		rgMenuItem[i - 1] = { i++, BATTLEUI_LABEL_STATUS, TRUE, PAL_XY(x, y + (i * 18)) };	//状态
		if (cfgShowBtlData)
			rgMenuItem[i - 1] = { i++, BATTLEUI_LABEL_DATA , TRUE, PAL_XY(x, y + (i * 18)) };	//数据
		rgMenuItem[i - 1] = { i++, BATTLEUI_LABEL_ENEMY_STATUS, TRUE, PAL_XY(x, y + (i * 18)) };//敌方状态
	};

	bItemNum = i;// sizeof(rgMenuItem) / sizeof(MENUITEM);
	PAL_CreateBox(PAL_XY(2, 20), bItemNum - 1, 1, 0, FALSE);

	for (i = 0; i < bItemNum; i++)
	{
		bColor = MENUITEM_COLOR;

		if (i == wCurrentItem)
		{
			if (fConfirmed)
			{
				bColor = MENUITEM_COLOR_CONFIRMED;
			}
			else
			{
				bColor = MENUITEM_COLOR_SELECTED;
			}
		}

		PAL_DrawText(PAL_GetWord(rgMenuItem[i].wNumWord), rgMenuItem[i].pos, bColor, TRUE, FALSE);
		rects.push_back(PAL_Rect(rgMenuItem[i].pos, PalSize(64, 16)));
	}
	return rects;
}

/*
* 即时战斗充能规则，
* 以第一个我方成员基础速度为基础 xb = (x0 +100)
* 实时速度除以基础速度为相对速度倒数并取 yn = （pow((xn++100)/xb,0.3)
* 充能条值为100，每个周期前进 yn ,
* 每个周期从0 开始，如多动，第二个周期起，充能条基础值从70开始
* 战斗动作周期期间不对其他对象充能
* 战斗中复活，充能周期从50开始
*/
const float  CFight::PAL_GetTimeChargingSpeed(int wDexterity)const
{
	//Calculate the time charging speed.
	float  x = PAL_GetPlayerDexterity(0) + 100.0;
	float fly = pow(wDexterity + 100.0 / x, 0.3);
	return fly;
}

const INT CFight::PAL_New_GetHealthyPlayerNum(VOID) const
{
	INT i, num;
	WORD wPlayerRole;
	for (i = 0, num = 0; i <= gpGlobals->wMaxPartyMemberIndex; i++)
	{
		wPlayerRole = gpGlobals->rgParty[i].wPlayerRole;
		if (gpGlobals->g.PlayerRoles.rgwHP[wPlayerRole] && PAL_New_IfPlayerCanMove(wPlayerRole))
		{
			num++;
		}
	}
	return num;
}

const INT CFight::PAL_CalcBaseDamage(DWORD wAttackStrength, DWORD wDefense) const
/*++
功能：    Calculate the base damage value of attacking.
计算基础伤害值
参数：    [IN]  wAttackStrength - attack strength of attacker.
[IN]  wDefense - defense value of inflictor.
攻击者武术值，被攻击者防御值
返回值：   The base damage value of the attacking.
基础伤害值
--*/
{
	INT            iDamage;

	//
	// Formula courtesy of palxex and shenyanduxing
	//
	if (cfgDmgFormula) {// #ifdef cfgDmgFormula
		if (wAttackStrength > 2 * wDefense)
		{
			iDamage = wAttackStrength * 1.5 - wDefense * 0.5;
		}
		else if (wAttackStrength > wDefense)
		{
			iDamage = wAttackStrength * 2 - wDefense * 1.5;
		}
		else if (wAttackStrength > wDefense * 0.5)
		{
			iDamage = wAttackStrength - wDefense * 0.5;
		}
		else
		{
			iDamage = 0;
		}
	}
	else //#else
	{
		if (wAttackStrength > wDefense)
		{
			iDamage = (INT)(wAttackStrength * 2 - wDefense * 1.6 + 0.5);
		}
		else if (wAttackStrength > wDefense * 0.6)
		{
			iDamage = (INT)(wAttackStrength - wDefense * 0.6 + 0.5);
		}
		else
		{
			iDamage = 0;
		}
	}//#endif

	return iDamage;
}

const DWORD CFight::PAL_GetPlayerActualDexterity(const int i) const
{
	auto wPlayerRole = gpGlobals->rgParty[i].wPlayerRole;
	DWORD wDexterity = PAL_GetPlayerDexterity(wPlayerRole);
	if (gpGlobals->rgPlayerStatus[wPlayerRole][kStatusHaste])
	{
		wDexterity *= 6;
	}

	switch (g_Battle.rgPlayer[i].action.ActionType)
	{
	case BATTLEACTIONTYPE::kBattleActionThrowItem://新增，对投掷物品加速
		//投掷物品加速 原无
		wDexterity *= 2;
		break;

	case BATTLEACTIONTYPE::kBattleActionCoopMagic:
		//合体魔法加速 原 10
		wDexterity *= 100;
		break;

	case BATTLEACTIONTYPE::kBattleActionDefend:
		//主动防御加速 原 5
		wDexterity *= 50;
		break;

	case BATTLEACTIONTYPE::kBattleActionMagic:
		if ((gpGlobals->g.rgObject[g_Battle.rgPlayer[i].action.wActionID].magic.wFlags &
			kMagicFlagUsableToEnemy) == 0)
		{
			//对己方释放的魔法加速 原 3
			wDexterity *= 8;
		}
		break;

	case BATTLEACTIONTYPE::kBattleActionFlee:
		//逃跑加速 原 0.5
		wDexterity >>= 1;
		break;

	case BATTLEACTIONTYPE::kBattleActionUseItem:
		//使用物品加速 原 3
		wDexterity *= 15;
		break;

	default:
		break;
	}

	if (PAL_IsPlayerDying(wPlayerRole))
	{
		//濒死状态降低速度 原 0.5
		wDexterity *= 0.7;
	}
	return wDexterity;
}

/*
 *Charge per cycle
*/
void CFight::PAL_BattleAddTimeMeter()
{
	for (int i = 0; i <= gpGlobals->wMaxPartyMemberIndex; i++)
	{
		g_Battle.rgPlayer[i].flTimeMeter += g_Battle.rgPlayer[i].flTimeChargingSpeed;
	}
	for (int i = 0; i <= g_Battle.wMaxEnemyIndex; i++)
	{
		g_Battle.rgEnemy[i].flTimeMeter += g_Battle.rgEnemy[i].flTimeChargingSpeed;
	}
}

void CFight::PAL_BattleClearTimeMeter()
{
	for (int i = 0; i < MAX_PLAYERS_IN_PARTY; i++)
	{
		g_Battle.rgPlayer[i].flTimeMeter = 0;
	}
	for (int i = 0; i < MAX_ENEMIES_IN_TEAM; i++)
	{
		g_Battle.rgEnemy[i].flTimeMeter = 0;
	}
}

//显示战斗进度条
void CFight::PAL_ShowProgressBar()
{
	//
	if (spriteTexture.isEmpty())
		spriteTexture.creat(250, 60, textType::rgba32);
	spriteTexture.fill(getTransColor());
	//
	if (g_Battle.spriteList.empty())
		PAL_MakeProgressBar(g_Battle.spriteList);
	//画中线
	SDL_Color color = { 125, 125, 125, 255 };
	SDL_Rect drect = { 0,29,250,2 };
	RenderBlendCopy(spriteTexture, spriteTexture, 255, RenderMode::rmMode2, &color, &drect);
	//画敌方
	for (int i = 0; i <= g_Battle.wMaxEnemyIndex; i++)
	{
		int j = i + MAX_PLAYERS_IN_PARTY;
		if (g_Battle.rgEnemy[i].dwActualHealth == 0)
			continue;
		SDL_Rect  drect = { g_Battle.rgEnemy[i].flTimeMeter * 2 + 12,3,25,25 };

		if (s_iFrame)
		{
			//加亮
			RenderBlendCopy(spriteTexture, g_Battle.spriteList[j], 255, RenderMode::rmMode8, 0, &drect);
		}
		else
		{
			RenderBlendCopy(spriteTexture, g_Battle.spriteList[j], 255, RenderMode::rmMode1, 0, &drect);
		}
	}
	//画我方
	for (int i = 0; i <= gpGlobals->wMaxPartyMemberIndex; i++)
	{
		WORD w = gpGlobals->rgParty[i].wPlayerRole;
		SDL_Rect  drect = { g_Battle.rgPlayer[i].flTimeMeter * 2 + 12,33,25,25 };
		SDL_Color color{};
		if (gpGlobals->g.PlayerRoles.rgwHP[w])
		{
			color = { 255,255,255,255 };
		}
		else
		{
			//死亡
			color = { 125,125,125,255 };
		}
		if (s_iFrame)
		{
			//加亮
			RenderBlendCopy(spriteTexture, g_Battle.spriteList[i], 255, RenderMode::rmMode8, &color, &drect);
		}
		else
		{
			RenderBlendCopy(spriteTexture, g_Battle.spriteList[i], 255, RenderMode::rmMode1, &color, &drect);
		}
	}
	drect = { 560,0,80,30 };
	//最终显示
	RenderBlendCopy(gpTextureReal, spriteTexture, 255, RenderMode::rmMode3, 0, &drect);
}

//生成角色的缩微列表
void CFight::PAL_MakeProgressBar(std::vector<sprite>& spriteList)const
{
	auto makeOne = [&](int i, LPCBITMAPRLE IMBR)->void {
		if (IMBR)
		{
			int wigth = PAL_RLEGetWidth(IMBR);
			int height = PAL_RLEGetHeight(IMBR);
			SDL_Surface* surface = SDL_CreateRGBSurface(0, wigth, height, 8, 0, 0, 0, 0);
			SDL_SetSurfacePalette(surface, gpPalette);
			SDL_FillRect(surface, nullptr, 255);
			PAL_RLEBlitToSurface(IMBR, surface, PAL_XY(0, 0), 0, 0);
			PalTexture s = PalTexture(surface);
			SDL_FreeSurface(surface);
			sprite t = PalTexture(32, 32);
			RenderBlendCopy(t, s);
			spriteList[i] = std::move(t);
		}
		else
		{
			sprite t = PalTexture(1, 1);
			spriteList[i] = std::move(t);
		}
		};

	//将数组各项清空
	spriteList = std::vector<sprite>(MAX_PLAYERS_IN_PARTY + MAX_ENEMIES_IN_TEAM);

	for (int i = 0; i <= gpGlobals->wMaxPartyMemberIndex; i++)
	{
		auto IBMR = PAL_SpriteGetFrame(g_Battle.rgPlayer[i].lpSprite, 0);
		makeOne(i, IBMR);
	}
	for (int i = 0; i <= g_Battle.wMaxEnemyIndex; i++)
	{
		auto IBMR = PAL_SpriteGetFrame(g_Battle.rgEnemy[i].lpSprite, 0);
		makeOne(i + MAX_PLAYERS_IN_PARTY, IBMR);
	}

}

/*
 *returns the number to which the attack is to be carried out next.
 * Return 0-4 player 5-9 enemy, -1 no, -2 All combat actions are executed
 */
const int CFight::PAL_BattleGetNextDoing()const
{
	int item{ -1 };
	FLOAT f{};
	for (int i = 0; i <= gpGlobals->wMaxPartyMemberIndex; i++)
	{
		WORD w = gpGlobals->rgParty[i].wPlayerRole;
		if (gpGlobals->g.PlayerRoles.rgwHP[w] == 0 && gpGlobals->rgPlayerStatus[w][kStatusPuppet] == 0)
			continue;
		//if (!PAL_New_IfPlayerCanMove(w) )
			//continue;

		if (f < g_Battle.rgPlayer[i].flTimeMeter
			&& g_Battle.rgPlayer[i].state == FIGHTERSTATE::kFighterAct
			)// doing the actual move
		{
			item = i;
			f = g_Battle.rgPlayer[i].flTimeMeter;
		}
	}
	for (int i = 0; i <= g_Battle.wMaxEnemyIndex; i++)
	{
		if (!g_Battle.rgEnemy[i].dwActualHealth)
			continue;
		int j = i + MAX_PLAYERS_IN_PARTY;
		if (f < g_Battle.rgEnemy[i].flTimeMeter)
		{
			item = j;
			f = g_Battle.rgEnemy[i].flTimeMeter;
		}
	}
	if (item < 0)//没有可以执行的
		return -2;
	//classic
	if (cfgRealtimeBtl)
		if (f < 100.0)
			return -1;
	return item;
}

