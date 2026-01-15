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

#include "cpalbaseio.h"
#include "cpaldata.h"
#include "caviplay.h"
#include <string>
#include "palsurface.h"

#define MAINMENU_BACKGROUND_FBPNUM (CPalEvent::ggConfig->fIsWIN95 ? 2 :60)
#define CONFIRMMENU_LABEL_NO               19
#define CONFIRMMENU_LABEL_YES              20


#define RIX_NUM_OPENINGMENU                4
#define MAINMENU_LABEL_NEWGAME             7
#define MENUITEM_COLOR                     0x4F
#define MENUITEM_COLOR_INACTIVE            0x1C
#define MENUITEM_COLOR_CONFIRMED           0x2C
#define MENUITEM_COLOR_SELECTED_INACTIVE   0x1F
#define MENUITEM_COLOR_SELECTED_FIRST      0xF9
#define MENUITEM_COLOR_SELECTED_TOTALNUM   6
#define LOADMENU_LABEL_SLOT_FIRST          43

#define MENUITEM_COLOR_SELECTED                                    \
   (MENUITEM_COLOR_SELECTED_FIRST +                                \
      SDL_GetTicks_New() / (600 / MENUITEM_COLOR_SELECTED_TOTALNUM)    \
      % MENUITEM_COLOR_SELECTED_TOTALNUM)

#define MENUITEM_COLOR_EQUIPPEDITEM        0xC8

#define MAINMENU_LABEL_LOADGAME            8




INT CPalBaseIO::PAL_OpeningMenu(
	VOID
)
/*++
  Purpose:

  Show the opening menu.

  Parameters:

  None.

  Return value:

  Which saved slot to load from (1-5). 0 to start a new game.

  --*/
{
	WORD          wItemSelected{ 0 };
	WORD          wDefaultItem{ 0 };

	MENUITEM      rgMainMenuItem[2] = {
		// value   label                     enabled   position
			{0, MAINMENU_LABEL_NEWGAME, TRUE, PAL_XY(125, 95),PalSize(64,16)},
			{1, MAINMENU_LABEL_LOADGAME, TRUE, PAL_XY(125, 112),PalSize(64,16)}
	};

	//auto& gConfig = gpGlobals->gConfig;
	//
	// Play the background music
	//
    PAL_PlayMUS(RIX_NUM_OPENINGMENU, TRUE, 1);

	//
	// Draw the background
	//
	setAlpha(0);
	PAL_DrawOpeningMenuBackground();
	PAL_FadeIn(0, FALSE, 2);

	while(!PalQuit)
	{
		//
		wItemSelected = PAL_ReadMenu(rgMainMenuItem, 2, wDefaultItem, MENUITEM_COLOR, NULL);

		if (wItemSelected == 0 || wItemSelected == MENUITEM_VALUE_CANCELLED)
		{
			//
			// Start a new game
			//
			wItemSelected = 0;
			break;
		}
        else
		{
			//
			// Load game
			//
            wItemSelected = PAL_SaveSlotMenu(1);
			if (wItemSelected != MENUITEM_VALUE_CANCELLED)
			{
				break;
			}
			wDefaultItem = 1;
		}
	}
	ClearScreen();
	//
	// Fade out the screen and the music
	//
    PAL_PlayMUS(0, FALSE, 1);
	PAL_FadeOut(1);

if(ggConfig->fEnableAviPlay)//has avi
	{
		CAviPlay m;
		m.PAL_PlayAVI(va("%s/avi/3.AVI", PalDir.c_str()));
	}

	return (INT)wItemSelected;
}

VOID CPalBaseIO::PAL_DrawOpeningMenuBackground(VOID)
/*++
  Purpose:

  Draw the background of the main menu.

  Parameters:

  None.

  Return value:

  None.

  --*/
{
	ByteArray     buf(320 * 200);
	PAL_SetPalette(0, FALSE);
	PalSurface mSurf(gpPalette, 320, 200);
	//
	// Read the picture from fbp.mkf.
	//
	PAL_MKFDecompressChunk(buf.data(), 320 * 200, MAINMENU_BACKGROUND_FBPNUM, gpGlobals->f.fpFBP);

	//
	// ...and blit it to the screen buffer.
	//
	PAL_FBPBlitToSurface(buf.data(), mSurf.get());
	RenderBlendCopy(gpTextureReal, mSurf.get());
}


WORD CPalBaseIO::PAL_ReadMenu(
	LPMENUITEM                rgMenuItem,
	INT                       nMenuItem,
	WORD                      wDefaultItem,
	BYTE                      bLabelColor,
	LPITEMCHANGED_CALLBACK    lpfnMenuItemChanged
)
/*++
  Purpose:

	Execute a menu.

  Parameters:

	[IN]  lpfnMenuItemChanged - Callback function which is called when user
								changed the current menu item.

	[IN]  rgMenuItem - Array of the menu items.

	[IN]  nMenuItem - Number of menu items.

	[IN]  wDefaultItem - default item index.

	[IN]  bLabelColor - color of the labels.

  Return value:

	Return value of the selected menu item. MENUITEM_VALUE_CANCELLED if cancelled.

--*/
{
	int               i;
	WORD              wCurrentItem = (wDefaultItem < nMenuItem) ? wDefaultItem : 0;

	PAL_ClearKeyAll();
	isInScene = FALSE;
	std::vector <PAL_Rect> itemRects;
	for (int n = 0; n < nMenuItem; n++)
	{
		itemRects.push_back(PAL_Rect(rgMenuItem[n].pos, rgMenuItem[n].size));
	}

	while(!PalQuit)
	{
		isInScene = FALSE;
		//
		// Draw all the menu texts.
		//
		for (i = 0; i < nMenuItem; i++)
		{
			BYTE bColor = bLabelColor;

			if (!rgMenuItem[i].fEnabled)
			{
				if (i == wCurrentItem)
				{
					bColor = MENUITEM_COLOR_SELECTED_INACTIVE;
				}
				else
				{
					bColor = MENUITEM_COLOR_INACTIVE;
				}
			}

			PAL_DrawText(PAL_GetWord(rgMenuItem[i].wNumWord), rgMenuItem[i].pos,
				bColor, TRUE, TRUE);
		}

		if (lpfnMenuItemChanged != NULL)
		{
			lpfnMenuItemChanged(rgMenuItem[wCurrentItem].wValue);
		}

		if (rgMenuItem[wCurrentItem].fEnabled)
		{
			PAL_DrawText(PAL_GetWord(rgMenuItem[wCurrentItem].wNumWord),
				rgMenuItem[wCurrentItem].pos, MENUITEM_COLOR_SELECTED, FALSE, TRUE);
		}

		PAL_ProcessEvent();
		if (int item = isRectsAtPoint(itemRects, g_InputMouse))
		{
			if (wCurrentItem == item - 1)
				//已经选择，再次点击确认
			{
				return rgMenuItem[wCurrentItem].wValue;
			}
			wCurrentItem = item - 1;
			PAL_ClearKeyAll();
		};
		if (getKeyPress())
		{
			if (getKeyPress() & (kKeyDown | kKeyRight))
			{
				//
				// User pressed the down or right arrow key
				//
				if (rgMenuItem[wCurrentItem].fEnabled)
				{
					//
					// Dehighlight the unselected item.
					//
					PAL_DrawText(PAL_GetWord(rgMenuItem[wCurrentItem].wNumWord),
						rgMenuItem[wCurrentItem].pos, bLabelColor, FALSE, TRUE);
				}
				else
				{
					PAL_DrawText(PAL_GetWord(rgMenuItem[wCurrentItem].wNumWord),
						rgMenuItem[wCurrentItem].pos, MENUITEM_COLOR_INACTIVE, FALSE, TRUE);
				}

				wCurrentItem++;

				if (wCurrentItem >= nMenuItem)
				{
					wCurrentItem = 0;
				}

				//
				// Highlight the selected item.
				//
				if (rgMenuItem[wCurrentItem].fEnabled)
				{
					PAL_DrawText(PAL_GetWord(rgMenuItem[wCurrentItem].wNumWord),
						rgMenuItem[wCurrentItem].pos, MENUITEM_COLOR_SELECTED, FALSE, TRUE);
				}
				else
				{
					PAL_DrawText(PAL_GetWord(rgMenuItem[wCurrentItem].wNumWord),
						rgMenuItem[wCurrentItem].pos, MENUITEM_COLOR_SELECTED_INACTIVE, FALSE, TRUE);
				}
			}
			else if (getKeyPress() & (kKeyUp | kKeyLeft))
			{
				//
				// User pressed the up or left arrow key
				//
				if (rgMenuItem[wCurrentItem].fEnabled)
				{
					//
					// Dehighlight the unselected item.
					//
					PAL_DrawText(PAL_GetWord(rgMenuItem[wCurrentItem].wNumWord),
						rgMenuItem[wCurrentItem].pos, bLabelColor, FALSE, TRUE);
				}
				else
				{
					PAL_DrawText(PAL_GetWord(rgMenuItem[wCurrentItem].wNumWord),
						rgMenuItem[wCurrentItem].pos, MENUITEM_COLOR_INACTIVE, FALSE, TRUE);
				}

				if (wCurrentItem > 0)
				{
					wCurrentItem--;
				}
				else
				{
					wCurrentItem = nMenuItem - 1;
				}

				//
				// Highlight the selected item.
				//
				if (rgMenuItem[wCurrentItem].fEnabled)
				{
					PAL_DrawText(PAL_GetWord(rgMenuItem[wCurrentItem].wNumWord),
						rgMenuItem[wCurrentItem].pos, MENUITEM_COLOR_SELECTED, FALSE, TRUE);
				}
				else
				{
					PAL_DrawText(PAL_GetWord(rgMenuItem[wCurrentItem].wNumWord),
						rgMenuItem[wCurrentItem].pos, MENUITEM_COLOR_SELECTED_INACTIVE, FALSE, TRUE);
				}

				if (lpfnMenuItemChanged != NULL)
				{
					(lpfnMenuItemChanged)(rgMenuItem[wCurrentItem].wValue);
				}
			}
			else if (getKeyPress() & kKeyMenu)
			{
				//
				// User cancelled
				//
				if (rgMenuItem[wCurrentItem].fEnabled)
				{
					PAL_DrawText(PAL_GetWord(rgMenuItem[wCurrentItem].wNumWord),
						rgMenuItem[wCurrentItem].pos, bLabelColor, FALSE, TRUE);
				}
				else
				{
					PAL_DrawText(PAL_GetWord(rgMenuItem[wCurrentItem].wNumWord),
						rgMenuItem[wCurrentItem].pos, MENUITEM_COLOR_INACTIVE, FALSE, TRUE);
				}

				break;
			}
			else if (getKeyPress() & kKeySearch)
			{
				//
				// User pressed Enter
				//
				if (rgMenuItem[wCurrentItem].fEnabled)
				{
					PAL_DrawText(PAL_GetWord(rgMenuItem[wCurrentItem].wNumWord),
						rgMenuItem[wCurrentItem].pos, MENUITEM_COLOR_CONFIRMED, FALSE, TRUE);

					return rgMenuItem[wCurrentItem].wValue;
				}
			}
			VIDEO_UpdateScreen(gpTextureReal);
			ClearScreen();
			PAL_ClearKeyAll();
		}
		//
		// Use delay function to avoid high CPU usage.
		//
		PAL_Delay(20);
	}
	//ClearScreen();
	return MENUITEM_VALUE_CANCELLED;
}


INT CPalBaseIO::PAL_SaveSlotMenu(
	WORD        wDefaultSlot
)
/*++
  Purpose:

  Show the load game menu.

  Parameters:

  [IN]  wDefaultSlot - default save slot number (1-5).
  [IN]  BIsSave - is save true save?
  Return value:

  Which saved slot to load from (1-5). MENUITEM_VALUE_CANCELLED if cancelled.

  --*/
{
	auto maxSaveFile = ggConfig->m_Function_Set[47] + ggConfig->m_Function_Set[53];
	//定义选择按键函数
	auto getKey = [this](int& menuItem,int& topLine,int maxLine)->int {
		PAL_WaitForKey(2);
		if ((GetKeyPress() & kKeyUp) || (GetKeyPress() & kKeyLeft))
		{
			if (menuItem > 0)
			{
				menuItem--;
				if (topLine > menuItem)
					topLine--;
			}
		}
		else if ((GetKeyPress() & kKeyDown)||( GetKeyPress() & kKeyLeft))
		{
			if (menuItem < maxLine - 1)
			{
				menuItem++;
				if (menuItem >= topLine + 5)
					topLine++;
			}
		}
		else if ((GetKeyPress() & kKeyPgUp))
		{
			menuItem -= 5;		
			topLine -= 5;
			if (menuItem < 0)
				menuItem = 0;
			if (topLine < 0)
				topLine = 0;
		}
		else if (GetKeyPress() & kKeyPgDn && (menuItem < maxLine - 5))
		{
			menuItem += 5;
			topLine += 5;
			if (menuItem >= maxLine)
				menuItem = maxLine - 1;
			if (topLine >= maxLine - 5)
				topLine = maxLine - 5;

		}
		else if (GetKeyPress() & kKeyMenu)
		{
			menuItem = MENUITEM_VALUE_CANCELLED;
			return 1;
		}
		if (GetKeyPress() & kKeySearch)
		{
			menuItem++;
			return 1;
		}
		return 0;
		};

	//
	// Create the boxes and create the menu items
	//

	LPBOX cpBox = PAL_CreateBox(PAL_XY(195, 3), 8, 4, 1, TRUE);
	int wCurrentItem = wDefaultSlot - 1;
	int wFirstItem{};
	if (wCurrentItem >= 5)
		wFirstItem = (wCurrentItem / 5) * 5;
	SDL_Rect  rect = { 195, 3, cpBox->wWidth, cpBox->wHeight };
	PAL_ClearKeyAll();
	while (!PalQuit)
	{
		// Draw the numbers of saved times
		//
		std::vector<PAL_Rect> itemRects;
		for (size_t i = 0; i < 5; i++)
		{
			WORD bColor{};
			if ((i + wFirstItem) == wCurrentItem)
			{
				bColor = MENUITEM_COLOR_SELECTED_INACTIVE;
			}
			else
			{
				bColor = MENUITEM_COLOR_INACTIVE;
			};
			std::string wStr;
			/*
			if (i + wFirstItem < 10)
				wStr = PAL_GetWord(i + wFirstItem + 20012);
			else
			*/
			wStr = PAL_GetWord(20022) + va("%d", i + wFirstItem + 1
				- ggConfig->m_Function_Set[53]);//快速存档从0开始。普通从1 开始

			auto pos = PAL_XY(210, 12 + 35 * i);
			auto msize = PAL_DrawText(wStr, pos, bColor, TRUE, TRUE);
			itemRects.push_back(PAL_Rect( pos,msize));

			UINT wSavedTimes;
			if (gpGlobals->rgSaveData.at(i + wFirstItem).empty())
				wSavedTimes = 0;
			else
				wSavedTimes = *((WORD*)gpGlobals->rgSaveData.at(i + wFirstItem).data());
			//
			// Draw the number
			//
			PAL_DrawNumber((UINT)wSavedTimes, 4, pos + PAL_XY(55, 5),
				kNumColorYellow, kNumAlignRight, 10);
			
			//
		}
		auto oldItem = wCurrentItem;
		if(g_InputMouse)
		if (int item = isRectsAtPoint(itemRects, g_InputMouse))
		{
			
			if (wCurrentItem == item - 1 + wFirstItem)
				//已经选择，再次点击确认
			{
				wCurrentItem++;
				break;
			}
			wCurrentItem = item - 1 + wFirstItem;
			PAL_ClearKeyAll();
			continue;
		};

		if (getKey(wCurrentItem,wFirstItem,maxSaveFile))
			break;
		RenderPresent(gpTextureReal);
		//新的选择，已更新屏幕
		if (oldItem != wCurrentItem)
			PAL_CreateBox(PAL_XY(195, 3), 8, 4, 1, FALSE);
	}
	//
	// Delete the boxes
	//
	PAL_DeleteBox(cpBox);
	VIDEO_UpdateScreen(gpTextureReal);
	
	return wCurrentItem;
}

LPBOX CPalBaseIO::PAL_CreateBox(PAL_POS pos, INT nRows, INT nColumns, INT iStyle, BOOL fSaveScreen
)
/*++
  Purpose:

	Create a box on the screen.

  Parameters:

	[IN]  pos - position of the box.

	[IN]  nRows - number of rows of the box.

	[IN]  nColumns - number of columns of the box.

	[IN]  iStyle - style of the box (0 or 1).

	[IN]  fSaveScreen - whether save the used screen area or not.

  Return value:

	Pointer to a BOX structure. NULL if failed.
	If fSaveScreen is false, then always returns NULL.

--*/
{
	int              i, j, x, m, n;
	LPCBITMAPRLE     rglpBorderBitmap[3][3] = { 0 };
	LPBOX            lpBox = NULL;
	SDL_Rect         rect{0};
	auto& gpSpriteUI = gpGlobals->gpSpriteUI;

	//
	// Get the bitmaps
	//
	for (i = 0; i < 3; i++)
	{
		for (j = 0; j < 3; j++)
		{
			rglpBorderBitmap[i][j] = PAL_SpriteGetFrame(gpSpriteUI, i * 3 + j + iStyle * 9);
		}
	}

	rect.x = PAL_X(pos);
	rect.y = PAL_Y(pos);
	rect.w = 0;
	rect.h = 0;

	//
	// Get the total width and total height of the box
	//
	for (i = 0; i < 3; i++)
	{
		if (i == 1)
		{
			rect.w += PAL_RLEGetWidth(rglpBorderBitmap[0][i]) * nColumns;
			rect.h += PAL_RLEGetHeight(rglpBorderBitmap[i][0]) * nRows;
		}
		else
		{
			rect.w += PAL_RLEGetWidth(rglpBorderBitmap[0][i]);
			rect.h += PAL_RLEGetHeight(rglpBorderBitmap[i][0]);
		}
	}

	if (fSaveScreen)
	{
		//
		// Save the used part of the screen
		//
		SDL_Rect bRect;
		bRect = rect;
		setPictureRatio(&rect);
		auto save = PalTexture(rect.w, rect.h, textType::rgba32);

		if (save.isNull())
		{
			return NULL;
		}

		lpBox = new BOX;
		if (lpBox == NULL)
		{
			return NULL;
		}

		RenderBlendCopy(save, gpTextureReal, 255, RenderMode::rmMode1, 0, NULL, &rect);
		lpBox->lpSavedArea = std::move(save);
		//
		rect = bRect;
		lpBox->pos = pos;
		lpBox->wWidth = rect.w;
		lpBox->wHeight = rect.h;
	}

	//
	// Border takes 2 additional rows and columns...
	//
	nRows += 2;
	nColumns += 2;

	//
	// Draw the box
	//
	for (i = 0; i < nRows; i++)
	{
		x = rect.x;
		
		m = (i == 0) ? 0 : ((i == nRows - 1) ? 2 : 1);

		for (j = 0; j < nColumns; j++)
		{
			n = (j == 0) ? 0 : ((j == nColumns - 1) ? 2 : 1);
			PAL_RLEBlitToSurface(rglpBorderBitmap[m][n], gpTextureReal, PAL_XY(x, rect.y));
			x += PAL_RLEGetWidth(rglpBorderBitmap[m][n]);
		}

		rect.y += PAL_RLEGetHeight(rglpBorderBitmap[m][0]);
	}
	//RenderBlendCopy(gpTextureReal, gpScreen.get());
	return lpBox;
}

VOID CPalBaseIO::PAL_DeleteBox(LPBOX lpBox
)
/*++
  Purpose:

	Delete a box and restore the saved part of the screen.

  Parameters:

	[IN]  lpBox - pointer to the BOX struct.

  Return value:

	None.

--*/
{
	SDL_Rect        rect{0};

	//
	// Check for NULL pointer.
	//
	if (lpBox == NULL)
	{
		return;
	}

	//
	// Restore the saved screen part
	//
	rect.x = PAL_X(lpBox->pos);
	rect.y = PAL_Y(lpBox->pos);
	rect.w = lpBox->wWidth;
	rect.h = lpBox->wHeight;

	setPictureRatio(&rect);
	RenderBlendCopy(gpTextureReal, lpBox->lpSavedArea, 255, RenderMode::rmMode1, 0, &rect, 0);
	//
	// Free the memory used by the box
	//
	delete lpBox;
}

BOOL CPalBaseIO::PAL_ConfirmMenu(LPCSTR text)
/*++
  Purpose:

  Show a "Yes or No?" confirm box.


  Parameters:

  text Drow words if no NULL

  Return value:

  TRUE if user selected Yes, FALSE if selected No.

  --*/
{
	LPBOX           rgpBox[2] { 0 };
	LPBOX			textBox = NULL;
	MENUITEM        rgMenuItem[2]{0};
	int             i;
	WORD            wReturnValue;

	const SDL_Rect  rect = { 130, 90, 125, 50 };
	
	//备份调色版
	SDL_Color  oldPalette[256];
	memcpy(oldPalette, gpPalette->colors, sizeof(oldPalette));

	PAL_SetPalette(0, 0);

	//
	// Create menu items
	//
	rgMenuItem[0].fEnabled = TRUE;
	rgMenuItem[0].pos = PAL_XY(126, 110);
	rgMenuItem[0].wValue = 0;
	rgMenuItem[0].wNumWord = CONFIRMMENU_LABEL_NO;

	rgMenuItem[1].fEnabled = TRUE;
	rgMenuItem[1].pos = PAL_XY(200, 110);
	rgMenuItem[1].wValue = 1;
	rgMenuItem[1].wNumWord = CONFIRMMENU_LABEL_YES;

	if (text)
	{
		textBox = PAL_CreateSingleLineBox(PAL_XY(98, 66), 8, TRUE);
		PAL_DrawText(text, PAL_XY(108, 78), 0, FALSE, FALSE);
	}
	//
	// Create the boxes
	//
	for (i = 0; i < 2; i++)
	{
		rgpBox[i] = PAL_CreateSingleLineBox(PAL_XY(110 + 75 * i, 100), 2, TRUE);
		if (!rgpBox[i])
			return TRUE;;
		rgMenuItem[i].size = rgpBox[i]->getSize();
	}
	isInScene = FALSE;
	PAL_Delay(1);
	VIDEO_UpdateScreen(gpTextureReal);

	//
	// Activate the menu
	//
	wReturnValue = PAL_ReadMenu(rgMenuItem, 2, 1, MENUITEM_COLOR, NULL);

	//
	// Delete the boxes
	//
	for (i = 0; i < 2; i++)
	{
		PAL_DeleteBox(rgpBox[i]);
	}
	if (text)
		PAL_DeleteBox(textBox);
	VIDEO_UpdateScreen(gpTextureReal);
	//恢复调色版
	VIDEO_SetPalette(oldPalette);
	
	return (wReturnValue == MENUITEM_VALUE_CANCELLED || wReturnValue == 0) ? FALSE : TRUE;
}

LPBOX  CPalBaseIO::PAL_CreateSingleLineBox(
	PAL_POS        pos,
	INT            nLen,
	BOOL           fSaveScreen
)
/*++
  Purpose:

	Create a single-line box on the screen.

  Parameters:

	[IN]  pos - position of the box.

	[IN]  nLen - length of the box.

	[IN]  fSaveScreen - whether save the used screen area or not.

  Return value:

	Pointer to a BOX structure. NULL if failed.
	If fSaveScreen is false, then always returns NULL.

--*/
{
	static const int      iNumLeftSprite = 44;
	static const int      iNumMidSprite = 45;
	static const int      iNumRightSprite = 46;
	auto& gpSpriteUI = gpGlobals->gpSpriteUI;

	LPCBITMAPRLE          lpBitmapLeft;
	LPCBITMAPRLE          lpBitmapMid;
	LPCBITMAPRLE          lpBitmapRight;
	SDL_Rect              rect{0};
	LPBOX                 lpBox = NULL;
	int                   i;

	//
	// Get the bitmaps
	//
	lpBitmapLeft = PAL_SpriteGetFrame(gpSpriteUI, iNumLeftSprite);
	lpBitmapMid = PAL_SpriteGetFrame(gpSpriteUI, iNumMidSprite);
	lpBitmapRight = PAL_SpriteGetFrame(gpSpriteUI, iNumRightSprite);

	rect.x = PAL_X(pos);
	rect.y = PAL_Y(pos);

	//
	// Get the total width and total height of the box
	//
	rect.w = PAL_RLEGetWidth(lpBitmapLeft) + PAL_RLEGetWidth(lpBitmapRight);
	rect.w += PAL_RLEGetWidth(lpBitmapMid) * nLen;
	rect.h = PAL_RLEGetHeight(lpBitmapLeft);

	if (fSaveScreen)
	{
		//
		// Save the used part of the screen
		//
		SDL_Rect bRect = rect;
		setPictureRatio(&rect);
		auto save = PalTexture(rect.w, rect.h, textType::rgba32);
		if (save.isNull())
		{
			return NULL;
		}

		lpBox = new BOX;
		if (lpBox == NULL)
		{
			return NULL;
		}

		RenderBlendCopy(save, gpTextureReal, 255, RenderMode::rmMode1, 0, NULL, &rect);
		rect = bRect;
		lpBox->pos = pos;
		lpBox->lpSavedArea = std::move(save);
		lpBox->wHeight = (WORD)rect.h;
		lpBox->wWidth = (WORD)rect.w;
	}

	//
	// Draw the box
	//
	
	SDL_Color* pcolor = PAL_GetPalette(0, 0);

	PAL_RLEBlitToSurface(lpBitmapLeft, gpTextureReal, pos, 0, 1);

	rect.x += PAL_RLEGetWidth(lpBitmapLeft);

	for (i = 0; i < nLen; i++)
	{
		PAL_RLEBlitToSurface(lpBitmapMid, gpTextureReal, PAL_XY(rect.x, rect.y), 0, 1);
		rect.x += PAL_RLEGetWidth(lpBitmapMid);
	}

	PAL_RLEBlitToSurface(lpBitmapRight, gpTextureReal, PAL_XY(rect.x, rect.y), 0, 1);
	return lpBox;
}
