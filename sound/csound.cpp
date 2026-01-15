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

#include <assert.h>
#include <thread> 

#include "csound.h"
#include "caudiodevice.h"
#include "soundseting.h"
#include "../main/cpaldata.h"

CSound::CSound()
{

}

CSound::~CSound()
{
    //等待进程退出
    while (PalSoundIn) {
        PAL_Delay(1);
    };

    if (gAudioDevice)
        delete gAudioDevice;
    gAudioDevice = nullptr;
}

VOID CSound::AUDIO_PlaySound(int iSoundNum)
{
    AudioPlay.locked = 1;
    AudioPlay.sound = iSoundNum;
    AudioPlay.locked = 0;

}

VOID CSound::SOUND_Play(int i) {
    return AUDIO_PlaySound(i);
}


VOID CSound::soundRun()
{

    PalSoundIn = TRUE;
    if (AUDIO_OpenDevice())
    {
        //初始化失败返回
        PalSoundIn = FALSE;
        return;
    };
    //
    MusicBuf.resize( gAudioData.AudioBufferSize * gAudioData.AudioChannels);
    //播放循环
    while (!PalQuit)
    {
        if (AudioPlay.locked)//等待解锁
        {
            SDL_Delay(1);
            continue;
        }

        if (AudioPlay.Music > 0)
        {
            AUDIO_PlayMusicInRun(AudioPlay.Music, AudioPlay.musicLoop, AudioPlay.flFadeTime);
            AudioPlay.Music = 0;
        }

        if (AudioPlay.sound > 0)
        {
            if (gAudioDevice->pSoundPlayer)
            {
                gAudioDevice->pSoundPlayer->Player(abs(AudioPlay.sound), FALSE, 0.0f);
            }
            AudioPlay.sound = 0;
        }

            //以下运行原回调函数,如果在缓存中的字节少于，执行
#if USING_SDL3
        if (!gAudioDevice)
            break;
        if (!gAudioDevice->pAudioStream)
            break;
        const auto streamSize = SDL_GetAudioStreamQueued(gAudioDevice->pAudioStream);
        const auto Len = 8192 * ggConfig->iAudioChannels;
        if (streamSize < Len)
        {
            AUDIO_FillBuffer(MusicBuf, 2048);
            if(!SDL_PutAudioStreamData(gAudioDevice->pAudioStream, MusicBuf.data(), 2048))
                printMsg("err SDL_PutAudioStreamData %s", SDL_GetError());
            ;
        }
#else
        if (gAudioDevice && SDL_GetQueuedAudioSize(gAudioDevice->audioID)
            < 8192 * ggConfig->iAudioChannels)
        {
            AUDIO_FillBuffer(MusicBuf, MusicBuf.size());
            SDL_QueueAudio(gAudioDevice->audioID, MusicBuf.data(), MusicBuf.size());
        }
#endif
        PAL_Delay(10);
    }
    //
    printMsg("声音系统退出\n");
    //shutdown
    AudioPlay.Music = 0;
    //清理退出
    delete gAudioDevice; 
    gAudioDevice = nullptr;
    PalSoundIn = FALSE;
}


PalErr CSound::AUDIO_OpenDevice()
{
    AudioPlayer::lpAudioData = &gAudioData;
    //
    gAudioData.mAudioDevice = ggConfig->iAudioDevice;
    gAudioData.SampleRate = ggConfig->iSampleRate;
    gAudioData.ResampleQuality = ggConfig->iResampleQuality;
    gAudioData.AudioChannels = ggConfig->iAudioChannels;
    gAudioData.FirstUseMp3 = ggConfig->firstUseMp3;
    gAudioData.MusicVolume = ggConfig->iMusicVolume;
    gAudioData.SoundVolume = ggConfig->iSoundVolume;
    gAudioData.AudioBufferSize = ggConfig->wAudioBufferSize;
    gAudioData.NoMusic = false;
    gAudioData.NoSound = false;
    gAudioData.MusicType = ggConfig->eMusicType;
    gAudioData.PalDir = PalDir;
    gAudioData.IsWIN95 = ggConfig->fIsWIN95;
    gAudioData.OPLChip = ggConfig->eOPLChip;
    gAudioData.OPLCore = ggConfig->eOPLCore;
    gAudioData.UseSurroundOPL = ggConfig->fUseSurroundOPL;
    gAudioData.OPLSampleRate = ggConfig->iOPLSampleRate;
    gAudioData.ResampleQuality = ggConfig->iResampleQuality;
    gAudioData.SurroundOPLOffset = ggConfig->iSurroundOPLOffset;
	//创建音频设备
    gAudioDevice = new CAudioDevice(&gAudioData);
#if USING_SDL3
    if (!gAudioDevice || !gAudioDevice->pAudioStream)
        return 1;
    SDL_ResumeAudioStreamDevice(gAudioDevice->pAudioStream);
#else
    if (!gAudioDevice || gAudioDevice->audioID < 0)
        return 1;
#endif
    gAudioData.lpAudioDevice = gAudioDevice;
    return 0;
}




VOID CSound::AUDIO_PlayMusic(INT iNumRIX, BOOL fLoop, FLOAT flFadeTime)
{
    AudioPlay.locked = 1;
    AudioPlay.Music = iNumRIX;
    AudioPlay.musicLoop = fLoop;
    AudioPlay.flFadeTime = flFadeTime;
    AudioPlay.locked = 0;
}




VOID CSound::PAL_PlayMUS(INT iNumRIX, BOOL fLoop, FLOAT flFadeTime)
{
    return AUDIO_PlayMusic(iNumRIX, fLoop, flFadeTime);
}

VOID CSound::SoundRunThread()
{
    std::thread _soundThread([this]()->void {
        soundRun();
        });
    _soundThread.detach();
};

VOID SDLCALL CSound::AUDIO_FillBuffer(ByteArray& stream, INT len)
{
    return gAudioDevice->AUDIO_FillBuffer(stream, len);
}

VOID CSound::AUDIO_PlayMusicInRun(INT iNumRIX, BOOL fLoop, FLOAT flFadeTime)
{
    int ret{};
    if (CPalEvent::ggConfig->firstUseMp3 && gAudioDevice->pMp3Player)//优先使用MP3
    {
#if USING_SDL3
        // On SDL3 stream mode don't lock the stream (SDL_LockAudioStream can hang). Call player directly.
        auto ret = gAudioDevice->pMp3Player->Player(iNumRIX, fLoop, flFadeTime);
#else
        SDL_LockAudioDevice(gAudioDevice->audioID);
        auto ret = gAudioDevice->pMp3Player->Player(iNumRIX, fLoop, flFadeTime);
        SDL_UnlockAudioDevice(gAudioDevice->audioID);
#endif
        if (ret) iNumRIX = 0;
        else gAudioDevice->pMp3Player->iMusic = 0;
    }
    if (gAudioDevice->pMusPlayer)
    {
#if USING_SDL3
        // On SDL3 stream mode don't lock the stream (SDL_LockAudioStream can hang). Call player directly.
        auto ret = gAudioDevice->pMusPlayer->Player(iNumRIX, fLoop, flFadeTime);
#else
        SDL_LockAudioDevice(gAudioDevice->audioID);
        auto ret = gAudioDevice->pMusPlayer->Player(iNumRIX, fLoop, flFadeTime);
        SDL_UnlockAudioDevice(gAudioDevice->audioID);
#endif
        if (ret)
            iNumRIX = 0;
        else
            gAudioDevice->pMusPlayer->iMusic = 0;
    }
    
}



