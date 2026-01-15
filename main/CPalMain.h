#ifndef CPALMAIN_H
#define CPALMAIN_H
#include "cbattle.h"
#include "command.h"


class CPalMain :public CBattle

{
    //游戏主控结构
public:
    CPalMain();
    ~CPalMain();
    VOID PAL_GameUpdate(BOOL fTrigger);
    VOID PAL_PartyWalkTo(INT x, INT y, INT h, INT iSpeed);
    VOID PAL_SceneFade(INT iPaletteNum, BOOL fNight, INT iStep);
    int  getQuit() { return PalQuit; }

//private:
    int PAL_TrademarkScreen();
    int PAL_SplashScreen();
public:
    VOID PAL_GameStart(VOID);

    VOID PAL_GameMain(VOID);

    VOID PAL_StartFrame(VOID);


};


#endif // PALMAIN_H
