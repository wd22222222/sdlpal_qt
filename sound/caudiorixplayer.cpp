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

//
#include "caudiodevice.h"
#include "soundseting.h"
#include "adplug/convertopl.h"
#include "../main/cpaleventhandler.h"
#include <cmath>
#define AUDIO_IsIntegerConversion(a) (((a) % lpAudioData->SampleRate) == 0 || (lpAudioData->SampleRate % (a)) == 0)

AudioRixPlayer::AudioRixPlayer()
{
#ifdef  USE_EVENTHANDLER
    CPalEventHandler _m;
    Uint32 customEventType = _m.getCustomEventHandler(eventTypeCustom::eventRixMusicPlay);
    //注册播放音乐函数
    _m.registCallback(customEventType, [this](const SDL_Event* event) {
        //传入 1 指向音乐缓存 2 缓存长度
        auto stream = static_cast<ByteArray*>(event->user.data1);
        //将需要的数据传入缓存，stream 来自 CAudioDevice::PAL_FillAudioBuffer
        //在此过程中改写stream->data()指向的内存
        fillBuffer(*stream, static_cast<int>(
            reinterpret_cast<intptr_t>(event->user.data2)));
        CPalEventHandler _m{};
        //解锁,在音乐播放csound 中加的锁，确保传递成功
        _m.setCustomMutex((UINT)eventTypeCustom::eventRixMusicPlay, FALSE);
        });
#endif //USE_EVENTHANDLER

    
    //auto gConfig = CPalEvent::gConfig;
    auto chip = (Copl::ChipType)lpAudioData->OPLChip;
    if (chip == Copl::TYPE_OPL2 && lpAudioData->UseSurroundOPL)
    {
        chip = Copl::TYPE_DUAL_OPL2;
    }

    Copl* mOpl = CEmuopl::CreateEmuopl((OPLCORE::TYPE)lpAudioData->OPLCore, chip, lpAudioData->OPLSampleRate);
    if (NULL == mOpl)
    {
        return;
    }

    if (lpAudioData->UseSurroundOPL)
    {
        Copl* tmpopl = new CSurroundopl(lpAudioData->OPLSampleRate, lpAudioData->SurroundOPLOffset, mOpl);
        if (NULL == tmpopl)
        {
            delete mOpl;
            mOpl = nullptr;
            return;
        }
        mOpl = tmpopl;
    }

    sOpl = new CConvertopl(mOpl, true, lpAudioData->AudioChannels == 2);
    if (sOpl == NULL)
    {
        delete mOpl;
        mOpl = nullptr;
        return;
    }

    rix = new CrixPlayer(sOpl);
    if (rix == NULL)
    {
        delete sOpl;
        sOpl = nullptr;
        return;
    }

    //
    // Load the MKF file.
    //
    std::string s = lpAudioData->PalDir + "mus.mkf";
    if (!rix->load(s.data(), CProvider_Filesystem()))
    {
        delete rix;
        rix = nullptr;
        delete sOpl;
        sOpl = nullptr;
        return;
    }

    if (lpAudioData->OPLSampleRate != lpAudioData->SampleRate)
    {
        for (int i = 0; i < lpAudioData->AudioChannels; i++)
        {
            resampler[i] = resampler_create();
            resampler_set_quality(resampler[i], AUDIO_IsIntegerConversion(lpAudioData->OPLSampleRate) ? RESAMPLER_QUALITY_MIN : lpAudioData->ResampleQuality);
            resampler_set_rate(resampler[i], (double)lpAudioData->OPLSampleRate / (double)lpAudioData->SampleRate);
        }
    }

#if USE_RIX_EXTRA_INIT
    if (gConfig->pExtraFMRegs && gConfig->pExtraFMVals)
    {
        pRixPlayer->rix->set_extra_init(gConfig->pExtraFMRegs, gConfig->pExtraFMVals, gConfig->dwExtraLength);
    }
#endif

    //
    // Success.
    //
    FadeType = AudioRixPlayer::NONE;
    iMusic = iNextMusic = -1;
    pos = NULL;
    fLoop = FALSE;
    fNextLoop = FALSE;
    fReady = FALSE;

    return;
}

AudioRixPlayer::~AudioRixPlayer()
{
    //auto& gConfig = CPalEvent::gConfig;
    fReady = FALSE;
    for (int i = 0; i < lpAudioData->AudioChannels; i++)
        if (resampler[i])
            resampler_delete(resampler[i]);
    delete rix;
    delete sOpl;
}

int AudioRixPlayer::Player(int iNumRIX, bool mfLoop, float flFadeTime)
{

    //auto gConfig = CPalEvent::gConfig;
    if (iNumRIX == iMusic && iNextMusic == -1)
    {
        /* Will play the same music without any pending play changes,
           just change the loop attribute */
        fLoop = mfLoop;
        return TRUE;
    }

    if (FadeType != AudioRixPlayer::FADE_OUT)
    {
        if (FadeType == AudioRixPlayer::FADE_IN && iTotalFadeInSamples > 0 && iRemainingFadeSamples > 0)
        {
            dwStartFadeTime = SDL_GetTicks() - (int)((float)iRemainingFadeSamples / iTotalFadeInSamples * flFadeTime * (1000 / 2));
        }
        else
        {
            dwStartFadeTime = SDL_GetTicks();
        }
        iTotalFadeOutSamples = (int)round(flFadeTime / 2.0f * lpAudioData->SampleRate) * lpAudioData->AudioChannels;
        iRemainingFadeSamples = iTotalFadeOutSamples;
        iTotalFadeInSamples = iTotalFadeOutSamples;
    }
    else
    {
        iTotalFadeInSamples = (int)round(flFadeTime / 2.0f * lpAudioData->SampleRate) * lpAudioData->AudioChannels;
    }

    iNextMusic = iNumRIX;
    FadeType = AudioRixPlayer::FADE_OUT;
    fNextLoop = fLoop;
    fReady = TRUE;

    return TRUE;
}

void AudioRixPlayer::fillBuffer(ByteArray& bstream, int mlen)
{
    //auto gConfig = CPalEvent::gConfig;
    auto stream = bstream.data();
    auto len = mlen;
	if (rix == nullptr)
		return;
    while (len > 0)
    {
        INT       volume, delta_samples = 0, vol_delta = 0;

        //
        // fading in or fading out
        //
        switch (FadeType)
        {
        case AudioRixPlayer::FADE_IN:
            if (iRemainingFadeSamples <= 0)
            {
                FadeType = AudioRixPlayer::NONE;
                volume = SDL_MIX_MAXVOLUME;
            }
            else
            {
                volume = (INT)(SDL_MIX_MAXVOLUME * (1.0 - (double)iRemainingFadeSamples / iTotalFadeInSamples));
                delta_samples = (int)(iTotalFadeInSamples / SDL_MIX_MAXVOLUME) & ~(lpAudioData->AudioChannels - 1); vol_delta = 1;
            }
            break;
        case AudioRixPlayer::FADE_OUT:
            if (iTotalFadeOutSamples == iRemainingFadeSamples && iTotalFadeOutSamples > 0)
            {
                UINT  now = SDL_GetTicks();
                INT u = AudioPlayer::lpAudioData->lpAudioDevice->spec.freq;
                INT   passed_samples = ((INT)(now - dwStartFadeTime) > 0) ? (INT)((now - dwStartFadeTime) * u / 1000) : 0;
                iRemainingFadeSamples -= passed_samples;
            }
            if (iMusic == -1 || iRemainingFadeSamples <= 0)
            {
                //
                // There is no current playing music, or fading time has passed.
                // Start playing the next one or stop playing.
                //
                if (iNextMusic > 0)
                {
                    iMusic = iNextMusic;
                    iNextMusic = -1;
                    fLoop = fNextLoop;
                    FadeType = AudioRixPlayer::FADE_IN;
                    if (iMusic > 0)
                        dwStartFadeTime += iTotalFadeOutSamples * 1000 / lpAudioData->SampleRate;
                    else
                        dwStartFadeTime = SDL_GetTicks();
                    iTotalFadeOutSamples = 0;
                    iRemainingFadeSamples = iTotalFadeInSamples;
                    rix->rewind(iMusic);
                    if (resampler[0]) resampler_clear(resampler[0]);
                    if (resampler[1]) resampler_clear(resampler[1]);
                    continue;
                }
                else
                {
                    iMusic = -1;
                    FadeType = AudioRixPlayer::NONE;
                    memset(bstream.data(), 0, mlen);
                    return;
                }
            }
            else
            {
                volume = (INT)(SDL_MIX_MAXVOLUME * ((double)iRemainingFadeSamples / iTotalFadeOutSamples));
                delta_samples = (int)(iTotalFadeOutSamples / SDL_MIX_MAXVOLUME) & ~(lpAudioData->AudioChannels - 1); vol_delta = -1;
            }
            break;
        default:
            if (iMusic <= 0)
            {
                //
                // No current playing music
                //
                memset(bstream.data(), 0, mlen);
                return;
            }
            else
            {
                volume = SDL_MIX_MAXVOLUME;
            }
        }

        //
        // Fill the buffer with sound data
        //
        int buf_max_len = lpAudioData->SampleRate / 70 * lpAudioData->AudioChannels * sizeof(short);
        bool fContinue = true;
        while (len > 0 && fContinue)
        {
            if (pos == NULL || pos - rixSoundBuf >= buf_max_len)
            {
                pos = rixSoundBuf;
                if (!rix)
                    return;
                if (!rix->update())
                {
                    if (!fLoop)
                    {
                        //
                        // Not loop, simply terminate the music
                        //
                        iMusic = -1;
                        if (FadeType != AudioRixPlayer::FADE_OUT && iNextMusic == -1)
                        {
                            FadeType = AudioRixPlayer::NONE;
                        }
                        //
                        return;
                    }
                    rix->rewindReInit(iMusic, false);
                    if (!rix->update())
                    {
                        //
                        // Something must be wrong
                        //
                        iMusic = -1;
                        FadeType = AudioRixPlayer::NONE;
                        return;
                    }
                }
                int sample_count = lpAudioData->SampleRate / 70;
                if (resampler[0])
                {
                    unsigned int samples_written = 0;
                    short* finalBuf = (short*)rixSoundBuf;

                    while (sample_count)
                    {
                        int to_write = resampler_get_free_count(resampler[0]);
                        if (to_write)
                        {
                            short* tempBuf = (short*)alloca(to_write * lpAudioData->AudioChannels * sizeof(short));
                            int temp_buf_read = 0;
                            sOpl->update(tempBuf, to_write);
                            for (int i = 0; i < to_write * lpAudioData->AudioChannels; i++)
                                resampler_write_sample(resampler[i % lpAudioData->AudioChannels], tempBuf[temp_buf_read++]);
                        }

                        int to_get = resampler_get_sample_count(resampler[0]);
                        if (to_get > sample_count) to_get = sample_count;
                        for (int i = 0; i < to_get * lpAudioData->AudioChannels; i++)
                            finalBuf[samples_written++] = resampler_get_and_remove_sample(resampler[i % lpAudioData->AudioChannels]);
                        sample_count -= to_get;
                    }
                }
                else
                {
                    sOpl->update((short*)(rixSoundBuf), sample_count);
                }
            }

            int l = buf_max_len - (pos - rixSoundBuf);
            l = (l > len) ? len / sizeof(short) : l / sizeof(short);

            //
            // Put audio data into buffer and adjust volume
            //
            if (FadeType != AudioRixPlayer::NONE)
            {
                short* ptr = (short*)stream;
                for (int i = 0; i < l && iRemainingFadeSamples > 0; volume += vol_delta)
                {
                    int j = 0;
                    for (j = 0; i < l && j < delta_samples; i++, j++)
                    {
                        *ptr++ = *(short*)pos * volume / SDL_MIX_MAXVOLUME;
                        pos += sizeof(short);
                    }
                    iRemainingFadeSamples -= j;
                }
                fContinue = (iRemainingFadeSamples > 0);
                len -= (LPBYTE)ptr - stream; stream = (LPBYTE)ptr;
            }
            else
            {
                memcpy(stream, pos, l * sizeof(short));
                pos += l * sizeof(short);
                stream += l * sizeof(short);
                len -= l * sizeof(short);
            }
        }
    }
}

