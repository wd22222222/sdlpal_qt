///* -*- mode: c; tab-width: 4; c-basic-offset: 4; c-file-style: "linux" -*- */
//
// Copyright (c) 2021-2026, Wu Dong.
// 
// All rights reserved.
//
// This file is part of Wudong.
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
#include "cscreen.h"
#include "CConfig.h"
#include "command.h"
#include "Convers.h"
#include "cpaldata.h"
#include "cpalevent.h"
#include "cpaleventhandler.h"
#include "pal_color.h"
#include "palgpgl.h"
#include "palsurface.h"
#include "paltexture.h"
#include <cstdint>
#include <cstdio>
#include <iostream>
#include <string>
#include <cstddef>


// 顶点着色器 (Vertex Shader)
// Vertex Shader
const static GLchar* const vertexShader = R"(#version 330
in vec4 intoposition;          // 输入顶点：xy=位置, zw=纹理坐标 | Input vertex: xy=position, zw=texCoord
uniform vec4 v_data[20];       // 统一变量数组，用于传递各种渲染参数 | Uniform array for rendering parameters
out vec2 sTexCoord;            // 输出纹理坐标到片元着色器 | Output texCoord to fragment shader

void main()
{
    vec2 position = intoposition.xy;
    vec2 TexCoord = intoposition.zw;
    vec4 zoom = v_data[4];     // 缩放参数 (x,y,z,w) | Zoom parameters
    vec4 roll = v_data[5];     // 旋转参数 (x=绕X轴, y=绕Y轴, z=绕Z轴, w=是否启用) | Rotation params (x/y/z angles, w=enable flag)

    gl_Position = vec4(position, 0.0, 1.0);
    sTexCoord = TexCoord;

    // 如果启用了缩放 (zoom.w > 0.0)
    // If zoom is enabled (zoom.w > 0.0)
    if(zoom.w > 0.0)
    {
        mat4 m_zoom = mat4(1.0);
        m_zoom[0][0] = zoom.x; // X轴缩放 | X scale
        m_zoom[1][1] = zoom.y; // Y轴缩放 | Y scale
        m_zoom[2][2] = zoom.z; // Z轴缩放 | Z scale
        m_zoom[3][3] = zoom.w; // 齐次坐标缩放（通常为1）| Homogeneous scale (usually 1)
        gl_Position = m_zoom * gl_Position;
    }

    // 如果启用了旋转 (roll.w > 1.0)
    // If rotation is enabled (roll.w > 1.0)
    if(roll.w > 1.0)
    {
        float s, c;
        mat4 m_roll = mat4(1.0);

        // 绕 X 轴旋转 | Rotate around X-axis
        m_roll[1][1] = m_roll[2][2] = cos(roll.x);
        m_roll[1][2] = sin(roll.x);
        m_roll[2][1] = -sin(roll.x);
        gl_Position = m_roll * gl_Position;

        // 绕 Y 轴旋转 | Rotate around Y-axis
        m_roll = mat4(1.0);
        m_roll[0][0] = m_roll[2][2] = cos(roll.y);
        m_roll[0][2] = sin(roll.y);
        m_roll[2][0] = -sin(roll.y);
        gl_Position = m_roll * gl_Position;

        // 绕 Z 轴旋转 | Rotate around Z-axis
        m_roll = mat4(1.0);
        m_roll[0][0] = m_roll[1][1] = cos(roll.z);
        m_roll[0][1] = sin(roll.z);
        m_roll[1][0] = -sin(roll.z);
        gl_Position = m_roll * gl_Position;
    }
})";

//片元
//mode == 0.0 混合 v_mode.y 结果的alpha值
//mode == 1.0 拷贝后乘颜色，乘v_mode.y
//mode == 2.0 置成单一颜色
//mode == 3.0 过滤颜色拷贝，乘混合因子
//mode == 4.0 与单一颜色混合，color颜色
//mode == 6.0 执行字模拷贝，用取得的纹理颜色值乘颜色
//mode == 7.0)//去除阴影
//mode == 8.0)//将图像转为黑白，后乘以alpha 再加上颜色
//mode == 9.0//将纹理初始到黑色透明
//mode = 10.0 执行屏幕翻转显示
// 输入 v_data 0颜色，1.x 模式，1.y alpha 值，
// 2 波动 2.x 2,y x 轴和 y轴 w 幅度 z 时间，
// 3 水波 x,y 中心 w 宽度 z 时间
// 20 起 调色版
// 片元着色器 (Fragment Shader)
// Fragment Shader
const static GLchar* fragmentShader = R"(#version 330
uniform sampler2D v_tex;           // 主纹理（可能是8位索引图或32位RGBA）| Main texture (8-bit indexed or 32-bit RGBA)
uniform sampler1D v_paletteID;     // 1D纹理作为调色板（用于8位图）| 1D texture as palette (for 8-bit images)
uniform vec4 v_data[20];
in vec2 sTexCoord;
out vec4 outColor;

void main()
{
    vec4 v_color = v_data[0];      // 混合/填充用的颜色 | Color for blending/fill
    vec4 v_mode = v_data[1];       // x=渲染模式, y=Alpha值, z=是否8位图, w=是否有调色板
                                   // x=render mode, y=alpha, z=1 if 8-bit, w=1 if palette used
    vec4 v_mave = v_data[2];       // 波浪特效参数 | Wave effect parameters
    vec4 v_ripple = v_data[3];     // 水波纹特效参数 | Ripple effect parameters
    vec4 v_transColor = v_data[6]; // 透明色（用于过滤）| Transparent color (for filtering)

    float dx = 0.0, dy = 0.0;
    float pi = 3.1415926;

    // 波浪特效：沿X/Y方向正弦扰动
    // Wave effect: sinusoidal distortion along X/Y
    if(v_mave.x !=0 || v_mave.y !=0)
    {
        float mavex = mod(sTexCoord.y * pi * v_mave.x, pi * 100.0);
        if(v_mave.x != 0.0) dx = sin(mavex + v_mave.w) * v_mave.z;
        float mavey = mod(sTexCoord.x * pi * v_mave.y, pi * 100.0);
        if(v_mave.y != 0.0) dy = sin(mavey + v_mave.w) * v_mave.z;
    }

    // 水波纹特效：以某点为中心的环形扰动
    // Ripple effect: circular distortion from a center point
    if(v_ripple.z != 0.0 && v_ripple.w != 0.0)
    {
        float distance = distance(sTexCoord, vec2(v_ripple.x, v_ripple.y));
        if ((v_ripple.w - v_ripple.z) > 0.0 &&
            (distance <= (v_ripple.w + v_ripple.z)) &&
            (distance >= (v_ripple.w - v_ripple.z)))
        {
            float x = (distance - v_ripple.w);
            // 使用平滑函数计算位移量 | Use smooth function to compute displacement
            float moveDis = 30.0 * x * (x - 0.12) * (x + 0.12);
            vec2 unitDirectionVec = normalize(sTexCoord - vec2(v_ripple.x, v_ripple.y));
            dx = unitDirectionVec.x * moveDis;
            dy = unitDirectionVec.y * moveDis;
        }
    }

    vec2 newcoord = sTexCoord + vec2(dx, dy); // 应用扰动后的纹理坐标 | Apply distorted tex coord
    vec4 t0;

    // 如果是8位图且有调色板，则通过1D纹理查找真实颜色
    // If 8-bit image with palette, look up real color via 1D texture
    if(v_mode.w == 1.0 && v_mode.z == 1.0)
    {
        float index = texture(v_tex, newcoord).r * 255.0;
        float paletteCoord = index / 256.0;
        t0 = texture(v_paletteID, paletteCoord);
    }
    else
    {
        t0 = texture(v_tex, newcoord);
    }

    int x = int(v_mode.x + 0.001); // 渲染模式 | Render mode

    if(x == 0) // 透明混合：保留原Alpha，但用v_mode.y覆盖 | Transparent blend: use v_mode.y as alpha
    {
        outColor = t0;
        outColor.a = v_mode.y;
    }
    else if(x == 1) // 拷贝并乘以Alpha，结果不透明 | Copy and multiply by alpha, output opaque
    {
        outColor = t0 * v_mode.y;
        outColor.a = 1.0;
    }
    else if(x == 2) // 单一颜色填充 | Solid color fill
    {
        outColor = v_color * v_mode.y;
    }
    else if(x == 3) // 过滤透明色：若等于v_transColor则丢弃 | Filter transparent color: discard if match
    {
        if(t0 == v_transColor)
        {
            discard;
            return;
        }
        outColor = t0 * v_mode.y;
        outColor.a = 1.0;
    }
    else if(x == 4) // 与单一颜色混合（线性插值）| Mix with solid color (lerp)
    {
        outColor = mix(t0, v_color, v_mode.y);
        outColor.a = 1.0;
    }
    else if(x == 6) // 字模渲染：仅非背景像素绘制指定颜色 | Font glyph: draw color only on non-background pixels
    {
        if(t0.r < 0.5) // 假设背景为黑色/低亮度 | Assume background is dark
        {
            discard;
        }
        outColor = v_color;
        outColor.a = 1.0;
    }
    else if(x == 7) // 去除数字阴影（仅保留亮部）| Remove shadow from digits (keep only bright parts)
    {
        if(t0.r + t0.g + t0.b < 1.0)
        {
            discard;
            return;
        }
        outColor = t0;
        outColor.a = 1.0;
    }
    else if(x == 8) // 转黑白 + 加颜色 + 乘Alpha | Convert to grayscale, add color, then apply alpha
    {
        if(t0 == v_transColor)
        {
            discard;
            return;
        }
        float gray = (t0.r + t0.g + t0.b) / 3.0;
        outColor = vec4(gray, gray, gray, 1.0) + v_color;
        outColor *= v_mode.y;
    }
    else if(x == 9) // 清屏为黑（不透明）| Clear to black (opaque)
    {
        outColor = vec4(0.0, 0.0, 0.0, 1.0);
    }
    else if(x == 10) // 屏幕垂直翻转 | Vertical flip (e.g., for correct OpenGL orientation)
    {
        outColor = texture(v_tex, vec2(sTexCoord.x, 1.0 - sTexCoord.y));
        outColor *= v_mode.y;
        outColor.a = 1.0;
    }
    else // 默认拷贝 | Default: direct copy
    {
        outColor = t0;
    }
})";

CScreen::CScreen() 
{
} 

CScreen::~CScreen()
{
    VideoShutDown();
}


void CScreen::VIDEO_ShakeScreen(WORD wShakeTime, WORD wShakeLevel)
{
    g_wShakeLevel = wShakeLevel;
    g_wShakeTime = wShakeTime;
}

VOID CScreen::VIDEO_FadeScreen(WORD  wSpeed)
{
    wSpeed++;
    DWORD time = SDL_GetTicks_New();
    RenderBlendCopy(gpRenderTexture, gpTextureRealBak);
    RenderPresent(gpRenderTexture);

    for (int i = 0; i < 64;i++)
    {
        if (g_wShakeTime)
        {
            if (g_wShakeTime & 1)
            {
                //setZoom(1.0 + g_wShakeLevel * 0.0075, 1.0 + g_wShakeLevel * 0.012, 1.0, 1.0);
                setZoom(1.0 , 1.0 , 1.0, 1.0 + g_wShakeLevel * 0.0075);
            }
            g_wShakeTime--;
        }
        RenderBlendCopy(gpRenderTexture, gpTextureRealBak);
        RenderBlendCopy(gpRenderTexture, gpTextureReal, i * 3 + 63, RenderMode::rmMode0);

        setZoom(0, 0, 0, 0);

        RenderPresent(gpRenderTexture);
        PAL_ProcessEvent();
        while (SDL_GetTicks_New() <= time)
        {
            PAL_ProcessEvent();
            PAL_Delay(2);
        }
        time += wSpeed * 6;
    }
    RenderBlendCopy(gpRenderTexture, gpTextureReal);
    RenderPresent(gpRenderTexture);
    time = SDL_GetTicks_New() - time;
    SDL_Delay(1);
}


VOID CScreen::VIDEO_BackupScreen(SDL_Surface * s)
{
    RenderBlendCopy(gpTextureRealBak, s);
}

VOID CScreen::VIDEO_BackupScreen(PalTexture& s)
{
    auto& ts = s.isNull() ? gpTextureReal : s;
    RenderBlendCopy(gpTextureRealBak, ts);
}

VOID CScreen::VIDEO_RestoreScreen()
{
    RenderBlendCopy(gpTextureReal, gpTextureRealBak);
}

VOID CScreen::VIDEO_UpdateScreen(PalTexture* dstText, 
    const SDL_Rect* lpSrcRect, const SDL_Rect* lpDstRect)
{
    return VIDEO_UpdateScreen(*dstText, lpSrcRect, lpDstRect);
}

VOID CScreen::VIDEO_UpdateSurfacePalette(SDL_Surface* pSurface)
/*++
  Purpose:

    Use the global palette to update the palette of pSurface.

  Parameters:

    [IN]  pSurface - the surface whose palette should be updated.

  Return value:

    None.

--*/
{
    SDL_SetSurfacePalette(pSurface, gpPalette);
}

#define fabs(a) ((a) > 0 ? (a):(-(a)) )

VOID CScreen::VIDEO_UpdateScreen(PalTexture& lpDstText, const SDL_Rect* lpSrcRect,const SDL_Rect* lpDstRect)
{
    //if (lpDstText.isVilid() && lpDstText.getID() != gpRenderTexture.getID())
        //RenderBlendCopy(gpRenderTexture,lpDstText,getAlpha(), RenderMode::rmMode1, getpColor(),lpSrcRect,lpDstRect);
    if (g_wShakeTime)
    {
        if (g_wShakeTime & 1)
        {
            setZoom(1.0 + g_wShakeLevel * 0.0075, 1.0 + g_wShakeLevel * 0.012, 0, 1);
        }
        g_wShakeTime--;
    }
    RenderBlendCopy(gpRenderTexture, lpDstText, getAlpha(), RenderMode::rmMode1, getpColor(), lpSrcRect, lpDstRect);
    setZoom(0, 0, 0, 0);
    RenderPresent(gpRenderTexture);
}

/*++
  Purpose:

    Update the screen area specified by lpRect.

  Parameters:

    [IN]  lpRect - Screen area to update.

  Return value:

    None.

--*/



VOID CScreen::PAL_FadeIn(
    INT         iPaletteNum,
    BOOL        fNight,
    INT         iDelay
)
/*++
  Purpose:

    Fade in the screen to the specified palette.

  Parameters:

    [IN]  iPaletteNum - number of the palette.

    [IN]  fNight - whether use the night palette or not.

    [IN]  iDelay - delay time for each step.

  Return value:

    None.

--*/
{
    VIDEO_SetPalette(PAL_GetPalette(iPaletteNum, fNight));

    if (iDelay <= 0) iDelay = 1;
    setAlpha(0);
    VIDEO_UpdateScreen(gpTextureReal);
    double v_time = (double)SDL_GetTicks_New() + (INT)iDelay * 500;
    while (v_time > SDL_GetTicks_New())
    {
        setAlpha(255 - (v_time - SDL_GetTicks_New()) * 255.0 / 500 / iDelay);
        VIDEO_UpdateScreen(gpTextureReal);
        PAL_Delay(1);
    }
    setAlpha(255);
    VIDEO_UpdateScreen(gpTextureReal);
}

VOID CScreen::PAL_FadeOut(
    INT         iDelay
)
/*++
  Purpose:

    Fadeout screen to black from the specified palette.

  Parameters:

    [IN]  iPaletteNum - number of the palette.

    [IN]  fNight - whether use the night palette or not.

    [IN]  iDelay - delay time for each step.

  Return value:

    None.

--*/
{
    if (iDelay <= 0) iDelay = 1;

    setAlpha(255);
    VIDEO_UpdateScreen(gpTextureReal);
    for(int n=0;n<16;n++)
    {
        setAlpha(255 - 12 * n);
        VIDEO_UpdateScreen(gpTextureReal);
        PAL_Delay(iDelay * 15);
    }
    //
    PalTexture paltext(RealWidth, RealHeight, textType::rgba32);
    RenderBlendCopy(gpTextureReal , paltext, 0, RenderMode::rmMode2);
    VIDEO_UpdateScreen(gpTextureReal);

    PAL_LARGE SDL_Color      newpalette[256];
    memset(newpalette, 0, sizeof(newpalette));
    VIDEO_SetPalette(newpalette);
    setAlpha(255); 
}

VOID CScreen::PAL_FadeOut(PalTexture &dstText, INT iDelay)
{
    PalTexture &rpText = (dstText.isEmpty()) ? gpRenderTexture : dstText;
    PalTexture tNull = PalTexture();
    for (int n = 0; n < 16; n++)
    {
        RenderBlendCopy(tNull, rpText, 255 - 12 * n, RenderMode::rmMode10);
        SDL_GL_SwapWindow(gpWindow);
        PAL_Delay(iDelay * 15);
    }
}

VOID CScreen::PAL_SetPalette(
    INT         iPaletteNum,
    BOOL        fNight
)
/*++
  Purpose:

    Set the screen palette to the specified one.

  Parameters:

    [IN]  iPaletteNum - number of the palette.

    [IN]  fNight - whether use the night palette or not.

  Return value:

    None.

--*/
{
    SDL_Color* p = PAL_GetPalette(iPaletteNum, fNight);

    if (p != NULL)
    {
        VIDEO_SetPalette(p);
    }
}
     
Pal_Size CScreen::PAL_DrawText(const std::string& lpszText, PAL_POS pos, BYTE bColor, BOOL fShadow, BOOL fUpdate, int size)
{
    //已经将输入改为UTF8格式
    auto mSize = PAL_DrawTextUTF8(lpszText, pos, bColor, fShadow, fUpdate, size);
    if (fUpdate)
    {
        RenderPresent(gpTextureReal);
    }
    return mSize;
}

Pal_Size CScreen::PAL_DrawWideText(const std::wstring& lpszTextR, PAL_POS pos, 
    BYTE bColor, BOOL fShadow, BOOL fUpdate, int size)
{
    //功能：在屏幕上打印宽字符串
    //返回：占用空间尺寸
    if (size < 2)
        size = 16;
    if (lpszTextR.empty())
        return { 0,0 };
    SDL_Color color = gpPalette->colors[bColor];
    gpTextureReal.setAsRenderTarget();
    glViewport(0, 0, gpTextureReal.size().cx(), gpTextureReal.size().cy());
    
    CopyFontToTexture(gpTextureReal, lpszTextR, &color, pos, 1.0);
   
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    if (fUpdate)
    {
        RenderPresent(gpTextureReal);
	}
	//计算尺寸
    int width = 0;
    for (const auto& ch : lpszTextR)
    {
        auto it = characters.find(ch);
        if (it != characters.end())
        {
            width += it->second.advance;
        }
    }
    return PalSize(width / PictureRatio, size);
}

Pal_Size CScreen::PAL_DrawTextUTF8(LPCSTR s, PAL_POS pos, BYTE bColor, BOOL fShadow, BOOL fUpdate, int size)
{
    return PAL_DrawTextUTF8((const std::string&)std::string(s), pos, bColor, fShadow, fUpdate, size);
}

Pal_Size CScreen::PAL_DrawTextUTF8(const std::string& lpszText,
    PAL_POS pos, BYTE bColor, BOOL fShadow, BOOL fUpdate, int size)
{
    if (lpszText.empty())
        return PalSize();
    Cls_Iconv m;
    std::wstring s = m.UTF8toWCHAR(lpszText);
	return PAL_DrawWideText(s, pos, bColor, fShadow, fUpdate, size);
}


VOID CScreen::VIDEO_SetPalette(SDL_Color rgPalette[256])
/*++
  Purpose:

    Set the palette of the screen.

  Parameters:

    [IN]  rgPalette - array of 256 colors.

  Return value:

    None.

--*/
{
    SDL_SetPaletteColors(gpPalette, rgPalette, 0, 256);
}

SDL_Color* CScreen::PAL_GetPalette(
    INT         iPaletteNum,
    BOOL        fNight
)
/*++
  Purpose:

    Get the specified palette in pat.mkf file.

  Parameters:

    [IN]  iPaletteNum - number of the palette.

    [IN]  fNight - whether use the night palette or not.

  Return value:

    Pointer to the palette. NULL if failed.

--*/
{
    static SDL_Color      palette[256]{0};
    PAL_LARGE BYTE        buf[1536]{0};
    INT                   i;
    FILE* fp;

    std::string path = PalDir + "pat.mkf";
    fp = fopen(path.c_str(),"rb");
    SDL_zero(buf);
    //
    // Read the palette data from the pat.mkf file
    //
    UINT     uiOffset = 0;
    UINT     uiNextOffset = 0;

    //i = PAL_MKFReadChunk(buf, 1536, iPaletteNum, fp);
    fseek(fp, 4 * iPaletteNum, SEEK_SET);
    fread(&uiOffset, 4, 1, fp);
    fread(&uiNextOffset, 4, 1, fp);
    uiOffset = SWAP32(uiOffset);
    uiNextOffset = SWAP32(uiNextOffset);
    i = uiNextOffset - uiOffset;
    
    if (i > 0)
    {
        fseek(fp, uiOffset, SEEK_SET);
        fread(buf, i, 1, fp);
    }
    fclose(fp);

    if (i < 0)
    {
        //
        // Read failed
        //
        return NULL;
    }
    else if (i <= 256 * 3)
    {
        //
        // There is no night colors in the palette
        //
        fNight = FALSE;
    }

    for (i = 0; i < 256; i++)
    {
        palette[i].r = buf[(fNight ? 256 * 3 : 0) + i * 3] << 2;
        palette[i].g = buf[(fNight ? 256 * 3 : 0) + i * 3 + 1] << 2;
        palette[i].b = buf[(fNight ? 256 * 3 : 0) + i * 3 + 2] << 2;
        palette[i].a = 255;
        if ( i && ggConfig->m_Function_Set[51] == 1)
        {
            palette[i].r += (255 - palette[i].r) / 16;
            palette[i].g += (255 - palette[i].g) / 16;
            palette[i].b += (255 - palette[i].b) / 16;
        }
    }

    return palette;
}

VOID CScreen::PAL_DrawNumber(
    UINT            iNum,
    UINT            nLength,
    PAL_POS         pos,
    NUMCOLOR        color,
    NUMALIGN        align,
    int iSize
)
/*++
  Purpose:

    Draw the specified number with the bitmaps in the UI sprite.

  Parameters:

    [IN]  iNum - the number to be drawn.

    [IN]  nLength - max. length of the number.

    [IN]  pos - position on the screen.

    [IN]  color - color of the number (yellow or blue).

    [IN]  align - align mode of the number.

  Return value:

    None.

--*/
{
    UINT          nActualLength, i;
    int           x, y;
    LPCBITMAPRLE  rglpBitmap[10];

    auto& gpSpriteUI = gpGlobals->gpSpriteUI;

    //
    // Get the bitmaps. Blue starts from 29, Cyan from 56, Yellow from 19.
    //
    x = (color == kNumColorBlue) ? 29 : ((color == kNumColorCyan) ? 56 : 19);

    for (i = 0; i < 10; i++)
    {
        rglpBitmap[i] = PAL_SpriteGetFrame(gpSpriteUI, (UINT)x + i);
    }

    i = iNum;
    nActualLength = 0;

    //
    // Calculate the actual length of the number.
    //
    while (i > 0)
    {
        i /= 10;
        nActualLength++;
    }

    if (nActualLength > nLength)
    {
        nActualLength = nLength;
    }
    else if (nActualLength == 0)
    {
        nActualLength = 1;
    }

    x = PAL_X(pos) - 6;
    y = PAL_Y(pos);

    switch (align)
    {
    case kNumAlignLeft:
        x += 6 * nActualLength;
        break;

    case kNumAlignMid:
        x += 3 * (nLength + nActualLength);
        break;

    case kNumAlignRight:
        x += 6 * nLength;
        break;
    }

    //
    // Draw the number.
    //
    while (nActualLength-- > 0)
    {
        PAL_RLEBlitToSurface(rglpBitmap[iNum % 10], gpTextureReal, PAL_XY(x, y));
        x -= 6;
        iNum /= 10;
    }

}
//在纹理上显示汉字
PalErr CScreen::CopyFontToTexture(PalTexture& rpRender, std::wstring text,
    const SDL_Color* rColor, const PAL_POS pos, const float scale)
{
    GLuint id{};
    int x = PAL_X(pos) * PictureRatio;
    int y = PAL_Y(pos) * PictureRatio;
    int err{};
    for (wchar_t c : text) {
        if (c == 32)
        {
            x += HZSIZE * 0.66f * scale; // 空格字符特殊处理
            continue;
        }
        // 加载失败，跳过该字符
        auto it = characters.find(c);
        if (it == characters.end()) {
            // 尝试动态加载字符
            if (loadCharacter(c)) {
                continue;
            }
            it = characters.find(c);
        }

        Character ch = it->second;

        id = getCharacter(c).textureID;
        //以下对纹理进行渲染
        // 计算顶点坐标
        SDL_Rect dstRect{ x,
            y + ((ch.bearingY < HZSIZE) && ch.bearingY > 0 ? (HZSIZE - ch.bearingY) : 0) - HZSIZE * 0.1f,
            ch.size.cx() * scale,
            ch.size.cy() * scale };
        PalSize msSize{ch.size};
        PalSize mdSize = rpRender.size();

        setRectToArr(nullptr, &dstRect, ch.size, mdSize);

        glEnable(GL_TEXTURE_2D);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, id);
        glUseProgram(v_programID);
        glEnableVertexAttribArray(v_vertexID);

        err = glGetError() + err;

        glVertexAttribPointer(v_vertexID, 4, GL_FLOAT, GL_FALSE, 0, g_vertices.data());
        err = glGetError() + err;
        SDL_fColor dataBuf[20]{};
        err = glGetError() + err;
        if (rColor == nullptr)
            dataBuf[0] = { 1.0f,1.0f,1.0f,1.0f };
        else
            dataBuf[0] = { (GLfloat)(rColor->r * Div255),(GLfloat)(rColor->g * Div255)
                ,(GLfloat)(rColor->b * Div255),(GLfloat)(rColor->a * Div255) };
        dataBuf[1] = { 6.0f, 1.0f , 1.0f , 0.0f };


        glUniform4fv(v_dataID, 20, (const float*)dataBuf);
        err = glGetError() + err;

        glUniform1i(v_texID, 0);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);//设置融合函数
        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
        glDisable(GL_BLEND);
        err = glGetError() + err;

        glDisableVertexAttribArray(v_vertexID);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, 0);

        err = glGetError() + err;
        x += ch.advance * scale;
    }
    return PalErr(0);
}

PalErr CScreen::CopyFontToTexture(PalTexture& rpRender, LPCWSTR text, const SDL_Color* rColor, const PAL_POS pos, const float scale)
{
    std::wstring s = text;
    return CopyFontToTexture(rpRender, s, rColor, pos, scale);
}

PalErr CScreen::RenderBlendCopy(PalTexture& rpRender,
    PalTexture& rpText1, const WORD rAlpha, const RenderMode mode,
    const SDL_Color* rColor, const SDL_Rect* dstRect, const SDL_Rect* srcRect)

    /*
    模式功能
    //mode = 0.0 混合 v_mode.y 混合因子
    //mode = 1.0 拷贝后乘颜色
    //mode = 2.0 置成单一颜色
    //mode = 3.0 过滤拷贝全为零的点不拷贝
    //mode = 4.0 与颜色混合，v_mode.y 混合因子
    //mode = 6.0 显示字模
    //mode = 9.0 将纹理设置成黑色不透明
    //mode = 10.0 实现显示反转
    //返回 错误值
    * */
{
    SDL_ClearError();
    while (glGetError());
    Pal_Size msSize = { PictureWidth,PictureHeight };
    if (rpText1.isVilid())
    {
        msSize = rpText1.getSize();
    }
    else
        msSize = { 0,0 };
    Pal_Size mdSize = { RealWidth,RealHeight };
    if (rpRender.isVilid())
    {
        //取纹理的尺寸
        mdSize = rpRender.size();
        glViewport(0, 0, mdSize.cx(), mdSize.cy());
    }
    else
    {   
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glViewport(gView.x, gView.y, gView.w, gView.h);        
    }
    
	// 计算顶点坐标
    setRectToArr(srcRect, dstRect, msSize, mdSize);
    
    SDL_Color vColor = { 255,255,255,255 };
    if (rColor == NULL)
        rColor = &vColor;
    else
        rColor = rColor;

    int err = 0;

    err = glGetError() + err;
    // 绑定渲染目标
    if (rpRender.isNull())
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    else if (rpRender.setAsRenderTarget())
        return err;//失败终止渲染
    
    glActiveTexture(GL_TEXTURE0);
    if (rpText1.isVilid())
    {
        glEnable(GL_TEXTURE_2D);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, rpText1.getID());
    }
    err = glGetError() + err;

    glUseProgram(v_programID);
    glEnableVertexAttribArray(v_vertexID);

    err = glGetError() + err;

    glVertexAttribPointer(v_vertexID, 4, GL_FLOAT, GL_FALSE, 0, g_vertices.data());
    err = glGetError() + err;
    SDL_fColor dataBuf[20]{};
    err = glGetError() + err;

    dataBuf[0] = { (GLfloat)(rColor->r * Div255),(GLfloat)(rColor->g * Div255)
        ,(GLfloat)(rColor->b * Div255),1.0 };
    dataBuf[1] = { (GLfloat)(mode),
        (GLfloat)(rAlpha * Div255) ,
        rpText1.getType() == textType::red8 ? 1.0f : 0.0f ,
        rpText1.getPaletteID() != 0 ? 1.0f : 0.0f };
    dataBuf[2] = g_Mave;//波纹
    dataBuf[3] = g_Ripple;//涟漪
    dataBuf[4] = g_Zoom;//缩放
    dataBuf[5] = g_Roll;//旋转
    dataBuf[6] = g_TransColor;//透明色
    
    glUniform4fv(v_dataID, 20, (const float*)dataBuf);
    err = glGetError() + err;

    glUniform1i(v_texID, 0);
    if (rpText1.getPaletteID()>=0)//调色版用
    {
        glEnable(GL_TEXTURE_1D);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_1D, rpText1.getPaletteID());
        glUniform1i(v_paletteID, 1);
        err = glGetError() + err;
    }
    if ((mode == RenderMode::rmMode0 || 
        mode == RenderMode::rmMode6) ||
        mode == RenderMode::rmMode5 )
    {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);//设置融合函数
        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
        glDisable(GL_BLEND);
    }
    else
        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    //********
    err = glGetError() + err;

    glDisableVertexAttribArray(v_vertexID);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, 0);

    err = glGetError() + err;

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, 0);

    err = glGetError() + err;

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    glUseProgram(0);
    glDisable(GL_TEXTURE_2D);

    err = glGetError() + err;
    return PalErr(err);
}

PalErr CScreen::RenderBlendCopy(PalTexture& rpRender, 
    SDL_Surface* rpSurf, const WORD rAlpha, const RenderMode mode,
    const SDL_Color* rColor, const SDL_Rect* dstRect, const SDL_Rect* srcRect)
{
    PalTexture srcText(rpSurf);
    //assert(PalTexture::isVilid(srcText));
    return RenderBlendCopy(rpRender, srcText, rAlpha, mode, rColor, dstRect, srcRect);
}
//**
PalErr CScreen::RenderBlendCopy(PalTexture* rpRender, SDL_Surface* rpSurf, 
    const WORD rAlpha, const RenderMode mode, const SDL_Color* rColor,
    const SDL_Rect* dstRect, const SDL_Rect* srcRect)
{
    return RenderBlendCopy(*rpRender,rpSurf,rAlpha,mode,rColor,dstRect,srcRect);
}

PalErr CScreen::RenderBlendCopy(PalTexture& rpRender,
    PalSurface& rpSurf, const WORD rAlpha, const RenderMode mode,
    const SDL_Color* rColor, const SDL_Rect* dstRect, const SDL_Rect* srcRect)
{
    
    PalTexture dpText;
    dpText.creat(rpSurf);
    return RenderBlendCopy(rpRender, dpText, rAlpha, mode, rColor, dstRect, srcRect);
}


PalErr CScreen::RenderPresent(PalTexture& glRender, INT dAlpha)
{
    if (ggConfig == nullptr)
        return PalErr(0);//还没有初始化
    if (glRender.getID() != gpRenderTexture.getID())
        RenderBlendCopy(gpRenderTexture, glRender, 255);
    PalTexture s;//空纹理
    if (mouseInputStart)
        PAL_DrawMouseInputBox();
    RenderBlendCopy(s, gpRenderTexture, dAlpha, RenderMode::rmMode10);
    SDL_GL_SwapWindow(gpWindow);
    SDL_ShowWindow(gpWindow);
    return  PalErr(0);
}

PalErr CScreen::RenderPresent(PalTexture* glRender, INT dAlpha)
{
    return RenderPresent(*glRender, dAlpha);
}


VOID CScreen::ClearScreen(const SDL_Rect* sRect)
{
    //将显示纹理 清除指定区域为黑色 
    //输入 sdl_rect 指针 ，如为空 全部区域，区域以 320*200为基准
    //返回 无
    const SDL_Color sdlColor = { 0,0,0,255 };
    PAL_Rect dRect{ 0,0,RealWidth,RealHeight };
    if (sRect)
    {
        dRect = PAL_Rect(sRect);
        dRect *= PictureRatio;
    }
    gpRenderTexture.fillRect(&sdlColor, &dRect);
}

void CScreen::PAL_DrawMouseInputBox()
/*++
*/
{
    if (MouseInputBoxText.isNull())
        PAL_CreateMouseInputBoxText();
    
    PAL_Rect r = mouseInputBoxRect;
    if (!mouseInputBoxRect.isValid())
    {
        r = PAL_Rect(272, 152, 48, 48);//默认右下角,每个格子大小为16 for 320*200
        //r *= PictureRatio;                        //实际显示大小为
        mouseInputBoxRect = r;
    }
    PAL_Color color(200, 200, 200, 255);
    r *= PictureRatio;                        //实际显示大小为

    for (int i = 0; i < 9; i++)
    {
            int x = r.x;
            int y = r.y;
            int w = r.w / 3;
            int h = r.h / 3;
            PAL_Rect dstRect(x + (i % 3) * w, y + (i / 3) * h, w, h);
            w = h = 32;
            PAL_Rect srcRect((i % 3) * w, (i / 3) * h, w, h);
            if (i == mouseInputKey - 1)
            {
                //鼠标按下并选择，对应键加亮
                RenderBlendCopy(gpRenderTexture, MouseInputBoxText, 100, RenderMode::rmMode0, &color, &dstRect, &srcRect);
            }
            else
                RenderBlendCopy(gpRenderTexture, MouseInputBoxText, 60, RenderMode::rmMode0, &color, &dstRect, &srcRect);
    }
    //初始化点击目标列表
    if (mouseInputList.empty())
    {
		//鼠标位置以320*200为基准
        r *= 0.5;
        int x = r.x;
        int y = r.y;
        int w = r.w / 3;
        int h = r.h / 3;
        for (int i = 0; i < 9; i++)
        {
            mouseInputList.push_back(PAL_Rect(x + (i % 3) * w, y + (i / 3) * h, w, h));
        }
        return;
    }
}

void CScreen::PAL_CreateMouseInputBoxText()
{
    //生成图片
#if 1
    ByteArray mouseBox;
    std::vector<UINT32> mouseBoxColors;
#include "mousebox.h"
    if (mouseBox.size())
    {
        SDL_Surface* e = SDL_CreateRGBSurface(0, 96, 96, 8, 0, 0, 0, 0);
        memcpy(e->pixels, mouseBox.data(), 96 * 96);
        SDL_Palette* palette = SDL_AllocPalette(256);
        memcpy(palette->colors, mouseBoxColors.data(), 1024);
        SDL_SetSurfacePalette(e, palette);
        MouseInputBoxText.creat(e);
        SDL_FreeSurface(e);
        SDL_FreePalette(palette);
    }
#else  
    {
        //从图片生成头文件
        std::string filepath = PROJECT_SOURCE_DIR;
        filepath += "/3.bmp";
        auto  e = SDL_LoadBMP(filepath.c_str());
        if (e)
        {
            {
                filepath = PROJECT_SOURCE_DIR;
                filepath += "/main/mousebox.h";
                FILE* fp = fopen(filepath.c_str(), "w");
                if (fp)
                {
                    fprintf(fp, "mouseBox = {\n");
                    for (int n = 0; n < 96 * 96; n++)
                    {
                        if ((n % 32) == 0)
                            fprintf(fp, "\n");
                        fprintf(fp, "%d,", static_cast<std::byte*>(e->pixels)[n]);
                    }
                    fprintf(fp, "};\n");
                    auto palette = SDL_GetSurfacePalette(e);
                    if (palette)
                    {
                        fprintf(fp, "mouseBoxColors ={ ");
                        //有调色版打印调色版
                        for (int n = 0; n < 256; n++)
                        {
                            if ((n % 16) == 0)
                                fprintf(fp, "\n");
                            PAL_Color color = palette->colors[n];
                            fprintf(fp, "%u,", color);
                        }
                        fprintf(fp, "};\n");
                    }
                    fclose(fp);
                }
            }
            MouseInputBoxText.creat(e);
            SDL_FreeSurface(e);
        }
    }
#endif
}


void CScreen::setRectToArr(const SDL_Rect* src, const SDL_Rect* dst, const Pal_Size srcSize, const Pal_Size dstSize)
{
	// src 目标区域,空则全图
	// dst 显示区域，空则全屏
	// srcSize 源尺寸
	// dstSize 目标尺寸
    // 转换成数组
    GLfloat minx, miny, maxx, maxy;
    GLfloat minu, maxu, minv, maxv;
    if (dst && dst->w && dst->h && dstSize.cx() && dstSize.cy())
    {
        minx = (GLfloat)dst->x / (GLfloat)dstSize.cx() * 2 - 1.0;
        maxx = (GLfloat)(dst->x + dst->w) / (GLfloat)dstSize.cx() * 2 - 1.0;
        miny = (GLfloat)dst->y / dstSize.cy() * 2 - 1.0;
        maxy = (GLfloat)(dst->y + dst->h) / (GLfloat)dstSize.cy() * 2 - 1.0;
    }
    else
    {
        minx = -1.0f;
        maxx = 1.0f;
        miny = -1.0f;
        maxy = 1.0f;
    }

    if (src && src->w && src->h && srcSize.cx() && srcSize.cy())
    {
        minu = ((GLfloat)src->x / (GLfloat)srcSize.cx());
        maxu = ((GLfloat)src->x + src->w) / (GLfloat)srcSize.cx();
        minv = ((GLfloat)src->y / (GLfloat)srcSize.cy());
        maxv = ((GLfloat)(src->y + src->h)) / (GLfloat)srcSize.cy();
    }
    else
    {
        minu = 0.0f;
        maxu = 1.0f;
        minv = 0.0f;
        maxv = 1.0f;
    }

    g_vertices = {
        minx, miny, minu, minv,//左上
		maxx, miny, maxu, minv,//右上
		maxx, maxy, maxu, maxv,//右下
		minx, maxy, minu, maxv, //左下
    };
}


SDL_Color* CScreen::VIDEO_GetPalette(VOID)
/*++
  Purpose:

    Get the current palette of the screen.

  Parameters:

    None.

  Return value:

    Pointer to the current palette.

--*/
{
    return gpPalette->colors;
}

VOID CScreen::VIDEO_SwitchScreen(
    WORD           wSpeed
)
/*++
  Purpose:

    Switch the screen from the backup screen buffer to the current screen buffer.
    NOTE: This will destroy the backup buffer.

  Parameters:

    [IN]  wSpeed - speed of fading (the larger value, the slower).

  Return value:

    None.

--*/
{
    wSpeed++;
    wSpeed *= 10;
    for (int i = 0; i < 6; i++)
    {
        RenderBlendCopy(gpRenderTexture, gpTextureRealBak);
        RenderBlendCopy(gpRenderTexture, gpTextureReal, i * 51, RenderMode::rmMode0);
        RenderPresent(gpRenderTexture);
        SDL_Delay(wSpeed);
    }
    RenderPresent(gpTextureReal);
}

VOID CScreen::PAL_ColorFade(
    INT        iDelay,
    BYTE       bColor,
    BOOL       fFrom
)
/*++
  Purpose:

    Fade the palette from/to the specified color.

  Parameters:

    [IN]  iDelay - the delay time of each step.

    [IN]  bColor - the color to fade from/to.

    [IN]  fFrom - if TRUE then fade from bColor, else fade to bColor.

  Return value:

    None.

--*/
{

    iDelay *= 10;

    if (iDelay == 0)
    {
        iDelay = 10;
    }

     for (int i = 0; i < 64; i++)
    {
        if (fFrom)
        {
            RenderBlendCopy(gpRenderTexture, gpTextureReal, 256 - (i << 2),RenderMode::rmMode4, gpPalette->colors + bColor);
        }
        else
        {
            RenderBlendCopy(gpRenderTexture, gpTextureReal, (i << 2), RenderMode::rmMode4, gpPalette->colors + bColor);
        }
        RenderPresent(gpRenderTexture);
        SDL_Delay(iDelay);
    }
}

VOID CScreen::PAL_FadeToRed()
/*++
  Purpose:

    Fade the whole screen to red color.(game over)

  Parameters:

    None.

  Return value:

    None.

--*/
{
    PAL_Color color(255, 1, 1, 255);
    RenderBlendCopy(gpTextureRealBak, gpTextureReal, 255, RenderMode::rmMode4, &color);

    for (int i = 60; i >= 45; i--)
    {
        RenderBlendCopy(gpRenderTexture, gpTextureRealBak);
        RenderBlendCopy(gpRenderTexture, gpTextureReal, i << 2, RenderMode::rmMode0, 0);
        VIDEO_UpdateScreen(gpRenderTexture);
        PAL_Delay(40);
    }
    RenderBlendCopy( gpTextureReal, gpRenderTexture);

 }


INT CScreen::PAL_RNGBlitTo(
    const uint8_t* rng,
    int length,
    VOID* dstData,
    SDATA pCallback
)
/*++
Purpose:

  Blit one frame in an RNG animation to an SDL surface.
  The surface should contain the last frame of the RNG, or blank if it's the first
  frame.

  NOTE: Assume the surface is already locked, and the surface is a 320x200 8-bit one.
  已增加对32-BIT
Parameters:

  [IN]  rng - Pointer to the RNG data.// 指源数据的指针

  [IN]  len - Length of the RNG data.//源数据长度

  IN    data 目标数据

  [OUT]  data //指向目标数据回调函数 返回0 成功 其他失败 （颜色，目标数据宽度，目标数据长度，目标数据指针）.

Return value:

  0 = success, -1 = error.

--*/

{
    int                   ptr = 0;
    int                   dst_ptr = 0;
    uint16_t              wdata = 0;
    int                   x, y, i, n;

    //
    // Check for invalid parameters.
    //
    SDL_Surface* lpDstSurface = (SDL_Surface*)dstData;
    
    if (lpDstSurface == NULL || length < 0)
    {
        return -1;
    }
    BOOL is32bit = get_Surface_bitsPerPixel(lpDstSurface) == 32 ? TRUE : FALSE;
    UINT32* mColor = (UINT32*)VIDEO_GetPalette();
    //
    // Draw the frame to the surface.
    // FIXME: Dirty and ineffective code, needs to be cleaned up
    //
    while (ptr < length)
    {
        uint8_t data = rng[ptr++];
        switch (data)
        {
        case 0x00:
        case 0x13:
            //
            // End
            //
            goto end;

        case 0x02:
            dst_ptr += 2;
            break;

        case 0x03:
            data = rng[ptr++];
            dst_ptr += (data + 1) * 2;
            break;

        case 0x04:
            wdata = rng[ptr] | (rng[ptr + 1] << 8);
            ptr += 2;
            dst_ptr += ((unsigned int)wdata + 1) * 2;
            break;

        case 0x0a:
            x = dst_ptr % 320;
            y = dst_ptr / 320;
            pCallback(rng[ptr++], x, y, dstData);
            if (++x >= 320)
            {
                x = 0;
                ++y;
            }
            pCallback(rng[ptr++], x, y, dstData);
            dst_ptr += 2;

        case 0x09:
            x = dst_ptr % 320;
            y = dst_ptr / 320;
            pCallback(rng[ptr++], x, y, dstData);
            if (++x >= 320)
            {
                x = 0;
                ++y;
            }
            pCallback(rng[ptr++], x, y, dstData);
            dst_ptr += 2;

        case 0x08:
            x = dst_ptr % 320;
            y = dst_ptr / 320;
            pCallback(rng[ptr++], x, y, dstData);
            if (++x >= 320)
            {
                x = 0;
                ++y;
            }
            pCallback(rng[ptr++], x, y, dstData);
            dst_ptr += 2;

        case 0x07:
            x = dst_ptr % 320;
            y = dst_ptr / 320;
            pCallback(rng[ptr++], x, y, dstData);
            if (++x >= 320)
            {
                x = 0;
                ++y;
            }
            pCallback(rng[ptr++], x, y, dstData);
            dst_ptr += 2;

        case 0x06:
            x = dst_ptr % 320;
            y = dst_ptr / 320;
            pCallback(rng[ptr++], x, y, dstData);
            if (++x >= 320)
            {
                x = 0;
                ++y;
            }
            pCallback(rng[ptr++], x, y, dstData);
            dst_ptr += 2;
            break;

        case 0x0b:
            data = *(rng + ptr++);
            for (i = 0; i <= data; i++)
            {
                x = dst_ptr % 320;
                y = dst_ptr / 320;
                pCallback(rng[ptr++], x, y, dstData);
                if (++x >= 320)
                {
                    x = 0;
                    ++y;
                }
                pCallback(rng[ptr++], x, y, dstData);
                dst_ptr += 2;
            }
            break;

        case 0x0c:
            wdata = rng[ptr] | (rng[ptr + 1] << 8);
            ptr += 2;
            for (i = 0; i <= wdata; i++)
            {
                x = dst_ptr % 320;
                y = dst_ptr / 320;
                pCallback(rng[ptr++], x, y, dstData);
                if (++x >= 320)
                {
                    x = 0;
                    ++y;
                }
                pCallback(rng[ptr++], x, y, dstData);
                dst_ptr += 2;
            }
            break;

        case 0x0d:
        case 0x0e:
        case 0x0f:
        case 0x10:
            for (i = 0; i < data - (0x0d - 2); i++)
            {
                x = dst_ptr % 320;
                y = dst_ptr / 320;
                pCallback(rng[ptr], x, y, dstData);
                if (++x >= 320)
                {
                    x = 0;
                    ++y;
                }
                pCallback(rng[ptr + 1], x, y, dstData);
                if (is32bit)
                {
                    ((UINT32*)(lpDstSurface->pixels))[y * lpDstSurface->pitch / 4 + x] = mColor[rng[ptr + 1]];
                }
                else
                    ((LPBYTE)(lpDstSurface->pixels))[y * lpDstSurface->pitch + x] = rng[ptr + 1];
                dst_ptr += 2;
            }
            ptr += 2;
            break;

        case 0x11:
            data = *(rng + ptr++);
            for (i = 0; i <= data; i++)
            {
                x = dst_ptr % 320;
                y = dst_ptr / 320;
                pCallback(rng[ptr], x, y, dstData);
                if (++x >= 320)
                {
                    x = 0;
                    ++y;
                }
                pCallback(rng[ptr + 1], x, y, dstData);
                dst_ptr += 2;
            }
            ptr += 2;
            break;

        case 0x12:
            n = (rng[ptr] | (rng[ptr + 1] << 8)) + 1;
            ptr += 2;
            for (i = 0; i < n; i++)
            {
                x = dst_ptr % 320;
                y = dst_ptr / 320;
                pCallback(rng[ptr], x, y, dstData);
                if (++x >= 320)
                {
                    x = 0;
                    ++y;
                }
                pCallback(rng[ptr + 1], x, y, dstData);
                dst_ptr += 2;
            }
            ptr += 2;
            break;
        }
    }

end:
    return 0;
}

INT CScreen::PAL_RNGBlitToSurface(
    const uint8_t* rng,
    int              length,
    SDL_Surface* lpDstSurface
)
/*++
  Purpose:

    Blit one frame in an RNG animation to an SDL surface.
    The surface should contain the last frame of the RNG, or blank if it's the first
    frame.

    NOTE: Assume the surface is already locked, and the surface is a 320x200 8-bit one.
    增加对32-BIT 表面支持
  Parameters:

    [IN]  rng - Pointer to the RNG data.

    [IN]  length - Length of the RNG data.

    [OUT] lpDstSurface - pointer to the destination SDL surface.

  Return value:

    0 = success, -1 = error.

--*/
{
    auto bCallback = [&](UINT8 srcVal, int x, int y, void* dstData)->int
    {

        SDL_Surface* lpDstSurface = (SDL_Surface*)dstData;
        //BOOL is32bit = lpDstSurface->format->BitsPerPixel == 32 ? TRUE : FALSE;
        BOOL is32bit = get_Surface_bitsPerPixel( lpDstSurface) == 32 ? TRUE : FALSE;
        UINT32* mColor = (UINT32*)VIDEO_GetPalette();
        if (is32bit)
        {
            ((UINT32*)(lpDstSurface->pixels))[y * (lpDstSurface->pitch >> 2) + x] = mColor[srcVal] ? mColor[srcVal] : 1;
        }
        else
            ((LPBYTE)(lpDstSurface->pixels))[y * lpDstSurface->pitch + x] = srcVal;
        return 0;
    };
    return PAL_RNGBlitTo(rng, length, lpDstSurface, bCallback);
}

PalErr CScreen::FontInit()
//返回：0 成功
{

    if (FT_Init_FreeType(&ft)) {
        std::cout << "ERROR::FREETYPE: Could not init FreeType Library" << std::endl;
        return PalErr(1);
    }
    int Success{};
    // 加载字体
    if (ggConfig->m_Function_Set[46])//使用粗体字
    {
        if (!FT_New_Face(ft, ggConfig->m_FontName.c_str(), 1, &face))
        {
            std::cout << ggConfig->m_FontName.c_str() << "使用粗体字成功" << std::endl;
            Success = 1;
        }
        else {
            std::cout << ggConfig->m_FontName.c_str() << "使用粗体字失败" << std::endl;
        }
    }
    if (!Success && FT_New_Face(ft, ggConfig->m_FontName.c_str(), 0, &face))
    {
        std::cout << "ERROR::FREETYPE: Failed to load font: "
            << ggConfig->m_FontName.c_str() << std::endl;
        FT_Done_FreeType(ft);
        return PalErr(1);
    }

    std::cout << "字体文件加载成功: " << ggConfig->m_FontName.c_str() << std::endl;

    // 设置字体大小
    FT_Set_Pixel_Sizes(face, 0, HZSIZE);

    return PalErr(0);
}

VOID CScreen::FontShutDown()
{
    for (auto& character : characters) {
        if (character.second.textureID) {
            glDeleteTextures(1, &character.second.textureID);
        }
    }
    characters.clear();
}

VOID CScreen::glPreInit()
{
    //要求在主窗口建立之前
    //opengl 3.1
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1); //设置多缓存的个数
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
}

GLuint CScreen::loadShader(GLenum shaderType, const GLchar* source)
{
    GLuint shaderId = glCreateShader(shaderType);
    glShaderSource(shaderId, 1, &source, NULL);
    glCompileShader(shaderId);
    GLint result = GL_FALSE;
    glGetShaderiv(shaderId, GL_COMPILE_STATUS, &result);
    //checkShader(shaderId);
    return shaderId;
}

GLuint CScreen::compileProgram(GLuint fragmentShaderId, GLuint vertexShaderId)
{
    GLuint programId = glCreateProgram();
    glAttachShader(programId, vertexShaderId);
    glAttachShader(programId, fragmentShaderId);
    glLinkProgram(programId);
    return programId;
}

PalErr CScreen::loadCharacter(wchar_t c)
{
    // 加载字符字形
    if (FT_Load_Char(face, c, FT_LOAD_RENDER)) {
        std::cout << "ERROR::FREETYTPE: Failed to load Glyph: " << (int)c << std::endl;
        return 1;
    }
    // 生成纹理
    GLuint textureID{};
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    //对齐1 字节
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexImage2D(
        GL_TEXTURE_2D,
        0,
        GL_R8,
        face->glyph->bitmap.width,
        face->glyph->bitmap.rows,
        0,
        GL_RED,
        GL_UNSIGNED_BYTE,
        face->glyph->bitmap.buffer
    );

    // 设置纹理选项
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    auto ch = Character{
    c,
    textureID,
    Pal_Size(face->glyph->bitmap.width, face->glyph->bitmap.rows),
    static_cast<unsigned int>(face->glyph->advance.x >> 6),
    static_cast<int>(face->glyph->bitmap_top)
    };
    characters.insert(std::pair<wchar_t,Character>(c, ch));

    return PalErr(0);
}

CScreen::Character CScreen::getCharacter(wchar_t c)
{
    // TODO: 在此处插入 return 语句
    // 检查字符是否已加载
    if (characters.find(c) == characters.end()) {
        if (loadCharacter(c)) {
            // 加载失败，返回空字符
            return Character();
        }
    }
    return characters[c];
}

INT CScreen::VideoInit() {

    glPreInit();

    SDL_SetHint(SDL_HINT_RENDER_DRIVER, "opengl");
#if USING_SDL3
    gpWindow = SDL_CreateWindow("载入 ......", WindowWidth, WindowHeight, 
        SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | 
        SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_HIDDEN);
#else
    gpWindow = SDL_CreateWindow("载入 ......",
        SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, WindowWidth, WindowHeight,
        SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_HIDDEN);
#endif
    // 创建第一个OpenGL上下文
    gpContext = SDL_GL_CreateContext(gpWindow);
#if USING_SDL3
    gpRenderer = SDL_CreateRenderer(gpWindow, 0);
#else
    gpRenderer = SDL_CreateRenderer(gpWindow, -1, SDL_RENDERER_ACCELERATED);
#endif
    auto SDL_GL_GetProcAddressAdd = [](const char* name)->void* {
        return reinterpret_cast<void*>(SDL_GL_GetProcAddress(name));
        };
    //装入glad函数指针
    if (!gladLoadGLLoader(SDL_GL_GetProcAddressAdd))
        exit(2);
    int err = 0;
    {
        v_verticesID = loadShader(GL_VERTEX_SHADER, vertexShader);
        err = glGetError() | err;
        v_fragmentID = loadShader(GL_FRAGMENT_SHADER, fragmentShader);
        err = glGetError() | err;
        v_programID = compileProgram(v_verticesID, v_fragmentID);
        err = glGetError() | err;
        v_dataID = glGetUniformLocation(v_programID, "v_data");
        err = glGetError() | err;
        v_vertexID = glGetAttribLocation(v_programID, "intoposition");
        err = glGetError() | err;
        v_texID = glGetUniformLocation(v_programID, "v_tex");
        err = glGetError() | err;
        v_paletteID = glGetUniformLocation(v_programID, "v_paletteID");
        err = glGetError() | err;
    }
    //初始化窗口大小
    if (isTestRun)
    {
#if USING_SDL3
        SDL_DisplayID windowDisplayID = SDL_GetDisplayForWindow(gpWindow);
#else
        UINT windowDisplayID = 0;
#endif
        SDL_Rect dRect{};
        SDL_GetDisplayUsableBounds(windowDisplayID, &dRect);
        SDL_SetWindowSize(gpWindow, dRect.w * 0.5, dRect.h * 0.7);
        SDL_SetWindowPosition(gpWindow, dRect.w * 0.1, dRect.h * 0.10);
    }
    else
    {
#if USING_SDL3
        SDL_DisplayID windowDisplayID = SDL_GetDisplayForWindow(gpWindow);
#else
        UINT windowDisplayID = 0;
#endif
        SDL_Rect dRect{};
        SDL_GetDisplayUsableBounds(windowDisplayID, &dRect);
        SDL_SetWindowSize(gpWindow, dRect.w * 0.7, dRect.h * 0.7);
        SDL_SetWindowPosition(gpWindow, dRect.w * 0.10, dRect.h * 0.10);
    }

    KeepAspectRatio = ggConfig->fKeepAspectRatio;//保持窗口比率
    //初始化文字
    int bIsBig5 = (gpGlobals->bIsBig5 ? 1 : 0) + (ggConfig->fIsUseBig5 ? 2 : 0);

    if (FontInit())
    {
        //初始字体建立失败，退出，退出码10
        return 1;
    }

    gpTextureReal.creat(RealWidth, RealHeight, textType::rgba32, nullptr);
    gpTextureRealBak.creat(RealWidth, RealHeight, textType::rgba32, nullptr);
    gpRenderTexture.creat(RealWidth, RealHeight, textType::rgba32, nullptr);

    CPalEventHandler _m;
    //注册window事件

#if USING_SDL3
    _m.registCallback(SDL_WINDOWEVENT_RESIZED, [this](const SDL_Event* e) {
        if (e->type == SDL_WINDOWEVENT_RESIZED)
#else
    _m.registCallback(SDL_WINDOWEVENT, [this](const SDL_Event* e) {
        if (e->type == SDL_WINDOWEVENT && e->window.event == SDL_WINDOWEVENT_RESIZED)
#endif
        {
            int x{}, y{};
            SDL_GetWindowSize(gpWindow, &x, &y);
            if (x < RealWidth || y < RealHeight)
            {
                x = x < RealWidth ? RealWidth : x;
                y = y < RealHeight ? RealHeight : y;
                SDL_SetWindowSize(gpWindow, x, y);
                return;
            }
            gView = { 0,0,x,y };
            if (KeepAspectRatio && (fabs((double)gView.w / gView.h - 1.6) > 0.02))
            {
                double ra = (double)gView.w / gView.h - 1.6;
                if (ra > 0)
                {
                    gView.x = (int)((gView.w - gView.h * 1.6) / 2.0);
                    gView.w -= gView.x * 2;
                }
                else
                {
                    gView.y = (gView.h - gView.w / 1.6) / 2;
                    gView.h -= gView.y * 2;
                    if(gView.y<0)
                    gView.y =  0 ;
                }
            }
        }
        });

#ifdef USE_EVENTHANDLER
    //播放AVI的sdl_surface
    Uint32 customEventType = _m.getCustomEventHandler(eventTypeCustom::eventTypeAvi);
    //注册播放视频函数
    _m.registCallback(customEventType, [this](const SDL_Event* event) {
        //传入 1 指向sdl_surface 已经加锁
        auto mSurf = static_cast<SDL_Surface*>(event->user.data1);
        //显示
        RenderBlendCopy(gpTextureReal, mSurf);
        RenderPresent(gpTextureReal);
        CPalEventHandler _m;
        //解锁,在播放 中加的锁，确保传递成功
        _m.setCustomMutex((UINT)eventTypeCustom::eventTypeAvi, FALSE);
        });
#endif //USE_EVENTHANDLER
	//发送一次窗口改变事件，初始化视口
    {
        SDL_Event e{};
#if USING_SDL3
        e.type = SDL_WINDOWEVENT_RESIZED;
#else
        e.window.event = SDL_WINDOWEVENT_RESIZED;
        e.type = SDL_WINDOWEVENT;
#endif
        SDL_PushEvent(&e);  
    }
    return 0;
}

VOID CScreen::VideoShutDown()
{
    PalTexture::clearAll();
    FontShutDown();
    SDL_DestroyRenderer(gpRenderer);
    gpRenderer = nullptr;
    SDL_GL_DeleteContext(gpContext);
    gpContext = nullptr;
    if (gpWindow)
        SDL_DestroyWindow(gpWindow);
    gpWindow = nullptr;
}
