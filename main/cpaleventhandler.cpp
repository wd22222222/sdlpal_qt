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

#include "cpaleventhandler.h"

CPalEventHandler::CPalEventHandler() {}

CPalEventHandler::~CPalEventHandler() {}

void CPalEventHandler::handleEvents(SDL_Event* event)
{
	auto it = callbacks.find(event->type);
	if (it != callbacks.end()) {
		CPalEventHandler::EventCallback c = it->second;
		c(event);
	}
}

void CPalEventHandler::eventClear()
{
	SDL_Event evt{};
	while( SDL_PollEvent(&evt));
}

void CPalEventHandler::registCallback(UINT eventType, EventCallback callback)
{
	callbacks[eventType] = callback;
}

#ifdef USE_EVENTHANDLER
//取得自定义事件句柄
Uint32 CPalEventHandler::getCustomEventHandler(eventTypeCustom n)
{
	//注册自定义事件
	if (!customEventType.size())
		//第一次进入，生成列表
		for (UINT32 s = 0; s < (UINT32)eventTypeCustom::eventTypeCustomEnd; s++)
		{
			UINT32 type = registerCustomEventType();
			customEventType.push_back(type);
		}
	return customEventType[(UINT32)n];
}
#endif

int CPalEventHandler::PushEvent(SDL_Event* lpevent)
{ 
	return  SDL_PushEvent(lpevent);
}

// 注册自定义事件类型
Uint32 CPalEventHandler::registerCustomEventType() {

	return SDL_RegisterEvents(1);
}

//取得锁
BOOL CPalEventHandler::getCustomMutex(UINT eventType) {
	auto it = customMutex.find(eventType);
	if (it != customMutex.end()) {
		return it->second;
	}
	return FALSE;

}

//设置锁
void CPalEventHandler::setCustomMutex(UINT eventType, BOOL v)
{
	customMutex[eventType] = v;
}


//

VOID CPalEventHandler::PAL_ProcessEvent()
{
	while (PAL_PollEvent(NULL));
}

INT CPalEventHandler::PAL_PollEvent(SDL_Event* event)
{
	SDL_Event evt{};
	int ret = SDL_PollEvent(&evt);
	if (ret != 0)
	{
		handleEvents(&evt);
	}
	if (event != NULL)
	{
		*event = evt;
	}
	return ret;
}

