#ifndef CENEMY_H
#define CENEMY_H

#include "CGameUI.h"

class CEnemy:public CGameUI
{
public:
    CEnemy();
    VOID PAL_New_DecreaseHPForEnemy(WORD wEnemyIndex, INT iDamage);
    VOID PAL_New_CurePoisonForEnemyByKind(WORD wEnemyIndex, WORD wPoisonID);
    VOID PAL_New_CurePoisonForEnemyByLevel(WORD wEnemyIndex, WORD wMaxLevel);
    VOID PAL_New_AddPoisonForEnemy(WORD wEnemyIndex, WORD wPoisonID);
    VOID PAL_New_SortPoisonsForEnemyByLevel(WORD wEnemyIndex);
    VOID PAL_New_SetEnemyStatus(WORD wEnemyIndex, WORD wStatusID, WORD wNumRound);
    const INT PAL_New_GetPoisonIndexForEnemy(WORD wEnemyIndex, WORD wPoisonID)const;
    const WORD PAL_New_GetEnemySorceryResistance(WORD wEnemyIndex)const;
    const WORD PAL_New_GetEnemyPoisonResistance(WORD wEnemyIndex)const;
    const WORD PAL_New_GetEnemyPhysicalResistance(WORD wEnemyIndex)const;
    const WORD PAL_New_GetEnemyElementalResistance(WORD wEnemyIndex, INT iAttrib)const;
    const WORD PAL_New_GetEnemyAttackStrength(WORD wEnemyIndex)const;
    const WORD PAL_New_GetEnemyMagicStrength(WORD wEnemyIndex)const;
    const WORD PAL_New_GetEnemyDefense(WORD wEnemyIndex)const;
    const DWORD PAL_New_GetEnemyDexterity(WORD wEnemyIndex)const;
    const WORD PAL_New_GetEnemyFleeRate(WORD wEnemyIndex)const ;
    const DWORD PAL_New_EstimateEnemyHealthByLevel(WORD wEnemyLevel)const;
    const DWORD PAL_GetEnemyActualDexterity(WORD wEnemyIndex)const;
};

#endif // CENEMY_H
