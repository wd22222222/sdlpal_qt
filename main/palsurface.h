#pragma once
#include "command.h"
#include "sdl2_compat.h"
#include <stdexcept>
#include "sdl2_compat.h"
#include <vector>


//系统基本显示画面尺度
constexpr int PictureWidth = 320;
constexpr int PictureHeight = 200;

constexpr int RealWidth = 640;
constexpr int RealHeight = 400;

constexpr int WindowWidth = 800;
constexpr int WindowHeight = 500;

constexpr float Div255 = 0.003921568628;

constexpr float  PictureRatio = RealWidth / PictureWidth;


class PalSize
{
private:    
    int w{}, h{};
public:
    PalSize() {};
    PalSize(int aw, int ah) :w(aw), h(ah) {};
    PalSize(const PalSize& a) :w(a.w), h(a.h) {};
    PalSize& operator=(const PalSize& a) { w = a.w; h = a.h; return *this; };
    PalSize& operator=(const int& a) { w = a & 0xff; h = a << 8; return *this; };
    bool operator==(const PalSize& a) { return w == a.w && h == a.h; };
    bool operator!=(const PalSize& a) { return w != a.w || h != a.h; };
    bool operator==(const int& a) { return w == (a & 0xff) && h == (a >> 8); };
    bool operator!=(const int& a) { return w != (a & 0xff) || h != (a >> 8); };
    operator int() { return w | (h << 8); };
    operator SDL_Point() { return SDL_Point{ w,h }; };
    operator SDL_Rect() { return SDL_Rect{ 0,0,w,h }; };
    int cx()const{ return w; }
    int cy()const{ return h; }
};

class PAL_Rect :public SDL_Rect
{
public:
    //自定义转换方法
    PAL_Rect() { x = y = 0; w = h = 0; };
    PAL_Rect(const int ax, const int ay, const int aw, const int ah) {
        x = ax, y = ay, w = aw, h = ah; };
    PAL_Rect(const SDL_Rect r) { x = r.x, y = r.y, w = r.w, h = r.h; };
    PAL_Rect(const SDL_Rect* r) { x = r->x, y = r->y, w = r->w, h = r->h; };
    PAL_Rect(const PAL_POS p, const PalSize t) {
        x = PAL_X(p), y = PAL_Y(p), w = t.cx(), h = t.cy();
    };
    void set(const int ax, const int ay, const int aw, const int ah) {
        x = ax, y = ay, w = aw, h = ah;
    };
    PAL_Rect& operator*= (const double d) {
        this->x *= d;this->y *= d; this->h *= d; this->w *= d; return *this;
    };
    PalSize toSize() const {
        return PalSize(w, h);
    };
    //点 ｛x,y｝是否包含在区域中
    bool isInclude(const int px, const int py)const {
        return px >= x && px <= x + w && py >= y && py <= y + h;
    };
    bool isInclude(const PAL_POS p) const {
        int px = PAL_X(p);
        int py = PAL_Y(p);
        return px >= x && px <= x + w && py >= y && py <= y + h; 
	};
    bool isValid() const { return w > 0 && h > 0; };
};


typedef class PalSize Pal_Size;

class PalSurface {
private:
    SDL_Surface* e{};

public:
    // 构造函数
    PalSurface() {};
    //构造
    PalSurface(int width, int height, int depth = 32,
        Uint32 rmast = 0, Uint32 gmasr = 0, Uint32 bmast = 0, Uint32 amast = 0) {
        e = SDL_CreateRGBSurface(0, width, height, depth, rmast, gmasr, bmast, amast);
    }
    //构造8位带索引的表面
    PalSurface(SDL_Palette* p, int width, int height)
    {
        e = SDL_CreateRGBSurface(0, width, height, 8, 0, 0, 0, 0);
        SDL_SetSurfacePalette(e, p);
        //透明色255
        SDL_FillRect(e, nullptr, 255);
    }
    // 拷贝构造函数（如果需要的话）
    PalSurface(const PalSurface& other) {
#if USING_SDL3
        e = SDL_DuplicateSurface(other.e);
#else
        e = SDL_ConvertSurfaceFormat(other.e, other.e->format->format, 0);
#endif
    }
    // 移动构造函数（如果需要的话）
    PalSurface(PalSurface&& other) noexcept : e(other.e) {
        other.e = nullptr; // 转移所有权，避免双重释放
    }

    // 移动赋值运算符（如果需要的话）
    PalSurface& operator=(PalSurface&& other) noexcept {
        if (this != &other) {
            SDL_FreeSurface(e); // 释放当前资源
            e = other.e; // 转移资源指针
            other.e = nullptr; // 避免双重释放
        }
        return *this;
    }

    // 析构函数
    ~PalSurface() {
        if (e)
            SDL_FreeSurface(e); // 清理资源
        e = nullptr;
    }

    VOID clear();
    int w() const { return e->w; }
    int h() const { return e->h; }
    PalSize getSize() const { return PalSize(w(), h()); }
    void* pixels() { return e->pixels; }
    bool isNull() { return e == nullptr; }

    void fill();
    
    // 获取底层SDL_Surface指针（谨慎使用）
    SDL_Surface* get() const { return e; };
    //获取调色版
    SDL_Palette* getPalette() const;;
};

inline VOID setPictureRatio(SDL_Rect* rect)
{
    rect->x *= PictureRatio;
    rect->y *= PictureRatio;
    rect->w *= PictureRatio;
    rect->h *= PictureRatio;
}
