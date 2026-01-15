#pragma once

#include "sdl2_compat.h"
#include <functional>
#include <condition_variable>
#include <vector>
#include <unordered_map>
#include "command.h"


enum PALKEY
{
	kKeyMenu = (1 << 0),
	kKeySearch = (1 << 1),
	kKeyDown = (1 << 2),
	kKeyLeft = (1 << 3),
	kKeyUp = (1 << 4),
	kKeyRight = (1 << 5),
	kKeyPgUp = (1 << 6),
	kKeyPgDn = (1 << 7),
	kKeyRepeat = (1 << 8),
	kKeyAuto = (1 << 9),
	kKeyDefend = (1 << 10),
	kKeyUseItem = (1 << 11),
	kKeyThrowItem = (1 << 12),
	kKeyFlee = (1 << 13),
	kKeyStatus = (1 << 14),
	kKeyForce = (1 << 15),

//#ifdef  SHOW_DATA_IN_BATTLE
	kKeyGetInfo = (1 << 16),
//#endif

#ifdef  SHOW_ENEMY_STATUS
	kKeyEnemyInfo = (1 << 17),
#endif // SHOW_ENEMY_STATUS
	//自动存档
	kKeyAutoSave = (1 << 18),
};

typedef enum tagPALDIRECTION
{
	kDirSouth = 0,
	kDirWest,
	kDirNorth,
	kDirEast,
	kDirUnknown
} PALDIRECTION, * LPPALDIRECTION;

typedef struct tagPALINPUTSTATE
{
	PALDIRECTION           dir{}, prevdir{};
	DWORD                  dwKeyPress{};
} PALINPUTSTATE;

//事件类型
enum class eventTypeCustom {
	//avi显示处理事件
	eventTypeAvi,
	//avi音乐播放事件
	eventAviMusicPlay,
	//mp3音乐播放事件
	eventMp3MusicPlay,
	//Rix音乐播放事件
	eventRixMusicPlay,
	//sound音乐播放事件
	eventSoundPlay,
	//midev音乐播放事件
	eventMidiMusicPlay,
#if USING_OGG
	//ogg音乐播放事件
	eventOggMusicPlay,
#endif
	//自定义事件结止
	eventTypeCustomEnd,
};


class CPalEventHandler
{
private:
	using EventCallback = std::function<void(const SDL_Event*)>;
	//自定义消息队列
	inline static std::unordered_map<Uint32, EventCallback> callbacks;
	//锁队列
	inline static std::unordered_map<Uint32, volatile bool> customMutex;
	inline static std::vector<Uint32> customEventType;
public:
	CPalEventHandler();
	~CPalEventHandler();
	//处理一个事件
	void handleEvents(SDL_Event *event);
	void eventClear();
	//注册事件回调
	void registCallback(UINT eventType, EventCallback callback);
	//取得自定义事件句柄
	Uint32 getCustomEventHandler(eventTypeCustom n);
	//注入event
	int PushEvent(SDL_Event* lpevent);
	// 注册自定义事件类型
	Uint32 registerCustomEventType();
	//取得锁
	BOOL  getCustomMutex(UINT eventType);
	//设置锁
	void  setCustomMutex(UINT eventType, BOOL v);
	//
	VOID PAL_ProcessEvent();
	INT PAL_PollEvent(SDL_Event* event);
};
