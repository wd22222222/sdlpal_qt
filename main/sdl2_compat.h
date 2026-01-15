// sdl2_compat.h
#pragma once


#define SDL_oldnames_h_

#if USING_SDL3
#include <SDL3/SDL.h>
#include <SDL3/SDL_opengl.h>
#include <SDL3/SDL_stdinc.h>

#else

#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#include <SDL2/SDL_stdinc.h>
#include <SDL2/SDL_rwops.h>
#include <SDL2/SDL_audio.h>

#endif

// ============================================================================
// 事件类型兼容性宏
// ============================================================================

#if USING_SDL3
    // 窗口事件
#define SDL_WINDOWEVENT              SDL_EVENT_WINDOW_FIRST
#define SDL_WINDOWEVENT_SHOWN        SDL_EVENT_WINDOW_SHOWN
#define SDL_WINDOWEVENT_HIDDEN       SDL_EVENT_WINDOW_HIDDEN
#define SDL_WINDOWEVENT_EXPOSED      SDL_EVENT_WINDOW_EXPOSED
#define SDL_WINDOWEVENT_MOVED        SDL_EVENT_WINDOW_MOVED
#define SDL_WINDOWEVENT_RESIZED      SDL_EVENT_WINDOW_RESIZED
#define SDL_WINDOWEVENT_SIZE_CHANGED SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED
#define SDL_WINDOWEVENT_MINIMIZED    SDL_EVENT_WINDOW_MINIMIZED
#define SDL_WINDOWEVENT_MAXIMIZED    SDL_EVENT_WINDOW_MAXIMIZED
#define SDL_WINDOWEVENT_RESTORED     SDL_EVENT_WINDOW_RESTORED
#define SDL_WINDOWEVENT_ENTER        SDL_EVENT_WINDOW_MOUSE_ENTER
#define SDL_WINDOWEVENT_LEAVE        SDL_EVENT_WINDOW_MOUSE_LEAVE
#define SDL_WINDOWEVENT_FOCUS_GAINED SDL_EVENT_WINDOW_FOCUS_GAINED
#define SDL_WINDOWEVENT_FOCUS_LOST   SDL_EVENT_WINDOW_FOCUS_LOST
#define SDL_WINDOWEVENT_CLOSE        SDL_EVENT_WINDOW_CLOSE_REQUESTED
#define SDL_GL_DeleteContext         SDL_GL_DestroyContext
#define SDL_WINDOW_ALLOW_HIGHDPI     SDL_WINDOW_HIGH_PIXEL_DENSITY
#define SDL_RWFromConstMem           SDL_IOFromConstMem
#undef  SDL_RWread
//#define SDL_RWread                   SDL_ReadIO
#define AUDIO_S16                    SDL_AUDIO_S16LE
//#define SDL_ConvertSurfaceFormat     SDL_ConvertSurface

// 键盘事件
#define SDL_KEYDOWN                  SDL_EVENT_KEY_DOWN
#define SDL_KEYUP                    SDL_EVENT_KEY_UP
#define SDL_TEXTEDITING              SDL_EVENT_TEXT_EDITING
#define SDL_TEXTINPUT                SDL_EVENT_TEXT_INPUT

// 鼠标事件
#define SDL_MOUSEMOTION              SDL_EVENT_MOUSE_MOTION
#define SDL_MOUSEBUTTONDOWN          SDL_EVENT_MOUSE_BUTTON_DOWN
#define SDL_MOUSEBUTTONUP            SDL_EVENT_MOUSE_BUTTON_UP
#define SDL_MOUSEWHEEL               SDL_EVENT_MOUSE_WHEEL

// 退出事件
#define SDL_QUIT                     SDL_EVENT_QUIT
#endif

// ============================================================================
// 键盘修饰键兼容性宏
// ============================================================================

#if USING_SDL3
#define KMOD_NONE    SDL_KMOD_NONE
#define KMOD_LSHIFT  SDL_KMOD_LSHIFT
#define KMOD_RSHIFT  SDL_KMOD_RSHIFT
#define KMOD_LCTRL   SDL_KMOD_LCTRL
#define KMOD_RCTRL   SDL_KMOD_RCTRL
#define KMOD_LALT    SDL_KMOD_LALT
#define KMOD_RALT    SDL_KMOD_RALT
#define KMOD_LGUI    SDL_KMOD_LGUI
#define KMOD_RGUI    SDL_KMOD_RGUI
#define KMOD_NUM     SDL_KMOD_NUM
#define KMOD_CAPS    SDL_KMOD_CAPS
#define KMOD_MODE    SDL_KMOD_MODE
#define KMOD_CTRL    SDL_KMOD_CTRL
#define KMOD_SHIFT   SDL_KMOD_SHIFT
#define KMOD_ALT     SDL_KMOD_ALT
#define KMOD_GUI     SDL_KMOD_GUI
#endif

// ============================================================================
// 键盘按键兼容性宏
// ============================================================================

#if USING_SDL3
    // 字母键
#define SDLK_a SDLK_A
#define SDLK_b SDLK_B
#define SDLK_c SDLK_C
#define SDLK_d SDLK_D
#define SDLK_e SDLK_E
#define SDLK_f SDLK_F
#define SDLK_g SDLK_G
#define SDLK_h SDLK_H
#define SDLK_i SDLK_I
#define SDLK_j SDLK_J
#define SDLK_k SDLK_K
#define SDLK_l SDLK_L
#define SDLK_m SDLK_M
#define SDLK_n SDLK_N
#define SDLK_o SDLK_O
#define SDLK_p SDLK_P
#define SDLK_q SDLK_Q
#define SDLK_r SDLK_R
#define SDLK_s SDLK_S
#define SDLK_t SDLK_T
#define SDLK_u SDLK_U
#define SDLK_v SDLK_V
#define SDLK_w SDLK_W
#define SDLK_x SDLK_X
#define SDLK_y SDLK_Y
#define SDLK_z SDLK_Z

#define SDL_OK      (1)
#define SDL_FAIL    (0)
#define SDL_SWSURFACE 0UL

#endif

// ============================================================================
// 函数兼容性包装
// ============================================================================

#if USING_SDL3
// SDL3 环境下的函数包装
//
#define SDL_mutex SDL_Mutex

// 表面函数
#define SDL_FreeSurface(surface) SDL_DestroySurface(surface)
//#define SDL_CreateRGBSurface(flags, width, height, depth, Rmask, Gmask, Bmask, Amask) \
    //SDL_CreateSurface(width, height, SDL_PIXELFORMAT_RGBA32)
#define SDL_CreateRGBSurfaceFrom(data, width, height, depth, pitch, Rmask, Gmask, Bmask, Amask) \
    SDL_CreateSurfaceFrom(data, width, height, pitch, SDL_PIXELFORMAT_RGBA32)
#define SDL_AllocPalette SDL_CreatePalette
#define SDL_FillRect SDL_FillSurfaceRect
// 渲染函数
#define SDL_RenderCopy(renderer, texture, srcrect, dstrect) \
    SDL_RenderTexture(renderer, texture, srcrect, dstrect)
#define SDL_RenderCopyEx(renderer, texture, srcrect, dstrect, angle, center, flip) \
    SDL_RenderTextureRotated(renderer, texture, srcrect, dstrect, angle, center, flip)
//调色板函数
#define SDL_FreePalette SDL_DestroyPalette
// 图像加载 (SDL_image 功能)
#define IMG_Load(file) SDL_LoadImage(file)
#define IMG_LoadTexture(renderer, file) SDL_LoadTexture(renderer, file)
#define IMG_Load_RW(src, freesrc) SDL_LoadImage_RW(src, freesrc)

// 字体渲染 (SDL_ttf 功能)
#define TTF_OpenFont(file, ptsize) SDL_OpenFont(file, ptsize, 0)
#define TTF_RenderText_Solid(font, text, fg) SDL_RenderText(font, text, fg)
//#define TTF_RenderUTF8_Solid(font, text, fg) SDL_RenderText(font, text, fg)
#define TTF_RenderUNICODE_Solid(font, text, fg) SDL_RenderText(font, text, fg)
#define TTF_CloseFont(font) SDL_CloseFont(font)
// 音频 (SDL_mixer 类似功能)
#define Mix_LoadWAV(file) SDL_LoadWAV(file)
#define Mix_FreeChunk(chunk) SDL_DestroyAudioChunk(chunk)
#define SDL_RWops SDL_IOStream
#define AUDIO_U8 SDL_AUDIO_U8
#define AUDIO_S16LSB SDL_AUDIO_S16LE
#define AUDIO_S16SYS SDL_AUDIO_S16
#define SDL_SwapLE16 SDL_Swap16LE

// 获取修改键状态
#define SDL_GetModState() SDL_GetModifierState()
//锁
#define SDL_mutexP      SDL_LockMutex;
#define SDL_mutexV      SDL_UnlockMutex;
//
#define SDL_SwapLE32    SDL_Swap32LE
#define SDL_RWclose     SDL_CloseIO
#define SDL_RWFromFile  SDL_IOFromFile
//#define SDL_LockAudioDevice SDL_LockAudioStream
//#define SDL_UnlockAudioDevice SDL_UnlockAudioStream
#define SDL_FreeRW_x(x)
#undef SDL_RWseek
#define SDL_RWseek SDL_SeekIO

#else
//USING_SDL3  0

#define SDL_FreeRW_x(x) SDL_FreeRW(x)

#endif

// ============================================================================
// 事件结构访问兼容性
// ============================================================================
inline int get_SDL_EnevtKey(const SDL_Event* lpEvent)
{
#if(USING_SDL3)
    return lpEvent->key.key;
#else
    return lpEvent->key.keysym.sym;
#endif
}
inline int get_SDL_EnevtKeyMod(const SDL_Event* lpEvent)
{
#if(USING_SDL3)
    return lpEvent->key.mod;
#else
    return lpEvent->key.keysym.mod;
#endif
}
inline void set_SDL_EventKey(SDL_Event* lpEvent,const int k)
{
#if(USING_SDL3)
    lpEvent->key.key = k;
#else
    lpEvent->key.keysym.sym = k;
#endif
}

inline void set_SDL_EventType(SDL_Event* lpEvent, const int k)
{
//////
#if(USING_SDL3)
    lpEvent->key.key = k;
#else
    lpEvent->key.keysym.sym = k;
#endif
}


#if USING_SDL3
// 事件类型访问包装
Uint8 SDL_GetWindowEventType(const SDL_Event* event);

// 键盘事件访问
SDL_Keycode SDL_GetKeyboardKeycode(const SDL_Event* event);

// 鼠标事件访问
Uint8 SDL_GetMouseButton(const SDL_Event* event);

int SDL_GetMouseX(const SDL_Event* event);

int SDL_GetMouseY(const SDL_Event* event);



#else

// SDL2 环境下的兼容性函数
inline Uint8 SDL_GetWindowEventType(const SDL_Event* event) {
    return (event->type == SDL_WINDOWEVENT) ? event->window.event : 0;
}

inline SDL_Keycode SDL_GetKeyboardKeycode(const SDL_Event* event) {
    return event->key.keysym.sym;
}

inline Uint8 SDL_GetMouseButton(const SDL_Event* event) {
    return event->button.button;
}

inline int SDL_GetMouseX(const SDL_Event* event) {
    return event->button.x;
}

inline int SDL_GetMouseY(const SDL_Event* event) {
    return event->button.y;
}
#endif

// ============================================================================
// 初始化函数兼容性
// ============================================================================

#if USING_SDL3
// SDL3 初始化返回 0 表示成功，非0表示失败
// 为了兼容 SDL2 的风格（返回 0 表示成功），我们可以包装一下
inline int SDL_Compat_Init(Uint32 flags) {
    return SDL_Init(flags) ? 0 : -1;
}

#define SDL_Init(flags) SDL_Compat_Init(flags)
#endif

// ============================================================================
// 音频转换兼容性 (SDL_AudioCVT)
// ============================================================================

#if USING_SDL3
// SDL3 音频转换兼容性
// 为了兼容性定义（不推荐长期使用）
#define SDL_MIX_MAXVOLUME 100.0f
#define SDL_MIX_MINVOLUME 1.0f
#define SDL_WINDOW_FULLSCREEN_DESKTOP 1

#define SDL_AUDIOCVT_MAX_FILTERS 9
typedef void (SDLCALL* SDL_AudioFilter) (struct SDL_AudioCVT* cvt, SDL_AudioFormat format);

typedef struct SDL_AudioCVT
{
    int needed;                 /**< Set to 1 if conversion possible */
    SDL_AudioFormat src_format; /**< Source audio format */
    SDL_AudioFormat dst_format; /**< Target audio format */
    double rate_incr;           /**< Rate conversion increment */
    Uint8* buf;                 /**< Buffer to hold entire audio data */
    int len;                    /**< Length of original audio buffer */
    int len_cvt;                /**< Length of converted audio buffer */
    int len_mult;               /**< buffer must be len*len_mult big */
    double len_ratio;           /**< Given len, final size is len*len_ratio */
    SDL_AudioFilter filters[SDL_AUDIOCVT_MAX_FILTERS + 1]; /**< NULL-terminated list of filter functions */
    int filter_index;           /**< Current audio conversion function */
}  SDL_AudioCVT;


inline size_t SDL_RWread(SDL_IOStream* stream, void* ptr, size_t size, size_t nitems)
{
    if (size > 0 && nitems > 0) {
        return SDL_ReadIO(stream, ptr, size * nitems) / size;
    }
    return 0;
}

// 简单的包装函数
int SDL_BuildAudioCVT(SDL_AudioCVT* cvt,
    SDL_AudioFormat src_format, Uint8 src_channels, int src_rate,
    SDL_AudioFormat dst_format, Uint8 dst_channels, int dst_rate);

SDL_Surface* SDL_ConvertSurfaceFormat(SDL_Surface* src,
    const SDL_PixelFormat fmt, Uint32 flags);

#else
// SDL2 - 使用原生函数

#endif

#if USING_SDL3
SDL_Surface* SDL_CreateRGBSurface(Uint32 flags, int width, int height, int depth, Uint32 Rmask, Uint32 Gmask, Uint32 Bmask, Uint32 Amask);

inline int SDL_GL_BindTexture(SDL_Texture* texture, float* texw, float* texh) {
    if (texture == 0)
        return -1;
    // 获取纹理尺寸
    SDL_GetTextureSize(texture, texw, texh);
    // 获取 OpenGL 纹理 ID
    Uint32 gl_texture_id;
    if (gl_texture_id = SDL_GetTextureProperties(texture)) {
        glBindTexture(GL_TEXTURE_2D, gl_texture_id);
    }
    return  gl_texture_id;
};

inline int SDLCALL SDL_GL_UnbindTexture(SDL_Texture* texture)
{
    if (texture == 0)
        return 1;
    Uint32 gl_texture_id;
    if (gl_texture_id = SDL_GetTextureProperties(texture)) {
        glActiveTexture(GL_TEXTURE0 + gl_texture_id);
        glBindTexture(GL_TEXTURE_2D, 0);
        return 0;
    }
    return 1;
}

int SDLCALL SDL_ConvertAudio(SDL_AudioCVT* cvt);

inline int SDLCALL SDL_QueryTexture(SDL_Texture* texture,
    Uint32* format, int* access,
    int* w, int* h)
{
    // 获取尺寸
    if (SDL_GetTextureSize(texture, (float*)w,(float*) h) != 0) {
        return -1;
    }
    //格式
    SDL_PropertiesID props = SDL_GetTextureProperties(texture);
    *format = SDL_GetNumberProperty(props, "texture.format", 0);
    *access = SDL_GetNumberProperty(props, "texture.access", 0);
}

inline int get_Surface_bitsPerPixel(SDL_Surface* s)
{
	//return surface->format.BitsPerPixel;//format 不是指针
    return (s->pitch / s->w) << 3;
}

#else

inline SDL_Color* get_Surface_Palette(SDL_Surface* sur)
{
    return sur->format->palette->colors;
}

inline int get_Surface_bitsPerPixel(SDL_Surface* surface)
{
    return surface->format->BitsPerPixel;
}

inline  SDL_Palette* SDLCALL SDL_GetSurfacePalette(SDL_Surface* surface)
{
    return surface->format->palette;
};

#endif