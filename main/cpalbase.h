#ifndef CPALAPP_H
#define CPALAPP_H

#include "command.h"
#include "palgpgl.h"
#include "cpalevent.h"
#include <string>
#include <time.h>
#include <random>

//本类定义文件系统相关

#define MENUITEM_COLOR                     0x4F
#define MENUITEM_COLOR_INACTIVE            0x1C
#define MENUITEM_COLOR_CONFIRMED           0x2C
#define MENUITEM_COLOR_SELECTED_INACTIVE   0x1F
#define MENUITEM_COLOR_SELECTED_FIRST      0xF9
#define MENUITEM_COLOR_SELECTED_TOTALNUM   6

#define MENUITEM_COLOR_SELECTED                                    \
   (MENUITEM_COLOR_SELECTED_FIRST +                                \
      SDL_GetTicks_New() / (600 / MENUITEM_COLOR_SELECTED_TOTALNUM)    \
      % MENUITEM_COLOR_SELECTED_TOTALNUM)

#define MENUITEM_COLOR_EQUIPPEDITEM        0xC8


//基本磁盘IO类，不需要调用其他类
class CPalBase:public CPalEvent
{
public:

public:
    CPalBase();
    ~CPalBase();
    PalErr InitPalBase();

    VOID destroyPalBase();

    static BOOL isCorrectDir(const std::string &dir);
    
    static INT PAL_MKFGetChunkSize(UINT uiChunkNum,const ByteArray& fp);
    static INT PAL_MKFGetChunkCount(const ByteArray& fp); 

    static int pByteToInt(LPCBYTE p);

    static INT PAL_MKFReadChunk(
        LPBYTE          lpBuffer,
        UINT            uiBufferSize,
        UINT            uiChunkNum,
        const ByteArray&      fp
    );
    static ByteArray PAL_MKFReadChunk(UINT uiChunkNum, const ByteArray& fp);

    static ByteArray PAL_MKFDecompressChunk(UINT uiChunkNum,const ByteArray& fp);

    static INT PAL_MKFDecompressChunk(
        LPBYTE          lpBuffer,
        UINT            uiBufferSize,
        UINT            uiChunkNum,
        const ByteArray&      fp
    );

    static INT PAL_MKFGetDecompressedSize(
        UINT    uiChunkNum,
        const ByteArray& fp
    );

    VOID UTIL_CloseFile(FILE* fp);
    static INT PAL_RNGReadFrame(LPBYTE lpBuffer, UINT uiBufferSize,
        UINT uiRngNum, UINT uiFrameNum, const ByteArray& fpRngMKF);

    std::string PAL_GetMsg(WORD wNumMsg);
    
 
    //读取说明
    std::string PAL_GetObjectDesc(LPOBJECTDESC   lpObjectDesc, WORD  wObjectID);
    std::string PAL_GetWord(WORD wNumWord);
    //载入说明
    //LPOBJECTDESC PAL_LoadObjectDesc(LPCSTR lpszFileName);


public:
    //
};

#endif // CPALAPP_H
