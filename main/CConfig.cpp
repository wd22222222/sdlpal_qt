///* -*- mode: c; tab-width: 4; c-basic-offset: 4; c-file-style: "linux" -*- */
//
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

#include <string>
#include "cconfig.h"
#include "cscript.h"

const int PALCFG_ALL_MAX = 30;

typedef enum tagPALCFG_ITEM
{
AudioDevice , //声音设备号
SurroundOPLOffset,//围绕OPL抵消
AudioChannels ,//连接数
SampleRate,   //采样率
OPLSampleRate ,//= 49716;//OPL频率
ResampleQuality,// = 4;//重新取样质量,重新取样质量最大
MusicVolume,// = 80;//音乐音量
SoundVolume,// = 80;//声音音量
LogLevel ,// LOGLEVEL::LOGLEVEL_ERROR;//日志级别
MusicType,// = MUSICTYPE::MUSIC_RIX;//音乐类型
OPLCore,// = OPLCORE_TYPE::OPLCORE_MAME;//OPL核心
OPLChip,// = OPLCHIP_TYPE::OPLC,//HIP_OPL2; //OPL Chip
MIDISynth,// = MIDISYNTHTYPE::SYNTH_NATIVE;// MIDI设置
AudioBufferSize,// = 1024;//缓存区尺寸
UseMp3,// = 1;//优先使用MP3
IsWIN95,// = 0;//
IsUseBig5,// = 0;//是否使用繁体字显示;0 不使用
UseSurroundOPL,// = 0;//使用环绕OPL
KeepAspectRatio,// = 1;//保持比例
FullScreen,// = 0;//满屏
EnableJoyStick,// = 0;//使用游戏使操纵杆
UseCustomScreenLayout,// = 0;//自定义屏幕布局
LaunchSetting,// = 1;//启动设置
EnableKeyRepeat,// = 0;//重复按键
UseTouchOverlay,// = 0;//触屏辅助
EnableAviPlay,// = 1;//是否过场动画AVI
Font,// = "c:/windows/fonts/simfang.ttf";//字体文件名
GamePath,//游戏路径
}PALCFG_ITEM;

typedef struct tagConfigItem
{
    PALCFG_ITEM        Item;//
    const char* Name;//名称，字符串
    const char* Notes;//说明，字符串
    int               Value ;// 值，如是字符串，值为-999;
    std::string       ValueStr;
} ConfigItem;

ConfigItem configValue[] =
{
    {FullScreen,"FullScreen",               "1 满屏显示,缺省为 0 " ,0,""},
    {AudioDevice ,"AudioDevice",            "-1 系统缺省",  -1,""},
    {SurroundOPLOffset,"SurroundOPLOffset", "缺省值384",384,""},
    {AudioChannels ,"Stereo",               "1 双声道，0 单声道",0,""},
    {SampleRate,"SampleRate",               "采样率 缺省为44100",44100,""},
    {OPLSampleRate ,"OPLSampleRate",        "缺省为 49716",49716,""},
    {ResampleQuality,"ResampleQuality",     "0 到 4 缺省为 4",4,""},
    {MusicVolume,"MusicVolume",             "缺省为100",100,""},
    {SoundVolume,"SoundVolume",             "缺省为100",100,""},
    {LogLevel ,"LogLevel",                  "0 到 5 级 ，所有，调试，信息，警告，错误，失败。缺省为 0",0,""},
    {MusicType,"Music",                     "0 MIDI,1 RIX 2 MP3 3 Ogg",1,""},
    {AudioBufferSize,"AudioBufferSize",     "缺省为1024",1024,""},// = 1024;//缓存区尺寸
    {UseMp3,"UseMp3",                       "1 优先使用MP3 ",1,""},// = 1;//优先使用MP3
    {IsUseBig5,"IsUseBig5",                 "1 使用Big5 繁体字",1,""},// = 0;//是否使用繁体字显示;0 不使用
    {UseSurroundOPL,"UseSurroundOPL",       "1 使用环绕 OPL 缺省为 0 ",0,""},// = 0;//使用环绕OPL
    {KeepAspectRatio,"KeepAspectRatio",     "1 保持窗口显示比例，缺省为 1",1,""},// = 1;//保持比例
    {EnableJoyStick,"EnableJoyStick",       "1 使用游戏杆 缺省为0 目前不支持",0,""},// = 0;//使用游戏使操纵杆
    {EnableKeyRepeat,"EnableKeyRepeat",     "1 重复按键 ,缺省为0 ",0,""},// = 0;//重复按键
    {UseTouchOverlay,"UseTouchOverlay",     "1 触屏辅助,目前不支持",0,""},// = 0;//触屏辅助
    {EnableAviPlay,"EnableAviPlay",         "是否使用过场动画AVI 1 是 ",1,""},// = 1;//是否过场动画AVI
    {Font,"Font",                           "缺省为c:/windows/fonts/simfang.ttf", -999,(LPSTR)"c:/windows/fonts/simfang.ttf"},
    {GamePath,"GamePath",                    "游戏路径缺省为 ./",-999,"./"},//游戏路径
};

void trim(char* str)
{
    if (!str)
        return;
    UINT8 * p = (LPBYTE)str;
    int i = 0;
    while (*p)
    {
        if ((*p) > 32)
            str[i++] = *p;
        p++;
    }
    str[i] = '\0';
}


CConfig::CConfig()
{
    m_Function_Set[0] = 1; //增加毒的烈度
    m_Function_Set[1] = 1; //修改伤害计算公式（包括敌人的属性计算方式）
    m_Function_Set[2] = 1; //在战斗中显示相关数据
    m_Function_Set[3] = 1; //在后期加强前期的敌人、队伍多于3人，敌人加强
    m_Function_Set[4] = 1; //修改灵葫炼丹为商店
    m_Function_Set[5] = 1; //当mp减少时也显示数值
    m_Function_Set[6] = 1; //为敌人添加状态总是成功（无视巫抗）
    m_Function_Set[7] = 1; //补体力真气时已满返回失败
    m_Function_Set[8] = 1; //显示驱魔香和十里香步数
    m_Function_Set[9] = 1; //主動防禦加強
    m_Function_Set[10] = 2; //设置冷却值 在之后的 n 次夺魂无效
    m_Function_Set[11] = 1; //修改战后补充属性
    m_Function_Set[12] = 1; //修改战后衩覆盖的脚本
    m_Function_Set[13] = 1; //修改附加经验计算方式
    m_Function_Set[14] = 1; //额外附加经验
    m_Function_Set[15] = 1; //额外恢复
    m_Function_Set[16] = 1; //不完全是随机选择目标
    m_Function_Set[17] = 3; //敌人最多行动次数，如果为2则是经典版
    m_Function_Set[18] = 1; //某些毒可以对任何人均命中（无视敌方巫抗或我方毒抗）
    m_Function_Set[19] = 1; //有特色的加强主角，灵儿初始五灵抗性20%，阿奴毒抗巫抗各30%，林月如额外恢复
    m_Function_Set[20] = 1; //天罡战气后，投掷偷武器伤害增加
    m_Function_Set[21] = 1; //使用梦蛇后，各项属性增加
    m_Function_Set[22] = 20;//自动防御比率，为零手动子防御
    m_Function_Set[23] = 25;//BATTLE_FRAME_TIME 战时每帧毫秒数 40
    m_Function_Set[24] = 60;//FRAME_TIME 非战时每帧毫秒数 100
    m_Function_Set[25] = 1;// = 1 怪物混乱攻击同伴, = 2 怪物混乱无同伴攻击我方
    m_Function_Set[26] = 0;//怪物分裂体力减半

    m_Function_Set[45] = 0;//字体加粗，0 - 4
    m_Function_Set[46] = 0;//使用粗休 1 粗体
    m_Function_Set[47] = 5;//最大存档文件数5-10
    m_Function_Set[48] = 0;//变速百分比 负数加速 正数减速
    m_Function_Set[49] = 0;//缺省认定系统字符集，0 自动，1 GB2312，2BIG5
    m_Function_Set[50] = 0;//是否使用即时对战模式
    m_Function_Set[51] = 1;//亮度增强
	m_Function_Set[52] = 0;//与BOSS战斗时 Q 键直接重新开始
    m_Function_Set[53] = 0;//自动存档
    m_Function_Set[54] = 0;//更多的隐藏经验

    iAudioDevice = -1; //声音设备号
    iSurroundOPLOffset = 384;//围绕OPL抵消
    iAudioChannels = 2;//连接数
    iSampleRate = 44100;   //采样率
    iOPLSampleRate = 49716;//OPL频率
    iResampleQuality = 4;//重新取样质量,重新取样质量最大
    iMusicVolume = 80;//音乐音量
    iSoundVolume = 80;//声音音量
    eMusicType = (UINT32)musicType::rix;//音乐类型
    eOPLCore = OPLCORE_TYPE::OPLCORE_MAME;//OPL核心
    eOPLChip = OPLCHIP_TYPE::OPLCHIP_OPL2; //OPL Chip
    eMIDISynth = MIDISYNTHTYPE::SYNTH_NATIVE;// MIDI设置
    wAudioBufferSize = 1024 ;//缓存区尺寸
    firstUseMp3 = 1;//优先使用MP3 
    fIsUseBig5 = 1;//是否使用繁体字显示;0 不使用
    fUseSurroundOPL = 0;//使用环绕OPL
    fKeepAspectRatio = 1;//保持比例
    fEnableAviPlay = 1;//是否过场动画AVI
}

CConfig::~CConfig()
{
}

size_t SDL_GetTicks_New()
{
    if (CPalEvent::ggConfig == nullptr)
        return 100;
    auto timeSpeed = CPalEvent::ggConfig->m_Function_Set[48] + 100;
    size_t t = timeSpeed * SDL_GetTicks();
    t /= 100;
    return t;
}
