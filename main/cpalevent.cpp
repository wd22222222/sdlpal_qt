///* -*- mode: c; tab-width: 4; c-basic-offset: 4; c-file-style: "linux" -*- */
//
// Copyright (c) 2009-2011, Wei Mingzhi <whistler_wmz@users.sf.net>.
// Copyright (c) 2011-2021, SDLPAL development team.
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
#include <iostream>
#include "sdl2_compat.h"
#include "cpalevent.h"
#include "cconfig.h"
#include "palgpgl.h"
#include "cscript.h"
#include "sdl2_compat.h"
#include <cstdio>
// 使用系统时间作为种子
//static unsigned seed;

// 随机数引擎
//static std::default_random_engine generator(seed);
static std::mt19937 generator(std::random_device{}());

CPalEvent::CPalEvent()
{
	//seed = std::chrono::system_clock::now().time_since_epoch().count();
	
	//注册退出事件
	_m.registCallback(SDL_QUIT, [this](const SDL_Event* e) {
		static BOOL reEvent = 0;
		if (reEvent)
			return;
		reEvent = TRUE;
		if (pCScript->PAL_ConfirmMenu("退出吗？"))
		{
			pCScript->PAL_PlayMUS(0, FALSE, 2);
			pCScript->PAL_FadeOut(2);
			PalQuit = TRUE;
		}
		reEvent = FALSE;
		//清除按键状态
		PAL_ClearKeyAll();
		});

	//注册：keydown信号
	_m.registCallback(SDL_KEYDOWN, std::bind(&CPalEvent::KeydownEvent,
		this, std::placeholders::_1));
	//注册：keyup信号
	_m.registCallback(SDL_KEYUP, std::bind(&CPalEvent::KeyupEvent,
		this, std::placeholders::_1));
	//注册鼠标事件
	_m.registCallback(SDL_MOUSEMOTION, std::bind(&CPalEvent::MouseMotionEvent,
		this, std::placeholders::_1));
	_m.registCallback(SDL_MOUSEBUTTONDOWN, std::bind(&CPalEvent::MouseDownEvent,
		this, std::placeholders::_1));
	_m.registCallback(SDL_MOUSEBUTTONUP, std::bind(&CPalEvent::MouseUpEvent,
		this, std::placeholders::_1));
	_m.registCallback(SDL_MOUSEWHEEL, std::bind(&CPalEvent::MouseWheelEvent,
		this, std::placeholders::_1));

}

CPalEvent::~CPalEvent()
{
	//ggConfig = nullptr;
}
int CPalEvent::PAL_PollEvent(SDL_Event* event)
{
	return _m.PAL_PollEvent(event);
}

void CPalEvent::KeydownEvent(const SDL_Event* lpEvent)
{
	//复合键
	if (get_SDL_EnevtKeyMod(lpEvent) & KMOD_ALT)
	{
		if (get_SDL_EnevtKey(lpEvent) == SDLK_RETURN || 
			get_SDL_EnevtKey(lpEvent) == SDLK_KP_ENTER)
		{
			return;
		}
		else if (get_SDL_EnevtKey(lpEvent) == SDLK_F4)
		{
			return;
		}
	}
	if ((CPalEvent::ggConfig->m_Function_Set[53] &&
		get_SDL_EnevtKeyMod(lpEvent) & KMOD_CTRL) &&
		get_SDL_EnevtKey(lpEvent) == SDLK_s)
	{
		g_InputState.dwKeyPress |= kKeyAutoSave;
		return;
	}
	//单键
	switch (get_SDL_EnevtKey(lpEvent))
	{
	case SDLK_t:
		g_InputState.dwKeyPress |= kKeyGetInfo;
		break;
	case SDLK_g:
		g_InputState.dwKeyPress |= kKeyEnemyInfo;
		break;

	case SDLK_UP:
	case SDLK_KP8:
	case SDLK_j:
		if (fInBattle || g_InputState.dir != kDirNorth)
		{
			g_InputState.prevdir = (fInBattle ? kDirUnknown : g_InputState.dir);
			g_InputState.dir = kDirNorth;
		}
		g_InputState.dwKeyPress |= kKeyUp;
		break;

	case SDLK_DOWN:
	case SDLK_KP2:
	case SDLK_n:
		if (fInBattle || g_InputState.dir != kDirSouth)
		{
			g_InputState.prevdir = (fInBattle ? kDirUnknown : g_InputState.dir);
			g_InputState.dir = kDirSouth;
		}
		g_InputState.dwKeyPress |= kKeyDown;
		break;

	case SDLK_LEFT:
	case SDLK_KP4:
	case SDLK_b:
		if (fInBattle || g_InputState.dir != kDirWest)
		{
			g_InputState.prevdir = (fInBattle ? kDirUnknown : g_InputState.dir);
			g_InputState.dir = kDirWest;
		}
		g_InputState.dwKeyPress |= kKeyLeft;
		break;

	case SDLK_RIGHT:
	case SDLK_KP6:
	case SDLK_m:
		if (fInBattle || g_InputState.dir != kDirEast)
		{
			g_InputState.prevdir = (fInBattle ? kDirUnknown : g_InputState.dir);
			g_InputState.dir = kDirEast;
		}
		g_InputState.dwKeyPress |= kKeyRight;
		break;

	case SDLK_ESCAPE:
	case SDLK_KP0:
		g_InputState.dwKeyPress |= kKeyMenu;
		break;

	case SDLK_RETURN:
	case SDLK_SPACE:
	case SDLK_KP_ENTER:
	case SDLK_LCTRL:
		g_InputState.dwKeyPress |= kKeySearch;
		break;

	case SDLK_PAGEUP:
	case SDLK_KP9:
		g_InputState.dwKeyPress |= kKeyPgUp;
		break;

	case SDLK_PAGEDOWN:
	case SDLK_KP3:
		g_InputState.dwKeyPress |= kKeyPgDn;
		break;

	case SDLK_7: //7 for mobile device
	case SDLK_r:
		g_InputState.dwKeyPress |= kKeyRepeat;
		break;

	case SDLK_2: //2 for mobile device
	case SDLK_a:
		g_InputState.dwKeyPress |= kKeyAuto;
		break;

	case SDLK_d:
		g_InputState.dwKeyPress |= kKeyDefend;
		break;

	case SDLK_e:
		g_InputState.dwKeyPress |= kKeyUseItem;
		break;

	case SDLK_w:
		g_InputState.dwKeyPress |= kKeyThrowItem;
		break;

	case SDLK_q:
		g_InputState.dwKeyPress |= kKeyFlee;
		break;

	case SDLK_s:
		g_InputState.dwKeyPress |= kKeyStatus;
		break;

	case SDLK_f:
	case SDLK_5: // 5 for mobile device
		g_InputState.dwKeyPress |= kKeyForce;
		break;

	case SDLK_HASH: //# for mobile device
	case SDLK_p:
		//VIDEO_SaveScreenshot();
		break;

	default:
		break;
	}
	//记录最后一键时间和键值
	lastKeyDownTime = SDL_GetTicks_New();
}

void CPalEvent::KeyupEvent(const SDL_Event* lpEvent) {
	//
	// Released a key
	//
	switch (get_SDL_EnevtKey(lpEvent))
	{
	case SDLK_UP:
	case SDLK_KP8:
	case SDLK_j:
		if (g_InputState.dir == kDirNorth)
		{
			g_InputState.dir = g_InputState.prevdir;
		}
		g_InputState.prevdir = kDirUnknown;
		break;

	case SDLK_DOWN:
	case SDLK_KP2:
	case SDLK_n:
		if (g_InputState.dir == kDirSouth)
		{
			g_InputState.dir = g_InputState.prevdir;
		}
		g_InputState.prevdir = kDirUnknown;
		break;

	case SDLK_LEFT:
	case SDLK_KP4:
	case SDLK_b:
		if (g_InputState.dir == kDirWest)
		{
			g_InputState.dir = g_InputState.prevdir;
		}
		g_InputState.prevdir = kDirUnknown;
		break;

	case SDLK_RIGHT:
	case SDLK_KP6:
	case SDLK_m:
		if (g_InputState.dir == kDirEast)
		{
			g_InputState.dir = g_InputState.prevdir;
		}
		g_InputState.prevdir = kDirUnknown;
		break;

	default:
		break;
	}
}

void CPalEvent::MouseDownEvent(const SDL_Event* lpEvent)
{
	//std::cout << "Mouse Button Down: " << (int)SDL_GetMouseButton(lpEvent)
		//<< " at (" << SDL_GetMouseX(lpEvent) << ", " 
		//<< SDL_GetMouseY(lpEvent) << ")\n";
	//双击测试
	static size_t lastTime = 0;
	auto time = SDL_GetTicks_New();
	if (time - lastTime < 300)
	{
		//双击
		//std::cout << "Mouse Double Clicked!\n";
		PAL_ClearKeyAll();
		//模拟按下空格键
		SetKeyPress(kKeySearch);
		return;
	}
	//右键测试
	if (SDL_GetMouseButton(lpEvent) == SDL_BUTTON_RIGHT)
	{
		//std::cout << "Mouse Right Button Clicked!\n";
		PAL_ClearKeyAll();
		//模拟按下Esc键
		SetKeyPress(kKeyMenu);
		return;
	}
	lastTime = time;
	//检测鼠标输入框点击
	int x = SDL_GetMouseX(lpEvent);
	int y = SDL_GetMouseY(lpEvent);
	x = (x - gView.x) * PictureWidth / gView.w;
	y = (y - gView.y) * PictureHeight / gView.h;
	g_InputMouse = PAL_XY(x, y);
	mouseInputKey = isRectsAtPoint(mouseInputList,g_InputMouse);
	if (mouseInputKey)
	{
		SDL_Event event{};
		switch (mouseInputKey)
		{
		case 1:
			set_SDL_EventKey(&event, SDLK_ESCAPE);
			break;
		case 2:
			set_SDL_EventKey(&event, SDLK_UP);
			break;
		case 3:
			set_SDL_EventKey(&event, SDLK_SPACE);
			break;
		case 4:
			set_SDL_EventKey(&event, SDLK_LEFT);
			break;
		case 5:
			if (fInBattle)
				set_SDL_EventKey(&event, SDLK_f);
			else
				set_SDL_EventKey(&event, SDLK_SPACE);
			break;
		case 6:
			set_SDL_EventKey(&event, SDLK_RIGHT);
			break;
		case 7:
			set_SDL_EventKey(&event, SDLK_PAGEUP);
			break;
		case 8:
			set_SDL_EventKey(&event, SDLK_DOWN);
			break;
		case 9:
			set_SDL_EventKey(&event, SDLK_PAGEDOWN);
			break;
		default:
			printf("鼠标输入错 err  \n");
			break;
		}
		//模拟按下按键
		//event.user.data1 = reinterpret_cast<void*>(mouseInputKey);
		event.type = SDL_KEYDOWN;
		//存最后鼠标键值，用于鼠标抬起
		lastMouseDir = get_SDL_EnevtKey(&event);
		SDL_PushEvent(&event);
		PAL_ProcessEvent();
		return;
	}
	//如果不在场景中，记录鼠标位置
	if (!isInScene)
	{
		std::cout << "Mouse Button Down: " << "x =" << x << ", y =" << y << "\n";
		return;//跳出
	}
	//北在右上方，南在左下方，西在左上方，东在右下方
	//模拟按键
	SDL_Event event{};
	if (2 * x < PictureWidth && 2 * y < PictureHeight)
	{
		//west
		set_SDL_EventKey(&event, SDLK_LEFT);
	}
	else if (2 * x >= PictureWidth && 2 * y < PictureHeight)
	{
		//north
		set_SDL_EventKey(&event, SDLK_UP);
	}
	else if (2 * x < PictureWidth && 2 * y >= PictureHeight)
	{
		//south
		set_SDL_EventKey(&event, SDLK_DOWN);
	}
	else
	{
		//east
		set_SDL_EventKey(&event, SDLK_RIGHT);
	}
	//模拟按下按键
	event.type = SDL_KEYDOWN;
	lastMouseDir = get_SDL_EnevtKey(&event);
	SDL_PushEvent(&event);
	PAL_ProcessEvent();
}

void CPalEvent::MouseUpEvent(const SDL_Event* lpEvent)
{
	SDL_Event e{};
	//模拟松开按键
	e.type = SDL_KEYUP;
	set_SDL_EventKey(&e, lastMouseDir);
	SDL_PushEvent(&e);
	PAL_ProcessEvent();
	mouseInputKey = 0;
}

void CPalEvent::MouseWheelEvent(const SDL_Event* lpEvent)
{
	//std::cout << "Mouse Wheel: " << lpEvent->wheel.y << "\n";
	if (!isInScene)
	{
		//向上滚动相当于按下上键
		if (lpEvent->wheel.y > 0)
		{
			if (fInBattle || g_InputState.dir != kDirNorth)
			{
				g_InputState.prevdir = (fInBattle ? kDirUnknown : g_InputState.dir);
				g_InputState.dir = kDirNorth;
			}
			g_InputState.dwKeyPress |= kKeyUp;

		}
		//向下滚动相当于按下下键
		else if (lpEvent->wheel.y < 0)
		{
			if (fInBattle || g_InputState.dir != kDirSouth)
			{
				g_InputState.prevdir = (fInBattle ? kDirUnknown : g_InputState.dir);
				g_InputState.dir = kDirSouth;
			}
			g_InputState.dwKeyPress |= kKeyDown;
		}
	}
}

void CPalEvent::MouseMotionEvent(const SDL_Event* lpEvent)
{
	//std::cout << "Mouse Motion to (" <<
		//SDL_GetMouseX(lpEvent) << ", " << SDL_GetMouseY(lpEvent) << ")\n";

}



int SDLCALL CPalEvent::PAL_EventFilter(SDL_Event* lpEvent)
{
	_m.handleEvents(lpEvent);
	return 0;
}

VOID CPalEvent::PAL_ClearKeyState(VOID) { 
	g_InputState.dwKeyPress = 0;
	g_InputMouse = 0;
}

VOID CPalEvent::PAL_ClearKeyAll()
{
	g_InputState.dwKeyPress = 0;
	g_InputMouse = 0;
	g_InputState.dir = kDirUnknown;
	g_InputState.prevdir = kDirUnknown;
}

DWORD CPalEvent::getKeyPress(VOID) const{ return g_InputState.dwKeyPress; }

VOID CPalEvent::setKeyPrevdir(const PALDIRECTION k)
{
	g_InputState.prevdir = k;
}


 VOID CPalEvent::set_switch_WINDOW_FULLSCREEN_DESKTOP(int flag) { (g_switch_WINDOW_FULLSCREEN_DESKTOP = flag); }

 VOID CPalEvent::PAL_Delay(UINT32 t) {
	UINT32 timeout = SDL_GetTicks_New() + t;
	PAL_ProcessEvent();
	while (SDL_GetTicks_New() < timeout)
	{
		if (PalQuit)return;
		PAL_ProcessEvent(); 
		SDL_Delay(1);
	}

}
 
 int CPalEvent::RandomLong(int min, int max) {
	 //static std::mt19937 engine(std::random_device{}());  
	 std::uniform_int_distribution<int> dist(min, max);
	 return dist(generator);
 }

 float CPalEvent::RandomFloat(float from, float to) {
	 std::uniform_real_distribution<double> real_distribution(from, to);
	 return real_distribution(generator);
 }

 //返回指定的点是否在 t 中 0 不在，>0 返回 t中 n+1 的位置
 int CPalEvent::isRectsAtPoint(const std::vector<PAL_Rect>& t, PAL_POS p)
 {
	 if (!p)
		 return 0;
	 for (size_t n = 0; n < t.size(); n++)
	 {
		 if (t[n].isInclude(p))
		 {
			 return static_cast<int>(n) + 1;
		 }
	 }
	 return 0;
 }

VOID CPalEvent::PAL_ProcessEvent()
{
	while (PAL_PollEvent(NULL));
}

//申请内存，参数为申请地址的单位元素长度,元素个数
void* CPalEvent::UTIL_calloc(size_t n, size_t size)
{
	// handy wrapper for operations we always forget, like checking calloc's returned pointer.

	void* buffer;

	// first off, check if buffer size is valid
	if (n == 0 || size == 0)
		TerminateOnError("UTIL_calloc() called Width invalid parameters\n");

	buffer = calloc(n, size); // allocate real memory space

							  // last check, check if malloc call succeeded
	if (buffer == NULL)
	{
		TerminateOnError("UTIL_calloc() failure for %d bytes (out of memory?)\n", size * n);
		return buffer;
	}
	SDL_memset(buffer, 0, size * n);

	return buffer; // nothing went wrong, so return buffer pointer
}

char* CPalEvent::va(const char* format, ...)
/*++
  Purpose:

	Does a varargs printf into a temp buffer, so we don't need to have
	varargs versions of all text functions.

  Parameters:

	format - the format string.

  Return value:

	Pointer to the result string.

--*/
{
	static char string[256];
	va_list     argptr;

	va_start(argptr, format);
	vsnprintf(string, 256, format, argptr);
	va_end(argptr);
	return string;
}

void CPalEvent::printMsg(const char* format, ...)
{
#if _DEBUG
	static char string[512]{};
	va_list     argptr;
	va_start(argptr, format);
	vsnprintf(string, 256, format, argptr);
	va_end(argptr);
	std::printf(string);
#endif
}

VOID CPalEvent::PAL_WaitForKey(WORD wTimeOut)
{
	DWORD     dwTimeOut = SDL_GetTicks_New() + wTimeOut;

	PAL_ClearKeyAll();

	while (wTimeOut == 0 || SDL_GetTicks_New() < dwTimeOut)
	{
		if (PalQuit)return;

		PAL_Delay(1);

		if (getKeyPress() & (kKeySearch | kKeyMenu))
		{
			break;
		}
	}
}

