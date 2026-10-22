#include "cbattlebase.h"
#include "cbattlebase.h"
#include "cbattlebase.h"
///* -*- mode: c; tab-width: 4; c-basic-offset: 4; c-file-style: "linux" -*- */
//
// Copyright (c) 2009-2011, Wei Mingzhi <whistler_wmz@users.sf.net>.
// Copyright (c) 2011-2021, SDLPAL development team.
// Copyright (c) 2021-2026, Wu Dong.
// 
// All rights reserved.
//
// This file is part of SDLPAL.
//
// SDLPAL is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License, version 3
// as published by the Free Software Foundation.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//

#define  NOMINMAX

#include "cbattlebase.h"
#include "cpaldata.h"
#include "paltexture.h"
#include <utility>
//#include <scclient.h>
#define  NOMINMAX
CBattleBase::CBattleBase()
{

}
/*++
功能：在场景缓存中生成新的战斗场景。
参数：    无。
返回值：    无。
结果：绘制在战斗场景缓存中。
--*/
VOID CBattleBase::PAL_BattleMakeScene()
{
	int          i;
	PAL_POS      pos;				
	LPCBITMAPRLE lBMR = NULL;

	//
	//
	// Draw the background	画背景
	// 背景层
	RenderBlendCopy(g_Battle.lpSceneBuf, g_Battle.lpBackground);
	if (g_Battle.sBackgroundColorShift)
	{
		//颜色偏移
		SDL_Color ColorShift = { 240,240,240, 255 };
		if (g_Battle.sBackgroundColorShift < 0)
			ColorShift = { 10,10,10,255 };
		RenderBlendCopy(g_Battle.lpSceneBuf, g_Battle.lpSceneBuf,
			abs(g_Battle.sBackgroundColorShift) * 8, RenderMode::rmMode0, &ColorShift);
	}
	//战场波动
	PAL_ApplyWave(g_Battle.lpSceneBuf);

	//精灵层
	auto sSurf = PalSurface(gpPalette, PictureWidth, PictureHeight);
	auto mSurf = sSurf.get();

	//
	//绘制敌人
	//

	for (i = g_Battle.wMaxEnemyIndex; i >= 0; i--)
	{
		pos = g_Battle.rgEnemy[i].pos;

		if (g_Battle.rgEnemy[i].rgwStatus[kStatusConfused] > 0 &&
			g_Battle.rgEnemy[i].rgwStatus[kStatusSleep] == 0 &&
			g_Battle.rgEnemy[i].rgwStatus[kStatusParalyzed] == 0)
		{
			//
			// Enemy is confused 
			//敌人陷入混乱状态s
			pos = PAL_XY(PAL_X(pos) + RandomLong(-1, 1), PAL_Y(pos));
		}
		lBMR = PAL_SpriteGetFrame(g_Battle.rgEnemy[i].lpSprite, g_Battle.rgEnemy[i].wCurrentFrame);

		pos = PAL_XY(PAL_X(pos) - PAL_RLEGetWidth(lBMR) / 2, PAL_Y(pos) - PAL_RLEGetHeight(lBMR));

		if (g_Battle.rgEnemy[i].wObjectID != 0)
		{
			if (g_Battle.rgEnemy[i].iColorShift)
			{
				PAL_RLEBlitWithColorShift(lBMR, mSurf, pos, g_Battle.rgEnemy[i].iColorShift, 0);
			}
			else
			{
				PAL_RLEBlitToSurface(lBMR, mSurf, pos, 0, 0);
			}
		}
	}

	if (g_Battle.lpSummonSprite.size())
	{
		//
		// Draw the summoned god	绘制召唤神的画面
		//
		lBMR = PAL_SpriteGetFrame(g_Battle.lpSummonSprite, g_Battle.iSummonFrame);
		pos = PAL_XY(PAL_X(g_Battle.posSummon) - PAL_RLEGetWidth(lBMR) / 2, PAL_Y(g_Battle.posSummon) - PAL_RLEGetHeight(lBMR));

		PAL_RLEBlitToSurface(lBMR, mSurf, pos, 0, 0);
	}
	else
	{
		//
		// Draw the players	
		//绘制玩家
		for (i = gpGlobals->wMaxPartyMemberIndex; i >= 0; i--)
		{
			pos = g_Battle.rgPlayer[i].pos;

			if (gpGlobals->rgPlayerStatus[gpGlobals->rgParty[i].wPlayerRole][kStatusConfused] != 0 &&
				gpGlobals->rgPlayerStatus[gpGlobals->rgParty[i].wPlayerRole][kStatusSleep] == 0 &&
				gpGlobals->rgPlayerStatus[gpGlobals->rgParty[i].wPlayerRole][kStatusParalyzed] == 0 &&
				gpGlobals->g.PlayerRoles.rgwHP[gpGlobals->rgParty[i].wPlayerRole] > 0)
			{
				//
				// Player is confused	
				//玩家陷入混乱状态
				continue;
			}

			lBMR = PAL_SpriteGetFrame(g_Battle.rgPlayer[i].lpSprite, g_Battle.rgPlayer[i].wCurrentFrame);
			pos = PAL_XY(PAL_X(pos) - PAL_RLEGetWidth(lBMR) / 2, PAL_Y(pos) - PAL_RLEGetHeight(lBMR));

			if (g_Battle.rgPlayer[i].iColorShift != 0)
			{
				PAL_RLEBlitWithColorShift(lBMR, mSurf, pos, g_Battle.rgPlayer[i].iColorShift, 0);
			}
			//else if (g_Battle.iHidingTime <= 0)
			else if (PAL_GetHidingTime() <= 0)
			{
				PAL_RLEBlitToSurface(lBMR, mSurf, pos, 0, 0);
			}
		}

		//
		// Confused players should be drawn on top of normal players
		//玩家的混乱状态应该画在普通状态上层
		for (i = gpGlobals->wMaxPartyMemberIndex; i >= 0; i--)
		{
			if (gpGlobals->rgPlayerStatus[gpGlobals->rgParty[i].wPlayerRole][kStatusConfused] != 0 &&
				gpGlobals->rgPlayerStatus[gpGlobals->rgParty[i].wPlayerRole][kStatusSleep] == 0 &&
				gpGlobals->rgPlayerStatus[gpGlobals->rgParty[i].wPlayerRole][kStatusParalyzed] == 0 &&
				gpGlobals->g.PlayerRoles.rgwHP[gpGlobals->rgParty[i].wPlayerRole] > 0)
			{
				//
				// Player is confused
				//玩家陷入混乱状态
				lBMR = PAL_SpriteGetFrame(g_Battle.rgPlayer[i].lpSprite, g_Battle.rgPlayer[i].wCurrentFrame);
				pos = PAL_XY(PAL_X(g_Battle.rgPlayer[i].pos), PAL_Y(g_Battle.rgPlayer[i].pos) + RandomLong(-1, 1));
				pos = PAL_XY(PAL_X(pos) - PAL_RLEGetWidth(lBMR) / 2, PAL_Y(pos) - PAL_RLEGetHeight(lBMR));

				if (g_Battle.rgPlayer[i].iColorShift != 0)
				{
					PAL_RLEBlitWithColorShift(lBMR, mSurf, pos, g_Battle.rgPlayer[i].iColorShift, 0);
				}
				else if (PAL_GetHidingTime() <= 0)
				{
					PAL_RLEBlitToSurface(lBMR, mSurf, pos, 0, 0);
				}
			}
		}
	}
	RenderBlendCopy(g_Battle.lpSceneBuf, mSurf, 255, RenderMode::rmMode3);
}

VOID CBattleBase::PAL_BattleBackupScene()
{	
    RenderBlendCopy(gpTextureRealBak, g_Battle.lpSceneBuf);
}


VOID CBattleBase::PAL_BattleUpdateFighters(
	VOID
)
/*++
  功能：    Update players' and enemies' gestures and locations in battle.
  参数：    None.
  返回值：  None.
  --*/
{
	int        i;
	WORD       wPlayerRole;

	// Update the gesture for all players
	for (i = 0; i <= gpGlobals->wMaxPartyMemberIndex; i++)
	{
		wPlayerRole = gpGlobals->rgParty[i].wPlayerRole;

		g_Battle.rgPlayer[i].pos = g_Battle.rgPlayer[i].posOriginal;
		g_Battle.rgPlayer[i].iColorShift = 0;

		if (gpGlobals->g.PlayerRoles.rgwHP[wPlayerRole] == 0)
		{
			if (gpGlobals->rgPlayerStatus[wPlayerRole][kStatusPuppet] == 0)
			{
				g_Battle.rgPlayer[i].wCurrentFrame = 2; // dead		//死亡
			}
			else
			{
				g_Battle.rgPlayer[i].wCurrentFrame = 0; // puppet	//傀儡虫作用
			}
		}
		else
		{
			if (gpGlobals->rgPlayerStatus[wPlayerRole][kStatusSleep] != 0 ||
				PAL_IsPlayerDying(wPlayerRole))
			{
				g_Battle.rgPlayer[i].wCurrentFrame = 1;
			}
			else if (g_Battle.rgPlayer[i].fDefending && !g_Battle.fEnemyCleared)
			{
				g_Battle.rgPlayer[i].wCurrentFrame = 3;
			}
			else
			{
				g_Battle.rgPlayer[i].wCurrentFrame = 0;
			}
		}
	}

	// Update the gesture for all enemies
	for (i = 0; i <= g_Battle.wMaxEnemyIndex; i++)
	{
		if (g_Battle.rgEnemy[i].wObjectID == 0)
		{
			continue;
		}

		g_Battle.rgEnemy[i].pos = g_Battle.rgEnemy[i].posOriginal;
		g_Battle.rgEnemy[i].iColorShift = 0;

		if (g_Battle.rgEnemy[i].rgwStatus[kStatusSleep] > 0 ||
			g_Battle.rgEnemy[i].rgwStatus[kStatusParalyzed] > 0)
		{
			g_Battle.rgEnemy[i].wCurrentFrame = 0;
			continue;
		}

		if (--g_Battle.rgEnemy[i].e.wIdleAnimSpeed == 0)
		{
			g_Battle.rgEnemy[i].wCurrentFrame++;
			g_Battle.rgEnemy[i].e.wIdleAnimSpeed =
				gpGlobals->g.lprgEnemy[gpGlobals->g.rgObject
				[g_Battle.rgEnemy[i].wObjectID].enemy.wEnemyID].wIdleAnimSpeed;
		}

		if (g_Battle.rgEnemy[i].wCurrentFrame >= g_Battle.rgEnemy[i].e.wIdleFrames)
		{
			g_Battle.rgEnemy[i].wCurrentFrame = 0;
		}
	}
}

BOOL  CBattleBase::PAL_IsPlayerDying(
	WORD		wPlayerRole
)const
/*++
  功能：    Check if the player is dying.
  检查角色是否濒死了
  参数：    [IN]  wPlayerRole - the player role ID.
  角色id
  返回值：  TRUE if the player is dying, FALSE if not.
  濒死返回TRUE，否则返回FALSE
  --*/
{
	return gpGlobals->g.PlayerRoles.rgwHP[wPlayerRole] < gpGlobals->g.PlayerRoles.rgwMaxHP[wPlayerRole] / 5
		&& gpGlobals->g.PlayerRoles.rgwHP[wPlayerRole] < 200;
}

INT CBattleBase::PAL_BattleSelectAutoTarget(VOID)
/*++
  功能：		Pick an enemy target automatically.
  自动选择一个目标敌人
  参数：		None.
  返回值：	The index of enemy. -1 if failed.
  怪物的序号，如果选择失败返回-1
  --*/
{
	int          i;

	i = (int)g_Battle.UI.wPrevEnemyTarget;

	if (i >= 0 && i <= g_Battle.wMaxEnemyIndex &&
		g_Battle.rgEnemy[i].wObjectID != 0 &&
		g_Battle.rgEnemy[i].dwActualHealth > 0)
	{
		return i;
	}

	for (i = 0; i <= g_Battle.wMaxEnemyIndex; i++)
	{
		if (g_Battle.rgEnemy[i].wObjectID != 0 &&
			g_Battle.rgEnemy[i].dwActualHealth > 0)
		{
			return i;
		}
	}

	return -1;
}

VOID CBattleBase::PAL_LoadBattleBackground(
	VOID
)
/*++
  功能：    Load the screen background picture of the battle.
  载入屏幕战斗背景图片。
  参数：    无。
  返回值：    无。
  --*/
{
	//
	// Create the surface
	//
	g_Battle.lpBackground = PalTexture (PictureWidth, PictureHeight);


	//
	// Load the picture
	//读取图片
	ByteArray buf(PictureWidth * PictureHeight);
	PAL_MKFDecompressChunk(buf.data(), PictureWidth * PictureHeight, 
		gpGlobals->wNumBattleField, gpGlobals->f.fpFBP);

	//
	// Draw the picture to the surface.
	//
	PAL_FBPBlitToSurface(buf.data(), g_Battle.lpBackground);
}

INT CBattleBase::PAL_New_GetAliveEnemyNum()
{
	int i, num;
	for (i = 0, num = 0; i <= g_Battle.wMaxEnemyIndex; i++)
	{
		if (g_Battle.rgEnemy[i].wObjectID != 0 && g_Battle.rgEnemy[i].dwActualHealth != 0)
		{
			num++;
		}
	}
	return num;
}

VOID CBattleBase::PAL_LoadBattleSprites(
	VOID
)
/*++
  目的：    Load all the loaded sprites.
  载入所有已读的字画面。
  参数：    无。
  返回值：    无。
  --*/
{
	int           i, l, x, y, s;
	PAL_FreeBattleSprites();
	auto& fp = gpGlobals->f.fpABC;
	//
	// Load battle sprites for players
	//读取玩家的战斗子画面
	for (i = 0; i <= gpGlobals->wMaxPartyMemberIndex; i++)
	{
		s = PAL_GetPlayerBattleSprite(gpGlobals->rgParty[i].wPlayerRole);
		
		l = PAL_MKFGetDecompressedSize(s, gpGlobals->f.fpF);

		if (l <= 0)
		{
			continue;
		}

		g_Battle.rgPlayer[i].lpSprite =	PAL_MKFDecompressChunk(s, gpGlobals->f.fpF);
		
		//
		// Set the default position for this player
		//设置此玩家的默认位置
		x = g_rgPlayerPos[gpGlobals->wMaxPartyMemberIndex][i][0];
		y = g_rgPlayerPos[gpGlobals->wMaxPartyMemberIndex][i][1];

		g_Battle.rgPlayer[i].posOriginal = PAL_XY(x, y);
		g_Battle.rgPlayer[i].pos = PAL_XY(x, y);
	}

	//
	// Load battle sprites for enemies
	//读取敌人的战斗子画面
	for (i = 0; i < MAX_ENEMIES_IN_TEAM; i++)
	{
		if (g_Battle.rgEnemy[i].wObjectID == 0)
		{
			continue;
		}
		//获取该敌人图像的长度
		l = PAL_MKFGetDecompressedSize(
			gpGlobals->g.rgObject[g_Battle.rgEnemy[i].wObjectID].enemy.wEnemyID, fp);

		if (l <= 0)
		{
			continue;
		}
		//申请内存
		g_Battle.rgEnemy[i].lpSprite.resize(l);

		//读入图像
		DWORD sEnemyID = gpGlobals->g.rgObject
			[g_Battle.rgEnemy[i].wObjectID].enemy.wEnemyID;
		g_Battle.rgEnemy[i].lpSprite = PAL_MKFDecompressChunk(sEnemyID, fp);

		//
		// Set the default position for this enemy
		//设置此敌人的默认位置
		x = gpGlobals->g.EnemyPos.pos[i][g_Battle.wMaxEnemyIndex].x;
		y = gpGlobals->g.EnemyPos.pos[i][g_Battle.wMaxEnemyIndex].y;

		y += g_Battle.rgEnemy[i].e.wYPosOffset;

		g_Battle.rgEnemy[i].posOriginal = PAL_XY(x, y);
		g_Battle.rgEnemy[i].pos = PAL_XY(x, y);
	}

}

VOID CBattleBase::PAL_FreeBattleSprites(
	VOID
)
/*++
  功能：    Free all the loaded sprites.
  释放所有已读取的（战斗）子画面。
  参数：    无。
  返回值：    无。
  --*/
{
	int         i;

	//
	// Free all the loaded sprites
	//释放左右已读取的子画面
	for (i = 0; i <= gpGlobals->wMaxPartyMemberIndex; i++)
	{
		g_Battle.rgPlayer[i].lpSprite.clear();
	}

	for (i = 0; i <= g_Battle.wMaxEnemyIndex; i++)
	{
		g_Battle.rgEnemy[i].lpSprite.clear();;
	}

	g_Battle.lpSummonSprite.clear();
}

VOID CBattleBase::PAL_BattleBackupStat(
	VOID
)
/*++
  功能：    Backup HP and MP values of all players and enemies.
  备份角色和敌人的hp和mp
  参数：    None.
  返回值：  None.
  --*/
{
	int          i;
	WORD         wPlayerRole;

	for (i = 0; i <= g_Battle.wMaxEnemyIndex; i++)
	{
		if (g_Battle.rgEnemy[i].wObjectID == 0)
		{
			continue;
		}
		g_Battle.rgEnemy[i].wPrevHP = g_Battle.rgEnemy[i].dwActualHealth;
	}

	for (i = 0; i <= gpGlobals->wMaxPartyMemberIndex; i++)
	{
		wPlayerRole = gpGlobals->rgParty[i].wPlayerRole;

		g_Battle.rgPlayer[i].wPrevHP =
			gpGlobals->g.PlayerRoles.rgwHP[wPlayerRole];
		g_Battle.rgPlayer[i].wPrevMP =
			gpGlobals->g.PlayerRoles.rgwMP[wPlayerRole];
	}
}


BOOL CBattleBase::PAL_New_IfEnemyCanMove(WORD wEnemyIndex)
{
	if (g_Battle.rgEnemy[wEnemyIndex].rgwStatus[kStatusSleep] == 0 &&
		g_Battle.rgEnemy[wEnemyIndex].rgwStatus[kStatusParalyzed] == 0 &&
		g_Battle.rgEnemy[wEnemyIndex].rgwStatus[kStatusConfused] == 0 &&
		//g_Battle.iHidingTime <= 0
		PAL_GetHidingTime() <= 0
		)
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

WORD CBattleBase::PAL_NEW_CheckAndGetLegalPlayerTarget(
	WORD wPlayerRole
)
/*++
  功能：    检查对象是否是一个合法的我方角色目标
  参数：    对象id
  返回值：  一个合法的我方角色目标id
  --*/
{
	INT		i;
	for (i = 0; i <= gpGlobals->wMaxPartyMemberIndex; i++)
	{
		if (wPlayerRole == gpGlobals->rgParty[i].wPlayerRole)
		{
			return wPlayerRole;
		}
	}

	if (i > gpGlobals->wMaxPartyMemberIndex)
	{
		i = RandomLong(0, gpGlobals->wMaxPartyMemberIndex);
		wPlayerRole = gpGlobals->rgParty[i].wPlayerRole;
	}
	return wPlayerRole;
}

WORD CBattleBase::PAL_NEW_CheckAndGetLegalEnemyTarget(
	WORD wEnemyIndex
)
/*++
  功能：    检查对象是否是一个合法的敌人目标
  参数：    对象id
  返回值：  一个合法的敌人目标
  --*/
{
	WORD		i = 0;
	if (wEnemyIndex >= MAX_ENEMIES_IN_TEAM || g_Battle.rgEnemy[wEnemyIndex].wObjectID == 0)
	{
		do
		{
			i = (WORD)RandomLong(0, g_Battle.wMaxEnemyIndex);
		} while (g_Battle.rgEnemy[i].wObjectID == 0);

		return i;
	}
	else
	{
		return wEnemyIndex;
	}
}

INT CBattleBase::PAL_New_EnemyNotMoved(WORD wEnemyIndex)
{
	//返回敌方不能活动的轮次
	INT i = 0;
    i = std::max((int)g_Battle.rgEnemy[wEnemyIndex].rgwStatus[kStatusSleep], i);
    i = std::max((int)g_Battle.rgEnemy[wEnemyIndex].rgwStatus[kStatusParalyzed], i);
    i = std::max((int)g_Battle.rgEnemy[wEnemyIndex].rgwStatus[kStatusConfused], i);
	//i = std::max((int)g_Battle.iHidingTime <= 0 ? 0 : g_Battle.iHidingTime, i);
	i = std::max(PAL_GetHidingTime() <= 0 ? 0 : PAL_GetHidingTime(), i);

	return i;
}



