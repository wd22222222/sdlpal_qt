#pragma once
#ifndef CSCREEN_H
#define CSCREEN_H

#include "command.h"
#include "../sound/csound.h"
#include <ft2build.h>
#include FT_FREETYPE_H

#include "paltexture.h"
#include "pal_color.h"
#include <map>
#include <vector>

#define  MastA 0xFF000000
#define  MastB 0x00FF0000
#define  MastG 0x0000FF00
#define  MastR 0x000000FF
#define  MENUITEM_VALUE_CANCELLED      0xFFFF


typedef enum tagNUMALIGN
{
    kNumAlignLeft,
    kNumAlignMid,
    kNumAlignRight
} NUMALIGN;

typedef enum tagNUMCOLOR
{
    kNumColorYellow,
    kNumColorBlue,
    kNumColorCyan
} NUMCOLOR;


enum class RenderMode {
    rmMode0 = 0,            // 混合
    rmMode1,                // 拷贝后乘颜色
    rmMode2,                // 置成单一颜色
    rmMode3,                // 过滤颜色
    rmMode4,                // 与单一颜色混合
    rmMode5,                // 源Alpha 为度混合
    rmMode6 = 6,            // 执行字模拷贝
    rmMode7,                // 去除阴影
    rmMode8,                // 将图像转为黑白，后乘以alpha再加上颜色
    rmMode9,                // 将纹理初始到黑色透明
    rmMode10 = 10,          // 执行屏幕翻转显示
};
//typedef class GL_Render GL_Render;

typedef std::function< int(UINT8 srcVal, int w, int h, VOID* dstData)> SDATA;

//using SDATA = [](UINT8 srcVal, int x, int y, void* dstData)->int;
//本类用于实现屏幕操作各种相关函数
class CScreen :public CSound
{
    struct Character {
        wchar_t      c{};
        GLuint       textureID{};    // 字形纹理ID
        Pal_Size     size{};          // 字形大小
        unsigned int advance{}; // 原点到下一个字形原点的距离
        int          bearingY{};  //字符底位置高度  = face->glyph->bitmap_top;
    };

public:
    CScreen();
    ~CScreen();

public:
    //设置屏幕摇动参数，次数和水平
    void VIDEO_ShakeScreen(WORD wShakeTime, WORD wShakeLevel);
    //从备份缓存区过渡到屏幕缓存区，输入参数 速度，独占时间，之中屏幕可以抖动
    void VIDEO_FadeScreen(WORD speed);
    //SDL_Surface* VIDEO_CreateCompatibleSurface(SDL_Surface* pSource);
    INT PAL_RNGBlitTo(const uint8_t* rng, int len, VOID* data, SDATA ddata);
    INT PAL_RNGBlitToSurface(const uint8_t* rng, int length, SDL_Surface* lpDstSurface);
    INT VideoInit();
    VOID VideoShutDown();
    VOID VIDEO_BackupScreen(SDL_Surface* s);
    VOID VIDEO_BackupScreen(PalTexture& s);
    VOID VIDEO_RestoreScreen();
    VOID VIDEO_UpdateScreen(PalTexture& dstText, const SDL_Rect* lpSrcRect = NULL, const SDL_Rect* lpDstRect = NULL);
    //*版本
    VOID VIDEO_UpdateScreen(PalTexture* dstText, const SDL_Rect* lpSrcRect = NULL, const SDL_Rect* lpDstRect = NULL);
    VOID VIDEO_UpdateSurfacePalette(SDL_Surface* pSurface);
    SDL_Color* VIDEO_GetPalette(VOID);
    VOID VIDEO_SwitchScreen(WORD wSpeed);
    VOID PAL_ColorFade(INT iDelay, BYTE bColor, BOOL fFrom);
    VOID PAL_FadeToRed();
    VOID PAL_FadeIn(INT iPaletteNum, BOOL fNight, INT iDelay);
    VOID PAL_FadeOut(INT iDelay);
    VOID PAL_FadeOut(PalTexture &dstText, INT iDelay);
    VOID PAL_SetPalette(INT iPaletteNum, BOOL fNight);
    VOID PAL_DrawText(LPCSTR lpszText, PAL_POS  pos, BYTE  bColor, BOOL fShadow, BOOL fUpdate, int size = 16)
    {
        PAL_DrawText(std::string(lpszText), pos, bColor, fShadow, fUpdate, size);
    }
    Pal_Size PAL_DrawText(const std::string& lpszText, PAL_POS  pos, BYTE  bColor, BOOL fShadow, BOOL fUpdate, int size = 16);
    Pal_Size PAL_DrawWideText(const std::wstring& lpszTextR, PAL_POS pos, BYTE bColor,
        BOOL fShadow, BOOL fUpdate, int size = 16);
    Pal_Size PAL_DrawTextUTF8(LPCSTR lpszText, PAL_POS pos, BYTE bColor, BOOL fShadow, BOOL fUpdate, int size = 16);
    Pal_Size PAL_DrawTextUTF8(const std::string& s, PAL_POS pos, BYTE bColor, BOOL fShadow, BOOL fUpdate, int size);
    VOID VIDEO_SetPalette(SDL_Color rgPalette[256]);
    SDL_Color* PAL_GetPalette(INT iPaletteNum, BOOL fNight);
    VOID PAL_DrawNumber(UINT iNum, UINT nLength, PAL_POS pos, NUMCOLOR color, NUMALIGN align, int size = 8);

    //在纹理上显示汉字
    PalErr CopyFontToTexture(PalTexture& rpRender, std::wstring text,
        const SDL_Color* rColor, const PAL_POS pos, const float scale);
    PalErr CopyFontToTexture(PalTexture& rpRender, LPCWSTR text,
        const SDL_Color* rColor, const PAL_POS pos, const float scale);

    PalErr RenderBlendCopy(PalTexture& rpRender, PalTexture& rpText1,
        const WORD rAlpha = 255, const RenderMode mode = RenderMode::rmMode1, const SDL_Color* rColor = NULL,
        const SDL_Rect* dstRect = NULL, const SDL_Rect* srcRect = NULL);

    PalErr RenderBlendCopy(PalTexture& rpRender, SDL_Surface* rpSurf,
        const WORD rAlpha = 255, const RenderMode mode = RenderMode::rmMode1, const SDL_Color* rColor = NULL,
        const SDL_Rect* dstRect = NULL, const SDL_Rect* srcRect = NULL);
    PalErr RenderBlendCopy(PalTexture* rpRender, SDL_Surface* rpSurf,
        const WORD rAlpha = 255, const RenderMode mode = RenderMode::rmMode1, const SDL_Color* rColor = NULL,
        const SDL_Rect* dstRect = NULL, const SDL_Rect* srcRect = NULL);

    PalErr RenderBlendCopy(PalTexture& rpRender, PalSurface& rpSurf,
        const WORD rAlpha = 255, const RenderMode mode = RenderMode::rmMode1, const SDL_Color* rColor = NULL,
        const SDL_Rect* dstRect = NULL, const SDL_Rect* srcRect = NULL);

    PalErr RenderPresent(PalTexture& glRender, INT dAlpha = 255);
    //*版本
    PalErr RenderPresent(PalTexture* glRender, INT dAlpha = 255);

    inline VOID setAlpha(const WORD alpha) { g_Alpha = alpha; }
    inline WORD getAlpha(VOID) { return g_Alpha; }
    inline VOID setColor(const SDL_Color color) { g_Color = color; }
    inline VOID setColor(const int r, const int g, const int b) { g_Color.r = r; g_Color.g = g; g_Color.b = b; }
    inline const SDL_Color getColor(VOID) { return g_Color; };
    inline const SDL_Color* getpColor(VOID) { return &g_Color; };
    inline VOID setTransColor(const  SDL_Color& a) { 
        g_TransColor = PAL_fColor( a); 
    };
    VOID ClearScreen(const SDL_Rect* sRect = NULL);
    void PAL_DrawMouseInputBox();
    void PAL_CreateMouseInputBoxText();
private:
 

private:
    WORD g_wShakeTime{ 0 };
    WORD g_wShakeLevel{ 0 };
    PAL_Color g_Color = { 255,255,255,255 };
    WORD   g_Alpha{ 255 };//

    PAL_fColor g_Mave{};//波纹
    PAL_fColor g_Ripple{};//水波
    PAL_fColor g_Zoom{};//缩放
    PAL_fColor g_Roll{};//旋转
    PAL_fColor g_TransColor{};//透明色
    //着色器参数
    GLuint v_verticesID{};//顶点ID
    GLuint v_fragmentID{};//片断ID
    GLuint v_programID{};//GLSL过程ID
    //
    GLint  v_vertexID{-1};	//位置顶点数组ID
    GLint  v_texcoordID{-1}; //纹理顶点数组1ID
    GLint  v_texID{-1};//纹理ID
    //GLint  v_tex1ID{-1};//纹理1 ID
    GLint  v_dataID{-1};//参数数组ID
    GLint  v_paletteID{-1};//调色版纹理ID

    SDL_GLContext gpContext{};
    PalTexture MouseInputBoxText;
protected:
    //设置屏幕波动，x 垂直分段，y 水平幅度 ,z 抖动，w 时间 
    inline VOID setMave(GLfloat x, GLfloat y, GLfloat z, GLfloat w) {
        g_Mave = PAL_fColor(x, y, z, w);
    }
    //设置水波纹 x y 波纹中心点坐标，z 水波纹宽度，w 起始波纹
    inline VOID setRipple(GLfloat x, GLfloat y, GLfloat z, GLfloat w) 
    {
        g_Ripple = PAL_fColor(x, y, z, w);
    }
    //设置缩放 x  y 中心点 z 缩放比率，w  缩放开关
    inline VOID setZoom(GLfloat x, GLfloat y, GLfloat z, GLfloat w) {
        g_Zoom = PAL_fColor(x, y, z, w);
    }
    //设置旋转 角度 x 延x轴 y 延y轴 z 延z轴，w  旋转开关
    inline VOID setRoll(GLfloat x, GLfloat y, GLfloat z, GLfloat w) {
        g_Roll = PAL_fColor(x, y, z, w);
    }
    //设置透明色 -1 不透明
    inline VOID setTransColor(GLfloat r, GLfloat g, GLfloat b, GLfloat a) {
        g_TransColor = PAL_fColor(r, g, b, a);
    };
    
public:
    PalTexture     gpTextureReal{};//显示纹理
    PalTexture     gpTextureRealBak{};//显示纹理备份
    PalTexture     gpRenderTexture{};//用于最终显示的纹理
    SDL_Window*     gpWindow{};
    SDL_Renderer*   gpRenderer{};
private:

    void  setRectToArr(const SDL_Rect* src, const SDL_Rect* dst, const Pal_Size srcSize, const Pal_Size dstSize);;
    std::vector<GLfloat> g_vertices;//位置顶点数组

    VOID glPreInit();
    int KeepAspectRatio{ TRUE };

    //编译着色器
    GLuint loadShader(GLenum shaderType, const GLchar* source);
    GLuint compileProgram(GLuint fragmentShaderId, GLuint vertexShaderId);
    //字符显示相关
    PalErr FontInit();
    VOID   FontShutDown();
    FT_Library ft{};
    FT_Face face{};
    std::map<wchar_t, Character> characters;
    PalErr loadCharacter(wchar_t c);
	Character getCharacter(wchar_t c);
};

#endif // CSCREEN_H
