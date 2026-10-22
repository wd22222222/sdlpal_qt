#include "palsurface.h"
#include "palsurface.h"
#include "palsurface.h"
#include "palsurface.h"
#include "palsurface.h"
#include "palsurface.h"
#include "palsurface.h"
#include "palsurface.h"
#include "palsurface.h"
#include "palsurface.h"
#include "palsurface.h"
#include "palsurface.h"
#include "palsurface.h"
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

#include "palsurface.h"

//构造
PalSurface::PalSurface(int width, int height, int depth, Uint32 rmast, Uint32 gmasr, Uint32 bmast, Uint32 amast) {
    e = SDL_CreateRGBSurface(0, width, height, depth, rmast, gmasr, bmast, amast);
}

//构造8位带索引的表面
PalSurface::PalSurface(SDL_Palette* p, int width, int height)
{
    e = SDL_CreateRGBSurface(0, width, height, 8, 0, 0, 0, 0);
    SDL_SetSurfacePalette(e, p);
    //透明色255
    SDL_FillRect(e, nullptr, 255);
}

// 移动构造函数
PalSurface::PalSurface(PalSurface&& other) noexcept : e(other.e) {
    other.e = nullptr; // 转移所有权，避免双重释放
}

// 移动赋值运算符
PalSurface& PalSurface::operator=(PalSurface&& other) noexcept {
    if (this != &other) {
        SDL_FreeSurface(e); // 释放当前资源
        e = other.e; // 转移资源指针
        other.e = nullptr; // 避免双重释放
    }
    return *this;
}

// 析构函数
PalSurface::~PalSurface() {
    if (e)
        SDL_FreeSurface(e); // 清理资源
    e = nullptr;
}

//重新创建
void PalSurface::creat(int width, int height, int depth, Uint32 rmast, Uint32 gmasr, Uint32 bmast, Uint32 amast)
{
    //如果已经建立，清除内容
    if (e)
    {
        SDL_FreeSurface(e); // 清理资源
    }
    e = SDL_CreateRGBSurface(0, width, height, depth, rmast, gmasr, bmast, amast);
}

//8位带索引的表面
void  PalSurface::creat(SDL_Palette* p, int width, int height)
{
    //如果已经建立，清除内容
    if (e)
    {
        SDL_FreeSurface(e); // 清理资源
    }
    e = SDL_CreateRGBSurface(0, width, height, 8, 0, 0, 0, 0);
    SDL_SetSurfacePalette(e, p);
    //透明色255
    SDL_FillRect(e, nullptr, 255);
}

VOID PalSurface::clear()
{
    if (e)
        SDL_FreeSurface(e); // 清理资源
    e = nullptr;
}

void PalSurface::fill() 
{
    if (get_Surface_bitsPerPixel(e) == 8)
        SDL_FillRect(e, 0, 255);
    else
        SDL_FillRect(e, 0, 0xff000000);
}

//获取调色版
SDL_Palette* PalSurface::getPalette() const
{
    if (e)
#if USING_SDL3
        return SDL_GetSurfacePalette(e);
#else
        return e->format->palette;
#endif
    else
        return nullptr;
}

