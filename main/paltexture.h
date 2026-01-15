#pragma once

#define USE_PBO 0
#include "palsurface.h"
#include "pal_color.h"
#include "command.h"

#define HZSIZE       32

#define  MastA 0xFF000000
#define  MastB 0x00FF0000
#define  MastG 0x0000FF00
#define  MastR 0x000000FF

typedef enum  class textType
{
    noType,
    red8,
    rgb16,
    rgb24,
    rgba32
}textType ;


// opengl二维纹理封装类
class PalTexture {
private:
    GLuint textureID{0};
    int w{};
    int h{};
    textType t{};
    GLuint	genFBOID{};//屏幕缓存区ID
#if USE_PBO
    GLuint  genPBOID{};//纹理缓存区ID
#endif    
    GLint   BitsPerPixel{};//每个像素占用Bit数，目前支持8和32位
    GLuint  paletteID {};
    PAL_Color transparent_color{};//透明色，8位纹理 为调色版255位
public:
    // 构造函数
    PalTexture();
    PalTexture(int aw, int ah, textType at = textType::rgba32,
        const SDL_Color * colors = nullptr);
    PalTexture(SDL_Surface* surface);
    PalTexture(PalSurface& surface);
    PalTexture(const PalTexture&) = delete;
    //移动构造函数
    PalTexture(PalTexture&& other) noexcept;;
    PalTexture& operator=(const PalTexture&) = delete;


    //移动构造函数
    PalTexture& operator=(PalTexture&& other)noexcept;
    
    ~PalTexture();
	
    //构建函数
 	PalErr creat(int aw, int ah, textType at, 
        const SDL_Color* colors = nullptr);
	PalErr creat(SDL_Surface* surface);
    PalErr creat(PalSurface& surface);

    PalErr UpdateTexture(const SDL_Rect* rRect, const void* pixels, INT pitch);

    bool isEmpty() const;
	GLuint getID() const { return textureID; };
	//将纹理设置为render target
	PalErr setAsRenderTarget();
	static PalErr setTextureAsRenderTarget(PalTexture* t);
	//取消渲染目标设置
    void unsetAsRenderTarget();
    // 清理纹理资源
    void clear();
    //将调色版缓存，转化为一维纹理 id  输入调色版缓存 返回错误码
    PalErr creatPaletteID(const SDL_Color* colors);
	//将纹理填充为单一颜色
    PalErr fillRect(const SDL_Color* color = nullptr, const PAL_Rect* pRect = nullptr);
    PalErr fill(const PAL_fColor& color);
public:
    //其他操作
    PalSize size() const {
        return PalSize(w, h);
    };
    int width() const {
        return w;
	};
    int height() const {
        return h;
	};  

    enum textType getType() { 
        return t;
    };
    //取调色版ID
    const GLint getPaletteID() const { return paletteID; };
    //设置透明色
    PalErr setTransparent_Color(const PAL_Color& color);
	//取得透明色
    PAL_Color getTransparent_Color();
	//取纹理像素数据
	ByteArray getData();
	//用数据更新纹理内容
    PalErr upSubData(const PAL_Rect* rect, void* data);
  
	//设置尺寸
    void setSize(int aw, int ah) {
        w = aw;
        h = ah;
	};
    Pal_Size getSize()
    {
        return Pal_Size(w, h);
    }
	//设置类型
    void setType(textType at) {
        t = at;
	};
    //将纹理设置为renderer 目标
	PalErr setToRendererTarget();

    //是否为空
    bool isNull()const { 
        return isEmpty();
    }
    //取每像素位数
    int getBitsPerPixel();
    //返回每像素占用字节数
    int getBytesPerPixel()
    {
        return getBitsPerPixel() << 3;
    }

    //输入bytesPerPixel 转换为 纹理类型
    const textType getTextType(UINT s) {
        switch (s)
        {
        case 1:
            return textType::red8;
        case 2:
            return textType::rgb16;
        case 3:
            return textType::rgb24;
        case 4:
            return textType::rgba32;
        
        default:
            return textType::noType;
        }
    }


    //检测纹理是否有效
    bool isVilid() const;
    static bool isVilid(const PalTexture & t );;

	//将所有纹理资源清理
    static void clearAll();

 };





