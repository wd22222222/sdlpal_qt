#ifndef CUIBATTLE_H
#define CUIBATTLE_H

#include "cbattlebase.h"

class CUIBattle: public CBattleBase
{
public:
    CUIBattle();
    BOOL PAL_BattleUIIsActionValid(BATTLEUIACTION ActionType);
    VOID PAL_BattleUIShowText(LPCSTR lpszText, WORD wDuration);
    VOID PAL_BattleUIUseItem(VOID);
    VOID PAL_BattleUIThrowItem(VOID);
    WORD PAL_BattleUIPickAutoMagic(WORD wPlayerRole, WORD wRandomRange);
    VOID PAL_New_BattleUIShowData(VOID);

#ifdef SHOW_ENEMY_STATUS
    VOID PAL_New_EnemyStatus(VOID);
#endif
};

#endif // CUIBATTLE_H
