#ifndef CFIGHT_H
#define CFIGHT_H

#include "cuibattle.h"
#include "palgpgl.h"
class CFight: public CUIBattle
{
    BOOL                     fOnlyPuppet{};
public:
    DWORD                    m_TimeCount{};//战斗进度计数器
public:
    CFight();
    VOID PAL_BattleDelay(WORD wDuration, WORD wObjectID, BOOL fUpdateGesture);
    void PAL_BattleStartFrame();
    VOID PAL_BattleShowPlayerPreMagicAnim(WORD wPlayerIndex, BOOL fSummon);
    VOID PAL_BattlePlayerValidateAction(WORD wPlayerIndex);
    VOID PAL_BattlePlayerPerformAction(WORD wPlayerIndex);
    VOID PAL_BattleStealFromEnemy(WORD wTarget, WORD wStealRate);
    VOID PAL_BattleSimulateMagic(SHORT sTarget, WORD wMagicObjectID, WORD wBaseDamage, BOOL fIncludeMagicDamage);
    VOID PAL_BattleFadeScene();
    VOID PAL_BattlePlayerEscape();

private:
    INT PAL_BattleEnemySelectTargetIndex()const;
    VOID PAL_BattleEnemyPerformAction(WORD wEnemyIndex);
    BOOL PAL_New_IfEnemyCanEscape(WORD wEnemyIndex);

    //重载加入自动防御加成
    INT PAL_CalcPhysicalAttackDamage(DWORD wAttackStrength, DWORD wDefense, DWORD wAttackResistance, INT wPlayer = -1);
    INT PAL_CalcPhysicalAttackDamageBase(DWORD wAttackStrength, DWORD wDefense, DWORD wAttackResistance);

    void PAL_BattleEnemyPhysical(WORD wEnemyID, WORD wPlayerID);
    void PAL_BattlePostActionCheck(BOOL fCheckPlayers);
    void PAL_BattleCommitAction(BOOL fRepeat);
    void PAL_New_ApplyMagicDamageToEnemy(SHORT sTarget, WORD wBaseDamage, WORD wMagicObjectID, BOOL fOneDamageAtLeast);
    void PAL_AutoBattle();
    void PAL_BattleProcessPlayerAction(int playerRole);
    void PAL_BattleProcessEnemyAction(int enemy);
    void PAL_BattleDrawPlayerinformationBar();
    void PAL_BattleDrawIconAndSelect();
    void PAL_BattleUISelectMove();
    void PAL_BattleMenuMain();
    VOID PAL_BattleUIPlayerReady(WORD wPlayerIndex);

    VOID PAL_BattleShowPlayerDefMagicAnim(WORD wPlayerIndex, WORD wObjectID, SHORT sTarget);
    VOID PAL_BattleShowPlayerOffMagicAnim(WORD wPlayerIndex, WORD wObjectID, SHORT sTarget, BOOL fSummon = FALSE);
    VOID PAL_BattleShowEnemyMagicAnim(WORD wObjectID, SHORT sTarget);
    VOID PAL_BattleShowPlayerSummonMagicAnim(WORD wPlayerIndex, WORD wObjectID);
    VOID PAL_BattleShowPostMagicAnim(VOID);

    void PAL_BattleUISelectTargetEnemy();
    void PAL_BattleUISelectTargetPlayer();
	WORD PAL_BattleUIMiscMenuUpdate();
	WORD PAL_BattleUIMiscItemSubMenuUpdate();
	const std::vector<PAL_Rect> PAL_BattleUIDrawMiscMenu(WORD wCurrentItem, BOOL fConfirmed);
    void PAL_BattleShowPlayerAttackAnim(WORD wPlayerIndex, BOOL fCritical);
    void PAL_BattleShowPlayerUseItemAnim(WORD wPlayerIndex, WORD wObjectID, SHORT sTarget);
    void PAL_BattlePlayerSelected();
    void PAL_BattleUIUpdate();
    //do action
    void PAL_BattlePlayerDoAction(int player);
    void PAL_BattleEnemyDoAction(int enemy);
    int  PAL_BattleStartFrameOne(int);
    VOID PAL_BattleUIShowNum(WORD wNum, PAL_POS pos, NUMCOLOR color);
    BOOL PAL_BattleDisplayStatChange();

    const INT PAL_CalcMagicDamage(WORD wMagicStrength, WORD wDefense,
        SHORT sElementalResistance[NUM_MAGIC_ELEMENTAL],
        SHORT sPoisonResistance, WORD wMagicID, INT wPlayerRole = -1)const;
    const INT PAL_CalcMagicDamageBase(DWORD wMagicStrength, DWORD wDefense,
        SHORT sElementalResistance[NUM_MAGIC_ELEMENTAL], SHORT sPoisonResistance, WORD wMagicID)const;
    const INT  PAL_GetManuallyDefend()const;
    const int  PAL_BattlePlayerCanOperation()const;
    const int  PAL_BattleStartFramePrepare();
    const int  PAL_BattleGetNextDoing()const;
    const float PAL_GetTimeChargingSpeed(int wDexterity)const ;
    const INT PAL_New_GetHealthyPlayerNum(VOID)const;
    const INT PAL_CalcBaseDamage(DWORD wAttackStrength, DWORD wDefense)const;
    const DWORD PAL_GetPlayerActualDexterity(const int i)const;
    void PAL_BattleAddTimeMeter();
    void PAL_BattleClearTimeMeter();
private:
    //显示战斗进度条
    void PAL_ShowProgressBar();
    //生成角色的缩微列表
    void PAL_MakeProgressBar(std::vector<sprite>&)const;
    PalTexture spriteTexture{};
};

#endif // CFIGHT_H
