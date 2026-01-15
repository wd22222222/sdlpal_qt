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

// PalApp.cpp: 定义应用程序的类行为。
//

#include <assert.h>
#include <crtdbg.h>
#include <string>
#include <sstream>
#include "cpalbase.h"
#include "cconfig.h"
#include "convers.h"
#include "cpaldata.h"
#define _CRTDBG_MAP_ALLOC
#ifdef _DEBUG
#define new new(_NORMAL_BLOCK , __FILE__ , __LINE__)
//#define new DEBUG_NEW
#endif


//---------------------------------------------------------------------------

#define SWAP16(X) (X)
#define SWAP32(x)  (x)


void
TerminateOnError(
    const char *fmt,
    ...
)
// This function terminates the game because of an error and
// prints the message string pointed to by fmt both in the
// console and in a messagebox.
{
    va_list argptr;
    char string[256];
    //extern VOID PAL_Shutdown(VOID);

    // concatenate all the arguments in one string
    va_start(argptr, fmt);
    vsnprintf(string, sizeof(string), fmt, argptr);
    va_end(argptr);


    //fprintf(stderr, "\nFATAL ERROR: %s\n", string);

#ifdef _DEBUG
    //assert(!"TerminateOnError()"); // allows jumping to debugger
#endif
    exit(14);

    //PAL_Shutdown();
}

int CPalBase::pByteToInt(LPCBYTE p) {
    return (int)(p[0] +
        (p[1] << 8) + (p[2] << 16) + (p[3] << 24));
}




//FILE* UTIL_OpenRequiredFile(LPCSTR  lpszFileName);

INT CPalBase::PAL_MKFGetChunkSize(UINT uiChunkNum,const ByteArray& fp)
{
    return CPalData::PAL_MKFGetChunkSize(uiChunkNum, fp);
}

INT CPalBase::PAL_MKFGetChunkCount(const ByteArray& fp)
{
    if (fp.empty())
    {
        return 0;
    }
    return pByteToInt(fp.data()) / 4 - 1;
}

INT CPalBase::PAL_MKFReadChunk(
    LPBYTE          lpBuffer,
    UINT            uiBufferSize,
    UINT            uiChunkNum,
    const ByteArray&      fp
)
{
    UINT     uiOffset = 0;
    UINT     uiNextOffset = 0;
    UINT     uiChunkCount = 0;
    UINT     uiChunkLen = 0;

    if (lpBuffer == NULL || fp.empty() || uiBufferSize == 0)
    {
        return -1;
    }

    //
    // Get the total number of chunks.
    //
    uiChunkCount = PAL_MKFGetChunkCount(fp);
    if (uiChunkNum >= uiChunkCount)
    {
        return -1;
    }

    //
    // Get the offset of the chunk.
    //
    uiOffset = pByteToInt(fp.data()+ 4 * uiChunkNum);
    uiNextOffset = pByteToInt(fp.data() + 4 * uiChunkNum + 4);
    uiOffset = SWAP32(uiOffset);
    uiNextOffset = SWAP32(uiNextOffset);

    //
    // Get the length of the chunk.
    //
    uiChunkLen = uiNextOffset - uiOffset;

    if (uiChunkLen > uiBufferSize)
    {
        return -2;
    }

    if (uiChunkLen != 0)
    {
         memcpy(lpBuffer, fp.data() + uiOffset, uiChunkLen);
    }
    else
    {
        return -1;
    }

    return (INT)uiChunkLen;
}

ByteArray CPalBase::PAL_MKFReadChunk(UINT uiChunkNum, const ByteArray& fp)
{
    ByteArray buf;
    int len = PAL_MKFGetChunkSize(uiChunkNum, fp);
    if (len <= 0 )
        return ByteArray();
    buf.resize(len);
    if (PAL_MKFReadChunk(buf.data(), len, uiChunkNum, fp) <= 0)
        return ByteArray();
    return buf;
}

INT CPalBase::PAL_MKFGetDecompressedSize(
    UINT    uiChunkNum,
    const ByteArray & fp
)
/*++
  Purpose:

  Get the decompressed size of a compressed chunk in an MKF archive.

  Parameters:

  [IN]  uiChunkNum - the number of the chunk in the MKF archive.

  [IN]  fp - pointer to the fopen'ed MKF file.

  Return value:

  Integer value which indicates the size of the chunk.
  -1 if the chunk does not exist.

  --*/
{
    //auto& gConfig = gpGlobals->gConfig;
    DWORD         buf[2]{};
    if (fp.empty())
    {
        return -1;
    }

    //
    // Get the total number of chunks.
    //
    auto uiChunkCount = PAL_MKFGetChunkCount(fp);
    if (uiChunkNum >= uiChunkCount)
    {
        return -1;
    }

    //
    // Get the offset of the chunk.
    //
    //fseek(fp, 4 * uiChunkNum, SEEK_SET);
    //fread(&uiOffset, 4, 1, fp);
    auto uiOffset = pByteToInt(fp.data() + 4 * uiChunkNum);
    uiOffset = SWAP32(uiOffset);
    auto uiLast = pByteToInt(fp.data() + 4 * (uiChunkNum + 1));
    uiLast = SWAP32(uiLast);
    if (uiOffset == uiLast)
        return 0;
    assert(CConfig::fisUSEYJ1DeCompress != -1);
    if (CConfig::fisUSEYJ1DeCompress == 0)
    {
        //fread(buf, sizeof(DWORD), 1, fp);
        //再读取下一个
        buf[0] = pByteToInt(fp.data() + uiOffset);
        buf[0] = SWAP32(buf[0]);

        return (INT)buf[0];
    }
    else
    {
        buf[0] = pByteToInt(fp.data() + uiOffset);
        buf[1] = pByteToInt(fp.data() + uiOffset + 4);
        buf[0] = SWAP32(buf[0]);
        buf[1] = SWAP32(buf[1]);
        //检测标识
        return (buf[0] != 0x315f4a59) ? -1 : (INT)buf[1];
    }
}

INT CPalBase::PAL_MKFDecompressChunk(
    LPBYTE          lpBuffer,
    UINT            uiBufferSize,
    UINT            uiChunkNum,
    const ByteArray&   fp
)
{
    //LPBYTE          buf;
    int             len;

    len = PAL_MKFGetChunkSize(uiChunkNum, fp);

    if (len <= 0)
    {
        return len;
    }

    auto buf = ByteArray(len);

    PAL_MKFReadChunk(buf.data(), len, uiChunkNum, fp);

    len = CPalData::PAL_DeCompress(buf.data(), lpBuffer, uiBufferSize);

    return len;
}

ByteArray CPalBase::PAL_MKFDecompressChunk(UINT uiChunkNum, const ByteArray& fp)
{
    int len = PAL_MKFGetDecompressedSize(uiChunkNum, fp);
    if (len <= 0)
    {
        return ByteArray();
    }
    ByteArray buf;
    buf.resize(len);
    if (PAL_MKFDecompressChunk(buf.data(), len, uiChunkNum, fp) <= 0)
        return ByteArray();
    return buf;
}


VOID CPalBase::UTIL_CloseFile(	FILE *fp )
/*++
  Purpose:

    Close a file.

  Parameters:

    [IN]  fp - file handle to be closed.

  Return value:

    None.

--*/
{
    if (fp != NULL)
    {
        fclose(fp);
    }
}


std::string CPalBase::PAL_GetObjectDesc(LPOBJECTDESC   lpObjectDesc,	WORD  wObjectID)
/*++
  Purpose:

    Get the object description string from the linked list.

  Parameters:

    [IN]  lpObjectDesc - the description data linked list.

    [IN]  wObjectID - the object ID.

  Return value:

    The description string. NULL if the specified object ID
    is not found.

--*/
{
    if (lpObjectDesc == NULL)
        return std::string();
    return lpObjectDesc->p[wObjectID];
}


std::string CPalBase::PAL_GetWord(WORD wNumWord) {
    if (wNumWord == 0)
        return std::string();
    if (gpGlobals)
        return gpGlobals->PAL_GetWord(wNumWord);
    return std::string();
}


std::string CPalBase::PAL_GetMsg(WORD  wNumMsg
)
/*++
  Purpose:

  Get the specified message.

  Parameters:

  [IN]  wNumMsg - the number of the requested message.

  Return value:

  Pointer to the requested message. NULL if not found.

  --*/
{
    auto g_TextLib = gpGlobals->g_TextLib;

    static char    buf[256];
    DWORD          dwOffset, dwSize;

    if (wNumMsg >= g_TextLib.nMsgs)
    {
        return std::string();
    }

    dwOffset = SWAP32(g_TextLib.lpMsgOffset[wNumMsg]);
    dwSize = SWAP32(g_TextLib.lpMsgOffset[wNumMsg + 1]) - dwOffset;
    assert(dwSize < 255);

    memcpy(buf, &gpGlobals->g_TextLib.lpMsgBuf[dwOffset], dwSize);
    buf[dwSize] = '\0';
    //assert(strlen(buf));
    return gpGlobals->PAL_TextToUTF8( buf);
}

INT CPalBase::PAL_RNGReadFrame(
    LPBYTE          lpBuffer,
    UINT            uiBufferSize,
    UINT            uiRngNum,
    UINT            uiFrameNum,
    const ByteArray&      fp
)
{
    UINT         uiOffset = 0;
    UINT         uiSubOffset = 0;
    UINT         uiNextOffset = 0;
    UINT         uiChunkCount = 0;
    INT          iChunkLen = 0;

    if (lpBuffer == NULL || fp.empty() || uiBufferSize == 0)
    {
        return -1;
    }

    //
    // Get the total number of chunks.
    //
    uiChunkCount = PAL_MKFGetChunkCount(fp);
    if (uiRngNum >= uiChunkCount)
    {
        return -1;
    }

    //
    // Get the offset of the chunk.
    //
    //fseek(fpRngMKF, 4 * uiRngNum, SEEK_SET);
    //fread(&uiOffset, sizeof(UINT), 1, fpRngMKF);
    //fread(&uiNextOffset, sizeof(UINT), 1, fpRngMKF);
    uiOffset = pByteToInt(fp.data() + 4 * uiRngNum);
    uiNextOffset = pByteToInt(fp.data() + 4 * uiRngNum + 4);
    uiOffset = SWAP32(uiOffset);
    uiNextOffset = SWAP32(uiNextOffset);

    //
    // Get the length of the chunk.
    //
    iChunkLen = uiNextOffset - uiOffset;
    if (iChunkLen <= 0)
    {
        return -1;
    }

    //
    // Get the number of sub chunks.
    //
    //fread(&uiChunkCount, sizeof(UINT), 1, fpRngMKF);
    uiChunkCount = pByteToInt(fp.data() + uiOffset);
    uiChunkCount = (SWAP32(uiChunkCount) - 4) / 4;
    if (uiFrameNum >= uiChunkCount)
    {
        return -1;
    }

    //
    // Get the offset of the sub chunk.
    //
    //fseek(fpRngMKF, uiOffset + 4 * uiFrameNum, SEEK_SET);
    //fread(&uiSubOffset, sizeof(UINT), 1, fpRngMKF);
    //fread(&uiNextOffset, sizeof(UINT), 1, fpRngMKF);
    uiSubOffset = pByteToInt(fp.data() + uiOffset + 4 * uiFrameNum);
    uiNextOffset = pByteToInt(fp.data() + uiOffset + 4 * uiFrameNum + 4);
    uiSubOffset = SWAP32(uiSubOffset);
    uiNextOffset = SWAP32(uiNextOffset);

    //
    // Get the length of the sub chunk.
    //
    iChunkLen = uiNextOffset - uiSubOffset;
    if ((UINT)iChunkLen > uiBufferSize)
    {
        return -2;
    }

    if (iChunkLen != 0)
    {
        //fseek(fpRngMKF, uiOffset + uiSubOffset, SEEK_SET);
        //fread(lpBuffer, iChunkLen, 1, fpRngMKF);
        memcpy(lpBuffer, fp.data() + uiOffset + uiSubOffset, iChunkLen);
    }
    else
    {
        return -1;
    }

    return iChunkLen;
}


