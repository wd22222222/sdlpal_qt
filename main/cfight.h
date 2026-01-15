#ifndef CFIGHT_H
#define CFIGHT_H

#include "cuibattle.h"
#include "palgpgl.h"
class CFight: public CUIBattle
{
public:
    CFight();
    VOID PAL_BattleDelay(WORD wDuration, WORD wObjectID, BOOL fUpdateGesture);
    BOOL PAL_BattleDisplayStatChange(VOID);
    VOID PAL_BattlePostActionCheck(BOOL fCheckPlayers);
    VOID PAL_BattleUpdateFighters(VOID);
    VOID PAL_BattleStartFrame(VOID);
    VOID PAL_BattleShowPlayerPreMagicAnim(WORD wPlayerIndex, BOOL fSummon);
    VOID PAL_BattleShowPlayerDefMagicAnim(WORD wPlayerIndex, WORD wObjectID, SHORT sTarget);
    VOID PAL_BattleShowPlayerOffMagicAnim(WORD wPlayerIndex, WORD wObjectID, SHORT sTarget, BOOL fSummon = FALSE);
    VOID PAL_BattleShowEnemyMagicAnim(WORD wObjectID, SHORT sTarget);
    VOID PAL_BattleShowPlayerSummonMagicAnim(WORD wPlayerIndex, WORD wObjectID);
    VOID PAL_BattleShowPostMagicAnim(VOID);
    VOID PAL_BattlePlayerValidateAction(WORD wPlayerIndex);
    VOID PAL_BattlePlayerPerformAction(WORD wPlayerIndex);
    INT PAL_BattleEnemySelectTargetIndex(VOID);
    BOOL PAL_New_IfEnemyCanEscape(WORD wEnemyIndex);
    VOID PAL_BattleEnemyPerformAction(WORD wEnemyIndex);
    VOID PAL_BattleStealFromEnemy(WORD wTarget, WORD wStealRate);
    VOID PAL_BattleSimulateMagic(SHORT sTarget, WORD wMagicObjectID, WORD wBaseDamage, BOOL fIncludeMagicDamage);
    VOID PAL_BattleUIUpdate(VOID);
    VOID PAL_BattleFadeScene();
    VOID PAL_BattlePlayerEscape(VOID);
    INT PAL_CalcPhysicalAttackDamage(DWORD wAttackStrength, DWORD wDefense, DWORD wAttackResistance,INT wPlayer = -1);
    INT PAL_CalcMagicDamage(WORD wMagicStrength, WORD wDefense,
        SHORT sElementalResistance[NUM_MAGIC_ELEMENTAL],
        SHORT sPoisonResistance, WORD wMagicID, INT wPlayerRole = -1);

    VOID PAL_BattleEnemyPhysical(WORD wEnemyID, WORD wPlayerID);
    INT  PAL_GetManuallyDefend();
private:
    void PAL_BattleCommitAction(BOOL fRepeat);
    void PAL_New_ApplyMagicDamageToEnemy(SHORT sTarget, WORD wBaseDamage, WORD wMagicObjectID, BOOL fOneDamageAtLeast);
    void PAL_AutoBattle();
    void PAL_BattleDrawPlayerinformationBar();
    void PAL_BattleDrawIconAndSelect();
    void PAL_BattleUISelectMove();
    void PAL_BattleMenuMain();
    void PAL_BattleUISelectTargetEnemy();
    void PAL_BattleUISelectTargetPlayer();
	WORD PAL_BattleUIMiscMenuUpdate();
	WORD PAL_BattleUIMiscItemSubMenuUpdate();
	std::vector<PAL_Rect> PAL_BattleUIDrawMiscMenu(WORD wCurrentItem, BOOL fConfirmed);
    void PAL_BattleShowPlayerAttackAnim(WORD wPlayerIndex, BOOL fCritical);
    void PAL_BattleShowPlayerUseItemAnim(WORD wPlayerIndex, WORD wObjectID, SHORT sTarget);

};

#endif // CFIGHT_H
