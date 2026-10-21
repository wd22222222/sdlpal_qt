#ifndef CCONFIG_H
#define CCONFIG_H
#pragma once

#include <string>
#include "command.h"
#include "../sound/soundseting.h"

#define  RangeValues(val,mix,max) {if(val<(mix))val=(mix);else if(val>(max))val=(max);} 

enum  tagConfigSet
{
	tagPoisonUp         ,// 增加毒的烈度
	tagDmgFormula       ,// 修改伤害计算公式（包括敌人的属性计算方式）
	tagShowBtlData      ,// 在战斗中显示相关数据
	tagBuffEarlyFoe     ,// 在后期加强前期的敌人、队伍多于3人，敌人加强
	tagGourdToShop      ,// 修改灵葫炼丹为商店
	tagShowMPDrop       ,// 当mp减少时也显示数值
	tagStatusNoResist   ,// 在一定情况下添加状态总是成功，无视巫抗
	tagFullRestoreFail  ,// 补体力真气时已满返回失败
	tagIncenseSteps     ,// 显示驱魔香和十里香步数
	tagDefBoostActive   ,// 主動防禦时，防御加強
	tagSnatchCD         ,// 设置冷却值 在之后的 n 次夺魂无效
	tagBonusAttrs       ,// 修改附加属性
	tagPostBtlScript    ,// 修改战后衩覆盖的脚本
	tagExpFormula       ,// 修改附加经验计算方式
	tagBonusExp         ,// 额外附加经验
	tagBonusRecover     ,// 额外恢复
	tagSemiRandomTgt    ,// 不完全是随机选择目标
	tagFoeMaxActions    ,// 敌人最多行动次数，如果为2则是经典版
	tagPoisonPierce     ,// 某些毒可以对任何人均命中（无视敌方巫抗或我方毒抗）
	tagHeroUniqueBuff   ,// 有特色的加强主角，灵儿初始五灵抗性20%，阿奴毒抗巫抗各30%，林月如额外恢复
	tagTiangangThrow    ,// 天罡战气后，投掷偷武器伤害增加
	tagDreamSnakeUp     ,// 使用梦蛇后，各项属性经验增加
	tagAutoDefRatio     ,// 自动防御比率，0 为手动防御
	tagBtlFrameMs       ,// cfgBtlFrameMs 战时每帧毫秒数 40
	tagIdleFrameMs      ,// 非战时每帧毫秒数 100
	tagConfuseMode      ,// = 1 怪物混乱攻击同伴, = 2 怪物混乱无同伴攻击我方
	tagSplitHPHalf      ,// 怪物分裂体力减半
	tagAudioDevID       ,// 音频设备号 -1 系统缺省
	tagSurroundOPL      ,// SurroundOPLOffset 缺省值384
	tagChannels         ,// 声道数2 双声道，1 单声道
	tagSampleRate       ,// 采样率 缺省为44100
	tagOPLSampleRate    ,// OPL 采样率，缺省为 49716
	tagSampleQual       ,// 采样质量 0 到 4 缺省为 4
	tagMusicVol         ,// 音乐音量 缺省为100
	tagSFXVol           ,// 效果音量 缺省为100
	tagMusicFmt         ,// 音乐格式 0 MIDE,1 RIX 2 MP3 3 OGG
	tagOPLCore          ,// OPL核心
	tagOPLChip          ,// OPLChip
	tagMIDISetup        ,// MIDI设置
	tagPreferMP3        ,// 优先使用MP3 1 是
	tagUseBig5          ,// 是否使用Big5 繁体字，1 是
	tagUseSurroundOPL   ,// 使用环绕OPL
	tagKeepAspect       ,// 保持窗口显示比例， 1 是
	tagUseAVI           ,// 是否使用过场动画AVI，1 是
	tagMusicBuf         ,// 音乐缓存区大小
	tagFontWeight       ,// 字体加粗，0 - 4
	tagBoldFont         ,// 使用粗体字， 1 是
	tagMaxSaves         ,// 最大存档文件数5-50
	tagSpeedMod         ,// 加速或减速 -80%--+500%
	tagCharSet          ,// 系统字符集，0自动，1 GB2312，2 BIG5
	tagRealtimeBtl      ,// 是否采用即时对战模式
	tagBrightBoost      ,// 亮度增强
	tagBossRetryQ       ,// 与BOSS战斗时 Q 键直接重新开始
	tagAutoSave         ,// 自动存档
	tagHiddenExp        ,// 更多的隐藏经验
	tagShowCursor       ,// 显示鼠标盘
	tagConfigCount		,//总数
};
typedef struct  tagConfigRange { int min{}, max{}, def{}; LPCSTR istr{}; } ConfigRange;

inline static ConfigRange configRange[tagConfigCount] = {};

//

#define cfgPoisonUp        CConfig::m_Function_Set[tagPoisonUp]        // 增加毒的烈度
#define cfgDmgFormula      CConfig::m_Function_Set[tagDmgFormula]      // 修改伤害计算公式（包括敌人的属性计算方式）
#define cfgShowBtlData     CConfig::m_Function_Set[tagShowBtlData]     // 在战斗中显示相关数据
#define cfgBuffEarlyFoe    CConfig::m_Function_Set[tagBuffEarlyFoe]    // 在后期加强前期的敌人、队伍多于3人，敌人加强
#define cfgGourdToShop     CConfig::m_Function_Set[tagGourdToShop]     // 修改灵葫炼丹为商店
#define cfgShowMPDrop      CConfig::m_Function_Set[tagShowMPDrop]      // 当mp减少时也显示数值
#define cfgStatusNoResist  CConfig::m_Function_Set[tagStatusNoResist]  // 在一定情况下添加状态总是成功，无视巫抗
#define cfgFullRestoreFail CConfig::m_Function_Set[tagFullRestoreFail] // 补体力真气时已满返回失败
#define cfgIncenseSteps    CConfig::m_Function_Set[tagIncenseSteps]    // 显示驱魔香和十里香步数
#define cfgDefBoostActive  CConfig::m_Function_Set[tagDefBoostActive]  // 主動防禦时，防御加強
#define cfgSnatchCD        CConfig::m_Function_Set[tagSnatchCD]        // 设置冷却值 在之后的 n 次夺魂无效
#define cfgBonusAttrs      CConfig::m_Function_Set[tagBonusAttrs]      // 修改附加属性
#define cfgPostBtlScript   CConfig::m_Function_Set[tagPostBtlScript]   // 修改战后覆盖的脚本
#define cfgExpFormula      CConfig::m_Function_Set[tagExpFormula]      // 修改附加经验计算方式
#define cfgBonusExp        CConfig::m_Function_Set[tagBonusExp]        // 额外附加经验
#define cfgBonusRecover    CConfig::m_Function_Set[tagBonusRecover]    // 额外恢复
#define cfgSemiRandomTgt   CConfig::m_Function_Set[tagSemiRandomTgt]   // 不完全是随机选择目标
#define cfgFoeMaxActions   CConfig::m_Function_Set[tagFoeMaxActions]   // 敌人最多行动次数，如果为2则是经典版
#define cfgPoisonPierce    CConfig::m_Function_Set[tagPoisonPierce]    // 某些毒可以对任何人均命中（无视敌方巫抗或我方毒抗）
#define cfgHeroUniqueBuff  CConfig::m_Function_Set[tagHeroUniqueBuff]  // 有特色的加强主角，灵儿初始五灵抗性20%，阿奴毒抗巫抗各30%，林月如额外恢复
#define cfgTiangangThrow   CConfig::m_Function_Set[tagTiangangThrow]   // 天罡战气后，投掷偷武器伤害增加
#define cfgDreamSnakeUp    CConfig::m_Function_Set[tagDreamSnakeUp]    // 使用梦蛇后，各项属性经验增加
#define cfgAutoDefRatio    CConfig::m_Function_Set[tagAutoDefRatio]    // 自动防御比率，0 为手动防御
#define cfgBtlFrameMs      CConfig::m_Function_Set[tagBtlFrameMs]      // cfgBtlFrameMs 战时每帧毫秒数 40
#define cfgIdleFrameMs     CConfig::m_Function_Set[tagIdleFrameMs]     // 非战时每帧毫秒数 100
#define cfgConfuseMode     CConfig::m_Function_Set[tagConfuseMode]     // = 1 怪物混乱攻击同伴, = 2 怪物混乱无同伴攻击我方
#define cfgSplitHPHalf     CConfig::m_Function_Set[tagSplitHPHalf]     // 怪物分裂体力减半
#define cfgAudioDevID      CConfig::m_Function_Set[tagAudioDevID]      // 音频设备号 -1 系统缺省
#define cfgSurroundOPL     CConfig::m_Function_Set[tagSurroundOPL]     // SurroundOPLOffset 缺省值384
#define cfgChannels        CConfig::m_Function_Set[tagChannels]        // 声道数2 双声道，1 单声道
#define cfgSampleRate      CConfig::m_Function_Set[tagSampleRate]      // 采样率 缺省为44100
#define cfgOPLSampleRate   CConfig::m_Function_Set[tagOPLSampleRate]   // OPL 采样率，缺省为 49716
#define cfgSampleQual      CConfig::m_Function_Set[tagSampleQual]      // 采样质量 0 到 4 缺省为 4
#define cfgMusicVol        CConfig::m_Function_Set[tagMusicVol]        // 音乐音量 缺省为100
#define cfgSoundVolume     CConfig::m_Function_Set[tagSFXVol]          // 效果音量 缺省为100
#define cfgMusicType	   CConfig::m_Function_Set[tagMusicFmt]        // 音乐格式 0 MIDE,1 RIX 2 MP3 3 OGG 
#define cfgOPLCore         CConfig::m_Function_Set[tagOPLCore]         // OPL核心
#define cfgOPLChip         CConfig::m_Function_Set[tagOPLChip]         // OPLChip
#define cfgMIDISetup       CConfig::m_Function_Set[tagMIDISetup]       // MIDI设置
#define cfgFirstUseMp3     CConfig::m_Function_Set[tagPreferMP3]       // 优先使用MP3 1 是
#define cfgUseBig5         CConfig::m_Function_Set[tagUseBig5]         // 是否使用Big5 繁体字，1 是
#define cfgUseSurroundOPL  CConfig::m_Function_Set[tagUseSurroundOPL]  // 使用环绕OPL
#define cfgKeepAspect      CConfig::m_Function_Set[tagKeepAspect]      // 保持窗口显示比例， 1 是
#define cfgUseAVI          CConfig::m_Function_Set[tagUseAVI]          // 是否使用过场动画AVI，1 是
#define cfgMusicBuf        CConfig::m_Function_Set[tagMusicBuf]        // 音乐缓存区大小
#define cfgFontWeight      CConfig::m_Function_Set[tagFontWeight]      // 字体加粗，0 - 4
#define cfgBoldFont        CConfig::m_Function_Set[tagBoldFont]        // 使用粗体字， 1 是
#define cfgMaxSaves        CConfig::m_Function_Set[tagMaxSaves]        // 最大存档文件数5-50
#define cfgSpeedMod        CConfig::m_Function_Set[tagSpeedMod]        // 加速或减速 -80%--+500%
#define cfgCharSet         CConfig::m_Function_Set[tagCharSet]         // 系统字符集，0自动，1 GB2312，2 BIG5
#define cfgRealtimeBtl     CConfig::m_Function_Set[tagRealtimeBtl]     // 是否采用即时对战模式
#define cfgBrightBoost     CConfig::m_Function_Set[tagBrightBoost]     // 亮度增强
#define cfgBossRetryQ      CConfig::m_Function_Set[tagBossRetryQ]      // 与BOSS战斗时 Q 键直接重新开始
#define cfgAutoSave        CConfig::m_Function_Set[tagAutoSave]        // 自动存档
#define cfgHiddenExp       CConfig::m_Function_Set[tagHiddenExp]       // 更多的隐藏经验
#define cfgShowCursor      CConfig::m_Function_Set[tagShowCursor]      // 显示鼠标盘

#define cfgIsWin95		   CConfig::fIsWIN95						   //系统格式是否是WIN95
#define cfgIsUSEYJ1DeCompress CConfig::fisUSEYJ1DeCompress			   //是否使用YJ1压缩格式
//#define STRENGTHEN_PLAYER	CConfig::m_Function_Set[19] //

enum tagDebugSwitch
{
	fIsInvincible = 0,        //无敌
	fIsQuickKill,             //快速击杀
	fIsNoShowNormalScript,    //不显示正常脚本
	fIsShowEquipScript,       //显示装备脚本
	fIsShowAutoScript,        //显示自动脚本
	fIsMarkObjectPos,         //标记对象位置
	fIsShowSimpleScript,      //显示简单脚本
	fIsPassObstacle,           //通过障碍物
	flsEnd,
};

class CConfig
{
public:
	INT				 iLogLevel = LOGLEVEL::LOGLEVEL_ERROR;//日志级别
	inline static  BOOL             fIsWIN95 = 0;//
	inline static BOOL	 fisUSEYJ1DeCompress = -1;
	BOOL             fFullScreen = 0;//满屏
	BOOL             fEnableJoyStick = 0;//使用游戏使操纵杆
	BOOL             fEnableKeyRepeat = 0;//重复按键
	BOOL             fUseTouchOverlay = 0;//触屏辅助

	inline static INT32		m_Function_Set[80]{ 0 };//功能设置
public:
	CConfig();
	~CConfig();
    typedef struct tagConfigRange { int min{}, max{}, def{}; LPCSTR istr{}; } ConfigRange;

    inline static ConfigRange configRange[tagConfigCount] = {};

	
	inline static std::string m_FontName = "c:/windows/fonts/simfang.ttf";
private:
	void InitConfigRange();
public:	
	void setConfigRange();
	static void setConfigValues();
};

size_t SDL_GetTicks_New();
void trim(char* str);
#endif