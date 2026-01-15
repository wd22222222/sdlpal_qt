#pragma once

#define Div255 0.003921568628
typedef uint8_t Uint8;

typedef struct SDL_fColor
{
    GLfloat r{};
    GLfloat g{};
    GLfloat b{};
    GLfloat a{};
} SDL_fColor;


//sdl_fcolor 4字节浮点数 16位宽，进入glsl 后可直接作为vec4使用
//sdl_frect 4字节浮点数 16位宽，进入glsl 后可直接作为vec4使用,与sdl_fcolor 结构相同
class PAL_fColor : public SDL_fColor
{
public:
    PAL_fColor()noexcept { r = g = b = a = 0; };
    PAL_fColor(const GLfloat fr, const GLfloat fg, const GLfloat fb, const GLfloat fa)noexcept
    {
        r = fr; g = fg; b = fb; a = fa;
    };

    PAL_fColor(const SDL_Color& color)noexcept {
        r = color.r * Div255;
        g = color.g * Div255;
        b = color.b * Div255;
        a = color.a * Div255;
    };

    PAL_fColor(const UINT c)noexcept
    {
        r = cRed(c) * Div255;
        g = cGreen(c) * Div255;
        b = cBlue(c) * Div255;
        a = cAlpha(c) * Div255;
    }
    
    PAL_fColor& operator= (const SDL_Color& color)noexcept
    {
        r = color.r;
        g = color.g;
        b = color.b;
        a = color.a;
        return *this;
    }
    //强制转换
    PAL_fColor(const SDL_Rect* c,const PalSize&  s )noexcept
    {
        if (c)
        {
            r = c->x;
            g = c->y;
            b = c->w;
            a = c->h;
        }
        else
        {
            r = g = 0;
            b = s.cx();
            a = s.cy();
        }
    }
    

    int cRed(int c) { return (c & 0xFF); };
    int cGreen(int c) { return ((c >> 8) & 0xFF); };
    int cBlue(int c) { return ((c >> 16) & 0xFF); };
    int cAlpha(int c) { return ((c >> 24) & 0xFF); };

 };

//sdl_color 4字节，RGBA 顺序，可在32位机器上直接用一个UINT32表示，
// 在32位图像系统中相当于一个像素点颜色值
//与pal_rect 不同 大小不一样，容纳范围也不一样，只限于颜色表示
//反过来
typedef class PAL_Color :public SDL_Color
{
public:
    PAL_Color() { r = 0, g = 0, b = 0, a = 0; };
    PAL_Color(const int mr , const int mg , const int mb , const int ma ) {
        r = mr, g = mg, b = mb, a = ma;
    }
    PAL_Color(const UINT32 c)
    {
        r = c & 0xff; g = (c >> 8) & 0xff; b = (c >> 16) & 0xff; a = c >> 24;
    }
    PAL_Color(const SDL_Color& c) { 
        r = c.r; g = c.g; b = c.b; a = c.a;
    };

    UINT32 toQColor() const
    {
        //qt的颜色与sdl 颜色不一致，B 与 R对调
        return b + (g << 8) + (r << 16) + (255 << 24);
    }
    UINT32 toUint32() const
    {
        return r + (g << 8) + (b << 16) + (a << 24);
    }
    PAL_fColor to_Pal_fColor() {
        PAL_fColor c(r, g, b, a);
        return c;
    }
} PAL_Color;


