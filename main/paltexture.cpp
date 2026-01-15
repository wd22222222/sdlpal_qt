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

#include <glad/glad.h>

#include "paltexture.h"
#include "sdl2_compat.h"
#include <cassert>
// 构造函数
PalTexture::PalTexture() 
//建立空纹理
{
}


PalTexture::PalTexture(int aw, int ah, textType at, 
    const SDL_Color* colors)
{
    creat(aw, ah, at,colors);
}

PalTexture::PalTexture(SDL_Surface* surface)
{
    if (surface)
        creat(surface);
}

PalTexture::PalTexture(PalSurface& surface)
{
    creat(surface.get());
}

//移动构造函数
PalTexture::PalTexture(PalTexture&& other) noexcept {
    textureID = other.textureID; other.textureID = 0;
    w = other.w;
    h = other.h;
    t = other.t;
    genFBOID = other.genFBOID; other.genFBOID = 0;
#if USE_PBO
    genPBOID = other.genPBOID; other.genPBOID = 0;
#endif
    BitsPerPixel = other.BitsPerPixel;
    paletteID = other.paletteID; other.paletteID = 0;
}

//移动构造函数
PalTexture& PalTexture::operator=(PalTexture&& other) noexcept
{
    clear();
    textureID = other.textureID; other.textureID = 0;
    w = other.w;
    h = other.h;
    t = other.t;
    genFBOID = other.genFBOID; other.genFBOID = 0;
#if USE_PBO
    genPBOID = other.genPBOID; other.genPBOID = 0;
#endif
    BitsPerPixel = other.BitsPerPixel;
    paletteID = other.paletteID; other.paletteID = 0;
    return *this;
}

PalTexture::~PalTexture() {
    clear();
}

PalErr PalTexture::creat(int aw, int ah, textType at,
    const SDL_Color* colors)
{
    w = aw;
    h = ah;
    t = at;
    if (textureID == 0)
        glGenTextures(1, &textureID);
#if USE_PBO
    if (genPBOID == 0)
        glGenBuffers(1, &genPBOID);
#endif
    int err = glGetError();
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, textureID);
    err = glGetError() || err;

    int m_Format = GL_RGBA;
    int m_Type = GL_UNSIGNED_BYTE;
    int m_internalformat = GL_RGBA8;

    switch (getBitsPerPixel())
    {
    case  8:
        m_Format = GL_RED;
        m_Type = GL_UNSIGNED_BYTE;
        m_internalformat = GL_R8;
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

        BitsPerPixel = 8;
        break;
    case 16:
    case 15:
        m_Format = GL_RGBA;
        m_Type = GL_UNSIGNED_SHORT_5_5_5_1;
        m_internalformat = GL_RGB5_A1;
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        BitsPerPixel = 16;
        break;
    case 24:
        m_Format = GL_RGB;
        m_Type = GL_UNSIGNED_BYTE;
        m_internalformat = GL_RGB8;

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

        BitsPerPixel = 24;
        break;

    case 32:
        m_Format = GL_RGBA;
        m_Type = GL_UNSIGNED_BYTE;
        m_internalformat = GL_RGBA8;

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

        BitsPerPixel = 32;
        break;
    default:
        assert(false);//出错
    }

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    if (at == textType::red8 && colors)
    {
        //将调色版转化为调色一维纹理
        err = creatPaletteID(colors);
    };
    glEnable(GL_TEXTURE_2D);
    //4字节对齐
    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
    glTexImage2D(GL_TEXTURE_2D,
        0, 
        m_internalformat,
        w, h, 0, 
        m_Format,
        m_Type, 
        NULL);
    err = glGetError() || err;
    err = glGetError() || err;
    glBindTexture(GL_TEXTURE_2D, 0);
    glDisable(GL_TEXTURE_2D);
    err = glGetError() || err;
    return PalErr(err);
}


PalErr PalTexture::creat(PalSurface& surface)
{
	return creat(surface.get());
}

PalErr PalTexture::creat(SDL_Surface* sSurf)
{
    //根据表面新建纹理
    int bits = get_Surface_bitsPerPixel(sSurf);
    if (bits != 8 &&
        bits != 32 && bits != 16 && bits !=24)
    {
        //特殊格式表面
        SDL_Surface* m_surf = SDL_ConvertSurfaceFormat(sSurf, SDL_PIXELFORMAT_ABGR8888, 0);
        SDL_Rect dst_rect = { 0,0,m_surf->w,m_surf->h };
        UpdateTexture(&dst_rect, m_surf->pixels, m_surf->pitch);
        SDL_FreeSurface(m_surf);
        return PalErr(0);
    }

    //if (sSurf->format->palette)
    if (SDL_GetSurfacePalette(sSurf) != nullptr)
    {

        //creat(sSurf->w, sSurf->h, textType::red8,
            //sSurf->format->palette->colors);
        creat(sSurf->w, sSurf->h, textType::red8,
            SDL_GetSurfacePalette(sSurf)->colors);
    }
    else
        //creat(sSurf->w, sSurf->h, getTextType(sSurf->format->BytesPerPixel));
        creat(sSurf->w, sSurf->h, getTextType(get_Surface_bitsPerPixel(sSurf) >> 3));

    UpdateTexture(NULL, sSurf->pixels, sSurf->pitch);
    return PalErr(0);
}


PalErr PalTexture::UpdateTexture(const SDL_Rect* rRect, const void* pixels, INT pitch)
{
    int err{};
    // = SDL_UpdateTexture;
    SDL_Rect sRect = { 0,0,w,h };
    if (rRect == NULL)
        rRect = &sRect;
    w = rRect->w;
    h = rRect->h;
    glEnable(GL_TEXTURE_2D);
    //绑定纹理
    if (isNull())//原纹理为空
    {
        textType type = getTextType((pitch / w));
        creat(w, h, type);//新建纹理
    }
    glBindTexture(GL_TEXTURE_2D, textureID);
#if USE_PBO
    //绑定PBO
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, genPBOID);
    err += glGetError();
    //申请一个空间操作
    //分配空间,载入数据
    glBufferData(GL_PIXEL_UNPACK_BUFFER,
        rRect->h * pitch,
        pixels, GL_STREAM_DRAW);
    err += glGetError();
#endif
    w = rRect->w;
    h = rRect->h;
    BitsPerPixel = pitch / w << 3;
    int m_format;
    int m_type;
    err += glGetError();
    switch (BitsPerPixel)
    {
    case 8:      
        m_type = GL_UNSIGNED_BYTE;
        m_format = GL_RED;
        break;
    case 16:
    case 15:
        m_type = GL_UNSIGNED_SHORT_1_5_5_5_REV;
        //m_type = GL_UNSIGNED_SHORT_5_5_5_1;
        m_format = GL_BGRA;
        break;
    case 24:
        m_type = GL_UNSIGNED_BYTE;
        m_format = GL_RGB;
        break;
    case 32:
        m_type = GL_UNSIGNED_BYTE;
        m_format = GL_RGBA;
        break;
    default:
        return PalErr(1);
    }
    t = getTextType(BitsPerPixel >> 3);
    err += glGetError();
#if USE_PBO
    glTexSubImage2D(GL_TEXTURE_2D, 0, rRect->x, rRect->y, rRect->w,
        rRect->h, m_format, m_type, NULL);//最后一个参数为空，申请空间
#else
    glTexSubImage2D(GL_TEXTURE_2D, 0, rRect->x, rRect->y, rRect->w,
        rRect->h, m_format, m_type, pixels);//最后一个参数为数据
#endif
    err += glGetError();//报错点
    glGenerateMipmap(GL_TEXTURE_2D);
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);//恢复之间的缺省绑定
    glBindTexture(GL_TEXTURE_2D, 0);//撤消绑定
    glDisable(GL_TEXTURE_2D);
    err += glGetError();

    return PalErr(err);
}


//检测纹理是否为空
bool PalTexture::isEmpty() const {
    if (!textureID)
        return true;
#if USE_PBO
    if (!genPBOID)
        return true;
#endif
    return false;
}

PalErr PalTexture::setAsRenderTarget()
{
    if (textureID == 0)
        return PalErr(1);//纹理无效
	if (t != textType::rgba32)
		return PalErr(2);//仅支持32位纹理作为渲染目标
    if (genFBOID == 0)
    {
        glGenFramebuffers(1, &genFBOID);
    }
    glBindFramebuffer(GL_FRAMEBUFFER, genFBOID);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureID, 0);
    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        return PalErr(4);//设置渲染目标失败
    }
    //glBindFramebuffer(GL_FRAMEBUFFER, 0);
    return PalErr(0);//成功
}

PalErr PalTexture::setTextureAsRenderTarget(PalTexture* t)
{
    if(t->textureID == 0)
		return PalErr(1);//纹理无效
	return t->setAsRenderTarget();
}

void PalTexture::unsetAsRenderTarget()
{
	//将渲染目标设置回默认帧缓冲区
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

// 清理纹理资源
void PalTexture::clear() { 
    if (textureID)
    {
        glDeleteTextures(1, &textureID);
#if USE_PBO
        glDeleteBuffers(1, &genPBOID);
        genPBOID = 0;
#endif
    }
    if (genFBOID)
    {
        glDeleteFramebuffers(1, &genFBOID);
    }
    if (paletteID )
        glDeleteTextures(1, &paletteID);
    textureID =  genFBOID = paletteID = 0;
}

//将调色版缓存，转化为一维纹理 id  输入调色版缓存 返回错误码
PalErr PalTexture::creatPaletteID(const SDL_Color * colors)
{
    int err{};
    if (paletteID == 0)
        glGenTextures(1, &paletteID);
    glBindTexture(GL_TEXTURE_1D, paletteID);
    glTexImage1D(GL_TEXTURE_1D, 0, GL_RGBA8, 256, 0,
        GL_RGBA, GL_UNSIGNED_BYTE, colors);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    err = glGetError();
    return PalErr(err);
}


//将纹理填充为单一颜色
PalErr PalTexture::fillRect(const SDL_Color* color, const PAL_Rect* pRect)
{
    if (!isVilid())
        return PalErr(1);//纹理无效 
    setAsRenderTarget();
    if (pRect)
        glViewport(pRect->x, pRect->y, pRect->w, pRect->h);
    else
        glClear(GL_COLOR_BUFFER_BIT);
    if (color)
        glClearColor(color->r * Div255, color->g * Div255, color->b * Div255, color->a * Div255);
    else
    {
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    }
    unsetAsRenderTarget();
    return PalErr(0);
}


PalErr PalTexture::fill(const PAL_fColor& color)
{
    if(!isVilid())
		return PalErr(1);//纹理无效 
    setAsRenderTarget();
    glClearColor(color.r , color.g,color.b , color.a );
	glClear(GL_COLOR_BUFFER_BIT);
	unsetAsRenderTarget();
    return PalErr(0);
}


//设置透明色
PalErr PalTexture::setTransparent_Color(const PAL_Color& color)
{

        transparent_color = color;
        return PalErr(0);
}
//获取透明色
PAL_Color PalTexture::getTransparent_Color()
{
    return transparent_color;
}
//取纹理像素数据
ByteArray PalTexture::getData()
{
    ByteArray buf;
    int bytes{};
    int m_Format = GL_RGBA;
    int m_Type = GL_UNSIGNED_BYTE;
    int m_internalformat = GL_RGBA8;
    int BitsPerPixel{};
    //设定纹理对齐方式，缺省为4 字节对齐
    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
    int err = 0;
    switch (t)
    {
    case  textType::red8:
        m_Format = GL_RED;
        m_Type = GL_UNSIGNED_BYTE;
        m_internalformat = GL_R8;
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

        BitsPerPixel = 1;
        break;
    case textType::rgb16:
        m_Format = GL_RGBA;
        m_Type = GL_UNSIGNED_SHORT_5_5_5_1;
        m_internalformat = GL_RGB5_A1;
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        BitsPerPixel = 2;
        break;
    case textType::rgb24:
        m_Format = GL_RGB;
        m_Type = GL_UNSIGNED_BYTE;
        m_internalformat = GL_RGB8;
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        BitsPerPixel = 3;
        break;
    case textType::rgba32:
        m_Format = GL_RGBA;
        m_Type = GL_UNSIGNED_BYTE;
        m_internalformat = GL_RGBA8;
        BitsPerPixel = 4;
        break;
    }
    buf.resize(w * h * BitsPerPixel + 4);
    glBindTexture(GL_TEXTURE_2D, textureID);
	glGetTexImage(GL_TEXTURE_2D, 0, m_Format, m_Type, buf.data());
    //清理
    glBindTexture(GL_TEXTURE_2D, 0);
    return buf;
}

PalErr PalTexture::upSubData(const PAL_Rect* rect, void* data)
{
    return PalErr(0);
}



//将纹理设置为渲染目标
PalErr PalTexture::setToRendererTarget()
{
	if (t != textType::rgba32)
		return PalErr(2);//仅支持32位纹理作为渲染目标
    if (genFBOID == 0)
    {
        glGenFramebuffers(1, &genFBOID);
	}
	glBindFramebuffer(GL_FRAMEBUFFER, genFBOID);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureID, 0);
	GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (status != GL_FRAMEBUFFER_COMPLETE)
	{   
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glDeleteFramebuffers(1, &genFBOID);
        genFBOID = 0;
		return PalErr(4);//设置渲染目标失败
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	return 0;;//成功
}

//取每像素位数
int PalTexture::getBitsPerPixel()
{
    switch (t)
    {
    case textType::red8:
        return 8;
    case textType::rgb16:
        return 16;
    case textType::rgb24:
        return 24;
    case textType::rgba32:
        return 32;
    default:
        return 0;
    }
}

//检测纹理是否有效
bool PalTexture::isVilid()const
{
    if (textureID == 0)
        return false;
    if (t == textType::noType)
        return false;
#if USE_PBO
    if (genPBOID == 0)
        return false;
#endif
    if (w == 0 || h == 0)
        return false;
    return true;
}

//检测纹理是否有效
bool PalTexture::isVilid(const PalTexture& t)
{
    if (t.textureID == 0)
        return false;
    if (t.t == textType::noType)
        return false;
#if USE_PBO
    if (t.genPBOID == 0)
        return false;
#endif
	if (t.w == 0 || t.h == 0)
        return false;
    return true;
}

void PalTexture::clearAll()
{
}


