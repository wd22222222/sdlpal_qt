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

#include "cpalui.h"
#include "cscript.h"
static BYTE PAL_CalcShadowColor( BYTE bSourceColor)
{
    return ((bSourceColor & 0xF0) | ((bSourceColor & 0x0F) >> 1));
}

CPalUI::CPalUI()
{
    //loadAllPalette();

    gpPalette = SDL_AllocPalette(256);
}
CPalUI::~CPalUI()
{
	if (gpPalette)
		SDL_FreePalette(gpPalette);
	gpPalette = nullptr;
}

INT CPalUI::PAL_RLEBlitToSurface(
    LPCBITMAPRLE      lpBitmapRLE,
    SDL_Surface*      lpDstSurface,
    PAL_POS           pos,
    BOOL              bShadow,
    BOOL              d  
)
/*++
  Purpose:

    Blit an RLE-compressed bitmap to an SDL surface.
    NOTE: Assume the surface is already locked, and the surface is a 8-bit one.

  Parameters:

    [IN]  lpBitmapRLE - pointer to the RLE-compressed bitmap to be decoded.

    [OUT] lpDstSurface - pointer to the destination SDL surface.

    [IN]  pos - position of the destination area.

    [IN]  bShadow - flag to mention whether blit source color or just shadow.

  Return value:

    0 = success, -1 = error.

--*/
{
    UINT          i, j, k, sx;
    INT           x, y;
    UINT          uiLen = 0;
    UINT          uiWidth = 0;
    UINT          uiHeight = 0;
    UINT          uiSrcX = 0;
    BYTE          T;
    INT           dx = PAL_X(pos);
    INT           dy = PAL_Y(pos);
    LPBYTE        p;

    //
    // Check for NULL pointer.
    //
    if (lpBitmapRLE == NULL || lpDstSurface == NULL)
    {
        return -1;
    }

    //
    // Skip the 0x00000002 in the file header.
    //
    if (lpBitmapRLE[0] == 0x02 && lpBitmapRLE[1] == 0x00 &&
        lpBitmapRLE[2] == 0x00 && lpBitmapRLE[3] == 0x00)
    {
        lpBitmapRLE += 4;
    }

    //
    // Get the width and height of the bitmap.
    //
    uiWidth = lpBitmapRLE[0] | (lpBitmapRLE[1] << 8);
    uiHeight = lpBitmapRLE[2] | (lpBitmapRLE[3] << 8);

    //
    // Check whether bitmap intersects the surface.
    //
    if (uiWidth + dx <= 0 || dx >= lpDstSurface->w ||
        uiHeight + dy <= 0 || dy >= lpDstSurface->h)
    {
        goto end;
    }

    //
    // Calculate the total length of the bitmap.
    // The bitmap is 8-bpp, each pixel will use 1 byte.
    //
    uiLen = uiWidth * uiHeight;

    //
    // Start decoding and blitting the bitmap.
    //
    lpBitmapRLE += 4;
    for (i = 0; i < uiLen;)
    {
        T = *lpBitmapRLE++;
        if ((T & 0x80) && T <= 0x80 + uiWidth)
        {
            i += T - 0x80;
            uiSrcX += T - 0x80;
            if (uiSrcX >= uiWidth)
            {
                uiSrcX -= uiWidth;
                dy++;
            }
        }
        else
        {
            //
            // Prepare coordinates.
            //
            j = 0;
            sx = uiSrcX;
            x = dx + uiSrcX;
            y = dy;

            //
            // Skip the points which are out of the surface.
            //
            if (y < 0)
            {
                j += -y * uiWidth;
                y = 0;
            }
            else if (y >= lpDstSurface->h)
            {
                goto end; // No more pixels needed, break out
            }

            while (j < T)
            {
                //
                // Skip the points which are out of the surface.
                //
                if (x < 0)
                {
                    j += -x;
                    if (j >= T) break;
                    sx += -x;
                    x = 0;
                }
                else if (x >= lpDstSurface->w)
                {
                    j += uiWidth - sx;
                    x -= sx;
                    sx = 0;
                    y++;
                    if (y >= lpDstSurface->h)
                    {
                        goto end; // No more pixels needed, break out
                    }
                    continue;
                }

                //
                // Put the pixels in row onto the surface
                //
                k = T - j;
                if (lpDstSurface->w - x < k) 
                    k = lpDstSurface->w - x;
                if (uiWidth - sx < k) 
                    k = uiWidth - sx;
                sx += k;
                p = ((LPBYTE)lpDstSurface->pixels) + y * lpDstSurface->pitch;
                if (bShadow)
                {
                    j += k;
                    for (; k != 0; k--)
                    {
                        p[x] = PAL_CalcShadowColor(p[x]);
                        /////
                        //if (p[x] == 0)p[x] = 1;
                        x++;
                    }
                }
                else
                {
                    for (; k != 0; k--)
                    {
                        p[x] = lpBitmapRLE[j];
                        /////
                        //if (p[x] == 0)p[x] = 1;
                        j++;
                        x++;
                    }
                }

                if (sx >= uiWidth)
                {
                    sx -= uiWidth;
                    x -= uiWidth;
                    y++;
                    if (y >= lpDstSurface->h)
                    {
                        goto end; // No more pixels needed, break out
                    }
                }
            }
            lpBitmapRLE += T;
            i += T;
            uiSrcX += T;
            while (uiSrcX >= uiWidth)
            {
                uiSrcX -= uiWidth;
                dy++;
            }
        }
    }

end:
    //
    // Success
    //
    return 0;
}

INT CPalUI::PAL_RLEBlitToSurface(LPCBITMAPRLE lpBitmapRLE, PalSurface& lpDstSurface, PAL_POS pos, BOOL bShadow, BOOL d)
{
    return PAL_RLEBlitToSurface(lpBitmapRLE, lpDstSurface.get(), pos, bShadow, d);
}

INT CPalUI::PAL_RLEBlitToSurface(LPCBITMAPRLE lpBitmapRLE, PalTexture &bDstTexture,
    PAL_POS pos, BOOL bShadow, BOOL d)
{
    
    if (lpBitmapRLE == NULL || bDstTexture.isNull())
    {
        return -1;
    }

    //
    // Skip the 0x00000002 in the file header.
    //
    if (lpBitmapRLE[0] == 0x02 && lpBitmapRLE[1] == 0x00 &&
        lpBitmapRLE[2] == 0x00 && lpBitmapRLE[3] == 0x00)
    {
        lpBitmapRLE += 4;
    }

    //
    // Get the width and height of the bitmap.
    //
    auto w = lpBitmapRLE[0] | (lpBitmapRLE[1] << 8);
    auto h = lpBitmapRLE[2] | (lpBitmapRLE[3] << 8);
    PalSurface s(gpPalette, w, h);
    PAL_RLEBlitToSurface(lpBitmapRLE, s.get(), 0, bShadow, d);
    PAL_Rect dRect(PAL_X(pos), PAL_Y(pos), w, h);
	dRect *= PictureRatio;
    pCScript->setTransColor(gpPalette->colors[255]);
    pCScript->RenderBlendCopy(pCScript->gpTextureReal, s, 255, RenderMode::rmMode3, 0, &dRect);
    return 0;
}



INT
CPalUI::PAL_RLEBlitWithColorShift(
    LPCBITMAPRLE      lpBitmapRLE,
    SDL_Surface*      lpDstSurface,
    PAL_POS           pos,
    INT               iColorShift,
    BOOL
)
/*++
  Purpose:

    Blit an RLE-compressed bitmap to an SDL surface.
    NOTE: Assume the surface is already locked, and the surface is a 8-bit one.

  Parameters:

    [IN]  lpBitmapRLE - pointer to the RLE-compressed bitmap to be decoded.

    [OUT] lpDstSurface - pointer to the destination SDL surface.

    [IN]  pos - position of the destination area.

    [IN]  iColorShift - shift the color by this value.

  Return value:

    0 = success, -1 = error.

--*/
{
    UINT          i, j, k, sx;
    INT           x, y;
    UINT          uiLen = 0;
    UINT          uiWidth = 0;
    UINT          uiHeight = 0;
    UINT          uiSrcX = 0;
    BYTE          T, b;
    INT           dx = PAL_X(pos);
    INT           dy = PAL_Y(pos);
    LPBYTE        p;

    //
    // Check for NULL pointer.
    //
    if (lpBitmapRLE == NULL || lpDstSurface == NULL)
    {
        return -1;
    }

    //
    // Skip the 0x00000002 in the file header.
    //
    if (lpBitmapRLE[0] == 0x02 && lpBitmapRLE[1] == 0x00 &&
        lpBitmapRLE[2] == 0x00 && lpBitmapRLE[3] == 0x00)
    {
        lpBitmapRLE += 4;
    }

    //
    // Get the width and height of the bitmap.
    //
    uiWidth = lpBitmapRLE[0] | (lpBitmapRLE[1] << 8);
    uiHeight = lpBitmapRLE[2] | (lpBitmapRLE[3] << 8);

    //
    // Check whether bitmap intersects the surface.
    //
    if (uiWidth + dx <= 0 || dx >= lpDstSurface->w ||
        uiHeight + dy <= 0 || dy >= lpDstSurface->h)
    {
        goto end;
    }

    //
    // Calculate the total length of the bitmap.
    // The bitmap is 8-bpp, each pixel will use 1 byte.
    //
    uiLen = uiWidth * uiHeight;

    //
    // Start decoding and blitting the bitmap.
    //
    lpBitmapRLE += 4;
    for (i = 0; i < uiLen;)
    {
        T = *lpBitmapRLE++;
        if ((T & 0x80) && T <= 0x80 + uiWidth)
        {
            i += T - 0x80;
            uiSrcX += T - 0x80;
            if (uiSrcX >= uiWidth)
            {
                uiSrcX -= uiWidth;
                dy++;
            }
        }
        else
        {
            //
            // Prepare coordinates.
            //
            j = 0;
            sx = uiSrcX;
            x = dx + uiSrcX;
            y = dy;

            //
            // Skip the points which are out of the surface.
            //
            if (y < 0)
            {
                j += -y * uiWidth;
                y = 0;
            }
            else if (y >= lpDstSurface->h)
            {
                goto end; // No more pixels needed, break out
            }

            while (j < T)
            {
                //
                // Skip the points which are out of the surface.
                //
                if (x < 0)
                {
                    j += -x;
                    if (j >= T) break;
                    sx += -x;
                    x = 0;
                }
                else if (x >= lpDstSurface->w)
                {
                    j += uiWidth - sx;
                    x -= sx;
                    sx = 0;
                    y++;
                    if (y >= lpDstSurface->h)
                    {
                        goto end; // No more pixels needed, break out
                    }
                    continue;
                }

                //
                // Put the pixels in row onto the surface
                //
                k = T - j;
                if (lpDstSurface->w - x < k) k = lpDstSurface->w - x;
                if (uiWidth - sx < k) k = uiWidth - sx;
                sx += k;
                p = ((LPBYTE)lpDstSurface->pixels) + y * lpDstSurface->pitch;
                for (; k != 0; k--)
                {
                    b = (lpBitmapRLE[j] & 0x0F);
                    if ((INT)b + iColorShift > 0x0F)
                    {
                        b = 0x0F;
                    }
                    else if ((INT)b + iColorShift < 0)
                    {
                        b = 0;
                    }
                    else
                    {
                        b += iColorShift;
                    }

                    p[x] = (b | (lpBitmapRLE[j] & 0xF0));
                    ///////.......由于0 是透明色采用1替代
                    if (!p[x])p[x] = 1;
                    j++;
                    x++;
                }

                if (sx >= uiWidth)
                {
                    sx -= uiWidth;
                    x -= uiWidth;
                    y++;
                    if (y >= lpDstSurface->h)
                    {
                        goto end; // No more pixels needed, break out
                    }
                }
            }
            lpBitmapRLE += T;
            i += T;
            uiSrcX += T;
            while (uiSrcX >= uiWidth)
            {
                uiSrcX -= uiWidth;
                dy++;
            }
        }
    }

end:
    //
    // Success
    //
    return 0;
}

INT CPalUI::PAL_RLEBlitWithColorShift(LPCBITMAPRLE lpBitmapRLE, PalTexture& lpDstSurface, PAL_POS pos, INT iColorShift, BOOL d)
{
    //
    // Check for NULL pointer.
    //
    if (lpBitmapRLE == NULL || lpDstSurface.isNull())
    {
        return -1;
    }

    //
    // Skip the 0x00000002 in the file header.
    //
    if (lpBitmapRLE[0] == 0x02 && lpBitmapRLE[1] == 0x00 &&
        lpBitmapRLE[2] == 0x00 && lpBitmapRLE[3] == 0x00)
    {
        lpBitmapRLE += 4;
    }

    //
    // Get the width and height of the bitmap.
    //
    auto uiWidth = lpBitmapRLE[0] | (lpBitmapRLE[1] << 8);
    auto uiHeight = lpBitmapRLE[2] | (lpBitmapRLE[3] << 8);
    
    auto p = PalSurface(gpPalette, uiWidth, uiHeight);
    PAL_RLEBlitWithColorShift(lpBitmapRLE, p.get(), 0, iColorShift, d);
    PAL_Rect dRect = { PAL_X(pos),PAL_Y(pos),uiWidth,uiHeight };
    dRect *= PictureRatio;
    return pCScript->RenderBlendCopy(lpDstSurface, p.get(), 255, RenderMode::rmMode3, nullptr, &dRect);
}


INT
CPalUI::PAL_FBPBlitToSurface(
    LPBYTE            lpBitmapFBP,
    SDL_Surface* lpDstSurface
)
/*++
  Purpose:

    Blit an uncompressed bitmap in FBP.MKF to an SDL surface.
    NOTE: Assume the surface is already locked, and the surface is a 8-bit 320x200 one.

  Parameters:

    [IN]  lpBitmapFBP - pointer to the RLE-compressed bitmap to be decoded.

    [OUT] lpDstSurface - pointer to the destination SDL surface.

  Return value:

    0 = success, -1 = error.

--*/
{
    int       x, y;
    //LPBYTE    p;

    if (lpBitmapFBP == NULL || lpDstSurface == NULL ||
        lpDstSurface->w != 320 || lpDstSurface->h != 200)
    {
        return -1;
    }

    //
    // simply copy everything to the surface
    //
    for (y = 0; y < 200; y++)
        if (get_Surface_bitsPerPixel(lpDstSurface) == 8)
        {
            LPBYTE p = (LPBYTE)(lpDstSurface->pixels) + y * lpDstSurface->pitch;
            for (x = 0; x < 320; x++)
            {
                *(p++) = *(lpBitmapFBP++);
            }
        }
        else
        {
            DWORD32 *p = (DWORD32*)(lpDstSurface->pixels) + y * lpDstSurface->pitch / 4;
            for (x = 0; x < 320; x++)
            {
                *(p++) = *((DWORD32*)(&gpPalette->colors[*(lpBitmapFBP++)]));
            }
        }

    return 0;
}

INT CPalUI::PAL_FBPBlitToSurface(LPBYTE lpBitmapFBP, PalTexture& lpDstSurface)
{
    PalSurface m(gpPalette, 320, 200);
    PAL_FBPBlitToSurface(lpBitmapFBP, m.get());
    pCScript->RenderBlendCopy(pCScript->gpTextureReal, m);
    return SDL_GetError() != nullptr;
}



LPCBITMAPRLE
CPalUI::PAL_SpriteGetFrame(
    LPCSPRITE       lpSprite,
    INT             iFrameNum
)
/*++
  Purpose:

    Get the pointer to the specified frame from a sprite.

  Parameters:

    [IN]  lpSprite - pointer to the sprite.

    [IN]  iFrameNum - number of the frame.

  Return value:

    Pointer to the specified frame. NULL if the frame does not exist.

--*/
{
    int imagecount, offset;

    if (lpSprite == NULL)
    {
        return NULL;
    }
    //PAL_LoadGame
    //
    // Hack for broken sprites like the Bloody-Mouth Bug
    //
 //   imagecount = (lpSprite[0] | (lpSprite[1] << 8)) - 1;
    imagecount = (lpSprite[0] | (lpSprite[1] << 8));

    if (iFrameNum < 0 || iFrameNum >= imagecount)
    {
        //
        // The frame does not exist
        //
        return NULL;
    }

    //
    // Get the offset of the frame
    //
    iFrameNum <<= 1;
    offset = ((lpSprite[iFrameNum] | (lpSprite[iFrameNum + 1] << 8)) << 1);
    if (offset == 0x18444) 
        offset = (WORD)offset;
    return &lpSprite[offset];
}

UINT CPalUI::PAL_RLEGetWidth(LPCBITMAPRLE lpBitmapRLE)
/*++
Purpose:

Get the width of an RLE-compressed bitmap.

Parameters:

[IN]  lpBitmapRLE - pointer to an RLE-compressed bitmap.

Return value:

Integer value which indicates the height of the bitmap.

--*/
{
    if (lpBitmapRLE == NULL)
    {
        return 0;
    }

    //
    // Skip the 0x00000002 in the file header.
    //
    if (lpBitmapRLE[0] == 0x02 && lpBitmapRLE[1] == 0x00 &&
        lpBitmapRLE[2] == 0x00 && lpBitmapRLE[3] == 0x00)
    {
        lpBitmapRLE += 4;
    }

    //
    // Return the width of the bitmap.
    //
    return lpBitmapRLE[0] | (lpBitmapRLE[1] << 8);
}

UINT CPalUI::PAL_RLEGetHeight(LPCBITMAPRLE lpBitmapRLE)
/*++
Purpose:

Get the height of an RLE-compressed bitmap.

Parameters:

[IN]  lpBitmapRLE - pointer of an RLE-compressed bitmap.

Return value:

Integer value which indicates the height of the bitmap.

--*/
{
    if (lpBitmapRLE == NULL)
    {
        return 0;
    }

    //
    // Skip the 0x00000002 in the file header.
    //
    if (lpBitmapRLE[0] == 0x02 && lpBitmapRLE[1] == 0x00 &&
        lpBitmapRLE[2] == 0x00 && lpBitmapRLE[3] == 0x00)
    {
        lpBitmapRLE += 4;
    }

    //
    // Return the height of the bitmap.
    //
    return lpBitmapRLE[2] | (lpBitmapRLE[3] << 8);
}

