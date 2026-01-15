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
//
#include "palsurface.h"

VOID PalSurface::clear() 
{
    if (e)
        SDL_FreeSurface(e); // 清理资源
    e = nullptr;
}

void PalSurface::fill() 
{
    if (get_Surface_bitsPerPixel(e) == 8)
        SDL_FillRect(e, 0, 0);
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

