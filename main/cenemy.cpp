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

#include "cenemy.h"
#include "cpaldata.h"
#include <assert.h>


CEnemy::CEnemy()
{

}

VOID CEnemy::PAL_New_DecreaseHPForEnemy(
	WORD			wEnemyIndex,
	INT				iDamage
)
{
	WORD wObjectID = g_Battle.rgEnemy[wEnemyIndex].wObjectID;
	if (wEnemyIndex >= MAX_ENEMIES_IN_TEAM || wObjectID == 0)
	{
		return;
	}

	if (iDamage > 0)
	{
		iDamage = std::min(iDamage, (int)g_Battle.rgEnemy[wEnemyIndex].dwActualHealth);
	}
	g_Battle.rgEnemy[wEnemyIndex].dwActualHealth -= iDamage;
#ifndef  LIN_TIAN_NAN_BUG
	//取消此行代码，可 解决幻影林天南 BUG
	g_Battle.rgEnemy[wEnemyIndex].dwMaxHealth = max(g_Battle.rgEnemy[wEnemyIndex].dwMaxHealth,
		g_Battle.rgEnemy[wEnemyIndex].dwActualHealth);
#endif
}

 
VOID CEnemy::PAL_New_CurePoisonForEnemyByKind(
	WORD		wEnemyIndex,
	WORD		wPoisonID
)
{
	int j;
	WORD wObjectID = g_Battle.rgEnemy[wEnemyIndex].wObjectID;
	if (wEnemyIndex >= MAX_ENEMIES_IN_TEAM || wObjectID == 0)
	{
		return;
	}

	for (j = 0; j < MAX_POISONS; j++)
	{
		if (g_Battle.rgEnemy[wEnemyIndex].rgPoisons[j].wPoisonID == wPoisonID)
		{
			if (cfgPoisonUp)
				memset(&g_Battle.rgEnemy[wEnemyIndex].rgPoisons[j], 0,
					sizeof(g_Battle.rgEnemy[wEnemyIndex].rgPoisons[j]));
			else
			{
				g_Battle.rgEnemy[wEnemyIndex].rgPoisons[j].wPoisonID = 0;
				g_Battle.rgEnemy[wEnemyIndex].rgPoisons[j].wPoisonScript = 0;
			}
			break;
		}
	}
}

VOID CEnemy::PAL_New_CurePoisonForEnemyByLevel(
	WORD		wEnemyIndex,
	WORD		wMaxLevel
)
{
	int		j;
	WORD       w;

	WORD wObjectID = g_Battle.rgEnemy[wEnemyIndex].wObjectID;
	if (wEnemyIndex >= MAX_ENEMIES_IN_TEAM || wObjectID == 0)
	{
		return;
	}

	for (j = 0; j < MAX_POISONS; j++)
	{
		w = g_Battle.rgEnemy[wEnemyIndex].rgPoisons[j].wPoisonID;
		if (gpGlobals->g.rgObject[w].poison.wPoisonLevel <= wMaxLevel)
		{

			if (cfgPoisonUp)
			{
				memset(&g_Battle.rgEnemy[wEnemyIndex].rgPoisons[j], 0, sizeof(g_Battle.rgEnemy[wEnemyIndex].rgPoisons[j]));
			}
			else
			{
				g_Battle.rgEnemy[wEnemyIndex].rgPoisons[j].wPoisonID = 0;
				g_Battle.rgEnemy[wEnemyIndex].rgPoisons[j].wPoisonScript = 0;
			}
			break;
		}
	}
}


const INT CEnemy::PAL_New_GetPoisonIndexForEnemy(
	WORD		wEnemyIndex,
	WORD		wPoisonID
)const
{
	int i;

	WORD wObjectID = g_Battle.rgEnemy[wEnemyIndex].wObjectID;
	if (wEnemyIndex >= MAX_ENEMIES_IN_TEAM || wObjectID == 0)
	{
		return 0;
	}

	for (i = 0; i < MAX_POISONS; i++)
	{
		if (g_Battle.rgEnemy[wEnemyIndex].rgPoisons[i].wPoisonID == wPoisonID)
		{
			return i;
		}
	}
	return -1;
}

VOID CEnemy::PAL_New_AddPoisonForEnemy(
	WORD		wEnemyIndex,
	WORD		wPoisonID
)
/*++
Purpose:   	对敌方增加指定的毒
Parameters:    [IN]  wEnemyIndex - 敌人在队伍中的序号.
[IN]  wPoisonID - the poison to be added.
Return value:    None.
注：不可以在该函数内进行毒排序，会造成脚本执行混乱
--*/
{
	int         j;
	WORD        w;

	WORD wObjectID = g_Battle.rgEnemy[wEnemyIndex].wObjectID;
	if (wEnemyIndex >= MAX_ENEMIES_IN_TEAM || wObjectID == 0)
	{
		return;
	}

	for (j = 0; j < MAX_POISONS; j++)
	{
		if (g_Battle.rgEnemy[wEnemyIndex].rgPoisons[j].wPoisonID == wPoisonID)
		{
			if (cfgPoisonUp)
			{
				INT iSuccessRate = 100;
				WORD wPoisonLevel = gpGlobals->g.rgObject[wPoisonID].poison.wPoisonLevel;
				iSuccessRate -= wPoisonLevel * 20;
                iSuccessRate = std::max(iSuccessRate, 0);
				iSuccessRate *= 2;
				if (PAL_New_GetTrueByPercentage(iSuccessRate))
				{
					g_Battle.rgEnemy[wEnemyIndex].rgPoisons[j].wPoisonIntensity++;
				}
				switch (wPoisonLevel)
				{
				case 0:
				case 1:
				case 2:
					g_Battle.rgEnemy[wEnemyIndex].rgPoisons[j].wPoisonIntensity =
                        std::min((int)g_Battle.rgEnemy[wEnemyIndex].rgPoisons[j].wPoisonIntensity, 4);
					break;
				case 3:
					g_Battle.rgEnemy[wEnemyIndex].rgPoisons[j].wPoisonIntensity =
                        std::min((int)g_Battle.rgEnemy[wEnemyIndex].rgPoisons[j].wPoisonIntensity, 2);
					break;
				default:
					g_Battle.rgEnemy[wEnemyIndex].rgPoisons[j].wPoisonIntensity =
                        std::min((int)g_Battle.rgEnemy[wEnemyIndex].rgPoisons[j].wPoisonIntensity, 1);
					break;
				}
			}
//#endif
			return;
		}
	}

	for (j = 0; j < MAX_POISONS; j++)
	{
		w = g_Battle.rgEnemy[wEnemyIndex].rgPoisons[j].wPoisonID;
		if (w == 0)
		{
			break;
		}
	}

    j = std::min(j, MAX_POISONS - 1);
	g_Battle.rgEnemy[wEnemyIndex].rgPoisons[j].wPoisonID = wPoisonID;
	g_Battle.rgEnemy[wEnemyIndex].rgPoisons[j].wPoisonScript = gpGlobals->g.rgObject[wPoisonID].poison.wEnemyScript;

	if (cfgPoisonUp)
		g_Battle.rgEnemy[wEnemyIndex].rgPoisons[j].wPoisonIntensity = 0;
}


VOID CEnemy::PAL_New_SortPoisonsForEnemyByLevel(
	WORD		wEnemyIndex
)
{

	WORD wObjectID = g_Battle.rgEnemy[wEnemyIndex].wObjectID;
	if (wEnemyIndex >= MAX_ENEMIES_IN_TEAM || wObjectID == 0)
	{
		return;
	}
	int PoisonNum{};
	for (int j = 0; j < MAX_POISONS; j++)
	{
		auto wPoisonID1 = g_Battle.rgEnemy[wEnemyIndex].rgPoisons[j].wPoisonID;
		if (wPoisonID1 == 0)
		{
			g_Battle.rgEnemy[wEnemyIndex].rgPoisons[j].wPoisonID = 0;
		}
		else
		{
			PoisonNum++;
		}
	}

	if (PoisonNum < 2)		//中毒数目小于2不用排序
	{
		return;
	}

	for (int i = 0; i < MAX_POISONS - 1; i++)
	{
		for (int j = 0; j < MAX_POISONS - i - 1; j++)
		{
			auto wPoisonID1 = g_Battle.rgEnemy[wEnemyIndex].rgPoisons[j].wPoisonID;
			auto wPoisonLevel1 = gpGlobals->g.rgObject[wPoisonID1].poison.wPoisonLevel;
			auto wPoisonID2 = g_Battle.rgEnemy[wEnemyIndex].rgPoisons[j + 1].wPoisonID;
			auto wPoisonLevel2 = gpGlobals->g.rgObject[wPoisonID2].poison.wPoisonLevel;

			if (wPoisonLevel1 < wPoisonLevel2)
			{
				auto tempPoison = g_Battle.rgEnemy[wEnemyIndex].rgPoisons[j];
				g_Battle.rgEnemy[wEnemyIndex].rgPoisons[j] = g_Battle.rgEnemy[wEnemyIndex].rgPoisons[j + 1];
				g_Battle.rgEnemy[wEnemyIndex].rgPoisons[j + 1] = tempPoison;
			}
		}
	}
	return;
}

const WORD CEnemy::PAL_New_GetEnemySorceryResistance(WORD wEnemyIndex)const
{
	WORD wObjectID = g_Battle.rgEnemy[wEnemyIndex].wObjectID;
	if (wEnemyIndex >= MAX_ENEMIES_IN_TEAM || wObjectID == 0)
	{
		return 0;
	}

	WORD w = gpGlobals->g.rgObject[wObjectID].enemy.wResistanceToSorcery * 10;

    return std::min((int)w, 100);
}

const WORD CEnemy::PAL_New_GetEnemyPoisonResistance(WORD wEnemyIndex)const
{
	WORD wObjectID = g_Battle.rgEnemy[wEnemyIndex].wObjectID;
	if (wEnemyIndex >= MAX_ENEMIES_IN_TEAM || wObjectID == 0)
	{
		return 0;
	}

	WORD w = g_Battle.rgEnemy[wEnemyIndex].e.wPoisonResistance * 10;

    return std::min((int)w, 100);
}

const WORD CEnemy::PAL_New_GetEnemyPhysicalResistance(WORD wEnemyIndex)const
{
	WORD wObjectID = g_Battle.rgEnemy[wEnemyIndex].wObjectID;
	if (wEnemyIndex >= MAX_ENEMIES_IN_TEAM || wObjectID == 0)
	{
		return 0;
	}

	WORD w = g_Battle.rgEnemy[wEnemyIndex].e.wPhysicalResistance * 10;

    return std::min((int)w, 100);
}

const WORD CEnemy::PAL_New_GetEnemyElementalResistance(
	WORD		wEnemyIndex,
	INT			iAttrib
)const
{
	WORD wObjectID = g_Battle.rgEnemy[wEnemyIndex].wObjectID;
	if (wEnemyIndex >= MAX_ENEMIES_IN_TEAM || wObjectID == 0)
	{
		return 0;
	}

	WORD w = g_Battle.rgEnemy[wEnemyIndex].e.wElemResistance[iAttrib] * 10;

    return std::min((int)w, 100);
}

VOID CEnemy::PAL_New_SetEnemyStatus(
	WORD		wEnemyIndex,
	WORD		wStatusID,
	WORD		wNumRound
)
{
	WORD wObjectID = g_Battle.rgEnemy[wEnemyIndex].wObjectID;
	if (wEnemyIndex >= MAX_ENEMIES_IN_TEAM || wObjectID == 0)
	{
		return;
	}

	WORD wNumRoundNow = g_Battle.rgEnemy[wEnemyIndex].rgwStatus[wStatusID];

	switch (wStatusID)
	{
	case kStatusConfused:
	case kStatusSleep:
	case kStatusSilence:
	case kStatusParalyzed:
	case kStatusPuppet:
	case kStatusBravery:
	case kStatusProtect:
	case kStatusDualAttack:
	case kStatusHaste:
	{
		if (wNumRoundNow == 0)
		{
			g_Battle.rgEnemy[wEnemyIndex].rgwStatus[wStatusID] = wNumRound;
		}
		else if (wNumRoundNow <= 2)
		{
			g_Battle.rgEnemy[wEnemyIndex].rgwStatus[wStatusID]++;
		}
		break;
	}

	default:
		assert(FALSE);
		break;
	}
}

const WORD CEnemy::PAL_New_GetEnemyAttackStrength(WORD wEnemyIndex)const
{
	WORD wObjectID = g_Battle.rgEnemy[wEnemyIndex].wObjectID;
	if (wEnemyIndex >= MAX_ENEMIES_IN_TEAM || wObjectID == 0)
	{
		return 0;
	}

	INT s = (SHORT)g_Battle.rgEnemy[wEnemyIndex].e.wAttackStrength;

	if (cfgDmgFormula)//#ifdef cfgDmgFormula
		s += g_Battle.rgEnemy[wEnemyIndex].e.wLevel * 7;
	else//#else
		s += (g_Battle.rgEnemy[wEnemyIndex].e.wLevel + 6) * 6;
	//#endif

#ifdef FINISH_GAME_MORE_ONE_TIME
	s += g_Battle.rgEnemy[wEnemyIndex].e.wLevel * gpGlobals->bFinishGameTime / 2;
#endif

    s = std::max(s, 0);
    s = std::min(s, 0xFFFF);

	return (WORD)s;
}

const WORD CEnemy::PAL_New_GetEnemyMagicStrength(WORD wEnemyIndex)const
{
	WORD wObjectID = g_Battle.rgEnemy[wEnemyIndex].wObjectID;
	if (wEnemyIndex >= MAX_ENEMIES_IN_TEAM || wObjectID == 0)
	{
		return 0;
	}

	INT s = (SHORT)g_Battle.rgEnemy[wEnemyIndex].e.wMagicStrength;

	if (cfgDmgFormula)//#ifdef cfgDmgFormula

		s += g_Battle.rgEnemy[wEnemyIndex].e.wLevel * 7;
	else//#else
		s += (g_Battle.rgEnemy[wEnemyIndex].e.wLevel + 6) * 6;
	//#endif

#ifdef FINISH_GAME_MORE_ONE_TIME
	s += g_Battle.rgEnemy[wEnemyIndex].e.wLevel * gpGlobals->bFinishGameTime;
#endif

	s = std::max(s, 0);
	s = std::min(s, 0xFFFF);

	return (WORD)s;
}

const WORD CEnemy::PAL_New_GetEnemyDefense(	WORD wEnemyIndex)const
{
	WORD wObjectID = g_Battle.rgEnemy[wEnemyIndex].wObjectID;
	if (wEnemyIndex >= MAX_ENEMIES_IN_TEAM || wObjectID == 0)
	{
		return 0;
	}

	INT s = (SHORT)g_Battle.rgEnemy[wEnemyIndex].e.wDefense;

	if (cfgDmgFormula)//#ifdef cfgDmgFormula
		s += g_Battle.rgEnemy[wEnemyIndex].e.wLevel * 5;
	else //#else
		s += (g_Battle.rgEnemy[wEnemyIndex].e.wLevel + 6) * 4;
	//#endif

#ifdef FINISH_GAME_MORE_ONE_TIME
	s += g_Battle.rgEnemy[wEnemyIndex].e.wLevel * gpGlobals->bFinishGameTime;
#endif

    s = std::max(s, 0);
    s = std::min(s, 0xFFFF);

	return (WORD)s;
}

const DWORD CEnemy::PAL_New_GetEnemyDexterity(WORD wEnemyIndex)const
{
	if (wEnemyIndex >= MAX_ENEMIES_IN_TEAM )
	{
		return 0;
	}

	WORD wObjectID = g_Battle.rgEnemy[wEnemyIndex].wObjectID;


	INT s = (SHORT)g_Battle.rgEnemy[wEnemyIndex].e.weDexterity;

	if (cfgDmgFormula)
		s += g_Battle.rgEnemy[wEnemyIndex].e.wLevel * 4;
	else
		s += (g_Battle.rgEnemy[wEnemyIndex].e.wLevel + 6) * 3;

#ifdef FINISH_GAME_MORE_ONE_TIME
	s += g_Battle.rgEnemy[wEnemyIndex].e.wLevel * (gpGlobals->bFinishGameTime - 1);
#endif

    s = std::max(s, 0);
    s = std::min(s, 0xFFFF);

	return (DWORD)s;
}

const WORD CEnemy::PAL_New_GetEnemyFleeRate(WORD wEnemyIndex)const
{
	WORD wObjectID = g_Battle.rgEnemy[wEnemyIndex].wObjectID;
	if (wEnemyIndex >= MAX_ENEMIES_IN_TEAM || wObjectID == 0)
	{
		return 0;
	}

	INT s = (SHORT)(g_Battle.rgEnemy[wEnemyIndex].e.wFleeRate);

	if (cfgDmgFormula)//#ifdef cfgDmgFormula
		s += g_Battle.rgEnemy[wEnemyIndex].e.wLevel * 3;
	else//#else
		s += (g_Battle.rgEnemy[wEnemyIndex].e.wLevel + 6) * 2;
	//#endif

#ifdef FINISH_GAME_MORE_ONE_TIME
	s += g_Battle.rgEnemy[wEnemyIndex].e.wLevel * gpGlobals->bFinishGameTime;
#endif

    s = std::max(s, 0);
    s = std::min(s, 0xFFFF);

	return (WORD)s;
}

const DWORD CEnemy::PAL_New_EstimateEnemyHealthByLevel(	WORD wEnemyLevel)const
{
	DWORD dwNewHP = 0;

	if (wEnemyLevel <= 50)
	{
		dwNewHP = 50 * wEnemyLevel + 200;
	}
	else if (wEnemyLevel <= 100)
	{
		dwNewHP = (DWORD)(0.01 * pow(wEnemyLevel, 3));
		dwNewHP += 1200;
	}
	else
	{
		dwNewHP = 11200;
		dwNewHP += 50 * (wEnemyLevel - 100) + 200;
	}
	return dwNewHP;
}

const DWORD CEnemy::PAL_GetEnemyActualDexterity(WORD wEnemyIndex)const
{
	DWORD wDexterity = PAL_New_GetEnemyDexterity(wEnemyIndex);
	return wDexterity;
}
