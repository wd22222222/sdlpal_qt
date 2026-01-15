#pragma once
#ifndef SOUNDSETING_H
#define	SOUNDSETING_H

//soundseting.h

#include "../main/sdl2_compat.h"
#include <string>
#include "../main/command.h"

class CAudioDevice;

class AudioData
{
public:
	BOOL	musicLoop{};
	FLOAT	flFadeTime{};
	BOOL	locked{};
	int		Music{};
	int		sound{};

public:
	INT MusicVolume{};
	INT SoundVolume{};
	INT AudioChannels{};
	INT SampleRate{};
	INT ResampleQuality{};
	INT AudioBufferSize{};
	INT mAudioDevice{ -1 };
	INT MusicType{};
	INT FirstUseMp3{};
	//SDL_AudioStream* stream{};
	//int AudioDeviceID{};
	INT OPLChip{};
	INT OPLCore{};
	INT OPLSampleRate{};
	INT UseSurroundOPL{};
	INT SurroundOPLOffset{};
	CAudioDevice* lpAudioDevice{};
	bool NoMusic{};
	bool NoSound{};
	bool IsWIN95{};
	std::string PalDir;
	AudioData() {};
	~AudioData() {};
};


typedef enum _OPLCORE_TYPE {
	OPLCORE_MAME,
	OPLCORE_DBFLT,
	OPLCORE_DBINT,
	OPLCORE_NUKED,
} OPLCORE_TYPE;

typedef enum _OPLCHIP_TYPE {
	OPLCHIP_OPL2,
	OPLCHIP_OPL3,
	OPLCHIP_DUAL_OPL2,
} OPLCHIP_TYPE;

typedef enum tagLOGLEVEL
{
	LOGLEVEL_MIN,
	LOGLEVEL_VERBOSE = LOGLEVEL_MIN,
	LOGLEVEL_DEBUG,
	LOGLEVEL_INFO,
	LOGLEVEL_WARNING,
	LOGLEVEL_ERROR,
	LOGLEVEL_FATAL,
	LOGLEVEL_MAX = LOGLEVEL_FATAL,
} LOGLEVEL;

/*typedef enum tagMUSICTYPE
{
	MUSIC_MIDI,
	MUSIC_RIX,
	MUSIC_MP3,
	MUSIC_OGG,
	MUSIC_OPUS
} MUSICTYPE, * LPMUSICTYPE;
*/
enum class musicType
{
	midi,
	rix,
	mp3,
#if USING_OGG
	ogg,
#endif
	endMusicType,
};

typedef enum tagMIDISYNTHTYPE
{
	SYNTH_NATIVE,
	SYNTH_TIMIDITY,
	SYNTH_TINYSOUNDFONT
} MIDISYNTHTYPE, * LPMIDISYNTHTYPE;

typedef struct tagWAVESPEC
{
	int                 size;
	int                 freq;
	SDL_AudioFormat     format;
	uint8_t             channels;
	uint8_t             align;
} WAVESPEC;

typedef const void* (*SoundLoader)(LPCBYTE, DWORD, WAVESPEC*);

typedef int(*ResampleMixer)(void* [2], const void*, const WAVESPEC*, void*, int, const void**);


typedef struct tagWAVEDATA
{
	struct tagWAVEDATA* next;

	void* resampler[2];	/* The resampler used for sound data */
	ResampleMixer       ResampleMix;
	const void* base;
	const void* current;
	const void* end;
	WAVESPEC            spec;
} WAVEDATA;

#endif // SOUNDSETING_H
