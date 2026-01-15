#pragma once

#ifndef CSOUND_H
#define CSOUND_H

#include "../main/cpalui.h"


class CAudioDevice;

class CSound : public CPalUI
{
private:
    ByteArray MusicBuf;
public:
    CAudioDevice* gAudioDevice{};
    AudioData gAudioData{};
    SDL_mutex* soundMutex{};

public:
    CSound();
    ~CSound();
    //播放声音
    VOID SOUND_Play(INT i);
    //播放音乐
    VOID PAL_PlayMUS(INT iNumRIX, BOOL fLoop, FLOAT flFadeTime);;
    VOID SoundRunThread();
private:
    VOID SDLCALL AUDIO_FillBuffer(ByteArray& stream, INT len);
    VOID AUDIO_PlayMusicInRun(INT iNumRIX, BOOL fLoop, FLOAT flFadeTime);
    VOID AUDIO_PlayMusic(INT iNumRIX, BOOL fLoop, FLOAT flFadeTime);
    VOID AUDIO_PlaySound(INT iSoundNum);
    VOID soundRun();
    PalErr AUDIO_OpenDevice();
};

#endif // CSOUND_H


