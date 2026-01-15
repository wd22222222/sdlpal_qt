//audiodevice.h

#pragma once
#include "../main/sdl2_compat.h"
#include "../main/command.h"
#include "../sound/adplug/emuopls.h"
#include "../sound/adplug/rix.h"
#include "../sound/adplug/convertopl.h"
#include "../sound/adplug/surroundopl.h"
#include "../sound/libmad/music_mad.h"
#include "../sound/libmad/resampler.h"
#include "../main/cpaleventhandler.h"

#define     PAL_MAX_SAMPLERATE          49716
#define     PAL_MAX_VOLUME              100

#if USING_SDL3
# define SDL_CloseAudio() SDL_DestroyAudioStream(lpAudioData->lpAudioDevice->pAudioStream)
#else
# define SDL_CloseAudio() SDL_CloseAudioDevice(lpAudioData->lpAudioDevice->audioID)
# define PAL_LockAudio() SDL_LockAudioDevice(lpAudioData->lpAudioDevice->audioID)
# define PAL_UnlockAudio() SDL_UnlockAudioDevice(lpAudioData->lpAudioDevice->audioID)
#endif

class AudioPlayState//将声音播放放入子进程使用的结构
{
public:
    BOOL musicLoop = 0;
    FLOAT flFadeTime = 0;
    BOOL locked = 0;
    int Music = 0;
    int sound = 0;
};

static AudioPlayState AudioPlay;

class AudioPlayer;

class CAudioDevice
{
public:
    SDL_AudioSpec       spec{};	/* Actual-used sound specification */
    AudioPlayer*        pMusPlayer{};
    AudioPlayer*        pMp3Player{};
    AudioPlayer*        pSoundPlayer{};
#if USING_SDL3
    SDL_AudioStream*    pAudioStream{};
#else
    SDL_AudioDeviceID   audioID{};
#endif

private:
	musicType 		          eCurrentMusicType{}; /* The current music type */
    ByteArray                 pSoundBuffer;   /* The output buffer for sound */
    INT                       bMusicVolume{};	/* The BGM volume ranged in [0, 128] for better performance */
    INT                       bSoundVolume{};	/* The sound effect volume ranged in [0, 128] for better performance */
    BOOL                      fOpened{};       /* Is the audio device opened? */

    void AUDIO_AdjustVolume(short* srcdst, int iVolume, int samples);
    void AUDIO_MixNative(short* dst, short* src, int samples);

public:
    CAudioDevice(const AudioData* set);
    ~CAudioDevice();
    VOID SDLCALL AUDIO_FillBuffer(ByteArray& stream, INT len);
private:
#ifdef USE_EVENTHANDLER
    void pushEventForPlay(eventTypeCustom eventType, ByteArray& stream, int len);
#endif
};

class AudioPlayer
{
public:
	//共享音频数据指针
    inline static AudioData* lpAudioData;

    INT                        iMusic{};
    BOOL                       fLoop{};

    AudioPlayer() {};
    virtual ~AudioPlayer() {};//shutdown
    virtual int  Player(int, bool, float) = 0;
    virtual void fillBuffer(ByteArray&, int len) = 0;
};




//MP3 播放结构
class Mp3player :public AudioPlayer
{
    mad_data* pMP3{};
public:
    Mp3player();
    virtual int Player(int, bool, float);
    VOID MP3_Close();
    virtual void fillBuffer(ByteArray&, int len);
    virtual ~Mp3player();;
};

//Midi 播放结构
class AudioMidiPlayer :public AudioPlayer
{
public:
    virtual int  Player(int, bool, float);
    virtual void fillBuffer(ByteArray&, int len);;

    AudioMidiPlayer();
    virtual ~AudioMidiPlayer();;
};

//Pix播放结构
class AudioRixPlayer :public AudioPlayer
{
    Copl* sOpl{};
    CrixPlayer* rix{};
    void* resampler[2]{};
    BYTE                       rixSoundBuf[(PAL_MAX_SAMPLERATE + 69) / 70 * sizeof(short) * 2]{};
    LPBYTE                     pos{};
    INT                        iNextMusic{}; // the next music number to switch to
    DWORD                      dwStartFadeTime{};
    INT                        iTotalFadeOutSamples{};
    INT                        iTotalFadeInSamples{};
    INT                        iRemainingFadeSamples{};
    enum { NONE, FADE_IN, FADE_OUT } FadeType{}; // fade in or fade out ?
    BOOL                       fNextLoop{};
    BOOL                       fReady{};
public:
    AudioRixPlayer();
    virtual ~AudioRixPlayer();;
    virtual int Player(int, bool, float);
    virtual void fillBuffer(ByteArray&, int len);;
};

#if USING_OGG
#include <vorbis/vorbisfile.h>

class COggPlayer :public AudioPlayer
{
public:
    COggPlayer();
    virtual ~COggPlayer();//shutdown

    virtual int  Player(int, bool, float)override ;
    virtual void fillBuffer(ByteArray&, int len)override ;
private:
    void OGG_Cleanup();
    void OGG_FillResample(ogg_int16_t* stream);
    BOOL OGG_Rewind();

    ogg_sync_state   oy; /* sync and verify incoming physical bitstream */
    ogg_stream_state os; /* take physical pages, weld into a logical stream of packets */
    ogg_page         og; /* one Ogg bitstream page. Vorbis packets are inside */
    vorbis_info      vi; /* struct that stores all the static vorbis bitstream settings */
    vorbis_comment   vc; /* struct that stores all the bitstream user comments */
    vorbis_dsp_state vd; /* central working state for the packet->PCM decoder */
    vorbis_block     vb; /* local working space for packet->PCM decode */

    FILE* fp{};
    void* resampler[2]{};
    INT              iFlags{};
    INT              iStage{};
    INT              nChannels{};
    BOOL             fReady{};
    BOOL             fUseResampler{};
};
#endif
