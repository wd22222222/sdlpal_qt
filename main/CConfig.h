#ifndef CCONFIG_H
#define CCONFIG_H
#pragma once

#include <string>
#include "command.h"
#include "../sound/soundseting.h"

#define configStatMark 27

#define iAudioDevice		m_Function_Set[configStatMark + 0] //声音设备号
#define iSurroundOPLOffset	m_Function_Set[configStatMark + 1] //围绕OPL抵消
#define iAudioChannels		m_Function_Set[configStatMark + 2] //连接数 
#define iSampleRate			m_Function_Set[configStatMark + 3] //采样率 44100
#define iOPLSampleRate		m_Function_Set[configStatMark + 4] //OPL频率 49716
#define iResampleQuality	m_Function_Set[configStatMark + 5] //重新取样质量,重新取样质量最大
#define iMusicVolume		m_Function_Set[configStatMark + 6] //音乐音量
#define iSoundVolume		m_Function_Set[configStatMark + 7] //iSoundVolume
#define eMusicType			m_Function_Set[configStatMark + 8] //音乐类型
#define eOPLCore			m_Function_Set[configStatMark + 9] //OPL核心
#define eOPLChip			m_Function_Set[configStatMark + 10]
#define eMIDISynth			m_Function_Set[configStatMark + 11] // MIDI设置
#define firstUseMp3			m_Function_Set[configStatMark + 12] //优先使用MP3
#define fIsUseBig5			m_Function_Set[configStatMark + 13] //UseBig5			
#define fUseSurroundOPL		m_Function_Set[configStatMark + 14] //使用环绕OPL
#define fKeepAspectRatio	m_Function_Set[configStatMark + 15] //保持比例
#define fEnableAviPlay		m_Function_Set[configStatMark + 16] //是否过场动画AVI
#define wAudioBufferSize	m_Function_Set[configStatMark + 17] //音乐缓存区尺寸 

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
	BOOL             fIsWIN95 = 0;//
	inline static BOOL	 fisUSEYJ1DeCompress = -1;
	BOOL             fFullScreen = 0;//满屏
	BOOL             fEnableJoyStick = 0;//使用游戏使操纵杆
	BOOL             fEnableKeyRepeat = 0;//重复按键
	BOOL             fUseTouchOverlay = 0;//触屏辅助
	
	INT32		m_Function_Set[80]{ 0 };//功能设置
public:
	CConfig();
	~CConfig();
	typedef struct tagConfigRange { int min{}, max{}; } ConfigRange;
	inline static ConfigRange configRange[] =
	{
		{0,1},{0,1},{0,1},{0,1},{0,1},{0,1},{0,1},{0,1},{0,1},{0,1},//1-10
		{0,5},{0,1},{0,1},{0,1},{0,1},{0,1},{0,1},{2,4},{0,1},{0,1},//1-20
		{0,1},{0,1},{0,100},{25,50},{50,100},{0,2},{0,1},{-1,100},{300,1000},{1,2},//21-30
		{10000,60000},{10000,60000},{0,4},{40,100},{40,100},{0,(int)musicType::endMusicType -1 },{0,1},{0,1},{0,1},{0,1},//31-40
		{0,1},{0,1},{0,1},{0,1},{512,20000},{0,4},{0,1},{5,MAXSAVEFILES},{-80,500},{0,2},//41-50
		{0,1},{0,1},{0,1},{0,1},{0,1},{0,1},//51 - 60[50-59]
	};
	std::string m_FontName = "c:/windows/fonts/simfang.ttf";
};

size_t SDL_GetTicks_New();
void trim(char* str);
#endif