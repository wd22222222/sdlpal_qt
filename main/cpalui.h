#ifndef CPALUI_H
#define CPALUI_H
#include "cpalbase.h"
#include "palsurface.h"
#include "paltexture.h"
//#include "cpalevent.h"
//#include "cgl_render.h"
//#include "sdl2_compat.h"

//本类继承基本OI 操作的 CPalApp 实现与SDL相关操作
class CPalUI : public CPalBase
{
public:
    CPalUI();
	~CPalUI();
    INT PAL_RLEBlitToSurface(LPCBITMAPRLE lpBitmapRLE,
        SDL_Surface* lpDstSurface, PAL_POS pos, BOOL bShadow = FALSE, BOOL d = TRUE);
    INT PAL_RLEBlitToSurface(LPCBITMAPRLE lpBitmapRLE,
        PalSurface& lpDstSurface, PAL_POS pos, BOOL bShadow = FALSE, BOOL d = TRUE);;
    INT PAL_RLEBlitToSurface(LPCBITMAPRLE lpBitmapRLE,
        PalTexture&  bDstTexture, PAL_POS pos, BOOL bShadow = FALSE, BOOL d = TRUE);
    INT PAL_RLEBlitWithColorShift(LPCBITMAPRLE lpBitmapRLE, SDL_Surface* lpDstSurface, PAL_POS pos, INT iColorShift, BOOL d = TRUE);
    INT PAL_RLEBlitWithColorShift(LPCBITMAPRLE lpBitmapRLE, PalTexture& lpDstSurface, PAL_POS pos, INT iColorShift, BOOL d = TRUE);
    INT PAL_FBPBlitToSurface(LPBYTE lpBitmapFBP, SDL_Surface* lpDstSurface);
    INT PAL_FBPBlitToSurface(LPBYTE lpBitmapFBP, PalTexture& lpDstSurface);
    LPCBITMAPRLE PAL_SpriteGetFrame(LPCSPRITE lpSprite, INT iFrameNum);

public:
	UINT PAL_RLEGetWidth(LPCBITMAPRLE    lpBitmapRLE);
    UINT PAL_RLEGetHeight(LPCBITMAPRLE  lpBitmapRLE);
};

#endif // CPALUI_H
