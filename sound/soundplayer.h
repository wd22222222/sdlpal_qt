#pragma once
#include "caudiodevice.h"

//声音播放结构
class SoundPlay :public AudioPlayer
{
private:
    FILE* mkf{};		                /* File pointer to the MKF file */
    SoundLoader         LoadSound{};	/* The function pointer for load WAVE/VOC data */
    WAVEDATA            soundlist{};
    int                 cursounds{};

    static int SOUND_ResampleMix_S16_Mono_Mono(
        void* resampler[2], const void* lpData, const WAVESPEC* lpSpec,
        void* lpBuffer, int iBufLen, const void** llpData);

    static int SOUND_ResampleMix_S16_Mono_Stereo(
        void* resampler[2], const void* lpData, const WAVESPEC* lpSpec,
        void* lpBuffer, int iBufLen, const void** llpData);

    static int SOUND_ResampleMix_S16_Stereo_Mono(
        void* resampler[2], const void* lpData, const WAVESPEC* lpSpec,
        void* lpBuffer, int iBufLen, const void** llpData);

    static int  SOUND_ResampleMix_S16_Stereo_Stereo(
        void* resampler[2], const void* lpData, const WAVESPEC* lpSpec,
        void* lpBuffer, int  iBufLen, const void** llpData);

    static int SOUND_ResampleMix_U8_Mono_Mono(
        void* resampler[2], const void* lpData, const WAVESPEC* lpSpec,
        void* lpBuffer, int iBufLen, const void** llpData);

    static int SOUND_ResampleMix_U8_Mono_Stereo(
        void* resampler[2], const void* lpData, const WAVESPEC* lpSpec,
        void* lpBuffer, int iBufLen, const void** llpData);

    static int SOUND_ResampleMix_U8_Stereo_Mono(
        void* resampler[2], const void* lpData, const WAVESPEC* lpSpec,
        void* lpBuffer, int iBufLen, const void** llpData);

    static int SOUND_ResampleMix_U8_Stereo_Stereo(
        void* resampler[2], const void* lpData, const WAVESPEC* lpSpec,
        void* lpBuffer, int iBufLen, const void** llpData);
    INT PAL_MKFGetChunkSize(UINT uiChunkNum, FILE* fp);
    INT PAL_MKFReadChunk(
        LPBYTE          lpBuffer,
        UINT            uiBufferSize,
        UINT            uiChunkNum,
        FILE* fp
    );
    INT PAL_MKFGetChunkCount(FILE* fp);



public:

    SoundPlay();

    virtual int Player(int iSoundNum, bool, float);
    virtual void fillBuffer(ByteArray&, int len);;
    virtual ~SoundPlay();;
};
