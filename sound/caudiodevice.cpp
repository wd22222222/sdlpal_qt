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

#include "caudiodevice.h"
#include "../main/command.h"
#include "../main/cpaleventhandler.h"
#include "../main/cpalevent.h"
#include "soundplayer.h"
#ifndef USE_EVENTHANDLER
#include "../main/caviplay.h"
#endif

void CAudioDevice::AUDIO_AdjustVolume(short* srcdst, int iVolume, int samples)
{
    if (iVolume == SDL_MIX_MAXVOLUME) return;
    if (iVolume == 0) { memset(srcdst, 0, samples << 1); return; }
    while (samples > 0)
    {
        *srcdst = *srcdst * iVolume / SDL_MIX_MAXVOLUME;
        samples--; srcdst++;
    }
}

void CAudioDevice::AUDIO_MixNative(short* dst, short* src, int samples)
{
    while (samples > 0)
    {
        int val = *src++ + *dst;
        if (val > SHRT_MAX)
            *dst++ = SHRT_MAX;
        else if (val < SHRT_MIN)
            *dst++ = SHRT_MIN;
        else
            *dst++ = (short)val;
        samples--;
    }
}

CAudioDevice::CAudioDevice(const AudioData* Audioset) {

    //lpAudioData = Audioset;
    const auto lpAudioData = Audioset;

    fOpened = FALSE;
    bMusicVolume = lpAudioData->MusicVolume * SDL_MIX_MAXVOLUME / PAL_MAX_VOLUME;
    bSoundVolume = lpAudioData->SoundVolume * SDL_MIX_MAXVOLUME / PAL_MAX_VOLUME;
    //
    // Initialize the resampler module
    //
    resampler_init();

    spec.freq = lpAudioData->SampleRate;
    spec.format = AUDIO_S16SYS;
    spec.channels = lpAudioData->AudioChannels;
    //spec.samples = lpAudioData->AudioBufferSize;
    //spec.callback = nullptr;
    //spec.userdata = nullptr;
    SDL_AudioSpec mSpec{};
    auto& mAudioDevice = lpAudioData->mAudioDevice;

#if USING_SDL3 == 0
    // SDL2: SDL_OpenAudioDevice(const char* device, int iscapture, const SDL_AudioSpec* desired, SDL_AudioSpec* obtained, int allowed_changes)
    audioID = SDL_OpenAudioDevice(
        (mAudioDevice >= 0 ? SDL_GetAudioDeviceName(mAudioDevice, 0) : nullptr),
        0,
        &spec,
        &mSpec,
        0
    );
    if(audioID<0)
		return;//初始化失败
    SDL_PauseAudioDevice(audioID, 0);
#else
    pAudioStream = SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &spec, 0, 0);
    if (!pAudioStream)
        CPalEvent::printMsg(SDL_GetError());
    SDL_ResumeAudioStreamDevice(pAudioStream);
#endif
    pSoundBuffer.resize(sizeof(short) * lpAudioData->AudioBufferSize * lpAudioData->AudioChannels);

    fOpened = true;

    pSoundPlayer = new SoundPlay;

    switch (lpAudioData->MusicType)
    {
    case (int)musicType::mp3:
        pMusPlayer = new Mp3player;		
        eCurrentMusicType = musicType::mp3;
        break;
    case (int)musicType::rix:
    {
        pMusPlayer = new AudioRixPlayer;
        eCurrentMusicType = musicType::rix;
        break;
    }
#if USING_OGG
    case (int)musicType::ogg:
        pMusPlayer = new COggPlayer;
		eCurrentMusicType = musicType::ogg;
        break;
#endif
    case (int)musicType::midi:
        pMusPlayer = new AudioMidiPlayer;
        eCurrentMusicType = musicType::midi;
        break;
    default:
        break;
    }

    if (lpAudioData->FirstUseMp3 && lpAudioData->MusicType != (int)musicType::mp3 )
    {
        pMp3Player = new Mp3player;
    }
    fOpened = TRUE;
    //SDL_PauseAudioDevice(id, 0);
#if USING_SDL3 == 0
#else
#endif
}

CAudioDevice::~CAudioDevice() {
    delete pMusPlayer;
    delete pSoundPlayer;
    delete pMp3Player;
    CPalEvent::printMsg("已经将各相关结构清除\n");
    pMusPlayer = nullptr;
	pSoundPlayer = nullptr;
	pMp3Player = nullptr;
    //SDL_CloseAudio();
	CPalEvent::printMsg("关闭音频设备\n");
#if USING_SDL3
    SDL_DestroyAudioStream(pAudioStream);
#else
	SDL_CloseAudioDevice(audioID);
#endif
}

VOID SDLCALL CAudioDevice::AUDIO_FillBuffer(ByteArray& stream, INT len)
/*++
Purpose:

SDL sound callback function.

Parameters:

[IN]  udata - pointer to user-defined parameters (Not used).

[OUT] stream - pointer to the stream buffer.

[IN]  len - Length of the buffer.

Return value:

None.

--*/
{
	
    auto& lpAudioData = AudioPlayer::lpAudioData;

    memset(stream.data(), 0, stream.size());

    if (!lpAudioData->NoMusic && bMusicVolume > 0)
    {
#ifdef USE_EVENTHANDLER
        if (pMp3Player && pMp3Player->iMusic > 0)
        {
            pushEventForPlay(eventTypeCustom::eventMp3MusicPlay, stream, len);
        }
        else  if (pMusPlayer)
        {
            switch (eCurrentMusicType)
            {
            case musicType::rix:
                pushEventForPlay(eventTypeCustom::eventRixMusicPlay, stream, len);
                break;
            case musicType::midi:
                pushEventForPlay(eventTypeCustom::eventMidiMusicPlay, stream, len);
                break;
#if USING_OGG
            case musicType::ogg:
                pushEventForPlay(eventTypeCustom::eventOggMusicPlay, stream, len);
                break;
#endif
            case musicType::mp3:
                pushEventForPlay(eventTypeCustom::eventMp3MusicPlay, stream, len);
                break;
            default:
                CPalEvent::printMsg("err musicType");
                break;
            }
		}
#else
        if (pMp3Player && pMp3Player->iMusic > 0)
        {
            pMp3Player->fillBuffer(stream, len);
        }

        else  if (pMusPlayer)
        {
            pMusPlayer->fillBuffer(stream, len);
        }
#endif
        //
        // Adjust volume for music
        //
        AUDIO_AdjustVolume((short*)stream.data(), bMusicVolume, len >> 1);
    }

    //
    // Play sound
    //
    if (!lpAudioData->NoSound && pSoundPlayer
        && bSoundVolume > 0)
    {
        memset(pSoundBuffer.data(), 0, len);
#ifdef USE_EVENTHANDLER
		pushEventForPlay(eventTypeCustom::eventSoundPlay, pSoundBuffer, len);
#else
        pSoundPlayer->fillBuffer(pSoundBuffer, len);
#endif
        //
        // Adjust volume for sound
        //
        AUDIO_AdjustVolume((short*)pSoundBuffer.data(), bSoundVolume, len >> 1);

        //
        // Mix sound & music
        //
        AUDIO_MixNative((short*)stream.data(), (short*)pSoundBuffer.data(), len >> 1);
    }

    //
    // Play sound for AVI
    //
#ifdef USE_EVENTHANDLER
	pushEventForPlay(eventTypeCustom::eventAviMusicPlay, stream, len);
#else
    //不使用EVENTHANDLER 传递数据
    CAviPlay::AVI_FillAudioBuffer(stream.data(), len);
#endif //USE_EVENTHANDLER
}


#ifdef USE_EVENTHANDLER
void CAudioDevice::pushEventForPlay(eventTypeCustom eventType,ByteArray& stream,int len)
{
    CPalEventHandler _m;
    SDL_Event e{};
    e.type = _m.getCustomEventHandler(eventType);
    //加锁
    _m.setCustomMutex((UINT)eventTypeCustom::eventAviMusicPlay, TRUE);
    //将地址传出,等待处理
    e.user.data1 = static_cast<void*>(&stream);
    e.user.data2 = reinterpret_cast<void*>(static_cast<int>(len));
    _m.PushEvent(&e);
    //等待处理完成
    do {
        _m.PAL_ProcessEvent();
    } while (_m.getCustomMutex((UINT)eventType));
}
#endif