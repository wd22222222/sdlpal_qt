

#include "cgetpaldata.h"
#include "cListScriptCall.h"
#include "CListScriptEntry.h"
#include "cmapedit.h"
#include "deploymap.h"
#include "FontFinder.h"
#include "mainwindow.h"
#include "packedpict_dlg.h"
#include <cassert>
#include <map>
#include <qdebug.h>
#include <qdir.h>
#include <qevent.h>
#include <qfiledialog.h>
#include <qmessagebox.h>
#include <qsplitter.h>

#include "cpixedit.h"
#include "ctableview.h"
#include "t_data.h"
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <functional>
#include <main/command.h>
#include <main/cpaldata.h>
#include <main/cpalevent.h>
#include <main/cscript.h>
#include <main/palgpgl.h>
#include <qcontainerfwd.h>
#include <qguiapplication.h>
#include <qicon.h>
#include <qlist.h>
#include <qlogging.h>
#include <qmainwindow.h>
#include <qnamespace.h>
#include <qrect.h>
#include <qscreen.h>
#include <qsizepolicy.h>
#include <qstring.h>
#include <qtextcursor.h>
#include <qtextedit.h>
#if QT_VERSION >= QT_VERSION_CHECK( 6,0, 0)
#include <qtmetamacros.h>
#include <qtpreprocessorsupport.h>
#endif
#include <qtreewidget.h>
#include <qwidget.h>
#include <string>
#include <thread>
#include <utility>
#include <variant>
#include <vector>
#include <Windows.h>

#if USING_OGG
#include "ogg/ogg.h"
#endif

//下拉表筛选 0 物品 1 法术 2 毒药 3 攻击附加 4.怪
constexpr uint32_t select_flag[] =
{
    kIsItem,
    kIsMagic,
    kIsPoison,
    kIsPoison | kIsItem | kIsMagic,
    kIsEnemy,
};
enum tagSelectTree
{
    WM_EDIT_DIR,
    WM_SET_TREE,
    WM_ARCHIVES,
    WM_ARCHIVES_END = WM_ARCHIVES + MAXSAVEFILES + 1,//maxSaveFiles = 30
    WM_RUN_TEST,//测试运行
    WM_RUN,//运行
    WM_BACKUP,//备份
    WM_RESTORE,//恢复
    WM_DEL_BACKUP,//删除备份文件
    WM_SAVE_ALL,//将修改后的数据存盘
    WM_EDIT_PLAYER,//player
    WM_EDIT_ENEMY,//怪物
    WM_EditEnemyTeam,//敌方队伍
    WM_EDIT_MAGIC,//魔法
    WM_EditObjectItem,//物品
    WM_EditPoison,//毒药编辑
    WM_PLAYERLEVELUPMAGIG,//升级魔法
    WM_EditBattleField,//战斗场所
    WM_EditEventObject,  //事件对象
    WM_EditScene,//场景编辑
    WM_EditStore,//商店
    WM_EditObjectName,//对象名称编辑
    WM_EditDialog,//对话编辑
    WM_EditExplain,//对象说明编辑
    WM_EditObjectPict,//对象图像编辑
    WM_ScriptEntry,//脚本入口
    WM_MapEdit,//地图编辑
    WM_NOTHING1,//
    WM_EditGameSeting,//游戏设置修改
    WM_EditFontLibraryPath,//字库路径
    WM_EDIT_PARAMETERS,//基本属性编辑
    WM_Inventory,//背包物品编辑
    WM_NOTHING2,//
    WM_OKReturn,//确定返回
    WM_EXIT,//退出程序
    WM_END,
};

//b 树的层次设置 1 tree0（0） 01b tree1) 010b tree（2）0100b
const struct ragItem { LPCSTR s;const  INT b;const int n; } lpItemName[] =
{
    {"修改目录", 1 ,WM_EDIT_DIR},
    {"修改初始数据", 1, WM_SET_TREE},
    {"修改存档数据", 1, WM_ARCHIVES},
    {"测试", 1,WM_RUN_TEST},
    {"运行",1,WM_RUN},
    {"备份",1,WM_BACKUP},
    {"恢复",1,WM_RESTORE},
    {"删除备份",1,WM_DEL_BACKUP},
    {"保存",1,WM_SAVE_ALL},
    {"我方属性",0b110,WM_EDIT_PLAYER},
    {"敌方属性",0b10,WM_EDIT_ENEMY},
    {"敌方队伍",0b10,WM_EditEnemyTeam},
    {"魔法",0b10,WM_EDIT_MAGIC},
    {"物品",0b110,WM_EditObjectItem},
    {"毒药",0b10,WM_EditPoison},
    {"升级魔法",0b10,WM_PLAYERLEVELUPMAGIG},
    {"战斗场所",0b10,WM_EditBattleField},
    {"事件对象",0b110,WM_EditEventObject},
    {"场景",0b10,WM_EditScene},
    {"商店",0b10,WM_EditStore},
    {"对象名称编辑",0b10,WM_EditObjectName},
    {"对话编辑",0b10,WM_EditDialog},
    {"对象说明编辑",0b10,WM_EditExplain},
    {"对象图像编辑",0b10,WM_EditObjectPict},
    {"脚本入口",0b10,WM_ScriptEntry},
    {"地图编辑",0b10,WM_MapEdit},
    {"----------",0b10,WM_NOTHING1},
    {"修改游戏设置",0b10,WM_EditGameSeting},
    {"修改字库路径",0b10,WM_EditFontLibraryPath},
    {"基本数据",0b100,WM_EDIT_PARAMETERS},
    {"背包物品",0b100,WM_Inventory},
    {"----------",0b10,WM_NOTHING2},
    {"返回 ",0b110,WM_OKReturn},
    {"退出 ",1,WM_EXIT},
    {nullptr,0,WM_END},
};

LPCSTR szsm[] =
{
    "增加毒的烈度",
    "修改伤害计算公式（包括敌人的属性计算方式）",
    "在战斗中显示相关数据",
    "在后期加强前期的敌人、队伍多于3人，敌人加强",
    "修改灵葫炼丹为商店",
    "当mp减少时也显示数值",
    "在一定情况下添加状态总是成功，无视巫抗",
    "补体力真气时已满返回失败",
    "显示驱魔香和十里香步数",
    "主動防禦j时，防御加強",//0-9
    "设置冷却值 在之后的 n 次夺魂无效",
    "修改附加属性",
    "修改战后衩覆盖的脚本",
    "修改附加经验计算方式",
    "额外附加经验",
    "额外恢复",
    "不完全是随机选择目标",
    "敌人最多行动次数，如果为2则是经典版",
    "某些毒可以对任何人均命中（无视敌方巫抗或我方毒抗）",
    "有特色的加强主角，灵儿初始五灵抗性20%，阿奴毒抗巫抗各30%，林月如额外恢复",//10-19
    "天罡战气后，投掷偷武器伤害增加",
    "使用梦蛇后，各项属性经验增加",
    "自动防御比率，0 为手动防御",
    "BATTLE_FRAME_TIME 战时每帧毫秒数 40",
    "非战时每帧毫秒数 100",
    "= 1 怪物混乱攻击同伴, = 2 怪物混乱无同伴攻击我方",
    "怪物分裂体力减半",
    "音频设备号 -1 系统缺省",
    "SurroundOPLOffset 缺省值384",
    "声道数2 双声道，1 单声道",//20-29
    "采样率 缺省为44100",
    "OPL 采样率，缺省为 49716",
    "采样质量 0 到 4 缺省为 4",
    "音乐音量 缺省为100",
    "效果音量 缺省为100",
#if USING_OGG
    "音乐格式 0 MIDE,1 RIX 2 MP3 3 OGG",
#else
    "音乐格式 0 MIDE,1 RIX 2 MP3",
#endif
    "OPL核心",
    "OPLChip",
    "MIDI设置",
    "优先使用MP3 1 是",//30-39
    "是否使用Big5 繁体字，1 是",
    "使用环绕OPL",
    "保持窗口显示比例， 1 是",
    "是否使用过场动画AVI，1 是",
    "音乐缓存区大小",
    "字体加粗，0 - 4",
    "使用粗体字， 1 是",//46
    "最大存档文件数5-30",
    "加速或减速 -80%--+500%。" ,
    "系统字符集，0自动，1 GB2312，2 BIG5",//40-49
    "是否采用即时对战模式",
    "亮度增强",
    "与BOSS战斗时 Q 键直接重新开始",//52
    "自动存档",
    "更多的隐藏经验",
};


QString MainWindow::q_SrcToStr(int i)
{
    //显示怪队伍时使用
    if (i == 0xffff) return"禁用";
    //以下正常
    if (i < 0 || i >= MAX_OBJECTS)
        return NULL;
    return QString(m_Pal->pal->PAL_GetWord(i).c_str());
}

void MainWindow::Edit_Game_Seting(int work)
{
    //修改游戏设置
    int totalCol = 3;
    WorkCtrl = work;
    const  int rowSize = sizeof(szsm) / sizeof(char*);
    T_DATA s_Data{};
    //建立表头
    //参数 文字，列宽，列号，（类型 0 什么都不做，1，列头固定(永远显示)，2，Edit），
    //（原数据类型 0  整数 ，1 WORD, 2 SHORT, 3 16进制数，4 浮点数，5 字符串）
    //(下拉框数据索引)，
    // （指向函数的指针，将源数据转化为显示数据字串），
    // （指向函数的指针，将显示字串转化为源）
    auto & w_ColData = s_Data.c_Array;
    w_ColData.resize(totalCol);
    w_ColData[0].GetData("序号", 90, 0, ctrl_Fix, tINT);
    w_ColData[1].GetData(" 值 ", 120, 1, ctrl_Edit, tINT);
    w_ColData[2].GetData("说明", 600, 2, ctrl_Null, tSTR);
    //设置输入范围
    w_ColData[1].f_chuck = [&](QString  into,int row)->int
    {
        int t = into.toInt();
        if (t > m_Pal->pal->ggConfig->configRange[row].max || t < m_Pal->pal->ggConfig->configRange[row].min)
        {
            CPalEvent::printMsg("输入校验失败 最大 %d 最小 %d",
                m_Pal->pal->ggConfig->configRange[row].max,
                m_Pal->pal->ggConfig->configRange[row].min);
            return 0;
        }
        return 1;
    };
    //m_Grid->SetColClass(totalCol, w_ColData);
    //m_Grid->set_t_Data(s_Data);
    //内容数据
    auto &s_RowData = s_Data.d_Array;
    s_RowData.resize(rowSize);
    for (int n = 0; n < rowSize; n++)
    {
        s_RowData[n].ColVarList.resize(totalCol);
        s_RowData[n].ColVarList[0] = n + 1;
        s_RowData[n].ColVarList[1] = m_Pal->pal->ggConfig->m_Function_Set[n];
        s_RowData[n].ColVarList[2] = szsm[n];
    }
    //m_Grid->SetDataClass(rowSize, totalCol, s_RowData);
    m_Grid->set_t_Data(s_Data);
    //不能插入
    m_Grid->m_popMenuFlags = 0b001011;//不显示16位查找，不增删改
    return ;
}

VOID MainWindow::Edit_EventObject(int work)
{
    int totalCol = 20;
    WorkCtrl = work;
    m_Grid->UndoCtrl = work;
    T_DATA s_Data{};
    //事件对象
    QString s("修改事件对象");
    SendMessages(s);
    auto &w_ColData = s_Data.c_Array;
    w_ColData.resize(totalCol);
    //制做表头
    //参数 文字，列宽，列号，（类型 0 什么都不做，1，列头固定(永远显示)，2，Edit），
    //（原数据类型 0  整数 ，1 WORD, 2 SHORT, 3 16进制数，4 浮点数，5 字符串）
    //(下拉框数据索引)，（指向函数的指针，将源数据转化为显示数据字串），（指向函数的指针，将显示字串转化为源）
    w_ColData[0].GetData("序号", 40, 1, ctrl_Fix, tWORD);
    w_ColData[1].GetData("场景号", 60, 0, ctrl_Fix, tWORD);
    w_ColData[2].GetData("隐时间", 60, 2, ctrl_Edit, tWORD);
    w_ColData[3].GetData("X", 60, 3, ctrl_Edit, tWORD);
    w_ColData[4].GetData("Y", 60, 4, ctrl_Edit, tWORD);
    w_ColData[5].GetData("层次", 60, 5, ctrl_Edit, tSHORT);
    w_ColData[6].GetData("目标脚本", 80, 6, ctrl_Edit, tHEX, 0, 0, 0, 1);
    w_ColData[7].GetData("自动脚本", 80, 7, ctrl_Edit, tHEX, 0, 0, 0, 1);
    w_ColData[8].GetData("状态", 60, 8, ctrl_Edit, tWORD);
    w_ColData[9].GetData("触发模式", 60, 9, ctrl_Edit, tWORD);
    w_ColData[10].GetData("形象号", 60, 10, ctrl_Edit, tWORD);
    w_ColData[11].GetData("形象数", 60, 11, ctrl_Edit, tWORD);
    w_ColData[12].GetData("方向", 60, 12, ctrl_Edit, tWORD);
    w_ColData[13].GetData("当前帧数", 60, 13, ctrl_Edit, tSHORT);
    w_ColData[14].GetData("空闲帧数", 60, 14, ctrl_Edit, tSHORT);
    w_ColData[15].GetData("无效", 60, 15, ctrl_Edit, tSHORT);
    w_ColData[16].GetData("总帧数", 60, 16, ctrl_Edit, tWORD);
    w_ColData[17].GetData("空闲计数", 60, 17, ctrl_Edit, tWORD);
    w_ColData[18].GetData("行号", 60, 18, ctrl_Null, tWORD);
    w_ColData[19].GetData("原行号", 50, 19, ctrl_Null, tWORD);

    //生成数据
    auto &s_RowData = s_Data.d_Array;
    auto p = &m_Pal->pal->gpGlobals->g;
    s_RowData.resize(p->nEventObject);

    int  n_Row, n_Scens, n_Object;
    for (n_Row = 0, n_Scens = 1; n_Scens <= p->nScene; n_Scens++)
    {
        for (n_Object = p->rgScene[n_Scens - 1].wEventObjectIndex;
            n_Scens <= MAX_SCENES &&
            (n_Scens == MAX_SCENES ||
                n_Object < (p->rgScene[n_Scens].wEventObjectIndex)) &&
            n_Row < p->nEventObject;
            n_Object++, n_Row++)
        {
            s_RowData[n_Row].ColVarList.resize(totalCol);
            s_RowData[n_Row].oldRow = n_Row + 1;
            s_RowData[n_Row].ColVarList[18] = n_Row + 1;
            s_RowData[n_Row].ColVarList[19] = n_Row + 1;

            LPEVENTOBJECT  peo = &(p->lprgEventObject[n_Object]);
            s_RowData[n_Row].ColVarList[0] = n_Row;
            s_RowData[n_Row].ColVarList[1] = n_Scens;
            WORD* wp = (WORD*)peo;
            for (int n = 2; n < 18; n++)
            {
                s_RowData[n_Row].ColVarList[n] = wp[n - 2];
            }
        }
    }

    if (JobCtrl > 0)
        m_Grid->m_popMenuFlags = 0b0011111;//存档文件不允许插入删除
    else
        m_Grid->m_popMenuFlags = 0b10010111111;//可插入删除，不允许尾部添加
    //
    s_Data.Set_Col_Data = [](ColumnClass* s,int row)->VOID
        {
            s->ColVarList[18] = row + 1;
            s->ColVarList[19] = (int)s->oldRow;
        };
    m_Grid->indexScene = 1;
    m_Grid->set_t_Data(s_Data); //
    return ;
}
//修改字库目录
void MainWindow::Edit_FontLibraryPath(int work)
{
    //修改字库目录 
    auto a = FontFinder(QString( m_Pal->pal->ggConfig->m_FontName.c_str()), this);
    if (a.exec())
    {
        m_Pal->pal->ggConfig->m_FontName = a.selectedFontFile.toStdString();
        //确认,数据更新
        haveModified = 1;
        //存缺省数据
        m_Pal->saveGameDataToCache();
    };
}
//
void MainWindow::Edit_BattleField(int swtch)
{
    assert(swtch == WM_EditBattleField);
    WorkCtrl = WM_EditBattleField;
    m_Grid->UndoCtrl = WM_EditBattleField;
    QString s = "修改战斗场所";
    SendMessages(s);

    //制作表头
    //参数 文字，列宽，列号，（类型 0 什么都不做，1，列头固定(永远显示)，2，Edit），
    //（原数据类型 0  整数 ，1 WORD, 2 SHORT, 3 16进制数，4 浮点数，5 字符串）
    //(下拉框数据索引)，（指向函数的指针，将源数据转化为显示数据字串），（指向函数的指针，将显示字串转化为源）
    T_DATA s_Data;
    ColArray & w_ColData = s_Data.c_Array;
    const int totalCol = 7;
    w_ColData.resize(totalCol);
    w_ColData[0].GetData("序号", 90, 0, ctrl_Fix, tWORD);
    w_ColData[1].GetData("风", 90, 1, ctrl_Edit, tSHORT);
    w_ColData[2].GetData("雷", 90, 2, ctrl_Edit, tSHORT);
    w_ColData[3].GetData("水", 90, 3, ctrl_Edit, tSHORT);
    w_ColData[4].GetData("火", 90, 4, ctrl_Edit, tSHORT);
    w_ColData[5].GetData("土", 90, 5, ctrl_Edit, tSHORT);
    w_ColData[6].GetData("波动水平", 90, 6, ctrl_Edit, tWORD);
    //m_Grid.SetColClass(7, w_ColData);

    //表数据
    DataArray  &s_RowData = s_Data.d_Array;
    auto p = &m_Pal->pal->gpGlobals->g;
    s_RowData.resize(p->nBattleField);//行数
    int j, k;
    for (j = 0; j < p->nBattleField; j++)
    {
        s_RowData[j].ColVarList.resize(totalCol);
        s_RowData[j].ColVarList[0] = j + 1;
        for (k = 0; k < NUM_MAGIC_ELEMENTAL; k++)
        {
            s_RowData[j].ColVarList[k + 1] = p->lprgBattleField[j].rgsMagicEffect[k];
        }
        s_RowData[j].ColVarList[6] = p->lprgBattleField[j].wScreenWave;
    }
    m_Grid->set_t_Data(s_Data);
	m_Grid->m_popMenuFlags = 0b01001111;//不允许插入删除，允许尾部添加
    return ;
}

void MainWindow::Edit_Dialog(int swtch)
{
    assert(swtch == WM_EditDialog);
    QString msg("对话编辑");
    SendMessages(msg);
    msg = "符号说明:字体颜色 _ _ 青色 \' \' 红色 \" \" 黄色";
    SendMessages(msg);
    msg = ("~00 延迟并退出 ( )设置等待图标 $00 文本显示延迟时间");
    SendMessages(msg);

    WorkCtrl = WM_EditDialog;
    m_Grid->UndoCtrl = WM_EditDialog;
    //建立表头
    //参数 文字，列宽，列号，（类型 0 什么都不做，1，列头固定(永远显示)，2，Edit），
    //（原数据类型 0  整数 ，1 WORD, 2 SHORT, 3 16进制数，4 浮点数，5 字符串）
    //(下拉框数据索引)，（指向函数的指针，将源数据转化为显示数据字串），（指向函数的指针，将显示字串转化为源）
    T_DATA s_Data;
    ColArray &w_ColData = s_Data.c_Array;
    w_ColData.resize(4);
    w_ColData[0].GetData("原序号", 50, 0, ctrl_Fix, tINT);
    w_ColData[1].GetData("16进制", 50, 1, ctrl_Null, tHEX);
    w_ColData[2].GetData("有效", 30, 1, ctrl_Null, tBOOL);
    w_ColData[3].GetData("内容", 600, 1, ctrl_Edit, tSTR);
    //m_Grid.SetColClass(4, w_ColData);

    DataArray &s_RowData = s_Data.d_Array;
    s_RowData.resize(m_Pal->pal->gpGlobals->g_TextLib.nMsgs);
    for (int n = 0; n < m_Pal->pal->gpGlobals->g_TextLib.nMsgs; n++)
    {
        s_RowData[n].ColVarList.resize(4);
        s_RowData[n].oldRow = n + 1;
        s_RowData[n].ColVarList[0] = s_RowData[n].oldRow;
        s_RowData[n].ColVarList[1] = n;
        s_RowData[n].ColVarList[2] = 0;
        auto sStr = m_Pal->pal->PAL_GetMsg(n);
        s_RowData[n].ColVarList[3] = sStr;
    }
    //确定对话是否被使用
    for (int n_Row = 0; n_Row < m_Pal->pal->gpGlobals->g.nScriptEntry; n_Row++)
    {
        auto p = &m_Pal->pal->gpGlobals->g;
        if (p->lprgScriptEntry[n_Row].wOperation == 0xffff &&
            p->lprgScriptEntry[n_Row].rgwOperand[0] < m_Pal->pal->gpGlobals->g_TextLib.nMsgs)
        {
            //已经被使用
            s_RowData[p->lprgScriptEntry[n_Row].rgwOperand[0]].ColVarList[2] = 1;
        }
    }
    m_Grid->set_t_Data(s_Data);
    //m_Grid.SetDataClass(s_RowData.size(), 4, s_RowData);
    //允许从尾部添加,不能插入和删除
    m_Grid->m_popMenuFlags = 0b01001111;
}

void MainWindow::Edit_Explain(int swtch)
{
    assert(WM_EditExplain == swtch);
    WorkCtrl = WM_EditExplain;
    QString msg("对象说明编辑");
    SendMessages(msg);
    //建立表头
    //参数 文字，列宽，列号，（类型 0 什么都不做，1，列头固定(永远显示)，2，Edit），
    //（原数据类型 0  整数 ，1 WORD, 2 SHORT, 3 16进制数，4 浮点数，5 字符串）
    //(下拉框数据索引)，（指向函数的指针，将源数据转化为显示数据字串），（指向函数的指针，将显示字串转化为源）
    T_DATA s_Data;
    ColArray &w_ColData = s_Data.c_Array;
    auto p_SrcToStr = std::bind(&MainWindow::q_SrcToStr, this, std::placeholders::_1);
    w_ColData.resize(3);
    w_ColData[0].GetData("对象号", 80, 0, ctrl_Fix, tINT);
    w_ColData[1].GetData("名称", 90, 1, ctrl_Fix, tINT, nullptr, p_SrcToStr);
    w_ColData[2].GetData("说明", 600, 2, ctrl_Edit, tSTR);

    DataArray &s_RowData = s_Data.d_Array;
    s_RowData.resize(m_Pal->pal->gpGlobals->g_TextLib.nWords);
    for (int n = 0; n < m_Pal->pal->gpGlobals->g_TextLib.nWords; n++)
    {
        s_RowData[n].ColVarList.resize(3);
        s_RowData[n].oldRow = n + 1;
        s_RowData[n].ColVarList[0] = n;
        s_RowData[n].ColVarList[1] = n;
        std::string pDesc = m_Pal->pal->PAL_GetObjectDesc(
            m_Pal->pal->gpGlobals->lpObjectDesc, n).c_str();
        s_RowData[n].ColVarList[2] = pDesc.empty() ? "  " : pDesc;
    }
    m_Grid->set_t_Data(s_Data);
    //不允许添加删除行
    m_Grid->m_popMenuFlags = 0b01011;

}

void MainWindow::Edit_ScriptEntry(int swtch)
{
    QString msg("脚本入口编辑");
    SendMessages(msg);
    WorkCtrl = swtch;
    m_Grid->UndoCtrl = swtch;

    T_DATA s_Data;
    ColArray &w_ColData = s_Data.c_Array;
    const int totalCol = 9;
    w_ColData.resize(totalCol);
    //制做表头
    //参数 文字，列宽，列号，（类型 0 什么都不做，1，列头固定(永远显示)，2，Edit），
    //（原数据类型 0  整数 ，1 WORD, 2 SHORT, 3 16进制数，4 浮点数，5 字符串）
    //(下拉框数据索引)，（指向函数的指针，将源数据转化为显示数据字串），（指向函数的指针，将显示字串转化为源）
    w_ColData[0].GetData("脚本ID", 80, 0, ctrl_Fix, tHEX);
    w_ColData[1].GetData("入口", 80, 1, ctrl_Edit, tHEX);
    w_ColData[2].GetData("参数1", 80, 2, ctrl_Edit, tHEX);
    w_ColData[3].GetData("参数2", 80, 3, ctrl_Edit, tHEX);
    w_ColData[4].GetData("参数3", 80, 4, ctrl_Edit, tHEX);
    w_ColData[5].GetData("调 用", 80, 5, ctrl_Null, tINT, 0, 0, 0, 2);//查看脚本调用

    auto obj = [](int i)->QString {
        return p_Script(i).c_str();
        };
    w_ColData[6].GetData("行号", 70, 6, ctrl_Null, tWORD);
    w_ColData[7].GetData("原行号", 70, 7, ctrl_Null, tWORD);
    w_ColData[8].GetData("说明", 900, 8, ctrl_Null, tSTR);//

    //标记脚本，更改字段pMark
    m_Pal->PAL_MarkScriptAll();

    //表数据
    int n_Row;
    auto p = &m_Pal->pal->gpGlobals->g;

    DataArray & s_RowData = s_Data.d_Array;
    s_RowData.resize(p->nScriptEntry);
    //以下测试脚本有效性

    for (n_Row = 0; n_Row < p->nScriptEntry; n_Row++)
    {
        s_RowData[n_Row].ColVarList.resize(9);
        s_RowData[n_Row].oldRow = n_Row + 1;
        s_RowData[n_Row].ColVarList[0] = (int)n_Row;
        s_RowData[n_Row].ColVarList[1] = p->lprgScriptEntry[n_Row].wOperation;
        s_RowData[n_Row].ColVarList[2] = p->lprgScriptEntry[n_Row].rgwOperand[0];
        s_RowData[n_Row].ColVarList[3] = p->lprgScriptEntry[n_Row].rgwOperand[1];
        s_RowData[n_Row].ColVarList[4] = p->lprgScriptEntry[n_Row].rgwOperand[2];
        s_RowData[n_Row].ColVarList[5] = (INT)m_Pal->pMark[n_Row].s.size();
        s_RowData[n_Row].ColVarList[6] = n_Row + 1;
        s_RowData[n_Row].ColVarList[7] = n_Row + 1;
        //s_RowData[n_Row].ColVarList[8] = p->lprgScriptEntry[n_Row].wOperation;
    }

    //每行显示时运行
    s_Data.Set_Col_Data = [](ColumnClass* s,int row)->VOID
        {
            s->ColVarList[0] = row;
            s->ColVarList[6] = row + 1;
            s->ColVarList[7] = (int)s->oldRow;
            //以下显示打印的文字
            {
                auto str = p_Script(std::get<int>(s->ColVarList[1]));
                if (std::get<int>(s->ColVarList[1]) == 0xffff)
                {
                    str += " 内容: " + m_Pal->pal->PAL_GetMsg(std::get<int>(s->ColVarList[2]));
                }
                s->ColVarList[8] = str;
            }
        };
    m_Grid->set_t_Data(s_Data);
	m_Grid->m_popMenuFlags = 0b011111111; //允许插入删除，允许尾部添加
}

//修改怪物属性
void MainWindow::Edit_Enemy(int swtch)
{
    assert(swtch == WM_EDIT_ENEMY);
    //编辑怪物属性
    WorkCtrl = WM_EDIT_ENEMY;
    m_Grid->UndoCtrl = WM_EDIT_ENEMY;
    QString s(("修改敌方怪物属性"));;
    SendMessages(s);
    //下拉表物品选择数据
    pSelect_Item p_Select_Item = &Select_Item_Array[0];
    pSelect_Item p_Select_Magic = &Select_Item_Array[1];
    pSelect_Item p_Select_All = &Select_Item_Array[3];
    //制作表头
    //参数 文字，列宽，列号，（类型 0 什么都不做，1，列头固定(永远显示)，2，Edit），
    //（原数据类型 0  整数 ，1 WORD, 2 SHORT, 3 16进制数，4 浮点数，5 字符串）
    //(下拉框数据索引)，（指向函数的指针，将源数据转化为显示数据字串），（指向函数的指针，将显示字串转化为源）
    int totalCol = 42;
    T_DATA s_Data{};
    auto& w_ColData = s_Data.c_Array;
    w_ColData.resize(totalCol);
    auto p_SrcToStr = std::bind(&MainWindow::q_SrcToStr, this, std::placeholders::_1);
    w_ColData[0].GetData("ID", 40, 0, ctrl_Fix, tHEX);
    w_ColData[1].GetData("名称", 90, 1, ctrl_Fix, tWORD, nullptr, p_SrcToStr);
    w_ColData[2].GetData("空闲帧数", 60, 2, ctrl_Edit, tWORD);
    w_ColData[3].GetData("魔法帧数", 60, 2, ctrl_Edit, tWORD);
    w_ColData[4].GetData("普攻帧数", 60, 2, ctrl_Edit, tWORD);
    w_ColData[5].GetData("空闲动速", 60, 2, ctrl_Edit, tWORD);
    w_ColData[6].GetData("等待帧数", 60, 2, ctrl_Edit, tWORD);
    w_ColData[7].GetData("Y位移", 60, 2, ctrl_Edit, tSHORT);
    w_ColData[8].GetData("物攻声", 60, 2, ctrl_Edit, tWORD);
    w_ColData[9].GetData("动作声", 60, 2, ctrl_Edit, tWORD);
    w_ColData[10].GetData("魔法声", 60, 2, ctrl_Edit, tWORD);
    w_ColData[11].GetData("死亡声", 60, 2, ctrl_Edit, tWORD);
    w_ColData[12].GetData("进入声", 60, 2, ctrl_Edit, tWORD);
    w_ColData[13].GetData("体力", 60, 2, ctrl_Edit, tWORD);
    w_ColData[14].GetData("经验", 60, 3, ctrl_Edit, tWORD);
    w_ColData[15].GetData("金钱", 60, 4, ctrl_Edit, tWORD);
    w_ColData[16].GetData("等级", 60, 5, ctrl_Edit, tWORD);
    w_ColData[17].GetData("魔法ID", 60, 6, ctrl_List, tNull, p_Select_Magic, p_SrcToStr);
    w_ColData[18].GetData("概率", 60, 7, ctrl_Edit, tWORD);
    w_ColData[19].GetData("攻击附加", 60, 8, ctrl_List, tNull, p_Select_All, p_SrcToStr);
    w_ColData[20].GetData("概率", 60, 9, ctrl_Edit, tWORD);
    w_ColData[21].GetData("偷窃物品", 60, 10, ctrl_List, tNull, p_Select_Item, p_SrcToStr);
    w_ColData[22].GetData("数量", 60, 11, ctrl_Edit, tWORD);
    w_ColData[23].GetData("物攻", 60, 12, ctrl_Edit, tSHORT);
    w_ColData[24].GetData("灵攻", 60, 13, ctrl_Edit, tSHORT);
    w_ColData[25].GetData("防御", 60, 14, ctrl_Edit, tSHORT);
    w_ColData[26].GetData("速度", 60, 15, ctrl_Edit, tSHORT);
    w_ColData[27].GetData("吉运", 60, 16, ctrl_Edit, tSHORT);
    w_ColData[28].GetData("毒抗", 60, 17, ctrl_Edit, tWORD);
    w_ColData[29].GetData("风抗", 60, 18, ctrl_Edit, tWORD);
    w_ColData[30].GetData("雷抗", 60, 19, ctrl_Edit, tWORD);
    w_ColData[31].GetData("水抗", 60, 20, ctrl_Edit, tWORD);
    w_ColData[32].GetData("火抗", 60, 21, ctrl_Edit, tWORD);
    w_ColData[33].GetData("土抗", 60, 22, ctrl_Edit, tWORD);
    w_ColData[34].GetData("物抗", 60, 23, ctrl_Edit, tWORD);
    w_ColData[35].GetData("双击", 60, 24, ctrl_Edit, tWORD);
    w_ColData[36].GetData("灵葫值", 60, 25, ctrl_Edit, tWORD);
    w_ColData[37].GetData("巫抗", 60, 26, ctrl_Edit, tWORD);
    w_ColData[38].GetData("战前脚本", 60, 27, ctrl_Edit, tHEX, 0, 0, 0, 1);
    w_ColData[39].GetData("战斗脚本", 60, 28, ctrl_Edit, tHEX, 0, 0, 0, 1);
    w_ColData[40].GetData("战后脚本", 60, 29, ctrl_Edit, tHEX, 0, 0, 0, 1);
    w_ColData[41].GetData("行号", 60, 30, ctrl_Null, tWORD);

    //m_Grid->SetColClass(42, w_ColData);

    //生成数据
    auto p = &m_Pal->pal->gpGlobals->g;
    
    int n_Count = 0;

    for (WORD j = 0; j < MAX_OBJECTS; j++)
    {
        if (m_Pal->gpObject_classify[j] != kIsEnemy)
            continue;
        WORD t{};
        t = p->rgObject[j].enemy.wEnemyID;
        assert(t >= 0 && t < m_Pal->pal->gpGlobals->g.nEnemy);
        ColumnClass ss{};
        LPENEMY pEnemy = &(p->lprgEnemy[t]);
        WORD* wp_State = &(pEnemy->wIdleFrames);

        ss.ColVarList.resize(totalCol);
        ss.oldRow = n_Count + 1;

        ss.ColVarList[0] = j;
        ss.ColVarList[1] = j;
        for (int k = 2; k < 37; k++)
        {
            if (k < 23)
                ss.ColVarList[k] = (int)(wp_State[k - 2]);
            else
                ss.ColVarList[k] = (SHORT)(wp_State[k - 2]);
        }
        OBJECT_ENEMY  &enemy = p->rgObject[j].enemy;
        ss.ColVarList[37] =  enemy.wResistanceToSorcery;
        ss.ColVarList[38] =  enemy.wScriptOnTurnStart;
        ss.ColVarList[39] =  enemy.wScriptOnReady;
        ss.ColVarList[40] =  enemy.wScriptOnBattleEnd;
        ss.ColVarList[41] = t;
        s_Data.d_Array.push_back(std::move(ss));
        n_Count++;
    }
    assert(s_Data.d_Array.size() == m_Pal->nEnemy);
    m_Grid->set_t_Data(s_Data);
    m_Grid->m_popMenuFlags = 0b0001011;
    return ;
}

void MainWindow::Edit_EnemyTeam(int swtch)
{
    WorkCtrl = WM_EditEnemyTeam;
    m_Grid->UndoCtrl = WM_EditEnemyTeam;
    QString s("敌方队伍");
    SendMessages(s);
    T_DATA s_Data;
    ColArray &w_ColData = s_Data.c_Array;
    int totalCol = 7;

    w_ColData.resize(totalCol);
    //制做表头
    //参数 文字，列宽，列号，（类型 0 什么都不做，1，列头固定(永远显示)，2，Edit），
    //（原数据类型 0  整数 ，1 WORD, 2 SHORT, 3 16进制数，4 浮点数，5 字符串）
    //(下拉框数据索引)，（指向函数的指针，将源数据转化为显示数据字串），（指向函数的指针，将显示字串转化为源）
    auto p_SrcToStr = std::bind(&MainWindow::q_SrcToStr, this, std::placeholders::_1);
    pSelect_Item p_Select_Enemy = &Select_Item_Array[4];//下拉表

    w_ColData[0].GetData("队伍号", 50, 0, ctrl_Fix, tWORD);
    w_ColData[1].GetData("位1名", 60, 1, ctrl_List, tNull, p_Select_Enemy, p_SrcToStr);
    w_ColData[2].GetData("位2名", 60, 2, ctrl_List, tNull, p_Select_Enemy, p_SrcToStr);
    w_ColData[3].GetData("位3名", 60, 3, ctrl_List, tNull, p_Select_Enemy, p_SrcToStr);
    w_ColData[4].GetData("位4名", 60, 4, ctrl_List, tNull, p_Select_Enemy, p_SrcToStr);
    w_ColData[5].GetData("位5名", 60, 5, ctrl_List, tNull, p_Select_Enemy, p_SrcToStr);
	w_ColData[6].GetData("原队伍号", 50, 6, ctrl_Null, tWORD);
    //生成数据
    DataArray  &s_RowData = s_Data.d_Array;
    auto p = &m_Pal->pal->gpGlobals->g;
    s_RowData.resize(p->nEnemyTeam);
    int n_EnemyTeam;
    for (n_EnemyTeam = 0; n_EnemyTeam < p->nEnemyTeam; n_EnemyTeam++)
    {
        s_RowData[n_EnemyTeam].ColVarList.resize(7);
        s_RowData[n_EnemyTeam].oldRow = n_EnemyTeam + 1;

		s_RowData[n_EnemyTeam].ColVarList[0] = n_EnemyTeam + 1; //队伍号从1开始
        for (int n = 0; n < MAX_ENEMIES_IN_TEAM; n++)
        {
            s_RowData[n_EnemyTeam].ColVarList[n + 1] = p->lprgEnemyTeam[n_EnemyTeam].rgwEnemy[n];
        }
		s_RowData[n_EnemyTeam].ColVarList[6] = n_EnemyTeam + 1; //原队伍号
    }
	//设置列数据函数
    s_Data.Set_Col_Data = [](ColumnClass* s, int row)->VOID
        {
            s->ColVarList[0] = row + 1;
            s->ColVarList[6] = (int)s->oldRow;
        };
    m_Grid->set_t_Data(s_Data);
	m_Grid->m_popMenuFlags = 0b01101011;//允许从尾部添加，允许插入行
}

//修改魔法
void MainWindow::Edit_Magic(int swtch)
{
    assert(swtch == WM_EDIT_MAGIC);
    WorkCtrl = WM_EDIT_MAGIC;
    m_Grid->UndoCtrl = WM_EDIT_MAGIC;
    QString msg("修改魔法");
    SendMessages(msg);
    //建立表头
    //参数 文字，列宽，列号，（类型 0 什么都不做，1，列头固定(永远显示)，2，Edit），
    //（原数据类型 0  整数 ，1 WORD, 2 SHORT, 3 16进制数，4 浮点数，5 字符串）
    //(下拉框数据索引)，（指向函数的指针，将源数据转化为显示数据字串），（指向函数的指针，将显示字串转化为源）
    const int totalCol = 24;
    T_DATA s_Data;
    ColArray& w_ColData = s_Data.c_Array;
    auto p_SrcToStr = std::bind(&MainWindow::q_SrcToStr, this, std::placeholders::_1);
    w_ColData.resize(totalCol);
    w_ColData[0].GetData("ID", 50, 0, ctrl_Fix, tHEX);
    w_ColData[1].GetData("名称", 80, 1, ctrl_Fix, tWORD, nullptr, p_SrcToStr, nullptr);
    w_ColData[2].GetData("形象号", 60, 2, ctrl_Edit, tSHORT);
    w_ColData[3].GetData("类型", 60, 3, ctrl_Edit, tSHORT);
    w_ColData[4].GetData("X位移", 60, 4, ctrl_Edit, tSHORT);
    w_ColData[5].GetData("Y位移", 60, 5, ctrl_Edit, tSHORT);
    w_ColData[6].GetData("效果号", 60, 6, ctrl_Edit, tSHORT);
    w_ColData[7].GetData("速度", 60, 7, ctrl_Edit, tSHORT);
    w_ColData[8].GetData("保持形象", 60, 8, ctrl_Edit, tSHORT);
    w_ColData[9].GetData("声效延迟", 60, 9, ctrl_Edit, tSHORT);
    w_ColData[10].GetData("耗时", 60, 10, ctrl_Edit, tSHORT);
    w_ColData[11].GetData("场景震动", 60, 11, ctrl_Edit, tSHORT);
    w_ColData[12].GetData("场景波动", 60, 12, ctrl_Edit, tSHORT);
    w_ColData[13].GetData("保留", 60, 13, ctrl_Edit, tWORD);
    w_ColData[14].GetData("耗兰", 60, 14, ctrl_Edit, tSHORT);
    w_ColData[15].GetData("基础伤害", 60, 15, ctrl_Edit, tSHORT);
    w_ColData[16].GetData("属性", 60, 16, ctrl_Edit, tSHORT);
    w_ColData[17].GetData("声效", 60, 17, ctrl_Edit, tWORD);
    w_ColData[18].GetData("标志", 60, 18, ctrl_Edit, tSHORT);
    w_ColData[19].GetData("使用脚本", 60, 19, ctrl_Edit, tHEX, 0, 0, 0, 1);
    w_ColData[20].GetData("成功脚本", 60, 20, ctrl_Edit, tHEX, 0, 0, 0, 1);
    w_ColData[21].GetData("魔法号", 60, 21, ctrl_Null, tSHORT);
    w_ColData[22].GetData("序号", 60, 22, ctrl_Null, tSHORT);
    w_ColData[23].GetData("行号", 50, 23, ctrl_Null, tINT);

    //表数据
    DataArray& s_RowData = s_Data.d_Array;
    auto p = &m_Pal->pal->gpGlobals->g;
    s_RowData.resize(p->nMagic);//行数
    int n_Count = 0;
    int j;
    for (j = 0; j < MAX_OBJECTS; j++)
    {
        if (!(m_Pal->gpObject_classify[j] & kIsMagic))
            continue;
        WORD k;
        WORD* obj;
            k = p->rgObject[j].magic.wMagicNumber;
            obj = p->rgObject[j].rgwData;
        LPMAGIC pMagic = &(p->lprgMagic[k]);
        WORD* wp_State = (WORD*)pMagic;
        s_RowData[n_Count].ColVarList.resize(totalCol);
        s_RowData[n_Count].ColVarList[0] = j;
        s_RowData[n_Count].ColVarList[1] = j;
        for (int s = 2; s < 18; s++)
        {
            s_RowData[n_Count].ColVarList[s] = (SHORT)wp_State[s - 2];
        }
        s_RowData[n_Count].ColVarList[18] = obj[6];// lpObj[j].magic.wFlags;//5-6
        s_RowData[n_Count].ColVarList[19] = obj[3];//lpObj[j].magic.wScriptOnUse;//3
        s_RowData[n_Count].ColVarList[20] = obj[2];// lpObj[j].magic.wScriptOnSuccess;//2
        s_RowData[n_Count].ColVarList[21] = obj[0];// lpObj[j].magic.wMagicNumber;//0

        s_RowData[n_Count].ColVarList[22] = k;//=21
        //
        s_RowData[n_Count].ColVarList[23] = n_Count + 1;
        n_Count++;
    }
    m_Grid->set_t_Data(s_Data);
    m_Grid->m_popMenuFlags = 0b0001011;
}

void MainWindow::Edit_Map(int swtch)
{
    Q_UNUSED(swtch);
    //地图编辑
    QString msg("地图编辑");
    SendMessages(msg);
    CPixEdit* w = new CMapEdit(m_Pal, this);
    w->resize(width(), height());
    w->exec();
    delete w;
}

//修改玩家属性
void MainWindow::Edit_PlayerAttributes(int work)
{
    WorkCtrl = work;
    m_Grid->UndoCtrl = work;;
    T_DATA s_Data{};
    auto &w_ColData = s_Data.c_Array;
    const int colCount{ 64 };
    w_ColData.resize(0);
    w_ColData.resize(colCount);
    //下拉表选择数据
    pSelect_Item p_Select_Item = &Select_Item_Array[0];
    pSelect_Item p_Select_Magic = &Select_Item_Array[1];
    
    SrcToStr p_SrcToStr = [&](WORD n)->QString {return m_Pal->pal->PAL_GetWord(n).c_str(); };
    //制做表头
    //参数 文字，列宽，列号，（类型 0 什么都不做，1，列头固定(永远显示)，2，Edit），
    //（原数据类型 0  整数 ，1 WORD, 2 SHORT, 3 16进制数，4 浮点数，5 字符串）
    //(下拉框数据索引)，（指向函数的指针，将源数据转化为显示数据字串），（指向函数的指针，将显示字串转化为源）
    w_ColData[0].GetData("编号", 40, 0, ctrl_Fix, tWORD);
    w_ColData[1].GetData("姓名", 80, 1, ctrl_Fix, tWORD, nullptr, p_SrcToStr);
    w_ColData[2].GetData("等级", 60, 2, ctrl_Edit, tWORD);
    w_ColData[3].GetData("体力MAX", 60, 3, ctrl_Edit, tWORD);
    w_ColData[4].GetData("真气MAX", 60, 4, ctrl_Edit, tWORD);
    w_ColData[5].GetData("体力", 60, 5, ctrl_Edit, tWORD);
    w_ColData[6].GetData("真气", 60, 6, ctrl_Edit, tWORD);
    w_ColData[7].GetData("武力", 60, 7, ctrl_Edit, tWORD);
    w_ColData[8].GetData("灵气", 60, 8, ctrl_Edit, tWORD);
    w_ColData[9].GetData("防御", 60, 9, ctrl_Edit, tWORD);
    w_ColData[10].GetData("速度", 60, 10, ctrl_Edit, tWORD);
    w_ColData[11].GetData("吉运", 60, 11, ctrl_Edit, tWORD);
    w_ColData[12].GetData("毒抗", 60, 12, ctrl_Edit, tWORD);
    w_ColData[13].GetData("风抗", 60, 13, ctrl_Edit, tWORD);
    w_ColData[14].GetData("雷抗", 60, 14, ctrl_Edit, tWORD);
    w_ColData[15].GetData("水抗", 60, 15, ctrl_Edit, tWORD);
    w_ColData[16].GetData("火抗", 60, 16, ctrl_Edit, tWORD);
    w_ColData[17].GetData("土抗", 60, 17, ctrl_Edit, tWORD);
    w_ColData[18].GetData("巫抗", 60, 18, ctrl_Edit, tWORD);
    w_ColData[19].GetData("物抗", 60, 19, ctrl_Edit, tWORD);
    w_ColData[20].GetData("巫攻", 60, 20, ctrl_Edit, tWORD);
    w_ColData[21].GetData("头", 90, 21, ctrl_List, tNull, p_Select_Item, p_SrcToStr);
    w_ColData[22].GetData("肩", 90, 22, ctrl_List, tNull, p_Select_Item, p_SrcToStr);
    w_ColData[23].GetData("身", 90, 23, ctrl_List, tNull, p_Select_Item, p_SrcToStr);
    w_ColData[24].GetData("手", 90, 24, ctrl_List, tNull, p_Select_Item, p_SrcToStr);
    w_ColData[25].GetData("脚", 90, 25, ctrl_List, tNull, p_Select_Item, p_SrcToStr);
    w_ColData[26].GetData("挂", 90, 26, ctrl_List, tNull, p_Select_Item, p_SrcToStr);
    w_ColData[27].GetData("救援", 90, 27, ctrl_Edit, tWORD);
    w_ColData[28].GetData("合体法术", 90, 28, ctrl_List, tNull, p_Select_Magic, p_SrcToStr);
    for (WORD n = 29; n < 61; n++)
    {
        w_ColData[n].GetData(m_Pal->pal->va("法术%2.2d", n - 28), 60, n, ctrl_List, tNull, p_Select_Magic, p_SrcToStr);
    }
    //队友死 位2
    w_ColData[61].GetData("队友死", 90, 61, ctrl_Edit, tHEX, 0, 0, 0, 1);//是脚本
    //濒死 位3
    w_ColData[62].GetData("濒死", 90, 62, ctrl_Edit, tHEX, 0, 0, 0, 1);//是脚本
    //行号
    w_ColData[63].GetData("行号", 60, 63, ctrl_Null, tINT);

    //m_Grid->SetColClass(colCount, w_ColData);

    //生成表数据
                //插入子项数据
            /*
            参数1：主角属性地址——基本属性
            00：状态表情图像
            01：战斗模型
            02：地图模型
            03：名字
            04：可否攻击全体
            05：无效？
            06：等级
            07：体力最大值
            08：真气最大值
            09：体力
            0A：真气
            0B-10,装备
            11：武术
            12：灵力
            13：防御
            14：身法
            15：吉运
            16：毒抗
            17：风抗
            18：雷抗
            19：水抗
            1A：火抗
            1B：土抗
            1C:	巫抗
            1D：物抗
            1E：巫攻
            1F：救援
            20--3F魔法
            40？......
            41：合体法术
            */
    auto& s_RowData = s_Data.d_Array;
    s_RowData.resize(MAX_PLAYER_ROLES);//行数

    WORD* p_list = (WORD*)&(m_Pal->pal->gpGlobals->g.PlayerRoles);

    for (int r = 0; r < MAX_PLAYER_ROLES; r++)
    {
        s_RowData[r].ColVarList.resize(colCount);
        s_RowData[r].ColVarList[0] = r + 1;//行号从1开始
        for (int n = 1, i; n < 62; n++)
        {
            if (n == 1)
                i = n + 2;//名称
            else if (n > 1 && n <= 6)
                i = n + 4;//等级-真气
            else if (n > 6 && n <= 20)
                i = n + 10;//武力--巫抗
            else if (n > 20 && n < 27)
                i = n - 10;//装备
            else if (n == 27)
                i = 0x1f;//救援
            else if (n == 28)
                i = 0x41;//合体法术
            else if (n >= 29 && n < 61)
                i = n + 3;
            s_RowData[r].ColVarList[n] = (WORD)p_list[(i)*MAX_PLAYER_ROLES + r];
        }
        WORD s1 = 0, s2 = 0;
        auto name = m_Pal->pal->gpGlobals->g.PlayerRoles.rgwName[r];
        s1 = m_Pal->pal->gpGlobals->g.rgObject[name].player.wScriptOnFriendDeath;
        s2 = m_Pal->pal->gpGlobals->g.rgObject[name].player.wScriptOnDying;
        //队友死
        s_RowData[r].ColVarList[61] = s1;
        //虚弱
        s_RowData[r].ColVarList[62] = s2;
        //行号
        s_RowData[r].ColVarList[63] = r + 1;
    } 
    //m_Grid->SetDataClass(MAX_PLAYER_ROLES, colCount, s_RowData);
    m_Grid->set_t_Data(s_Data);
    m_Grid->m_popMenuFlags = 0b01011;
}

//物品编辑
void MainWindow::Edit_ObjectItem(int swtch)
{
    WorkCtrl = WM_EditObjectItem;
    m_Grid->UndoCtrl = WM_EditObjectItem;

    //物品
    QString s("修改物品");
    SendMessages(s);
    T_DATA s_Data;

    ColArray& w_ColData = s_Data.c_Array;
    const int totalCol = 20;
    w_ColData.resize(totalCol);
    //制做表头
    //参数 文字，列宽，列号，（类型 0 什么都不做，1，列头固定(永远显示)，2，Edit），
    //（原数据类型 0  整数 ，1 WORD, 2 SHORT, 3 16进制数，4 浮点数，5 字符串,6 布尔）
    //(下拉框数据索引)，（指向函数的指针，将源数据转化为显示数据字串），（指向函数的指针，将显示字串转化为源）
    SrcToStr p_SrcToStr = std::bind(&MainWindow::q_SrcToStr, this, std::placeholders::_1);
    w_ColData[0].GetData("ID", 90, 0, ctrl_Fix, tHEX);
    w_ColData[1].GetData("品名", 120, 1, ctrl_Fix, tWORD, nullptr, p_SrcToStr);
    w_ColData[2].GetData("标志", 90, 2, ctrl_Edit, tHEX);
    w_ColData[3].GetData("价格", 90, 3, ctrl_Edit, tWORD);
    w_ColData[4].GetData("装备", 90, 4, ctrl_Edit, tHEX, 0, 0, 0, 1);
    w_ColData[5].GetData("投掷", 90, 5, ctrl_Edit, tHEX, 0, 0, 0, 1);
    w_ColData[6].GetData("使用", 90, 6, ctrl_Edit, tHEX, 0, 0, 0, 1);
    w_ColData[7].GetData("可使用", 90, 7, ctrl_Edit, tBOOL);
    w_ColData[8].GetData("可装备", 90, 8, ctrl_Edit, tBOOL);
    w_ColData[9].GetData("可投掷", 90, 9, ctrl_Edit, tBOOL);
    w_ColData[10].GetData("可消耗", 90, 10, ctrl_Edit, tBOOL);
    w_ColData[11].GetData("全体", 90, 11, ctrl_Edit, tBOOL);
    w_ColData[12].GetData("可出售", 90, 12, ctrl_Edit, tBOOL);
    w_ColData[13].GetData("李装备", 90, 13, ctrl_Edit, tBOOL);
    w_ColData[14].GetData("赵装备", 90, 14, ctrl_Edit, tBOOL);
    w_ColData[15].GetData("林装备", 90, 15, ctrl_Edit, tBOOL);
    w_ColData[16].GetData("巫装备", 90, 16, ctrl_Edit, tBOOL);
    w_ColData[17].GetData("阿装备", 90, 17, ctrl_Edit, tBOOL);
    w_ColData[18].GetData("盖装备", 90, 18, ctrl_Edit, tBOOL);
    w_ColData[19].GetData("图片号", 90, 3, ctrl_Edit, tWORD);

    //生成数据
    DataArray & s_RowData = s_Data.d_Array;
    s_RowData.resize(m_Pal->nItem);
    auto p = &m_Pal->pal->gpGlobals->g;
    int n_Row = 0, j;
    for (j = 0; j < MAX_OBJECTS; j++)
    {
        auto p = &m_Pal->pal->gpGlobals->g;
        if (!(m_Pal->gpObject_classify[j] & kIsItem))
            continue;
        auto s_Item = p->rgObject[j].item;
        auto lpObj = p->rgObject;
        WORD nFlags = lpObj[j].item.wFlags;
        s_RowData[n_Row].ColVarList.resize(totalCol);
        s_RowData[n_Row].oldRow = n_Row + 1;
        s_RowData[n_Row].ColVarList[0] = j;
        s_RowData[n_Row].ColVarList[1] = j;
        s_RowData[n_Row].ColVarList[2] = lpObj[j].item.wFlags;
        s_RowData[n_Row].ColVarList[3] = lpObj[j].item.wPrice;
        s_RowData[n_Row].ColVarList[4] = lpObj[j].item.wScriptOnEquip;
        s_RowData[n_Row].ColVarList[5] = lpObj[j].item.wScriptOnThrow;
        s_RowData[n_Row].ColVarList[6] = lpObj[j].item.wScriptOnUse;
        s_RowData[n_Row].ColVarList[7] = nFlags & (1 << 0) ? 1 : 0;
        s_RowData[n_Row].ColVarList[8] = nFlags & (1 << 1) ? 1 : 0;
        s_RowData[n_Row].ColVarList[9] = nFlags & (1 << 2) ? 1 : 0;
        s_RowData[n_Row].ColVarList[10] = nFlags & (1 << 3) ? 1 : 0;
        s_RowData[n_Row].ColVarList[11] = nFlags & (1 << 4) ? 1 : 0;
        s_RowData[n_Row].ColVarList[12] = nFlags & (1 << 5) ? 1 : 0;
        s_RowData[n_Row].ColVarList[13] = nFlags & (1 << 6) ? 1 : 0;
        s_RowData[n_Row].ColVarList[14] = nFlags & (1 << 7) ? 1 : 0;
        s_RowData[n_Row].ColVarList[15] = nFlags & (1 << 8) ? 1 : 0;
        s_RowData[n_Row].ColVarList[16] = nFlags & (1 << 9) ? 1 : 0;
        s_RowData[n_Row].ColVarList[17] = nFlags & (1 << 10) ? 1 : 0;
        s_RowData[n_Row].ColVarList[18] = nFlags & (1 << 11) ? 1 : 0;
        s_RowData[n_Row].ColVarList[19] = lpObj[j].item.wBitmap;
        n_Row++;
    }
    //m_Grid.SetDataClass(n_Row, 20, s_RowData);
    s_Data.d_Array.resize(n_Row);
    m_Grid->set_t_Data(s_Data);
    //不允许添加删除行
    m_Grid->m_popMenuFlags = 0b0001011;
}

void MainWindow::Edit_ObjectPict(int swtch)
{
    QString msg = ("对象图像编辑");
    SendMessages(msg);

    QString fileName = QFileDialog::getOpenFileName(
        this,
        tr("打开图像文件"),
        m_Pal->pal->PalDir.c_str(),
        tr("对象图像 (mgo.mkf);;敌方战斗图像 (abc.mkf);;我方战斗图像(f.mkf);;魔法特效(fire.mkf);;背景图片 (fbp.mkf)"
            ";;头像 (rgm.mkf);;对象图片 (ball.mkf);;地图文件 (map.mkf);;All Files(*.*)")
    ).toLower();

    if (fileName.isEmpty()) {
        // 空文件返回
        return;
    }
    CPixEdit* w = new PackedPict_Dlg(m_Pal, fileName.toStdString(), this);
    w->resize(width(), height());
    w->exec();
    delete w;
};

void MainWindow::Edit_Parameters(int swtch)
{
    WorkCtrl = WM_EDIT_PARAMETERS;
    m_Grid->UndoCtrl = WM_EDIT_PARAMETERS;
    QString msg = "修改基本属性";
    SendMessages(msg);
    //建立表头
    //参数 文字，列宽，列号，（类型 0 什么都不做，1，列头固定(永远显示)，2，Edit），
    //（原数据类型 0  整数 ，1 WORD, 2 SHORT, 3 16进制数，4 浮点数，5 字符串）
    //(下拉框数据索引)，（指向函数的指针，将源数据转化为显示数据字串），（指向函数的指针，将显示字串转化为源）
    T_DATA s_Data;
    ColArray &w_ColData = s_Data.c_Array;
    w_ColData.resize(2);
    w_ColData[0].GetData("项目", 90, 0, ctrl_Fix, tSTR);
    w_ColData[1].GetData("数额", 120, 1, ctrl_Edit, tWORD);

    DataArray & s_RowData = s_Data.d_Array;
    auto p = m_Pal->pal->gpGlobals;
    s_RowData.resize(2);//行数
    s_RowData[0].ColVarList.resize(2);
    s_RowData[0].ColVarList[0] = "金钱";
    s_RowData[0].ColVarList[1] = (int)p->dwCash;
    s_RowData[1].ColVarList.resize(2);
    s_RowData[1].ColVarList[0] = "灵壶值";
    s_RowData[1].ColVarList[1] = p->wCollectValue;
    m_Grid->set_t_Data(s_Data);
}

void MainWindow::Edit_PlayerLevelUPMagic(int swtch)
{
    assert(swtch == WM_PLAYERLEVELUPMAGIG);
    //升级魔法
    WorkCtrl = WM_PLAYERLEVELUPMAGIG;
    m_Grid->UndoCtrl = WM_PLAYERLEVELUPMAGIG;
    //下拉表选择数据
    pSelect_Item p_Select_Magic = &Select_Item_Array[1];
    //升级魔法
    QString s = "修改升级魔法";
    SendMessages(s);

    const int colCount{ 11 };
    T_DATA s_Data{};
    ColArray& w_ColData = s_Data.c_Array;
    w_ColData.resize(colCount);
    SrcToStr p_SrcToStr = std::bind(&MainWindow::q_SrcToStr, this, std::placeholders::_1);
    //制做表头
    //参数 文字，列宽，列号，（类型 0 什么都不做，1，列头固定(永远显示)，2，Edit），
    //（原数据类型 0  整数 ，1 WORD, 2 SHORT, 3 16进制数，4 浮点数，5 字符串）
    //(下拉框数据索引)，（指向函数的指针，将源数据转化为显示数据字串），（指向函数的指针，将显示字串转化为源）
    w_ColData[0].GetData("序号", 60, 0, ctrl_Fix, tWORD);
    w_ColData[1].GetData("李等级", 90, 1, ctrl_Edit, tWORD);
    w_ColData[2].GetData("魔法", 90, 2, ctrl_List, tNull, p_Select_Magic, p_SrcToStr);
    w_ColData[3].GetData("赵等级", 90, 3, ctrl_Edit, tWORD);
    w_ColData[4].GetData("魔法", 90, 4, ctrl_List, tNull, p_Select_Magic, p_SrcToStr);
    w_ColData[5].GetData("林等级", 90, 5, ctrl_Edit, tWORD);
    w_ColData[6].GetData("魔法", 90, 6, ctrl_List, tNull, p_Select_Magic, p_SrcToStr);
    w_ColData[7].GetData("巫后等级", 90, 7, ctrl_Edit, tWORD);
    w_ColData[8].GetData("魔法", 90, 8, ctrl_List, tNull, p_Select_Magic, p_SrcToStr);
    w_ColData[9].GetData("阿奴等级", 90, 9, ctrl_Edit, tWORD);
    w_ColData[10].GetData("魔法", 90, 10, ctrl_List, tNull, p_Select_Magic, p_SrcToStr);

    //表数据
    DataArray &s_RowData = s_Data.d_Array;
    s_RowData.resize(m_Pal->pal->gpGlobals->g.nLevelUpMagic);//行数
    int k = 0;
    for (k = 0; k < m_Pal->pal->gpGlobals->g.nLevelUpMagic; k++)
    {
        s_RowData[k].ColVarList.resize(colCount);
        s_RowData[k].oldRow = k + 1;

        s_RowData[k].ColVarList[0] = k + 1;
        for (size_t j = 0; j < MAX_PLAYABLE_PLAYER_ROLES; j++)
        {
            s_RowData[k].ColVarList[j * 2 + 1] = m_Pal->pal->gpGlobals->g.lprgLevelUpMagic[k].m[j].wLevel;
            s_RowData[k].ColVarList[j * 2 + 2] = m_Pal->pal->gpGlobals->g.lprgLevelUpMagic[k].m[j].wMagic;
        }
    }
    m_Grid->set_t_Data(s_Data);
    m_Grid->m_popMenuFlags = 0b11111011;
    return ;
}

//毒药编辑
void MainWindow::Edit_Poison(int swtch)
{
    WorkCtrl = WM_EditPoison;
    m_Grid->UndoCtrl = WM_EditPoison;
    //事件对象
    QString s("毒药属性编辑");
    SendMessages(s);

    const int totalCol = 6;
    T_DATA s_Data;
    ColArray &w_ColData = s_Data.c_Array;
    w_ColData.resize(totalCol);
    //制做表头
    //参数 文字，列宽，列号，（类型 0 什么都不做，1，列头固定(永远显示)，2，Edit），
    //（原数据类型 0  整数 ，1 WORD, 2 SHORT, 3 16进制数，4 浮点数，5 字符串）
    //(下拉框数据索引)，（指向函数的指针，将源数据转化为显示数据字串），（指向函数的指针，将显示字串转化为源）
    auto p_SrcToStr = std::bind(&MainWindow::q_SrcToStr, this, std::placeholders::_1);
    w_ColData[0].GetData("ID", 90, 0, ctrl_Fix, tHEX);
    w_ColData[1].GetData("品名", 120, 1, ctrl_Fix, tWORD, nullptr, p_SrcToStr);
    w_ColData[2].GetData("等级", 90, 2, ctrl_Edit, tWORD);
    w_ColData[3].GetData("颜色", 90, 3, ctrl_Edit, tWORD);
    w_ColData[4].GetData("我方", 90, 4, ctrl_Edit, tHEX, 0, 0, 0, 1);
    w_ColData[5].GetData("敌方", 90, 5, ctrl_Edit, tHEX, 0, 0, 0, 1);
    //m_Grid.SetColClass(6, w_ColData);

    //生成数据
    DataArray& s_RowData = s_Data.d_Array;
    s_RowData.resize(m_Pal->nPoisonID);
    auto p = &m_Pal->pal->gpGlobals->g;
    int n_Row = 0, j{};

    for (j = 0; j < MAX_OBJECTS; j++)
    {
        if (!(m_Pal->gpObject_classify[j] & kIsPoison))
            continue;
        s_RowData[n_Row].ColVarList.resize(totalCol);
        s_RowData[n_Row].oldRow = n_Row + 1;
        s_RowData[n_Row].ColVarList[0] = j;
        s_RowData[n_Row].ColVarList[1] = j;
        OBJECT_POISON* sPoison{};
        sPoison = &p->rgObject[j].poison;
        s_RowData[n_Row].ColVarList[2] = sPoison->wPoisonLevel;
        s_RowData[n_Row].ColVarList[3] = sPoison->wColor & 0xff;
        s_RowData[n_Row].ColVarList[4] = sPoison->wPlayerScript;
        s_RowData[n_Row].ColVarList[5] = sPoison->wEnemyScript;
        n_Row++;
    }
    //m_Grid.SetDataClass(n_Row, 6, s_RowData);
    assert(n_Row == m_Pal->nPoisonID);
    //不允许添加删除行
    m_Grid->m_popMenuFlags = 0b0001011;
    m_Grid->set_t_Data(s_Data);
}

void MainWindow::Edit_Scene(int swtch)
{
    WorkCtrl = WM_EditScene;
    m_Grid->UndoCtrl = WM_EditScene;
    //事件对象
    QString s("场景属性编辑");
    SendMessages(s);

    T_DATA s_Data;
    ColArray & w_ColData  = s_Data.c_Array;
    const int totalCol = 6;
    w_ColData.resize(totalCol);
    //制做表头
    //参数 文字，列宽，列号，（类型 0 什么都不做，1，列头固定(永远显示)，2，Edit），
    //（原数据类型 0  整数 ，1 WORD, 2 SHORT, 3 16进制数，4 浮点数，5 字符串）
    //(下拉框数据索引)，（指向函数的指针，将源数据转化为显示数据字串），（指向函数的指针，将显示字串转化为源）
    w_ColData[0].GetData("场景号", 100, 0, ctrl_Fix, tINT);
    w_ColData[1].GetData("进入脚本", 100, 1, ctrl_Edit, tHEX, 0, 0, 0, 1);
    w_ColData[2].GetData("传送脚本", 100, 2, ctrl_Edit, tHEX, 0, 0, 0, 1);
    w_ColData[3].GetData("开始索引", 100, 3, ctrl_Edit, tINT);
    w_ColData[4].GetData("结束索引", 100, 4, ctrl_Edit, tINT);
    w_ColData[5].GetData("地图号", 100, 5, ctrl_Edit, tINT);

    //m_Grid.SetColClass(6, w_ColData);

    //生成数据
    DataArray& s_RowData = s_Data.d_Array;
    auto p = &(m_Pal->pal->gpGlobals->g);
    s_RowData.resize(MAX_SCENES);
    int n_Row = 0, j;

    for (j = 0; j < MAX_SCENES; j++)
    {
        s_RowData[n_Row].ColVarList.resize(totalCol);
        s_RowData[n_Row].oldRow = n_Row + 1;
        s_RowData[n_Row].ColVarList[0] = n_Row + 1;
        s_RowData[n_Row].ColVarList[1] = p->rgScene[j].wScriptOnEnter;
        s_RowData[n_Row].ColVarList[2] = p->rgScene[j].wScriptOnTeleport;
        s_RowData[n_Row].ColVarList[3] = p->rgScene[j].wEventObjectIndex;
        s_RowData[n_Row].ColVarList[4] = j < MAX_SCENES - 1 ? p->rgScene[j + 1].wEventObjectIndex - 1 : 0;
        s_RowData[n_Row].ColVarList[5] = p->rgScene[j].wMapNum;
        n_Row++;
    }
    m_Grid->set_t_Data(s_Data);
	m_Grid->m_popMenuFlags = 0b10000001011;//支持在地图上部署第11位，
    m_Grid->indexScene = 0;
}

void MainWindow::Edit_Invenyory(int swtch)
{
    WorkCtrl = WM_Inventory;
    m_Grid->UndoCtrl = WM_Inventory;
    //事件对象
    QString s("背包物品编辑");
    SendMessages(s);

    T_DATA s_Data;
    ColArray& w_ColData = s_Data.c_Array;
    const int totalCol = 3;
    w_ColData.resize(totalCol);
    auto p_SrcToStr = std::bind(&MainWindow::q_SrcToStr, this, std::placeholders::_1);
    //制作下拉框数据
    pSelect_Item p_Select_Item = &Select_Item_Array[0];
    //制做表头
    //参数 文字，列宽，列号，（类型 0 什么都不做，1，列头固定(永远显示)，2，Edit），
    //（原数据类型 0  整数 ，1 WORD, 2 SHORT, 3 16进制数，4 浮点数，5 字符串）
    //(下拉框数据索引)，（指向函数的指针，将源数据转化为显示数据字串），（指向函数的指针，将显示字串转化为源）
    w_ColData[0].GetData("序号", 100, 0, ctrl_Fix, tNull);
    w_ColData[1].GetData("品名", 120, 1, ctrl_List, tNull, p_Select_Item, p_SrcToStr);
    w_ColData[2].GetData("数量", 100, 2, ctrl_Edit, tINT);

    //生成数据
    DataArray& s_RowData = s_Data.d_Array;
    s_RowData.resize(MAX_INVENTORY);
    int n_Row = 0, j;

    for (j = 0; j < MAX_INVENTORY; j++)
    {
        if (m_Pal->pal->gpGlobals->rgInventory[j].nAmount == 0)
            continue;
        s_RowData[n_Row].ColVarList.resize(totalCol);
        s_RowData[n_Row].oldRow = j + 1;
        s_RowData[n_Row].ColVarList[0] = n_Row + 1;
        s_RowData[n_Row].ColVarList[1] = m_Pal->pal->gpGlobals->rgInventory[j].wItem;
        s_RowData[n_Row].ColVarList[2] = m_Pal->pal->gpGlobals->rgInventory[j].nAmount;
        n_Row++;
    }
    s_RowData.resize(n_Row);
    s_Data.Set_Col_Data = [](ColumnClass* s, int row)->void {s->ColVarList[0] = row + 1; };
    m_Grid->set_t_Data(s_Data);
    m_Grid->m_popMenuFlags = 0b00011111011;
}
void MainWindow::Edit_ObjectName(int swtch)
{
    QString msg("对象名称编辑");
    SendMessages(msg);
    WorkCtrl = WM_EditObjectName;
    m_Grid->UndoCtrl = WM_EditObjectName;
    //建立表头
    //参数 文字，列宽，列号，（类型 0 什么都不做，1，列头固定(永远显示)，2，Edit），
    //（原数据类型 0  整数 ，1 WORD, 2 SHORT, 3 16进制数，4 浮点数，5 字符串）
    //(下拉框数据索引)，（指向函数的指针，将源数据转化为显示数据字串），（指向函数的指针，将显示字串转化为源）
    T_DATA s_Data;
    ColArray &w_ColData = s_Data.c_Array;
    w_ColData.resize(2);
    w_ColData[0].GetData("对象号", 90, 0, ctrl_Fix, tINT);
    w_ColData[1].GetData("名称", 120, 1, ctrl_Edit, tSTR);
    //m_Grid.SetColClass(2, w_ColData);
    DataArray &s_RowData = s_Data.d_Array;
    s_RowData.resize(m_Pal->pal->gpGlobals->g_TextLib.nWords);
    for (int n = 0; n < m_Pal->pal->gpGlobals->g_TextLib.nWords; n++)
    {
        s_RowData[n].ColVarList.resize(2);
        s_RowData[n].oldRow = n + 1;
        s_RowData[n].ColVarList[0] = n;
        s_RowData[n].ColVarList[1] = m_Pal->pal->PAL_GetWord(n);
    }

    m_Grid->set_t_Data(s_Data);
    //不允许添加删除行
    m_Grid->m_popMenuFlags = 0b01011;
}

void MainWindow::Edit_Store(int swtch)
{
    WorkCtrl = WM_EditStore;
    m_Grid->UndoCtrl = WM_EditStore;
    QString msg("修改商店");
    SendMessages(msg);

    //下拉表选择数据
    pSelect_Item p_Select_all = &Select_Item_Array[3];
 
    T_DATA s_Data;
    ColArray &w_ColData = s_Data.c_Array;
    const int totalCol = 11;
    w_ColData.resize(totalCol);
    //字段显示生成函数
    auto p_SrcToStr = std::bind(&MainWindow::q_SrcToStr, this, std::placeholders::_1);
    //lambda
    LPSTR(*p_IntToStr)(int d) = [](int d)-> LPSTR {
        static char buf[8]; sprintf_s(buf, " %3d", d);  return (LPSTR)buf; };
    //制做表头
    //参数 文字，列宽，列号，（类型 0 什么都不做，1，列头固定(永远显示)，2，Edit），
    //（原数据类型 0  整数 ，1 WORD, 2 SHORT, 3 16进制数，4 浮点数，5 字符串）
    //(下拉框数据索引)，（指向函数的指针，将源数据转化为显示数据字串），（指向函数的指针，将显示字串转化为源）
    w_ColData[0].GetData("序号", 40, 0, ctrl_Fix, tWORD);
    w_ColData[1].GetData("商店号", 50, 1, ctrl_Fix, tWORD, NULL, p_IntToStr);
    w_ColData[2].GetData("位置1", 69, 2, ctrl_List, tNull, p_Select_all, p_SrcToStr);
    w_ColData[3].GetData("位置2", 69, 3, ctrl_List, tNull, p_Select_all, p_SrcToStr);
    w_ColData[4].GetData("位置3", 69, 4, ctrl_List, tNull, p_Select_all, p_SrcToStr);
    w_ColData[5].GetData("位置4", 69, 5, ctrl_List, tNull, p_Select_all, p_SrcToStr);
    w_ColData[6].GetData("位置5", 69, 6, ctrl_List, tNull, p_Select_all, p_SrcToStr);
    w_ColData[7].GetData("位置6", 69, 7, ctrl_List, tNull, p_Select_all, p_SrcToStr);
    w_ColData[8].GetData("位置7", 69, 8, ctrl_List, tNull, p_Select_all, p_SrcToStr);
    w_ColData[9].GetData("位置8", 69, 9, ctrl_List, tNull, p_Select_all, p_SrcToStr);
    w_ColData[10].GetData("位置9", 69, 10, ctrl_List, tNull, p_Select_all, p_SrcToStr);

    //表数据
    int n_Row, n_Item;
    auto p = &m_Pal->pal->gpGlobals->g;
    DataArray &s_RowData = s_Data.d_Array;
    s_RowData.resize(p->nStore);
    for (n_Row = 0; n_Row < p->nStore; n_Row++)
    {
        s_RowData[n_Row].ColVarList.resize(11);
        s_RowData[n_Row].oldRow = n_Row + 1;
        s_RowData[n_Row].ColVarList[0] = n_Row + 1;
        s_RowData[n_Row].ColVarList[1] = n_Row;

        for (n_Item = 0; n_Item < MAX_STORE_ITEM; n_Item++)
        {
            s_RowData[n_Row].ColVarList[n_Item + 2] = p->lprgStore[n_Row].rgwItems[n_Item];
        }
    }
    //栏数据动态生成
    s_Data.Set_Col_Data = [](ColumnClass* s, int row)->VOID
        {
            s->ColVarList[0] = row + 1;
            s->ColVarList[1] = row;
        };
    m_Grid->set_t_Data(s_Data);
    m_Grid->m_popMenuFlags = 0b01001111;//可追加 不可删除 插入
}

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
{
    //设定程序图标
    auto iton = QIcon(QString(":/s.ico"));
    this->setWindowIcon(iton);

    if (objectName().isEmpty())
        setObjectName(QString::fromUtf8("MainWindow"));
    if (DoChangeDir() == -1)
        exit(1);//出错或放弃选择
    //装入游戏数据
    if (m_Pal)
        delete m_Pal;
    m_Pal = new CGetPalData(0,1);//////
    if (!m_Pal->pal->gpGlobals)//初始化失败
        exit(1);
    //初始化
    m_Tree = new    QTreeWidget(this);
    m_Grid = new    CTableView(this);
    m_Edit = new    QTextEdit(this);
    m_Edit->setFocusPolicy(Qt::NoFocus);
    m_Edit->append("\n\n\n\n");
    m_Edit->setFontPointSize(8);
    m_Edit->setFontWeight(8);
    //初始化
    m_Tree->setColumnCount(1);
    m_Tree->setHeaderHidden(true);
    //布局
	setupLayout();
    Select_Item_Array.clear();
    Select_Item_Array.resize(10);

    //以下制作下拉表数据 0 物品 1 法术 2 毒药 3 攻击附加 4 怪
    for (int i = 0; i < 5; i++)
    {
        pSelect_Item p_Select_Item = &Select_Item_Array[i];

        p_Select_Item->data.resize(MAX_OBJECTS);

        p_Select_Item->row = 0;
        //插入空项
        p_Select_Item->data[0].item = 0;
        p_Select_Item->data[0].row = 0;
        p_Select_Item->data[0].s = "";
        int row = 1;
        if (i == 4)
        {
            //多插入一行 值FFFF
            p_Select_Item->data[1].item = 0xffff;
            p_Select_Item->data[1].row = 1;
            p_Select_Item->data[1].s = "禁用";
            row = 2;
        }
        int maxObj = m_Pal->pal->gpGlobals->g_TextLib.nWords;
        while (m_Pal->pal->PAL_GetWord(maxObj - 1).empty())maxObj--;
        for (int n = row - 1; n < maxObj; n++)
        {
            if (!(m_Pal->gpObject_classify[n] & select_flag[i]))
                continue;
            p_Select_Item->data[row].item = n;
            p_Select_Item->data[row].row = row;
            p_Select_Item->data[row].s = q_SrcToStr(n).toStdString();
            row++;
        }
        p_Select_Item->data.resize(row);
    }
    //QStandardItemModel mode(MAX_OBJECTS, 3);

    Set_Tree();

    //获取主屏幕分辨率
    QScreen* screen = QGuiApplication::primaryScreen();
    if (screen) {
        QRect logicalScreen = screen->geometry();
        resize(logicalScreen.width() * 0.75, logicalScreen.height() * 0.75);
    }
    else {
        resize(800, 600); // fallback
    }
    
    connect(m_Tree, &QTreeWidget::itemClicked, this, &MainWindow::SelectTreeItem);//当tree选择变化时启用
    connect(this, &MainWindow::SendMessages, this, &MainWindow::ShowMessage);//向窗口发送文字信息
    connect(m_Grid, &CTableView::SendMessages, this, &MainWindow::ShowMessage);//向窗口发送文字信息,来源于表窗口
    connect(m_Grid, &CTableView::ListScriptEntry, this, &MainWindow::ListScriptEntry);
    connect(m_Grid, &CTableView::ListScriptCall, this, &MainWindow::ListScriptCall);
    connect(m_Grid, &CTableView::deployMap, this, [&](int scene) {//在地图上部属
        CPixEdit* w = new deployMap(scene,m_Grid->getDataArray(), this);
        w->resize(width(), height());
        w->exec();
        delete w;
        });
        
}

MainWindow::~MainWindow()
{
    //QString PalDir = m_Pal->PalDir.c_str();//备份游戏目录
    if (!m_Pal->pal->CheckHeapIntegrity())
        delete m_Pal;
    delete m_Grid;
    delete m_Tree;
    delete m_Edit;
    //至此所有打开文档已经关闭
 }

void MainWindow::Set_Tree(int muneNumber)
{
    static int lastJob;    //生成缺省树
    if (muneNumber == 2 && JobCtrl != lastJob)
    {
        //装入存档文件
        lastJob = JobCtrl;
        m_Pal->pal->gpGlobals->PAL_LoadGame(
            m_Pal->pal->gpGlobals->rgSaveData[JobCtrl -1]);
    }
    if (muneNumber == 1 && lastJob != -1)
    {
        lastJob = -1;
        m_Pal->pal->gpGlobals->PAL_LoadDefaultGame();
    }

    m_Tree->clear();//初始化树，清除结点
    {
        //设置窗口标题
        //std::sprintf;
        QString Title =
            QString::asprintf
            (tr("SDLPAL_EDIT 游戏修改工具  游戏数据目录为  %s").toUtf8(), m_Pal->pal->PalDir.c_str());
        setWindowTitle(Title);
    }

    QTreeWidgetItem* topItem1 = new QTreeWidgetItem(m_Tree);

    topItem1->setText(0, "初始属性");
    m_Tree->addTopLevelItem(topItem1);
    for (size_t i = 0; i < WM_END; i++)
    {
        QTreeWidgetItem* childItem{};
        int hidden{};
        //存档文件测试
        if (i > WM_ARCHIVES && i <= WM_ARCHIVES_END)
        {
            childItem = new QTreeWidgetItem(QStringList() <<
                QString::asprintf("%d.rpg", i - WM_ARCHIVES - 
                    CPalEvent::ggConfig->m_Function_Set[53]));
            //存档文件测试,从0开始，序号3 = 0
            if (i - 3 >= CPalEvent::ggConfig->m_Function_Set[47] +
                CPalEvent::ggConfig->m_Function_Set[53])
                hidden = TRUE;
            else if (m_Pal->pal->gpGlobals->rgSaveData[i - 3].size() == 0)
                hidden = TRUE;
            if (muneNumber )
                hidden = TRUE;
        }
        else {
            for (size_t j = 0; lpItemName[j].s; j++)
            {
                if (lpItemName[j].n == i)
                {
                    childItem = new QTreeWidgetItem(QStringList() << lpItemName[j].s);
                    if (!(lpItemName[j].b & (1 << muneNumber)))
                        hidden = TRUE;
                    break;
                }
            }
        }
        //auto h =  childItem->isHidden();
        topItem1->addChild(childItem);
        childItem->setHidden(hidden);
    }
    topItem1->setExpanded(1);
}


void MainWindow::resizeEvent(QResizeEvent* event)
{
    QMainWindow::resizeEvent(event); // 如果保留，只需调用基类}
}

// 在 MainWindow 的构造函数中（例如 setupUI() 或 initWidgets()）
void MainWindow::setupLayout()
{
    // 创建左侧树控件容器（固定最小/最大宽度）
    m_Tree->setMinimumWidth(180);
    m_Tree->setMaximumWidth(180); // 固定宽度
    m_Tree->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);

    // 创建右侧上半部分（表格）
    m_Grid->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    // 创建右侧下半部分（编辑区）
    m_Edit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    // 垂直分割器：用于右侧上下分割
    QSplitter* rightSplitter = new QSplitter(Qt::Vertical, this);
    rightSplitter->addWidget(m_Grid);
    rightSplitter->addWidget(m_Edit);
    rightSplitter->setHandleWidth(8); // 可选：调整分隔条粗细

    // 水平分割器：左侧树 + 右侧垂直分割器
    QSplitter* mainSplitter = new QSplitter(Qt::Horizontal, this);
    mainSplitter->addWidget(m_Tree);
    mainSplitter->addWidget(rightSplitter);
    mainSplitter->setHandleWidth(8);

    // 设置初始分割比例（可选）
    // 例如：左侧占 250px，右侧占剩余；上下各占一半
    //QList<int> mainSizes;
    //mainSizes << 250 << width() - 250; // 初始值可动态计算
    //mainSplitter->setSizes(mainSizes);

    QList<int> rightSizes;
    rightSizes << height() * 2 / 3 << height() / 3;
    rightSplitter->setSizes(rightSizes);

    // 设置为主窗口中心 widget
    setCentralWidget(mainSplitter);

    // 设置最小窗口尺寸（替代 resizeEvent 中的限制）
    setMinimumSize(800, 600);
}

void MainWindow::closeEvent(QCloseEvent* event)
{
    if (m_Grid->isUpdate())
    {
        DataUpDate(WorkCtrl);
        haveModified = 0;
        if (JobCtrl > 0)
        {
            //存档文件数据
            m_Pal->pal->gpGlobals->PAL_SaveGame(JobCtrl,
                *((WORD*)m_Pal->pal->gpGlobals->rgSaveData[JobCtrl - 1].data()), 1);
        }
        else
        {
            //缺省数据
            m_Pal->saveGameDataToCache();
        }
    }//

    if(!m_Pal->isSaveDataChaged())
	{
		//没有修改数据，直接关闭
		event->accept();
		return;
	}
    else
    {
        QString s = "注意！数据已经修改 保存吗？";
        QStringList buttons = { "是", "否", "取消" };
        int ret = CGetPalData::showChineseMessageBox(nullptr, "警告", s, QMessageBox::Warning, buttons);

        if (ret == 2 || ret == -1)
        {
            event->ignore();//忽略关闭信号，阻止窗体关闭
            return;
        }  
        if (ret == 0)
        {
            //
            m_Pal->saveDataFile();
        }
        event->accept();//默认情况下接收关闭信号，关闭窗体
    }
}

void MainWindow::ShowMessage(const QString& msg)
{
    m_Edit->moveCursor(QTextCursor::End);
    QTextCursor cursor = m_Edit->textCursor();
    cursor.deletePreviousChar();
    cursor.deletePreviousChar();
    m_Edit->append(msg +"\n\n");
}

void MainWindow::ListScriptEntry(WORD src)
{
    //qDebug() << src;
    if (!m_ListView)
        delete m_ListView;
    m_ListView = new CListScriptEntry(this, src);
}
void MainWindow::ListScriptCall(WORD src)
{
    if (m_ListView)
        delete m_ListView;
    m_ListView = new CListScriptCall(this, src);
}
void MainWindow::SelectTreeItem(QTreeWidgetItem* item)
{
    QString msg;
    msg = QString::asprintf("%s  被选中，序号为 %d", item->text(0).trimmed().toStdString().c_str(),
        item->parent() ? item->parent()->indexOfChild(item) : -1);
    emit SendMessages(msg);
    int swtch = item->parent() ? item->parent()->indexOfChild(item) : -1;
    if (m_Grid->isUpdate() || haveModified)
    {
        //更新数据表
        DataUpDate(WorkCtrl);
        //
        if (JobCtrl > 0)
        {
            //装入存档文件数据
            m_Pal->pal->gpGlobals->PAL_LoadGame(m_Pal->pal->gpGlobals->rgSaveData[JobCtrl - 1]);
        }
        else
        {
            //缺省数据
            m_Pal->pal->gpGlobals->PAL_LoadDefaultGame();
        }
    }
    haveModified = 0;
    //清除UNDO数据
    m_Grid->clear();
    //m_Grid->show();

    switch (swtch)
    {
    case WM_OKReturn:
        //返回
    {
        int maxSaveFile = CPalEvent::ggConfig->m_Function_Set[47] +
            CPalEvent::ggConfig->m_Function_Set[53];

        m_Pal->pal->gpGlobals->rgSaveData.resize(maxSaveFile);
        for (size_t i = 0; i < maxSaveFile; i++)
        {
            //将扩大存档文件数量部分装入
            std::string fileName = CPalEvent::PalDir + CPalEvent::va
            ("%d.rpg", i + 1 - CPalEvent::ggConfig->m_Function_Set[53]);
            if (m_Pal->pal->gpGlobals->IsFileExist(fileName) &&
                m_Pal->pal->gpGlobals->rgSaveData.at(i).size() == 0)
            {
                m_Pal->pal->gpGlobals->rgSaveData.at(i) =
                    m_Pal->pal->gpGlobals->readAll(fileName);
            }
        }

        haveModified = 0;
        m_Grid->clear();
		//m_Grid->show();
        Set_Tree(0);
        break;
    }
    default:
        break;
    }

    m_Grid->clear();
    switch (swtch)
    {
    case 	WM_EDIT_DIR:
        //修改目录
    {
        if (m_Pal->isSaveDataChaged())
        {
            LPCSTR s = "有未保存的数据！保存吗？……";
            QStringList buttons = { "保存","不保存","取消" };

            int ret = CGetPalData::showChineseMessageBox(nullptr, "警告", s, QMessageBox::Warning, buttons);

            if (ret == 2)
            {
                return;//取消
            }
            if (ret == 0)
            {
                //保存
                if (!m_Pal->saveDataFile())
                {
                    emit SendMessages("保存成功！");
                    //清除修改标志
                    m_Pal->clearSaveDataChangd();
                }
            }
        }
        //修改目录
        auto oldDir = m_Pal->pal->PalDir;
        auto ret = DoChangeDir(1);
        if (ret == -1)//强制改变目录
        {
            //修改失败
            m_Pal->pal->PalDir = oldDir;
            return;
        }
        else if (ret == 0)
            //取消
            return;
        //成功
        {
            auto dir = QDir::currentPath().toStdString();
            dir += "/";
            //重新读入文件
            delete m_Pal;
            m_Pal = new CGetPalData(0, 1);
            CPalEvent::PalDir = dir;
            QDir::setCurrent(m_Pal->pal->PalDir.c_str());
            Set_Tree(0);
        }
        break;
    }

    case WM_SET_TREE:
        //修改基础数据
        JobCtrl = -1;
        Set_Tree(1);
        break;

    case WM_RUN_TEST:
        //测试
    {
        this->hide();
        auto m_TestRunDialog = new testRun(m_Pal->pal->gpGlobals, this);
        m_TestRunDialog->exec();

        delete m_TestRunDialog;
        this->show();
        Set_Tree(0);
        break;
    }
    case WM_RUN://运行
    {
        this->hide();
        std::thread theThread([]()->void {
            CScript::isTestRun = false;//不生成调试信息
            auto m = new CScript(0, 0, m_Pal->pal->gpGlobals);//
            //将修改后的存档转到全局变量
            int maxSaveFile = CPalEvent::ggConfig->m_Function_Set[47];
            for (int i = 0; i < maxSaveFile; i++)
            {
                m_Pal->pal->gpGlobals->rgSaveData[i] = std::move(m->gpGlobals->rgSaveData[i]);
                m_Pal->pal->gpGlobals->rgSaveDataChaged[i] = m->gpGlobals->rgSaveDataChaged[i];
            }
            delete m;
            });
        theThread.join();
        this->show();
        Set_Tree(0);
        break;
    }
    case WM_BACKUP://备份
    {
        if (m_Pal->isSaveDataChaged())
        {
            LPCSTR s = "注意！有未保存的数据";

            QStringList buttons = { "保存", "不保存", "取消" };
            int ret = CGetPalData::showChineseMessageBox(nullptr, "请确认", s, QMessageBox::Warning, buttons);

            if (ret == 2)
            {
                break;//取消
            }
            if (ret == 0)
            {
                //
                if (!m_Pal->saveDataFile())
                {
                    emit SendMessages("保存成功！");
                    //清除修改标志
                    m_Pal->clearSaveDataChangd();
                }
            }
        }
        if (!m_Pal->backupFile())
            emit SendMessages("备份成功！");
        Set_Tree(0);
        break;
    }
    case WM_RESTORE://恢复
    {
        if (!m_Pal->restoreFile())
        {
            //重新载入
            delete m_Pal;
            m_Pal = new CGetPalData(0, 1);
            emit SendMessages("恢复成功！数据已经重新载入");
            Set_Tree(0);
        }
        else
            emit SendMessages("恢复失败");
        break;
    }
    case WM_DEL_BACKUP://删除备份文件
        if (!m_Pal->delBackupFile())
            emit SendMessages("删除成功！");
        else
            emit SendMessages("删除失败！");
        break;
    case WM_SAVE_ALL://保存数据文件
        if (!m_Pal->saveDataFile())
        {
            emit SendMessages("保存成功！");
            //清除修改标志
            m_Pal->clearSaveDataChangd();
        }
        break;
    case WM_EXIT:
        //退出

        close();
        break;
    case WM_EDIT_PLAYER:
    {
        //修改玩家属性
        Edit_PlayerAttributes(swtch);
        break;
    };
    case WM_EDIT_ENEMY:
        Edit_Enemy(swtch);
        break;
    case  WM_EditEnemyTeam:
        Edit_EnemyTeam(swtch);
        break;
    case WM_EDIT_MAGIC:
        Edit_Magic(swtch);
        break;
    case WM_EditGameSeting:
        //修改游戏设置 edit_Game_Seting
        Edit_Game_Seting(swtch);
        break;
    case WM_EditFontLibraryPath:
        //修改字库路径
        Edit_FontLibraryPath(swtch);
        break;
    case  WM_EditEventObject:
        Edit_EventObject(swtch);
        break;
    case WM_EditObjectItem:
        Edit_ObjectItem(swtch);
        break;
    case WM_EditPoison:
        Edit_Poison(swtch);
        break;
    case WM_PLAYERLEVELUPMAGIG:
        //升级魔法
        Edit_PlayerLevelUPMagic(swtch);
        break;
    case WM_EditBattleField:
        Edit_BattleField(swtch);
        break;
    case WM_EditScene:
        Edit_Scene(swtch);
        break;
    case WM_EditStore:
        Edit_Store(swtch);
        break;
    case WM_Inventory:
        Edit_Invenyory(swtch);
        break;
    case WM_EditObjectName:
        Edit_ObjectName(swtch);
        break;
    case WM_EditDialog:
        Edit_Dialog(swtch);
        break;
    case WM_EditExplain:
        Edit_Explain(swtch);
        break;
    case WM_ScriptEntry:
        Edit_ScriptEntry(swtch);
        break;
    case WM_EDIT_PARAMETERS:
        Edit_Parameters(swtch);
        break;
    case WM_EditObjectPict:
        Edit_ObjectPict(swtch);
        break;
    case WM_MapEdit:
        Edit_Map(swtch);
        break;
    default:
        if (swtch > WM_ARCHIVES && swtch <= WM_ARCHIVES_END)
        {
            //修改存档数据
            JobCtrl = swtch - 2;
            Set_Tree(2);
        }
        break;
    }
}

void MainWindow::DataUpDate(int Work)
{
    //数据更新模块
    if (Work <= 0) return;
    DWORD nsize = m_Grid->getDataArray()->d_Array.size();
    //复制数据
    SGAMEDATA backupData = m_Pal->pal->gpGlobals->g;

    auto& p = m_Pal->pal->gpGlobals->g;
    //更新行数据
    auto m_DataArray = std::move(m_Grid->getDataArray()->d_Array);
    auto showErr = [this]() {
        QString s = "数据更新失败，已经恢复";
        ShowMessage(s);
        QMessageBox::question(this, s, "请确认！",
            QMessageBox::StandardButton::Yes);
        };
    //清空表格
    m_Grid->clear();

    switch (Work)
    {
    case WM_EDIT_PLAYER://我方人物
    {
        //数据更新
        for (int r = 0; r < nsize; r++)
        {
            //一行数据
            WORD* p_list = (WORD*)&(m_Pal->pal->gpGlobals->g.PlayerRoles);
            for (int n = 1, i; n < 61; n++)
            {
                if (n == 1)
                    i = n + 2;//名称
                else if (n > 1 && n <= 6)
                    i = n + 4;//等级-真气
                else if (n > 6 && n <= 20)
                    i = n + 10;//武力--巫抗
                else if (n > 20 && n < 27)
                    i = n - 10;//装备
                else if (n == 27)
                    i = 0x1f;//救援
                else if (n == 28)
                {
                    i = 0x41;//合体法术
                    WORD obj = std::get<int>(m_DataArray[r].ColVarList[n]);
                    if (obj && (obj >= MAX_OBJECTS ||
                        m_Pal->gpObject_classify[obj] != kIsMagic))
                    {
                        //::MessageBox(AfxGetApp()->m_pMainWnd->m_hWnd, _T("作废修改"), _T("输入错"), MB_OK);
                        return;
                    }
                }
                else if (n > 28 && n < 61)
                {
                    i = n + 3;//魔法 
                    WORD obj = (WORD)std::get<int>(m_DataArray[r].ColVarList[n]);
                    if (obj && (obj >= MAX_OBJECTS ||
                        m_Pal->gpObject_classify[obj] != kIsMagic))
                    {
                        std::string str = "";
                        str += m_Pal->pal->PAL_GetWord(obj);
                        SendMessages(QString(str.c_str()));
                        //::MessageBox(AfxGetApp()->m_pMainWnd->m_hWnd, _T("作废修改"), _T("输入错"), MB_OK);
                        return;
                    }
                }
                p_list[i * MAX_PLAYER_ROLES + r] =
                    (WORD)std::get<int>(m_DataArray[r].ColVarList[n]);
            }

            WORD s1 = 0, s2 = 0;
            s1 = std::get<int>(m_DataArray[r].ColVarList[61]);
            s2 = std::get<int>(m_DataArray[r].ColVarList[62]);
            WORD name = ((m_Pal->pal->gpGlobals->g.PlayerRoles)).rgwName[r];
            m_Pal->pal->gpGlobals->g.rgObject[name].player.wScriptOnFriendDeath = s1;
            m_Pal->pal->gpGlobals->g.rgObject[name].player.wScriptOnDying = s2;
        }
        if (JobCtrl <= 0)
            m_Pal->saveGameDataToCache();
        else
        {
            int saveTime = *((WORD*)m_Pal->pal->gpGlobals->rgSaveData[JobCtrl - 1].data());
            //存档文件数据
            m_Pal->pal->gpGlobals->PAL_SaveGame(m_Pal->pal->gpGlobals->rgSaveData[JobCtrl - 1],saveTime );
        }
        break;
    }
    case WM_EDIT_ENEMY://怪物
    {
        //更新数据
        for (int r = 0; r < m_DataArray.size(); r++)
        {
            //一行数据
            if (m_DataArray[r].ColVarList.empty())
            {
                assert(false);
                continue;
            }
            WORD wEnemyID = get<int>(m_DataArray[r].ColVarList[41]);
            WORD wOenemyID = get<int>(m_DataArray[r].ColVarList[0]);
            LPENEMY  p_enemy = &(m_Pal->pal->gpGlobals->g.lprgEnemy[wEnemyID]);//ID
            WORD* p_list = &p_enemy->wIdleFrames;
            for (int n = 2; n < 37; n++)
            {
                p_list[(n - 2)] = get<int>(m_DataArray[r].ColVarList[n]);
            }
            //更新OBJECT数据
            auto& enemy = m_Pal->pal->gpGlobals->g.rgObject[wOenemyID].enemy;
            enemy.wResistanceToSorcery = get<int>(m_DataArray[r].ColVarList[37]);//巫抗
            enemy.wScriptOnTurnStart = get<int>(m_DataArray[r].ColVarList[38]);//战前脚本
            enemy.wScriptOnReady = get<int>(m_DataArray[r].ColVarList[39]);//战斗脚本
            enemy.wScriptOnBattleEnd = get<int>(m_DataArray[r].ColVarList[40]);//战后脚本
        }
        if (JobCtrl <= 0)
            m_Pal->saveGameDataToCache();
        else
        {
            int saveTime = *((WORD*)m_Pal->pal->gpGlobals->rgSaveData[JobCtrl - 1].data());
            //存档文件数据
            m_Pal->pal->gpGlobals->PAL_SaveGame(m_Pal->pal->gpGlobals->rgSaveData[JobCtrl - 1], saveTime);
        }
        break;
    }
    case WM_EDIT_MAGIC://魔法
    {
        //更新数据
        for (int r = 0; r < m_Pal->nMagic; r++)
        {
            //一行数据
            if (m_DataArray[r].ColVarList.size() < 23)
            {
                //数据不完整
                assert(false);
                continue;
            }
            WORD wMagicID = get<int>(m_DataArray[r].ColVarList[22]);
            WORD wOMagicID = get<int>(m_DataArray[r].ColVarList[1]);
            LPMAGIC  p_Magic = &(m_Pal->pal->gpGlobals->g.lprgMagic[wMagicID]);//ID
            WORD* p_list = &p_Magic->wEffect;
            for (int n = 2; n < 18; n++)
            {
                p_list[(n - 2)] = get<int>(m_DataArray[r].ColVarList[n]);
            }
            m_Pal->pal->gpGlobals->g.rgObject[wOMagicID].magic.wFlags = get<int>(m_DataArray[r].ColVarList[18]);
            m_Pal->pal->gpGlobals->g.rgObject[wOMagicID].magic.wScriptOnUse = get<int>(m_DataArray[r].ColVarList[19]);
            m_Pal->pal->gpGlobals->g.rgObject[wOMagicID].magic.wScriptOnSuccess = get<int>(m_DataArray[r].ColVarList[20]);
        }
        if (JobCtrl <= 0)
            m_Pal->saveGameDataToCache();
        else
        {
            int saveTime = *((WORD*)m_Pal->pal->gpGlobals->rgSaveData[JobCtrl - 1].data());
            //存档文件数据
            m_Pal->pal->gpGlobals->PAL_SaveGame(m_Pal->pal->gpGlobals->rgSaveData[JobCtrl - 1], saveTime);
        }
        break;
    }
    case WM_EditBattleField://战斗场所
    {
        //检查行数变化
        if (nsize != p.nBattleField)
        {
            //，重新分配内存
            p.lprgBattleField.resize(nsize);
            p.nBattleField = nsize;
            p.lprgBattleField.shrink_to_fit();
        }
        //更新数据
        for (int r = 0; r < p.nBattleField; r++)
        {
            //一行数据
            if (m_DataArray[r].ColVarList.size() < 7)
            {
                //数据不完整
                assert(false);
                continue;
            }
            for (int k = 0; k < NUM_MAGIC_ELEMENTAL; k++)
            {
                p.lprgBattleField[r].rgsMagicEffect[k] = 0xffff & get<int>(m_DataArray[r].ColVarList[k + 1]);
            }
            p.lprgBattleField[r].wScreenWave = 0xffff & get<int>(m_DataArray[r].ColVarList[6]);
        }
        if (JobCtrl <= 0)
            m_Pal->saveGameDataToCache();
        break;
    }
    ////////
    case WM_EditEventObject:  //事件对象
    {
        //检查行数变化
        if (p.nEventObject != m_DataArray.size())
        {
            //行数变化，重新分配内存
            p.lprgEventObject.resize(nsize);
            p.nEventObject = nsize;
            p.lprgEventObject.shrink_to_fit();
        }
        //更新数据
        for (int r = 0; r < p.nEventObject; r++)
        {
            //一行数据 
            if (m_DataArray[r].ColVarList.size() < 18)
            {
                //数据不完整
                assert(false);
                continue;
            }
            LPWORD d = (LPWORD)(&p.lprgEventObject[r]);
			//逐行替换数据 共16个数据
            for (int n = 0; n < 16; n++)
            {
				//数据从第三列开始，第一列第二列为序号和场景号 第19列和第20列为行号和原行号 共16个数据
                d[n] = get<int>(m_DataArray[r].ColVarList[n + 2]) & 0xffff;
            }
        }
        if (JobCtrl > 0)
        {
            int saveTime = *((WORD*)m_Pal->pal->gpGlobals->rgSaveData[JobCtrl - 1].data());
            //存档文件数据
            m_Pal->pal->gpGlobals->PAL_SaveGame(m_Pal->pal->gpGlobals->rgSaveData[JobCtrl - 1], saveTime);
            return;
        }
        //增加减少删除行处理
        {
            std::map<int,int> stOld;//生成新旧行号对照表 map<oldlRow,newrow>
            std::map<int, int> stNew;//生成新旧行号对照表 map<newrow,Oldrow>
            int insertRow{};
            for (int k = 0; k < nsize; k++)
            {
                auto newRow = k + 1;
                auto oldRow = m_DataArray[k].oldRow;
                if (oldRow == 0)
                    oldRow = --insertRow;
                stOld[oldRow] = newRow;
                stNew[newRow] = oldRow;
            }
            //更新脚本中的所有指向事件对象的值
            for (int n = 0; n < m_Pal->pal->gpGlobals->g.nScriptEntry; n++)
            {
                LPWORD p1{ 0 }, p2{ 0 };
                switch (m_Pal->pal->gpGlobals->g.lprgScriptEntry[n].wOperation)
                {
                case 0x0012://设置对象到相对于队伍的位置 参数1对象,参数2 X，参数3 Y
                case 0x0013://设置对象到指定的位置 参数1对象, 参数2 X，参数3 Y
                case 0x0016://设置对象的，方向和（ 手势），参数1对象不为0 ，参数2，方向，参数3，形象
                case 0x0024://设置对象自动脚本地址，参数1对象不等于0 参数2 地址
                case 0x0025://设置对象触发脚本地址，参数1对象不等于0 参数2 地址
                case 0x0040://设置对象触发模式 如参数1对象 ！= 0 ，参数2 设置
                case 0x0049://设置对象状态，参数1对象 参数2 状态
                case 0x006c://參數1對象 !=0，0xffff NPC走一步，参数2 X，参数3 Y
                case 0x006f://将当前事件对象状态与·另一个事件对象同步,参数1对象
                case 0x007d://移动对象位置，参数1对象 参数2 X，参数3 Y
                case 0x007e://设置对象的层，参数1对象，参数2
                case 0x0081://跳过，如果没有面对对象，参数1对象，参数2 改变触发方式，参数3地址
                case 0x0083://如果事件对象不在当前事件对象的指定区域，则跳过,参数1对象，参数3地址
                case 0x0084://将玩家用作事件对象的物品放置到场景中,参数1对象 位置，，失败跳到参数3
                case 0x0094://如果事件对象的状态是指定的，则跳转,参数1 对象，参数2 状态，参数3 地址
                case 0x0098://设置跟随对象,参数1对象 >= 0 设置跟随，0 取消                   
                    p1 = &m_Pal->pal->gpGlobals->g.lprgScriptEntry[n].rgwOperand[0];
                    break;
                case 0x009A://为多个对象设置状态，参数1 到参数2，设置为参数3
                    p1 = &m_Pal->pal->gpGlobals->g.lprgScriptEntry[n].rgwOperand[0];
                    p2 = &m_Pal->pal->gpGlobals->g.lprgScriptEntry[n].rgwOperand[1];
                    break;
				case 0x0004://运行子脚本，参数1 脚本地址，参数2 对象
                    p2 = &m_Pal->pal->gpGlobals->g.lprgScriptEntry[n].rgwOperand[1];
                    break;
                default:
                    break;
                }
                if (p1 && *p1 != 0xffff && *p1 != 0)
                {
                    std::map<int,int>::iterator pw;
                    pw = stOld.find(*p1);
                    if (pw == stOld.end())
                    {
                        //没有找到出错
                        auto ss = QString::asprintf("数据错，原有对象 %4.4X 被删除", *p1);
                        ShowMessage(ss);
                        return;                    
                    }
                    if (pw->first != pw->second)
                    {
                        //新旧入口不一致，需要修改，用新入口替换旧入口
                        *p1 = pw->second;
                    }
                }
                if (p2 && *p2)
                {
                    std::map<int, int>::iterator pw;
                    pw = stOld.find(*p2);
                    assert(pw != stOld.end());
                    if (pw->first != pw->second)
                    {
                        //新旧入口不一致，需要修改，用新入口替换旧入口
                        *p2 = pw->second;
                    }
                }
            }

            //更新场景表
            for (int n_scene = 0, wIndex = 0, wEventObject = 0; n_scene < m_Pal->pal->gpGlobals->g.nScene - 1; n_scene++)
            {
                wIndex = m_Pal->pal->gpGlobals->g.rgScene[n_scene + 1].wEventObjectIndex;
                while (wIndex > m_DataArray[wEventObject].oldRow)
                    wEventObject++;
                if (wIndex < m_DataArray[wEventObject].oldRow)
                    wEventObject--;
                m_Pal->pal->gpGlobals->g.rgScene[n_scene + 1].wEventObjectIndex = wEventObject + 1;
            }
            //将结果保存
            m_Pal->saveGameDataToCache();

            //使用新旧行对照表更新存盘数据中的事件对象 全部数据
            for (int n_save = 0; n_save < m_Pal->pal->gpGlobals->rgSaveData.size(); n_save++)
            {
                auto& p = m_Pal->pal->gpGlobals->g;
                if ((m_Pal->pal->gpGlobals->rgSaveData[n_save].size()) == 0)
                    continue;
                //从缓存中载入游戏数据
                m_Pal->pal->gpGlobals->PAL_LoadGame(m_Pal->pal->gpGlobals->rgSaveData[n_save]);
                //更新场景表
                for (int n_scene = 0, wIndex = 0, wEventObject = 0; n_scene < p.nScene - 1; n_scene++)
                {
                    wIndex = p.rgScene[n_scene + 1].wEventObjectIndex;
                    while (wIndex > m_DataArray[wEventObject].oldRow)
                    {
                        wEventObject++;
                        if (wEventObject >= m_DataArray.size())
                        {
                            wEventObject--;
                            break;
                        }
                    }
                    if (wIndex < m_DataArray[wEventObject].oldRow)
                        wEventObject--;
                    p.rgScene[n_scene + 1].wEventObjectIndex = wEventObject + 1;
                }
                //更新事件对象表
                for (int k = 0,oldRow = 1; k < m_DataArray.size(); k++)
                {                   
                    //原行号大于0 但不等于 oldRow
                    if (m_DataArray[k].oldRow && m_DataArray[k].oldRow != oldRow)
                    {
                        //属于 k 的原行已经被删除
                        p.lprgEventObject.erase(p.lprgEventObject.begin() + k);
                        k--;
                        oldRow++;
                        continue;
                    }
                    else  if (m_DataArray[k].oldRow == 0)
                    {
                        //新增行，插入
                        EVENTOBJECT newObj{};
                        p.lprgEventObject.insert(p.lprgEventObject.begin() + k, newObj);
                    }
                    else
                        oldRow++;
                }
				//保存到存盘数据
                int saveTime = *((WORD*)m_Pal->pal->gpGlobals->rgSaveData[n_save].data());
				m_Pal->pal->gpGlobals->PAL_SaveGame(m_Pal->pal->gpGlobals->rgSaveData[n_save], saveTime);
            }
        }
        break;
    }
    case WM_EditStore://商店
    {
        //检查行数变化
        if (nsize > p.nStore)
        {
            //，重新分配内存
            p.lprgStore.resize(nsize);
            p.nStore = nsize;
            p.lprgStore.shrink_to_fit();
        }
        //更新数据
        for (int r = 0; r < p.nStore; r++)
            for (size_t n_Row = 0; n_Row < nsize; n_Row++)
            {
                for (int n_Item = 0; n_Item < MAX_STORE_ITEM; n_Item++)
                {
                    p.lprgStore[n_Row].rgwItems[n_Item] = get<int>(m_DataArray[n_Row].ColVarList[n_Item + 2]);
                }
            }
        if (JobCtrl <= 0)
            m_Pal->saveGameDataToCache();
        break;
    }
    case WM_EditObjectItem://物品
    {
        for (int r = 0; r < m_DataArray.size(); r++)
        {
            int j = get<int>(m_DataArray[r].ColVarList[0]);
            assert(m_Pal->gpObject_classify[j] == kIsItem);
            p.rgObject[j].item.wPrice = get<int>(m_DataArray[r].ColVarList[3]);//价格
            p.rgObject[j].item.wBitmap = get<int>(m_DataArray[r].ColVarList[19]);//图标
            WORD wFlags = 0;
            for (int n = 7; n < 19; n++)
                wFlags |= get<int>(m_DataArray[r].ColVarList[n]) << (n - 7);
            p.rgObject[j].item.wFlags = wFlags;
        }
        if (JobCtrl <= 0)
            m_Pal->saveGameDataToCache();
        break;
    }
    case WM_EditEnemyTeam://敌方队伍
    {
        //检查行数变化.只允许增加行数
        bool rowChanged = false;
        if (nsize != p.lprgEnemyTeam.size())
        {
            //分配内存
            p.lprgEnemyTeam.resize(nsize);
            p.nEnemyTeam = nsize;
            p.lprgEnemyTeam.shrink_to_fit();
            rowChanged = true;
        }
        //更新数据
        for (int r = 0; r < m_DataArray.size(); r++)
            for (int n = 0; n < MAX_ENEMIES_IN_TEAM; n++)
            {
                WORD k_Enemy = get<int>(m_DataArray[r].ColVarList[n + 1]);
                if (!(k_Enemy == 0 || k_Enemy == 0xffff ||
                    (k_Enemy < MAX_OBJECTS && m_Pal->gpObject_classify[k_Enemy] == kIsEnemy)))
                {
                    assert(false);
                    continue;
                }
                p.lprgEnemyTeam[r].rgwEnemy[n] = k_Enemy;
            }
        if (rowChanged)
        {
            //新行号与旧行号不一致，修改
            auto& t = m_Pal->pal->gpGlobals->g.lprgScriptEntry;
            std::map <int, int> oldRowMap;
            for (int r = 0; r < m_DataArray.size(); r++)
            {
                if (m_DataArray[r].oldRow == r + 1 || m_DataArray[r].oldRow <= 0)
                    continue;//跳过行号没有变化的行
                //行号变化,加入oldRowMap
                oldRowMap[m_DataArray[r].oldRow - 1] = r;
            }
            for (int r = 0; r < t.size(); r++)
            {
                if (t[r].wOperation != 0x0007)
                    continue;//不是开始战斗脚本
                auto s = oldRowMap.find(t[r].rgwOperand[0]);
                if (s != oldRowMap.end())
                {
                    //队伍号变化，修改
                    t[r].rgwOperand[0] = s->second;
                }
            }
            //m_Grid->set_t_Data(m_Grid->getDataArray()->d_Array);
        }
        if (JobCtrl <= 0)
            m_Pal->saveGameDataToCache();
        break;
    }
    case WM_PLAYERLEVELUPMAGIG://升级魔法
    {
        //检查行数变化
        if (nsize != p.nLevelUpMagic)
        {
            //，重新分配内存
            p.lprgLevelUpMagic.resize(nsize);
            p.nLevelUpMagic = nsize;
            p.lprgLevelUpMagic.shrink_to_fit();
        }
        //更新数据
        auto p = &m_Pal->pal->gpGlobals->g;
        for (int r = 0; r < p->nLevelUpMagic; r++)
        {
            for (int j = 0; j < MAX_PLAYABLE_PLAYER_ROLES; j++)
            {
                p->lprgLevelUpMagic[r].m[j].wLevel = get<int>(m_DataArray[r].ColVarList[j * 2 + 1]);
                p->lprgLevelUpMagic[r].m[j].wMagic = get<int>(m_DataArray[r].ColVarList[j * 2 + 2]);
            }
        }
        if (JobCtrl <= 0)
            m_Pal->saveGameDataToCache();
        break;
    }
    case WM_EditPoison://毒药编辑
    {
        for (int r = 0; r < m_DataArray.size(); r++)
        {
            int j = get<int>(m_DataArray[r].ColVarList[0]);
            assert(m_Pal->gpObject_classify[j] == kIsPoison);
            p.rgObject[j].poison.wPoisonLevel = get<int>(m_DataArray[r].ColVarList[2]);//
            p.rgObject[j].poison.wColor = get<int>(m_DataArray[r].ColVarList[3]);//
            p.rgObject[j].poison.wPlayerScript = get<int>(m_DataArray[r].ColVarList[4]);//
            p.rgObject[j].poison.wEnemyScript = get<int>(m_DataArray[r].ColVarList[5]);//
        }
        if (JobCtrl <= 0)
            m_Pal->saveGameDataToCache();
        break;
    }
    case WM_ScriptEntry://脚本入口
    {

        auto paldatabak = m_Pal->pal->gpGlobals;
        //制作新旧行对照表
        MAPScript st;//生成新旧行号对照表
        for (int k = 0; k < nsize; k++)
        {
            st[m_DataArray[k].oldRow] = k;
        }

        //检查行数变化
        if (nsize != p.nScriptEntry)
        {
            //，重新分配内存
            p.lprgScriptEntry.resize(nsize);
            p.nScriptEntry = nsize;
            p.lprgScriptEntry.shrink_to_fit();
        }
        //更新数据
        for (DWORD r = 0; r < nsize; r++)
        {
            //一行数据
                //assert(std::get<int>(m_DataArray[r].ColVarList[0]) == (int)r);
            p.lprgScriptEntry[r].wOperation = std::get<int>(m_DataArray[r].ColVarList[1]);
            p.lprgScriptEntry[r].rgwOperand[0] = std::get<int>(m_DataArray[r].ColVarList[2]);
            p.lprgScriptEntry[r].rgwOperand[1] = std::get<int>(m_DataArray[r].ColVarList[3]);
            p.lprgScriptEntry[r].rgwOperand[2] = std::get<int>(m_DataArray[r].ColVarList[4]);
        }
        //更新添加和删除行关联数据
        {
            MAPScript st;//生成新旧行号对照表
            for (int k = 0; k < nsize; k++)
            {
                st[m_DataArray[k].oldRow - 1] = k;
            }
            //遍历数组，查找已经删除的行
            for (int i = 0; i < m_Pal->pMark.size(); i++)
            {
                if (m_Pal->pMark[i].s.size() == 0)
                    continue;
                auto sFind = st.find(i);
                if (sFind == st.end())
                {
                    auto& s = m_Pal->pMark[i].s;
                    if (s.size() == 0 ||
                        s.size() == 1 && s[0].from != 0)
                        continue;
                    //出错，删除不能删除的行
                    //失败恢复原数据
                    m_Pal->pal->gpGlobals = paldatabak;
                    showErr();
                    return;
                }
            }
            for (DWORD r = 0; r < nsize; r++)
                if (m_DataArray[r].oldRow != r + 1 && m_DataArray[r].oldRow)
                {
                    //单个脚本变动调整
                    //Single script changes
                    if (m_Pal->SingleScriptChange(m_DataArray[r].oldRow - 1, r, st))
                    {
                        //失败恢复原数据
                        m_Pal->pal->gpGlobals = paldatabak;
                        showErr();
                        return;
                    }
                }
        }
        if (JobCtrl <= 0)
            m_Pal->saveGameDataToCache();
        break;
    }

    case WM_EDIT_PARAMETERS://基本属性编辑
    {
        for (int r = 0; r < nsize; r++)
        {
            //一行数据
            m_Pal->pal->gpGlobals->dwCash = get<int>(m_DataArray[0].ColVarList[1]);
            m_Pal->pal->gpGlobals->wCollectValue = get<int>(m_DataArray[1].ColVarList[1]);
        }
        if (JobCtrl > 0)
        {
            int saveTime = *((WORD*)m_Pal->pal->gpGlobals->rgSaveData[JobCtrl - 1].data());
            //存档文件数据
            m_Pal->pal->gpGlobals->PAL_SaveGame(m_Pal->pal->gpGlobals->rgSaveData[JobCtrl - 1], saveTime);
            return;
        }
        break;
    }

    case WM_EditObjectName://对象名称编辑
    {
        for (int r = 0; r < nsize; r++)
        {
            std::string s = get<std::string>(m_DataArray[r].ColVarList[1]);
            if (s == m_Pal->pal->PAL_GetWord(r))
                continue;
            //
            m_Pal->Utf8ToSys(s);
            s += "           ";
            memcpy(m_Pal->pal->gpGlobals->g_TextLib.lpWordBuf + r * 10, s.c_str(), 10);
            s = m_Pal->pal->PAL_GetWord(r);
        }
        //标识修改
        m_Pal->ModifRecord[modRoc_wordDat] = TRUE;
        break;
    }

    case WM_EditExplain://说明编辑
    {
        auto& oldExplain = m_Pal->pal->gpGlobals->f.fpDesc;
        ByteArray newExplain;
        for (int r = 0; r < nsize; r++)
        {
            std::string p = std::get<std::string>(m_DataArray[r].ColVarList[2]);
            //去除尾部空格
            p.erase(p.find_last_not_of(" ") + 1);
            if (p.empty())
                continue;

            QString qs = QString::asprintf("%3.3x(%s)=%s\r\n", r,
                m_Pal->pal->PAL_GetWord(r).c_str(), p.c_str());
            std::string cs = qs.toStdString();
            m_Pal->Utf8ToSys(cs);
            newExplain.insert(newExplain.end(),cs.begin(),cs.end());
        }
        oldExplain = std::move(newExplain);
        m_Pal->pal->gpGlobals->lpObjectDesc = 
            m_Pal->pal->gpGlobals->PAL_LoadObjectDesc();
        //标识修改
        m_Pal->ModifRecord[modRoc_descDat] = TRUE;
        ;
        return;
    }
    
    case WM_EditDialog://对话编辑
    {
        auto &pData = m_Pal->pal->gpGlobals;
        //对话修改
        qDebug() << "修改对话" ;
        ByteArray tmpdata;
        std::vector<DWORD> tmpoff(nsize + 1);
        for (size_t r = 0; r < nsize ; r++)
        {
            tmpoff.at(r) = tmpdata.size();
            std::string p = std::get<std::string>(m_DataArray[r].ColVarList.at(3));
            m_Pal->Utf8ToSys(p);
            //在尾部追加
            tmpdata.insert(tmpdata.end(), p.begin(), p.end());
            tmpoff.at(r + 1) = tmpdata.size();
        }
        pData->f.fpMsg = std::move(tmpdata);
        pData->g_TextLib.lpMsgBuf = pData->f.fpMsg.data();
        pData->g_TextLib.lpMsgOffset = std::move(tmpoff);

        //标记修改处
        m_Pal->ModifRecord[modRoc_mMsg] = TRUE;
        m_Pal->ModifRecord[modRoc_wordDat] = TRUE;
        m_Pal->ModifRecord[modRoc_sssMkf] = TRUE;
        break;
    }

    case WM_EditScene://场景编辑
    {
        for (int r = 0; r < nsize; r++)
        {
            auto sence = (LPSCENE)m_Pal->pal->gpGlobals->g.rgScene.data();
            sence->wScriptOnEnter = std::get<int>(m_DataArray[r].ColVarList[1]);
            sence->wScriptOnTeleport = std::get<int>(m_DataArray[r].ColVarList[2]);
            sence->wEventObjectIndex = std::get<int>(m_DataArray[r].ColVarList[3]);
            sence->wMapNum = std::get<int>(m_DataArray[r].ColVarList[5]);
        }
        if (JobCtrl <= 0)
            m_Pal->saveGameDataToCache();
        break;
    }

    case WM_Inventory://背包编辑
    {
        for (int r = 0; r < nsize; r++)
        {
            DWORD j = r;
            WORD wItem = std::get<int>(m_DataArray[r].ColVarList[1]);

            if (wItem && (wItem >= MAX_OBJECTS ||
                ((m_Pal->gpObject_classify[wItem] != kIsItem) &&
                    m_Pal->gpObject_classify[wItem] != kIsPoison)))
            {
                return;
            }
            m_Pal->pal->gpGlobals->rgInventory[j].wItem = wItem;
            m_Pal->pal->gpGlobals->rgInventory[j].nAmount = get<int>(m_DataArray[r].ColVarList[2]);
        }
        //修改后的数据存到
        if(JobCtrl )
        {
            int saveTime = *((WORD*)m_Pal->pal->gpGlobals->rgSaveData[JobCtrl - 1].data());
            //存档文件数据
            m_Pal->pal->gpGlobals->PAL_SaveGame(m_Pal->pal->gpGlobals->rgSaveData[JobCtrl - 1], saveTime);
        }
        break;
    }

    case WM_EditGameSeting://修改游戏设置
    {
        for (int r = 0; r < nsize; r++)
        {
            //修改游戏设置
            m_Pal->pal->ggConfig->m_Function_Set[r] = get<int>(m_DataArray[r].ColVarList[1]);
        }
        if (int k = ( m_Pal->pal->ggConfig->m_Function_Set[47] + m_Pal->pal->ggConfig->m_Function_Set[53])
            != m_Pal->pal->gpGlobals->rgSaveData.size())
        {
            m_Pal->pal->gpGlobals->rgSaveData.resize(k);
        }
        if (JobCtrl <= 0)
            m_Pal->saveGameDataToCache();
        break;
    }
    case WM_EditFontLibraryPath:
        //修改字库路径
        break;
    default:
        break;
    }
}

//修改目录 ，没有改变返回0 ，否则返回 1,出错返回-1
INT MainWindow::DoChangeDir(bool change)
{
    //qt 取工作目录   
    QString currentDir = QDir::currentPath();
    auto oldDir = currentDir;
    std::string dir = currentDir.toStdString();
    dir += "/";
    if (CGetPalData::setCorrectDir(dir,change) )//选择目录错 
        return -1;
    currentDir = dir.c_str();
    oldDir += "/";
    if (oldDir== currentDir)
        return 0;
    QDir::setCurrent(currentDir);
    return 1;
}




