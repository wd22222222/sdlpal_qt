///* -*- mode: c; tab-width: 4; c-basic-offset: 4; c-file-style: "linux" -*- */
//
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

//soundplay.cpp
#include <cassert>
#include "soundplayer.h"


#define AUDIO_IsIntegerConversion(a) (((a) % lpAudioData->SampleRate) == 0 || (lpAudioData->SampleRate % (a)) == 0)

#define RIFF_RIFF (((uint32_t)'R') | (((uint32_t)'I') << 8) | (((uint32_t)'F') << 16) | (((uint32_t)'F') << 24))
#define RIFF_WAVE (((uint32_t)'W') | (((uint32_t)'A') << 8) | (((uint32_t)'V') << 16) | (((uint32_t)'E') << 24))
#define WAVE_fmt  (((uint32_t)'f') | (((uint32_t)'m') << 8) | (((uint32_t)'t') << 16) | (((uint32_t)' ') << 24))
#define WAVE_data (((uint32_t)'d') | (((uint32_t)'a') << 8) | (((uint32_t)'t') << 16) | (((uint32_t)'a') << 24))

typedef struct RIFFHeader
{
    uint32_t signature;         /* 'RIFF' */
    uint32_t length;            /* Total length minus eight, little-endian */
    uint32_t type;              /* 'WAVE', 'AVI ', ... */
} RIFFHeader;


typedef struct RIFFChunkHeader
{
    uint32_t type;             /* 'fmt ', 'hdrl', 'movi' and so on */
    uint32_t length;           /* Total chunk length minus eight, little-endian */
} RIFFChunkHeader;


typedef struct WAVEFormatPCM
{
    uint16_t wFormatTag;      /* format type */
    uint16_t nChannels;       /* number of channels (i.e. mono, stereo, etc.) */
    uint32_t nSamplesPerSec;  /* sample rate */
    uint32_t nAvgBytesPerSec; /* for buffer estimation */
    uint16_t nBlockAlign;     /* block size of data */
    uint16_t wBitsPerSample;
} WAVEFormatPCM;


typedef struct tagVOCHEADER
{
    char    signature[0x14];	/* "Creative Voice File\x1A" */
    WORD    data_offset;		/* little endian */
    WORD	version;
    WORD	version_checksum;
} VOCHEADER, * LPVOCHEADER;
typedef const VOCHEADER* LPCVOCHEADER;


typedef struct RIFFChunk
{
    RIFFChunkHeader header;
    uint8_t         data[1];
} RIFFChunk;

typedef struct RIFFListHeader
{
    uint32_t signature;        /* 'LIST' */
    uint32_t length;           /* Total list length minus eight, little-endian */
    uint32_t type;             /* 'fmt ', 'hdrl', 'movi' and so on */
} RIFFListHeader;

typedef union RIFFBlockHeader
{
    struct {
        uint32_t  type;
        uint32_t  length;
    };
    RIFFChunkHeader chunk;
    RIFFListHeader  list;
} RIFFBlockHeader;



static const void* SOUND_LoadVOCData(LPCBYTE lpData, DWORD dwLen, struct tagWAVESPEC* lpSpec)
/*++
  Purpose:

    Return the VOC data pointer inside the input buffer. Currently supports type 01 block only.

  Parameters:

    [IN]  lpData - pointer to the buffer of the VOC file.

    [IN]  dwLen - length of the buffer of the VOC file.

    [OUT] lpSpec - pointer to the SDL_AudioSpec structure, which contains
                   some basic information about the VOC file.

  Return value:

    Pointer to the WAVE data inside the input buffer, NULL if failed.

    Reference: http://sox.sourceforge.net/AudioFormats-11.html
--*/
{
    LPCVOCHEADER lpVOC = (LPCVOCHEADER)lpData;

    if (dwLen < sizeof(VOCHEADER) || memcmp(lpVOC->signature, "Creative Voice File\x1A", 0x14) || SDL_SwapLE16(lpVOC->data_offset) >= dwLen)
    {
        return NULL;
    }

    lpData += SDL_SwapLE16(lpVOC->data_offset);
    dwLen -= SDL_SwapLE16(lpVOC->data_offset);

    while (dwLen && *lpData)
    {
        DWORD len;
        if (dwLen >= 4)
        {
            len = lpData[1] | (lpData[2] << 8) | (lpData[3] << 16);
            if (dwLen >= len + 4)
                dwLen -= len + 4;
            else
                return NULL;
        }
        else
        {
            return NULL;
        }
        if (*lpData == 0x01)
        {
            if (lpData[5] != 0) return NULL;	/* Only 8-bit is supported */

            lpSpec->format = AUDIO_U8;
            lpSpec->channels = 1;
            lpSpec->freq = ((1000000 / (256 - lpData[4]) + 99) / 100) * 100; /* Round to next 100Hz */
            lpSpec->size = len - 2;
            lpSpec->align = 1;

            return lpData + 6;
        }
        else
        {
            lpData += len + 4;
        }
    }

    return NULL;
}

static const void* SOUND_LoadWAVEData(LPCBYTE lpData, DWORD dwLen, struct tagWAVESPEC* lpSpec)
{
    const RIFFHeader* lpRiff = (const RIFFHeader*)lpData;
    const RIFFChunkHeader* lpChunk = NULL;
    const WAVEFormatPCM* lpFormat = NULL;
    const uint8_t* lpWaveData = NULL;
    uint32_t len, type;

    if (dwLen < sizeof(RIFFHeader) || SDL_SwapLE32(lpRiff->signature) != RIFF_RIFF ||
        SDL_SwapLE32(lpRiff->type) != RIFF_WAVE || dwLen < SDL_SwapLE32(lpRiff->length) + 8)
    {
        return NULL;
    }

    lpChunk = (const RIFFChunkHeader*)(lpRiff + 1); dwLen -= sizeof(RIFFHeader);
    while (dwLen >= sizeof(RIFFChunkHeader))
    {
        len = SDL_SwapLE32(lpChunk->length);
        type = SDL_SwapLE32(lpChunk->type);
        if (dwLen >= sizeof(RIFFChunkHeader) + len)
            dwLen -= sizeof(RIFFChunkHeader) + len;
        else
            return NULL;

        switch (type)
        {
        case WAVE_fmt:
            lpFormat = (const WAVEFormatPCM*)(lpChunk + 1);
            if (len != sizeof(WAVEFormatPCM) || lpFormat->wFormatTag != SDL_SwapLE16(0x0001))
            {
                return NULL;
            }
            break;
        case WAVE_data:
            lpWaveData = (const uint8_t*)(lpChunk + 1);
            dwLen = 0;
            break;
        }
        lpChunk = (const RIFFChunkHeader*)((const uint8_t*)(lpChunk + 1) + len);
    }

    if (lpFormat == NULL || lpWaveData == NULL)
    {
        return NULL;
    }

    lpSpec->channels = SDL_SwapLE16(lpFormat->nChannels);
    lpSpec->format = (SDL_SwapLE16(lpFormat->wBitsPerSample) == 16) ? AUDIO_S16 : AUDIO_U8;
    lpSpec->freq = SDL_SwapLE32(lpFormat->nSamplesPerSec);
    lpSpec->size = len;
    lpSpec->align = SDL_SwapLE16(lpFormat->nChannels) * SDL_SwapLE16(lpFormat->wBitsPerSample) >> 3;

    return lpWaveData;
}


INT SoundPlay::PAL_MKFGetChunkSize(UINT uiChunkNum, FILE* fp)
/*++
Purpose:

Get the size of a chunk in an MKF archive.

Parameters:

[IN]  uiChunkNum - the number of the chunk in the MKF archive.

[IN]  fp - pointer to the fopen'ed MKF file.

Return value:

Integer value which indicates the size of the chunk.
-1 if the chunk does not exist.

--*/
{
    UINT    uiOffset = 0;
    UINT    uiNextOffset = 0;
    UINT    uiChunkCount = 0;
    //
    // Get the total number of chunks.
    //
    uiChunkCount = PAL_MKFGetChunkCount(fp);
    if (uiChunkNum >= uiChunkCount)
    {
        return -1;
    }
    //
    // Get the offset of the specified chunk and the next chunk.
    //
    fseek(fp, 4 * uiChunkNum, SEEK_SET);
    fread(&uiOffset, sizeof(UINT), 1, fp);
    fread(&uiNextOffset, sizeof(UINT), 1, fp);
    //uiOffset = SWAP32(uiOffset);
    //uiNextOffset = SWAP32(uiNextOffset);
    // Return the length of the chunk.
    return uiNextOffset - uiOffset;
}

INT SoundPlay::PAL_MKFReadChunk(LPBYTE lpBuffer, UINT uiBufferSize, UINT uiChunkNum, FILE* fp)
/*++
Purpose:

Read a chunk from an MKF archive into lpBuffer.

Parameters:

[OUT] lpBuffer - pointer to the destination buffer.

[IN]  uiBufferSize - size of the destination buffer.

[IN]  uiChunkNum - the number of the chunk in the MKF archive to read.

[IN]  fp - pointer to the fopen'ed MKF file.

Return value:

Integer value which indicates the size of the chunk.
-1 if there are error in parameters.
-2 if buffer size is not enough.

--*/
{
    UINT     uiOffset = 0;
    UINT     uiNextOffset = 0;
    UINT     uiChunkCount = 0;
    UINT     uiChunkLen = 0;

    if (lpBuffer == NULL || fp == NULL || uiBufferSize == 0)
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
    fseek(fp, 4 * uiChunkNum, SEEK_SET);
    fread(&uiOffset, 4, 1, fp);
    fread(&uiNextOffset, 4, 1, fp);
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
        fseek(fp, uiOffset, SEEK_SET);
        size_t nsize = fread(lpBuffer, uiChunkLen, 1, fp);
        assert(nsize == 1);
    }
    else
    {
        return -1;
    }

    return (INT)uiChunkLen;
}

INT SoundPlay::PAL_MKFGetChunkCount(FILE* fp)
/*++
Purpose:
Get the number of chunks in an MKF archive.
Parameters:
[IN]  fp - pointer to an fopen'ed MKF file.
Return value:
Integer value which indicates the number of chunks in the specified MKF file.
--*/
{
    INT iNumChunk;
    if (fp == NULL)
    {
        return 0;
    }

    fseek(fp, 0, SEEK_SET);
    fread(&iNumChunk, sizeof(INT), 1, fp);

    iNumChunk = (SWAP32(iNumChunk) - 4) / 4;
    return iNumChunk;
}

SoundPlay::SoundPlay() {
#ifdef  USE_EVENTHANDLER
    CPalEventHandler _m;
    Uint32 customEventType = _m.getCustomEventHandler(eventTypeCustom::eventSoundPlay);
    //注册播放音乐函数
    _m.registCallback(customEventType, [this](const SDL_Event* event) {
        //传入 1 指向音乐缓存 2 缓存长度
        auto stream = static_cast<ByteArray*>(event->user.data1);
        //将需要的数据传入缓存，stream 来自 CAudioDevice::PAL_FillAudioBuffer
        //在此过程中改写stream->data()指向的内存
        fillBuffer(*stream, static_cast<int>(
            reinterpret_cast<intptr_t>(event->user.data2)));
        CPalEventHandler _m;
        //解锁,在音乐播放csound 中加的锁，确保传递成功
        _m.setCustomMutex((UINT)eventTypeCustom::eventSoundPlay, FALSE);
        });
#endif //USE_EVENTHANDLER

    const char* mkfs[2]{ 0 };
    SoundLoader func[2]{ 0 };
    int i;
    //auto gConfig = CPalEvent::gConfig;
    if (lpAudioData->IsWIN95)
    {
        mkfs[0] = "sounds.mkf"; func[0] = &SOUND_LoadWAVEData;
        mkfs[1] = "voc.mkf"; func[1] = &SOUND_LoadVOCData;
    }
    else
    {
        mkfs[0] = "voc.mkf"; func[0] = &SOUND_LoadVOCData;
        mkfs[1] = "sounds.mkf"; func[1] = &SOUND_LoadWAVEData;
    }

    for (i = 0; i < 2; i++)
    {
        std::string s = lpAudioData->PalDir + mkfs[i];
        FILE* mMkf = fopen(s.data(), "rb");
        if (mMkf)
        {
            memset(&soundlist, 0, sizeof(WAVEDATA));
            LoadSound = func[i];
            mkf = mMkf;
            soundlist.resampler[0] = resampler_create();
            soundlist.resampler[1] = resampler_create();
            cursounds = 0;
            return;
        }
    }
}

int SoundPlay::Player(int iSoundNum, bool, float) {
    const SDL_AudioSpec* devspec = &lpAudioData->lpAudioDevice->spec;
    WAVESPEC         wavespec;
    ResampleMixer    mixer;
    WAVEDATA* cursnd;
    void* mBuf;
    const void* snddata;
    int           len, i;

    //auto gConfig = CPalEvent::gConfig;
    //
    // Check for NULL pointer.
    //
    //
    // Get the length of the sound file.
    //
    len = PAL_MKFGetChunkSize(iSoundNum, mkf);
    if (len <= 0)
    {
        return FALSE;
    }

    mBuf = malloc(len);
    if (mBuf == NULL)
    {
        return FALSE;
    }

    //
    // Read the sound file from the MKF archive.
    //
    PAL_MKFReadChunk((LPBYTE)mBuf, len, iSoundNum, mkf);

    snddata = LoadSound((LPBYTE)mBuf, len, &wavespec);
    if (snddata == NULL)
    {
        free(mBuf);
        return FALSE;
    }

    if (wavespec.channels == 1 && devspec->channels == 1)
        mixer = (wavespec.format == AUDIO_S16) ? SOUND_ResampleMix_S16_Mono_Mono : SOUND_ResampleMix_U8_Mono_Mono;
    else if (wavespec.channels == 1 && devspec->channels == 2)
        mixer = (wavespec.format == AUDIO_S16) ? SOUND_ResampleMix_S16_Mono_Stereo : SOUND_ResampleMix_U8_Mono_Stereo;
    else if (wavespec.channels == 2 && devspec->channels == 1)
        mixer = (wavespec.format == AUDIO_S16) ? SOUND_ResampleMix_S16_Stereo_Mono : SOUND_ResampleMix_U8_Stereo_Mono;
    else if (wavespec.channels == 2 && devspec->channels == 2)
        mixer = (wavespec.format == AUDIO_S16) ? SOUND_ResampleMix_S16_Stereo_Stereo : SOUND_ResampleMix_U8_Stereo_Stereo;
    else
    {
        free(mBuf);
        return FALSE;
    }

    //PAL_LockAudio();
    cursnd = &soundlist;
    while (cursnd->next && cursnd->base)
        cursnd = cursnd->next;
    if (cursnd->base)
    {
        WAVEDATA* obj = (WAVEDATA*)malloc(sizeof(WAVEDATA));
        memset(obj, 0, sizeof(WAVEDATA));
        cursnd->next = obj;
        cursnd = cursnd->next;
    }

    for (i = 0; i < wavespec.channels; i++)
    {
        if (!cursnd->resampler[i])
            cursnd->resampler[i] = resampler_create();
        else
            resampler_clear(cursnd->resampler[i]);
        resampler_set_quality(cursnd->resampler[i], AUDIO_IsIntegerConversion(wavespec.freq) ? RESAMPLER_QUALITY_MIN : lpAudioData->ResampleQuality);
        resampler_set_rate(cursnd->resampler[i], (double)wavespec.freq / (double)devspec->freq);
    }

    cursnd->base = mBuf;
    cursnd->current = snddata;
    cursnd->end = (const uint8_t*)snddata + wavespec.size;
    cursnd->spec = wavespec;
    cursnd->ResampleMix = mixer;
    cursounds++;
    //PAL_UnlockAudio();
    return TRUE;
}

void SoundPlay::fillBuffer(ByteArray& stream, int len) {

    WAVEDATA* cursnd = &soundlist;
    int sounds = 0;
    do
    {
        if (cursnd->base)
        {
            cursnd->ResampleMix(cursnd->resampler, cursnd->current, &cursnd->spec, stream.data(), len, &cursnd->current);
            cursnd->spec.size = (const uint8_t*)cursnd->end - (const uint8_t*)cursnd->current;
            if (cursnd->spec.size < cursnd->spec.align)
            {
                free((void*)cursnd->base);
                cursnd->base = cursnd->current = cursnd->end = NULL;
                cursounds--;
            }
            else
                sounds++;
        }
    } while ((cursnd = cursnd->next) && sounds < cursounds);

}

SoundPlay::~SoundPlay() {
    WAVEDATA* cursnd = &soundlist;
    do
    {
        if (cursnd->resampler[0]) resampler_delete(cursnd->resampler[0]);
        if (cursnd->resampler[1]) resampler_delete(cursnd->resampler[1]);
        if (cursnd->base) free((void*)cursnd->base);
    } while ((cursnd = cursnd->next) != NULL);
    cursnd = soundlist.next;
    while (cursnd)
    {
        WAVEDATA* old = cursnd;
        cursnd = cursnd->next;
        free(old);
    }
    if (mkf) fclose(mkf);
}





int SoundPlay::SOUND_ResampleMix_S16_Mono_Mono(void* resampler[2], const void* lpData, const WAVESPEC* lpSpec, void* lpBuffer, int iBufLen, const void** llpData)
/*++
Purpose:

Resample 16-bit signed (little-endian) mono PCM data into 16-bit signed (native-endian) mono PCM data.

Parameters:

[IN]  resampler - array of pointers to the resampler instance.

[IN]  lpData - pointer to the buffer of the input PCM data.

[IN]  lpSpec - pointer to the WAVESPEC structure, which contains
some basic information about the input PCM data.

[IN]  lpBuffer - pointer of the buffer of the output PCM data.

[IN]  iBufLen - length of the buffer of the output PCM data.

[OUT] llpData - pointer to receive the pointer of remaining input PCM data.

Return value:

The number of output buffer used, in bytes.
--*/
{
    int src_samples = lpSpec->size >> 1;
    const short* src = (const short*)lpData;
    short* dst = (short*)lpBuffer;
    int channel_len = iBufLen, total_bytes = 0;

    while (total_bytes < channel_len && src_samples > 0)
    {
        int j, to_write = resampler_get_free_count(resampler[0]);
        if (to_write > src_samples) to_write = src_samples;
        for (j = 0; j < to_write; j++)
            resampler_write_sample(resampler[0], SDL_SwapLE16(*src++));
        src_samples -= to_write;
        while (total_bytes < channel_len && resampler_get_sample_count(resampler[0]) > 0)
        {
            int sample = (resampler_get_sample(resampler[0]) >> 8) + *dst;
            *dst++ = (sample <= 32767) ? ((sample >= -32768) ? sample : -32768) : 32767;
            total_bytes += sizeof(short);
            resampler_remove_sample(resampler[0]);
        }
    }

    if (llpData) *llpData = src;
    return total_bytes;
}

int SoundPlay::SOUND_ResampleMix_S16_Mono_Stereo(void* resampler[2], const void* lpData, const WAVESPEC* lpSpec, void* lpBuffer, int iBufLen, const void** llpData)
/*++
Purpose:

Resample 16-bit signed (little-endian) mono PCM data into 16-bit signed (native-endian) stereo PCM data.

Parameters:

[IN]  resampler - array of pointers to the resampler instance.

[IN]  lpData - pointer to the buffer of the input PCM data.

[IN]  lpSpec - pointer to the WAVESPEC structure, which contains
some basic information about the input PCM data.

[IN]  lpBuffer - pointer of the buffer of the output PCM data.

[IN]  iBufLen - length of the buffer of the output PCM data.

[OUT] llpData - pointer to receive the pointer of remaining input PCM data.

Return value:

The number of output buffer used, in bytes.
--*/
{
    int src_samples = lpSpec->size >> 1;
    const short* src = (const short*)lpData;
    short* dst = (short*)lpBuffer;
    int channel_len = iBufLen >> 1, total_bytes = 0;

    while (total_bytes < channel_len && src_samples > 0)
    {
        int j, to_write = resampler_get_free_count(resampler[0]);
        if (to_write > src_samples) to_write = src_samples;
        for (j = 0; j < to_write; j++)
            resampler_write_sample(resampler[0], SDL_SwapLE16(*src++));
        src_samples -= to_write;
        while (total_bytes < channel_len && resampler_get_sample_count(resampler[0]) > 0)
        {
            int sample = (resampler_get_sample(resampler[0]) >> 8) + *dst;
            dst[0] = dst[1] = (sample <= 32767) ? ((sample >= -32768) ? sample : -32768) : 32767;
            total_bytes += sizeof(short); dst += 2;
            resampler_remove_sample(resampler[0]);
        }
    }

    if (llpData) *llpData = src;
    return total_bytes;
}

int SoundPlay::SOUND_ResampleMix_S16_Stereo_Mono(void* resampler[2], const void* lpData, const WAVESPEC* lpSpec, void* lpBuffer, int iBufLen, const void** llpData)
/*++
Purpose:

Resample 16-bit signed (little-endian) stereo PCM data into 16-bit signed (native-endian) mono PCM data.

Parameters:

[IN]  resampler - array of pointers to the resampler instance.

[IN]  lpData - pointer to the buffer of the input PCM data.

[IN]  lpSpec - pointer to the WAVESPEC structure, which contains
some basic information about the input PCM data.

[IN]  lpBuffer - pointer of the buffer of the output PCM data.

[IN]  iBufLen - length of the buffer of the output PCM data.

[OUT] llpData - pointer to receive the pointer of remaining input PCM data.

Return value:

The number of output buffer used, in bytes.
--*/
{
    int src_samples = lpSpec->size >> 2;
    const short* src = (const short*)lpData;
    short* dst = (short*)lpBuffer;
    int channel_len = iBufLen, total_bytes = 0;

    while (total_bytes < channel_len && src_samples > 0)
    {
        int j, to_write = resampler_get_free_count(resampler[0]);
        if (to_write > src_samples) to_write = src_samples;
        for (j = 0; j < to_write; j++)
        {
            resampler_write_sample(resampler[0], SDL_SwapLE16(*src++));
            resampler_write_sample(resampler[1], SDL_SwapLE16(*src++));
        }
        src_samples -= to_write;
        while (total_bytes < channel_len && resampler_get_sample_count(resampler[0]) > 0)
        {
            int sample = (((resampler_get_sample(resampler[0]) >> 8) + (resampler_get_sample(resampler[1]) >> 8)) >> 1) + *dst;
            *dst++ = (sample <= 32767) ? ((sample >= -32768) ? sample : -32768) : 32767;
            total_bytes += sizeof(short);
            resampler_remove_sample(resampler[0]);
            resampler_remove_sample(resampler[1]);
        }
    }

    if (llpData) *llpData = src;
    return total_bytes;
}

int SoundPlay::SOUND_ResampleMix_S16_Stereo_Stereo(void* resampler[2], const void* lpData, const WAVESPEC* lpSpec, void* lpBuffer, int iBufLen, const void** llpData)
/*++
Purpose:

Resample 16-bit signed (little-endian) stereo PCM data into 16-bit signed (native-endian) stereo PCM data.

Parameters:

[IN]  resampler - array of pointers to the resampler instance.

[IN]  lpData - pointer to the buffer of the input PCM data.

[IN]  lpSpec - pointer to the WAVESPEC structure, which contains
some basic information about the input PCM data.

[IN]  lpBuffer - pointer of the buffer of the output PCM data.

[IN]  iBufLen - length of the buffer of the output PCM data.

[OUT] llpData - pointer to receive the pointer of remaining input PCM data.

Return value:

The number of output buffer used, in bytes.
--*/
{
    int src_samples = lpSpec->size >> 2;
    const short* src = (const short*)lpData;
    short* dst = (short*)lpBuffer;
    int channel_len = iBufLen >> 1, total_bytes = 0;

    while (total_bytes < channel_len && src_samples > 0)
    {
        int j, to_write = resampler_get_free_count(resampler[0]);
        if (to_write > src_samples) to_write = src_samples;
        for (j = 0; j < to_write; j++)
        {
            resampler_write_sample(resampler[0], SDL_SwapLE16(*src++));
            resampler_write_sample(resampler[1], SDL_SwapLE16(*src++));
        }
        src_samples -= to_write;
        while (total_bytes < channel_len && resampler_get_sample_count(resampler[0]) > 0)
        {
            int sample;
            sample = (resampler_get_sample(resampler[0]) >> 8) + *dst;
            *dst++ = (sample <= 32767) ? ((sample >= -32768) ? sample : -32768) : 32767;
            sample = (resampler_get_sample(resampler[1]) >> 8) + *dst;
            *dst++ = (sample <= 32767) ? ((sample >= -32768) ? sample : -32768) : 32767;
            total_bytes += sizeof(short);
            resampler_remove_sample(resampler[0]);
            resampler_remove_sample(resampler[1]);
        }
    }

    if (llpData) *llpData = src;
    return total_bytes;
}

int SoundPlay::SOUND_ResampleMix_U8_Mono_Mono(void* resampler[2], const void* lpData, const WAVESPEC* lpSpec, void* lpBuffer, int iBufLen, const void** llpData)
/*++
Purpose:

Resample 8-bit unsigned mono PCM data into 16-bit signed (native-endian) mono PCM data.

Parameters:

[IN]  resampler - array of pointers to the resampler instance.

[IN]  lpData - pointer to the buffer of the input PCM data.

[IN]  lpSpec - pointer to the WAVESPEC structure, which contains
some basic information about the input PCM data.

[IN]  lpBuffer - pointer of the buffer of the output PCM data.

[IN]  iBufLen - length of the buffer of the output PCM data.

[OUT] llpData - pointer to receive the pointer of remaining input PCM data.

Return value:

The number of output buffer used, in bytes.
--*/
{
    int src_samples = lpSpec->size;
    const uint8_t* src = (const uint8_t*)lpData;
    short* dst = (short*)lpBuffer;
    int channel_len = iBufLen, total_bytes = 0;

    while (total_bytes < channel_len && src_samples > 0)
    {
        int j, to_write = resampler_get_free_count(resampler[0]);
        if (to_write > src_samples) to_write = src_samples;
        for (j = 0; j < to_write; j++)
            resampler_write_sample(resampler[0], (*src++ ^ 0x80) << 8);
        src_samples -= to_write;
        while (total_bytes < channel_len && resampler_get_sample_count(resampler[0]) > 0)
        {
            int sample = (resampler_get_sample(resampler[0]) >> 8) + *dst;
            *dst++ = (sample <= 32767) ? ((sample >= -32768) ? sample : -32768) : 32767;
            total_bytes += sizeof(short);
            resampler_remove_sample(resampler[0]);
        }
    }

    if (llpData) *llpData = src;
    return total_bytes;
}

int SoundPlay::SOUND_ResampleMix_U8_Mono_Stereo(void* resampler[2], const void* lpData, const WAVESPEC* lpSpec, void* lpBuffer, int iBufLen, const void** llpData)
/*++
Purpose:

Resample 8-bit unsigned mono PCM data into 16-bit signed (native-endian) stereo PCM data.

Parameters:

[IN]  resampler - array of pointers to the resampler instance.

[IN]  lpData - pointer to the buffer of the input PCM data.

[IN]  lpSpec - pointer to the WAVESPEC structure, which contains
some basic information about the input PCM data.

[IN]  lpBuffer - pointer of the buffer of the output PCM data.

[IN]  iBufLen - length of the buffer of the output PCM data.

[OUT] llpData - pointer to receive the pointer of remaining input PCM data.

Return value:

The number of output buffer used, in bytes.
--*/
{
    int src_samples = lpSpec->size;
    const uint8_t* src = (const uint8_t*)lpData;
    short* dst = (short*)lpBuffer;
    int channel_len = iBufLen >> 1, total_bytes = 0;

    while (total_bytes < channel_len && src_samples > 0)
    {
        int j, to_write = resampler_get_free_count(resampler[0]);
        if (to_write > src_samples) to_write = src_samples;
        for (j = 0; j < to_write; j++)
            resampler_write_sample(resampler[0], (*src++ ^ 0x80) << 8);
        src_samples -= to_write;
        while (total_bytes < channel_len && resampler_get_sample_count(resampler[0]) > 0)
        {
            int sample = (resampler_get_sample(resampler[0]) >> 8) + *dst;
            dst[0] = dst[1] = (sample <= 32767) ? ((sample >= -32768) ? sample : -32768) : 32767;
            total_bytes += sizeof(short); dst += 2;
            resampler_remove_sample(resampler[0]);
        }
    }

    if (llpData) *llpData = src;
    return total_bytes;
}

int SoundPlay::SOUND_ResampleMix_U8_Stereo_Mono(void* resampler[2], const void* lpData, const WAVESPEC* lpSpec, void* lpBuffer, int iBufLen, const void** llpData)
/*++
Purpose:

Resample 8-bit unsigned stereo PCM data into 16-bit signed (native-endian) mono PCM data.

Parameters:

[IN]  resampler - array of pointers to the resampler instance.

[IN]  lpData - pointer to the buffer of the input PCM data.

[IN]  lpSpec - pointer to the WAVESPEC structure, which contains
some basic information about the input PCM data.

[IN]  lpBuffer - pointer of the buffer of the output PCM data.

[IN]  iBufLen - length of the buffer of the output PCM data.

[OUT] llpData - pointer to receive the pointer of remaining input PCM data.

Return value:

The number of output buffer used, in bytes.
--*/
{
    int src_samples = lpSpec->size >> 1;
    const uint8_t* src = (const uint8_t*)lpData;
    short* dst = (short*)lpBuffer;
    int channel_len = iBufLen, total_bytes = 0;

    while (total_bytes < channel_len && src_samples > 0)
    {
        int j, to_write = resampler_get_free_count(resampler[0]);
        if (to_write > src_samples) to_write = src_samples;
        for (j = 0; j < to_write; j++)
        {
            resampler_write_sample(resampler[0], (*src++ ^ 0x80) << 8);
            resampler_write_sample(resampler[1], (*src++ ^ 0x80) << 8);
        }
        src_samples -= to_write;
        while (total_bytes < channel_len && resampler_get_sample_count(resampler[0]) > 0)
        {
            int sample = (((resampler_get_sample(resampler[0]) >> 8) + (resampler_get_sample(resampler[1]) >> 8)) >> 1) + *dst;
            *dst++ = (sample <= 32767) ? ((sample >= -32768) ? sample : -32768) : 32767;
            total_bytes += sizeof(short);
            resampler_remove_sample(resampler[0]);
            resampler_remove_sample(resampler[1]);
        }
    }

    if (llpData) *llpData = src;
    return total_bytes;
}

int SoundPlay::SOUND_ResampleMix_U8_Stereo_Stereo(void* resampler[2], const void* lpData, const WAVESPEC* lpSpec, void* lpBuffer, int iBufLen, const void** llpData)
/*++
Purpose:

Resample 8-bit unsigned stereo PCM data into 16-bit signed (native-endian) stereo PCM data.

Parameters:

[IN]  resampler - array of pointers to the resampler instance.

[IN]  lpData - pointer to the buffer of the input PCM data.

[IN]  lpSpec - pointer to the WAVESPEC structure, which contains
some basic information about the input PCM data.

[IN]  lpBuffer - pointer of the buffer of the output PCM data.

[IN]  iBufLen - length of the buffer of the output PCM data.

[OUT] llpData - pointer to receive the pointer of remaining input PCM data.

Return value:

The number of output buffer used, in bytes.
--*/
{
    int src_samples = lpSpec->size >> 1;
    const uint8_t* src = (const uint8_t*)lpData;
    short* dst = (short*)lpBuffer;
    int channel_len = iBufLen >> 1, total_bytes = 0;

    while (total_bytes < channel_len && src_samples > 0)
    {
        int j, to_write = resampler_get_free_count(resampler[0]);
        if (to_write > src_samples) to_write = src_samples;
        for (j = 0; j < to_write; j++)
        {
            resampler_write_sample(resampler[0], (*src++ ^ 0x80) << 8);
            resampler_write_sample(resampler[1], (*src++ ^ 0x80) << 8);
        }
        src_samples -= to_write;
        while (total_bytes < channel_len && resampler_get_sample_count(resampler[0]) > 0)
        {
            int sample;
            sample = (resampler_get_sample(resampler[0]) >> 8) + *dst;
            *dst++ = (sample <= 32767) ? ((sample >= -32768) ? sample : -32768) : 32767;
            sample = (resampler_get_sample(resampler[1]) >> 8) + *dst;
            *dst++ = (sample <= 32767) ? ((sample >= -32768) ? sample : -32768) : 32767;
            total_bytes += sizeof(short);
            resampler_remove_sample(resampler[0]);
            resampler_remove_sample(resampler[1]);
        }
    }

    if (llpData) *llpData = src;
    return total_bytes;
}
