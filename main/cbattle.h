#ifndef CBATTLE_H
#define CBATTLE_H

#include "cfight.h"


class CBattle:public CFight
{
public:
    CBattle();
	~CBattle() {};
	BATTLERESULT PAL_BattleMain(VOID);
	VOID PAL_BattleWon(VOID);
	VOID PAL_BattleEnemyEscape(VOID);
	BATTLERESULT PAL_StartBattle(WORD wEnemyTeam, BOOL fIsBoss);
	VOID Pal_New_RecoverAfterBattle(WORD wPlayerRole, WORD wPercent);

	BATTLEENEMY PAL_New_StrengthenEnemy(BATTLEENEMY be);
};

#endif // CBATTLE_H

