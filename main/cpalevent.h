#pragma once  
#include "sdl2_compat.h"  
#include <string>  

#ifdef _WIN32
#include <windows.h>
#include <sys/stat.h>
#endif

#include "command.h"  
#include "cconfig.h"  
#include "cpaleventhandler.h"  
#include "palgpgl.h"  
#include <vector>
#include "palsurface.h"

typedef unsigned char Uint8;  

typedef class GL_Render GL_Render;  
class CPalData;

class CPalEvent  
{  

    CPalEventHandler _m{};  
public:  
   CPalEvent();  
   ~CPalEvent();  
private:  
   int SDLCALL PAL_EventFilter(SDL_Event* lpEvent);  
   int PAL_PollEvent(SDL_Event* event);  

   inline static BOOL                   g_switch_WINDOW_FULLSCREEN_DESKTOP;  
   inline static int                    glSeed;  
   inline static volatile PALINPUTSTATE g_InputState;

   int                                  lastMouseDir{};//上次鼠标方向
public:  
   inline static BOOL           PalQuit;  
   inline static BOOL           fInBattle;  
   inline static std::string    PalDir;  
   inline static class CConfig* ggConfig;//用作全局变量
   inline static BOOL           PalSoundIn; //声音进程存在标志
   inline static bool           isTestRun;//调试开关标志
   inline static bool           isShowScript{true}; //是否显示脚本调试信息
   inline static int            debugSwitch[flsEnd]{};//调试开关
   inline static size_t         lastKeyDownTime{};//最后一次按键时间
   //鼠标相关变量
   inline static PAL_POS        g_InputMouse;//最后一次鼠标按下位置
   inline static bool		    isInScene;   //是否在场景中
   inline static std::vector<PAL_Rect>    mouseInputList;
   inline static PAL_Rect       mouseInputBoxRect;//鼠标输入框区域,以640*400为基准
   inline static int            mouseInputKey;//鼠标输入框点击区域 ，0 正常 点击返回 N+1
   inline static int            mouseInputStart;  //鼠标输入开始标志
   
   SDL_Rect                     gView{};       //当前视口

   class  CScript*              pCScript{};    //指向全局的指针
   const  CPalData*             p_DataCopy{};  //

   SDL_Palette* gpPalette{};  
   class CPalData* gpGlobals{};  
   bool  isRunEnd{};
public:  
   void KeydownEvent(const union SDL_Event* lpEvent);  
   void KeyupEvent(const union SDL_Event* lpEvent);  
   //鼠标事件处理函数
   void MouseDownEvent(const SDL_Event* lpEvent);
   void MouseUpEvent(const SDL_Event* lpEvent);
   void MouseWheelEvent(const SDL_Event* lpEvent);
   void MouseMotionEvent(const SDL_Event* lpEvent);

   VOID             PAL_ClearKeyState(VOID);  
   VOID             PAL_ClearKeyAll();
   unsigned long    getKeyPress(VOID)const;  
   VOID             setKeyPrevdir(const PALDIRECTION k);;
   static BOOL      get_switch_WINDOW_FULLSCREEN(VOID)  
   {  
       return g_switch_WINDOW_FULLSCREEN_DESKTOP;  
   }  
   VOID set_switch_WINDOW_FULLSCREEN_DESKTOP(int flag);  
   VOID PAL_ProcessEvent();  

   DWORD GetKeyPress() const { return g_InputState.dwKeyPress ; }
   void SetKeyPress(DWORD m) { g_InputState.dwKeyPress = m; }  

   PALDIRECTION getDirEction() const { return g_InputState.dir; }  
   void setDirEction(PALDIRECTION m) { g_InputState.dir = m; }  

   PALDIRECTION getPrevdirEction() const { return g_InputState.prevdir; }  
   void setPrevdirEction(PALDIRECTION m) { g_InputState.prevdir = m; }  

   VOID PAL_WaitForKey(WORD wTimeOut);  

public:  
   static char* va(const char* format, ...);  
   static void printMsg(const char* format, ...);
   static void* UTIL_calloc(size_t n, size_t size);

   VOID PAL_Delay(UINT32 t);  

   static int RandomLong(int min, int max);

   static float RandomFloat(float from, float to);

   static FILE* UTIL_OpenFile(LPCSTR lpszFileName)
   {
       std::string  fstr = PalDir + lpszFileName;
       FILE* fp;
       fp = fopen(fstr.c_str(), "rb");
       return fp;
   }
   static void  UTIL_CloseFile(FILE* fp)
   {
       if (fp)
       {
           fclose(fp);
       }
   }
   //返回指定的点是否在 t 中 0 不在，>0 返回 t中 n+1 的位置
   static int isRectsAtPoint(const std::vector<PAL_Rect>& t, PAL_POS p);

   static PalErr CheckHeapIntegrity()
#ifdef _WIN32 
   {
       HANDLE heap = GetProcessHeap();
       if (!HeapValidate(heap, 0, NULL))
           return TRUE;
       if (HeapCompact(heap, 0))
           return TRUE;
       return FALSE;
   }
#else
   {
       return FALSE;
   }
#endif

};
