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
//coggplayer.cpp

//预定义宏USING_OGG = 1以启用此代码
#if USING_OGG
#include <vorbis/vorbisfile.h>

#include "caudiodevice.h"

#include "../main/cconfig.h"
#include "../main/cpalevent.h"

#define FLAG_OY 0x01
#define FLAG_VI 0x02
#define FLAG_VC 0x04
#define FLAG_OS 0x08
#define FLAG_VD 0x10
#define FLAG_VB 0x20

#define STAGE_PAGEOUT    1
#define STAGE_PACKETOUT  2
#define STAGE_PCMOUT	 3
#define STAGE_REWIND     4

#define OGG_BUFFER_LENGTH 4096

#define AUDIO_IsIntegerConversion(a) (((a) % CPalEvent::ggConfig->iSampleRate) == 0 || (CPalEvent::ggConfig->iSampleRate % (a)) == 0)

#if defined(_MSC_VER)
# define PAL_FORCE_INLINE static SDL_FORCE_INLINE
#else
# define PAL_FORCE_INLINE SDL_FORCE_INLINE
#endif

#ifdef _WIN32

# include <windows.h>
# include <io.h>
#include <cmath>

# if defined(_MSC_VER)
#  if _MSC_VER < 1900
#   define vsnprintf _vsnprintf
#   define snprintf _snprintf
#  endif
#  define strdup _strdup
#  define access _access
#  pragma warning (disable:4244)
# endif
#endif



PAL_FORCE_INLINE ogg_int16_t OGG_GetSample(float pcm)
{
	int val = (int)(floor(pcm * 32767.f + .5f));
	/* might as well guard against clipping */
	if (val > 32767) {
		val = 32767;
	}
	else if (val < -32768) {
		val = -32768;
	}
	return (ogg_int16_t)val;
}

void COggPlayer::OGG_FillResample(ogg_int16_t* stream)
{
	if (CPalEvent::ggConfig->iAudioChannels == 2) {
		stream[0] = resampler_get_and_remove_sample(resampler[0]);
		stream[1] = (vi.channels > 1) ? resampler_get_and_remove_sample(resampler[1]) : stream[0];
	}
	else {
		if (vi.channels > 1) {
			*stream = (short)((int)(resampler_get_and_remove_sample(resampler[0]) + resampler_get_and_remove_sample(resampler[1])) >> 1);
		}
		else {
			*stream = resampler_get_and_remove_sample(resampler[0]);
		}
	}
}

COggPlayer::COggPlayer()
{
#ifdef  USE_EVENTHANDLER
	CPalEventHandler _m;
	Uint32 customEventType = _m.getCustomEventHandler(eventTypeCustom::eventOggMusicPlay);
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
		_m.setCustomMutex((UINT)eventTypeCustom::eventOggMusicPlay, FALSE);
		});
#endif //USE_EVENTHANDLER
	iMusic = -1;
	fLoop = FALSE;
	fp = NULL;
	iMusic = -1;
	iFlags = 0;
	iStage = 0;
	fLoop = FALSE;
	fReady = FALSE;
	fUseResampler = FALSE;

	resampler[0] = resampler_create();
	if (resampler[0])
	{
		resampler[1] = resampler_create();
		if (resampler[1] == NULL)
		{
			resampler_delete(resampler[0]);
			resampler[0] = NULL;
		}
	}
}

COggPlayer::~COggPlayer()
{
	OGG_Cleanup();
	resampler_delete(resampler[0]);
	resampler_delete(resampler[1]);
	CPalEvent::UTIL_CloseFile(fp);
	fp = NULL;
	resampler[0] = nullptr;
	resampler[1] = nullptr;
}

int COggPlayer::Player(int iNum, bool fLoop, float flFadeTime)
{

	this->fLoop = fLoop;

	if (iNum == iMusic)
	{
		return TRUE;
	}

	fReady = FALSE;
	OGG_Cleanup();
	if (fp)
	{
		CPalEvent::UTIL_CloseFile(fp);
		fp = NULL;
	}

	iMusic = iNum;

	if (iNum == -1)
	{
		return TRUE;
	}
	std::string fname = CPalEvent::va("OGG\\%.2d.ogg", iNum);

	fp = CPalEvent::UTIL_OpenFile(fname.c_str());
	if (fp == NULL)
	{
		return FALSE;
	}

	if (!OGG_Rewind())
	{
		CPalEvent::UTIL_CloseFile(fp);
		fp = NULL;
		return FALSE;
	}

	return TRUE;
}

void COggPlayer::OGG_Cleanup()
{
	int i;
	for (i = 0; i < CPalEvent::ggConfig->iAudioChannels; i++) 
		resampler_clear(resampler[0]);
	/* Do various cleanups */
	if (iFlags & FLAG_VB) vorbis_block_clear(&vb);
	if (iFlags & FLAG_VD) vorbis_dsp_clear(&vd);
	if (iFlags & FLAG_OS) ogg_stream_clear(&os);
	if (iFlags & FLAG_VC) vorbis_comment_clear(&vc);
	if (iFlags & FLAG_VI) vorbis_info_clear(&vi);  /* must be called last */
	if (iFlags & FLAG_OY) ogg_sync_clear(&oy);
	iFlags = iStage = 0;
	fReady = FALSE;
}

BOOL COggPlayer::OGG_Rewind()
{
	ogg_packet       op; /* one raw packet of data for decode */
	char* buffer;
	int i, bytes;

	OGG_Cleanup();

	fseek(fp, 0, SEEK_SET);

	ogg_sync_init(&oy); iFlags = FLAG_OY;

	/* grab some data at the head of the stream. We want the first page
	(which is guaranteed to be small and only contain the Vorbis
	stream initial header) We need the first page to get the stream
	serialno. */

	/* submit a 4k block to libvorbis' Ogg layer */
	buffer = ogg_sync_buffer(&oy, OGG_BUFFER_LENGTH);
	bytes = fread(buffer, 1, OGG_BUFFER_LENGTH, fp);
	ogg_sync_wrote(&oy, bytes);

	/* Get the first page. */
	if (ogg_sync_pageout(&oy, &og) != 1) {
		/* have we simply run out of data?  If so, we're done. */
		/* error case.  Must not be Vorbis data */
		OGG_Cleanup();
		return (fReady = FALSE);
	}

	/* Get the serial number and set up the rest of decode. */
	/* serialno first; use it to set up a logical stream */
	ogg_stream_init(&os, ogg_page_serialno(&og));
	iFlags |= FLAG_OS;

	/* extract the initial header from the first page and verify that the
	Ogg bitstream is in fact Vorbis data */

	/* I handle the initial header first instead of just having the code
	read all three Vorbis headers at once because reading the initial
	header is an easy way to identify a Vorbis bitstream and it's
	useful to see that functionality seperated out. */

	vorbis_info_init(&vi); iFlags |= FLAG_VI;
	vorbis_comment_init(&vc); iFlags |= FLAG_VC;
	if (ogg_stream_pagein(&os, &og) < 0) {
		/* error; stream version mismatch perhaps */
		OGG_Cleanup();
		return (fReady = FALSE);
	}

	if (ogg_stream_packetout(&os, &op) != 1) {
		/* no page? must not be vorbis */
		OGG_Cleanup();
		return (fReady = FALSE);
	}

	if (vorbis_synthesis_headerin(&vi, &vc, &op) < 0) {
		/* error case; not a vorbis header */
		OGG_Cleanup();
		return (fReady = FALSE);
	}

	/* At this point, we're sure we're Vorbis. We've set up the logical
	(Ogg) bitstream decoder. Get the comment and codebook headers and
	set up the Vorbis decoder */

	/* The next two packets in order are the comment and codebook headers.
	They're likely large and may span multiple pages. Thus we read
	and submit data until we get our two packets, watching that no
	pages are missing. If a page is missing, error out; losing a
	header page is the only place where missing data is fatal. */

	i = 0;
	while (i < 2) {
		while (i < 2) {
			int result = ogg_sync_pageout(&oy, &og);
			if (result == 0)break; /* Need more data */
			/* Don't complain about missing or corrupt data yet. We'll
			catch it at the packet output phase */
			if (result == 1) {
				ogg_stream_pagein(&os, &og); /* we can ignore any errors here
															 as they'll also become apparent
															 at packetout */
				while (i < 2) {
					result = ogg_stream_packetout(&os, &op);
					if (result == 0)break;
					if (result < 0) {
						/* Uh oh; data at some point was corrupted or missing!
						We can't tolerate that in a header.  Die. */
						OGG_Cleanup();
						return (fReady = FALSE);
					}
					result = vorbis_synthesis_headerin(&vi, &vc, &op);
					if (result < 0) {
						OGG_Cleanup();
						return (fReady = FALSE);
					}
					i++;
				}
			}
		}
		/* no harm in not checking before adding more */
		buffer = ogg_sync_buffer(&oy, OGG_BUFFER_LENGTH);
		bytes = fread(buffer, 1, OGG_BUFFER_LENGTH, fp);
		if (bytes == 0 && i < 2) {
			OGG_Cleanup();
			return (fReady = FALSE);
		}
		ogg_sync_wrote(&oy, bytes);
	}

	if (vorbis_synthesis_init(&vd, &vi) == 0) { /* central decode state */
		vorbis_block_init(&vd, &vb);            /* local state for most of the decode
																so multiple block decodes can
																proceed in parallel. We could init
																multiple vorbis_block structures
																for vd here */
		iStage = STAGE_PAGEOUT;
		iFlags |= FLAG_VD | FLAG_VB;
		fUseResampler = (vi.rate != CPalEvent::ggConfig->iSampleRate);

		if (fUseResampler) {
			double factor = (double)vi.rate / (double)CPalEvent::ggConfig->iSampleRate;
			for (i = 0; i < std::min(vi.channels, 2); i++)
			{
				resampler_set_quality(resampler[i], AUDIO_IsIntegerConversion(vi.rate) ? RESAMPLER_QUALITY_MIN : CPalEvent::ggConfig->iResampleQuality);
				resampler_set_rate(resampler[i], factor);
				resampler_clear(resampler[i]);
			}
		}
		return (fReady = TRUE);
	}
	else {
		OGG_Cleanup();
		return (fReady = FALSE);
	}
}

void COggPlayer::fillBuffer(
	ByteArray& stream,
	INT         len
)
{

	if (fReady) {
		ogg_packet       op; /* one raw packet of data for decode */
		int total_bytes = 0, stage = iStage;

		while (total_bytes < len) {
			float** pcm;
			int samples, result;

			switch (stage)
			{
			case STAGE_PAGEOUT: /* PAGEOUT stage */
				result = ogg_sync_pageout(&oy, &og);
				if (result > 0) {
					/* can safely ignore errors at this point */
					ogg_stream_pagein(&os, &og);
					stage = STAGE_PACKETOUT;
				}
				else {
					if (result == 0) { /* need more data */
						char* buffer = ogg_sync_buffer(&oy, OGG_BUFFER_LENGTH);
						int bytes = fread(buffer, 1, OGG_BUFFER_LENGTH, fp);
						ogg_sync_wrote(&oy, bytes);
						stage = (bytes > 0) ? STAGE_PAGEOUT : STAGE_REWIND;
					}
					break;
				}
			case STAGE_PACKETOUT:
				result = ogg_stream_packetout(&os, &op);
				if (result > 0) {
					/* we have a packet.  Decode it */
					if (vorbis_synthesis(&vb, &op) == 0) { /* test for success! */
						vorbis_synthesis_blockin(&vd, &vb);
					}
					stage = STAGE_PCMOUT;
				}
				else {
					if (result == 0) { /* need more data */
						if (ogg_page_eos(&og)) {
							if (fLoop) {
								stage = STAGE_REWIND;
							}
							else {
								OGG_Cleanup();
								CPalEvent::UTIL_CloseFile(fp);
								fp = NULL;
								return;
							}
						}
						else {
							stage = STAGE_PAGEOUT;
						}
					}
					break;
				}
			case STAGE_PCMOUT:
				if ((samples = vorbis_synthesis_pcmout(&vd, &pcm)) > 0) {
					int bout;
					if (fUseResampler) { /* Resampler is available and should be used */
						bout = 0;
						while (total_bytes < len && samples > 0) { /* Fill as many samples into resampler as possible */
							int i, j, to_write = resampler_get_free_count(resampler[0]);

							if (to_write > 0) {
								if (to_write >= samples) to_write = samples;

								for (i = 0; i < std::min(vi.channels, 2); i++) {
									float* mono = pcm[i] + bout;
									for (j = 0; j < to_write; j++) {
										resampler_write_sample(resampler[i], OGG_GetSample(mono[j]));
									}
								}
							}

							/* Fetch resampled samples if available */
							j = resampler_get_sample_count(resampler[0]);
							while (total_bytes < len && resampler_get_sample_count(resampler[0]) > 0) {
								OGG_FillResample((ogg_int16_t*)(stream.data() + total_bytes));
								total_bytes += CPalEvent::ggConfig->iAudioChannels * sizeof(ogg_int16_t);
							}

							samples -= to_write; bout += to_write;
						}
					}
					else {
						int i;
						ogg_int16_t* ptr = (ogg_int16_t*)(stream.data() + total_bytes);
						bout = (len - total_bytes) / CPalEvent::ggConfig->iAudioChannels / sizeof(ogg_int16_t);
						if (bout > samples) bout = samples;
						for (i = 0; i < bout; i++) {
							if (CPalEvent::ggConfig->iAudioChannels == 2) {
								ptr[0] = OGG_GetSample(pcm[0][i]);
								ptr[1] = (vi.channels > 1) ? OGG_GetSample(pcm[1][i]) : ptr[0];
							}
							else {
								if (vi.channels > 1) {
									ptr[0] = (short)((int)(OGG_GetSample(pcm[0][i]) + OGG_GetSample(pcm[1][i])) >> 1);
								}
								else {
									ptr[0] = OGG_GetSample(pcm[0][i]);
								}
							}
							ptr += CPalEvent::ggConfig->iAudioChannels;
						}

						total_bytes += bout * CPalEvent::ggConfig->iAudioChannels * sizeof(ogg_int16_t);
					}
					/* tell libvorbis how many samples we actually consumed */
					vorbis_synthesis_read(&vd, bout);
				}
				else {
					stage = STAGE_PACKETOUT;
				}
				break;
			case STAGE_REWIND:
				if (vi.rate != CPalEvent::ggConfig->iSampleRate) { /* If there are samples in the resampler, fetch them first */
					while (total_bytes < len && resampler_get_sample_count(resampler[0]) > 0) {
						OGG_FillResample((ogg_int16_t*)(stream.data() + total_bytes));
						total_bytes += CPalEvent::ggConfig->iAudioChannels * sizeof(ogg_int16_t);
					}
					/* Break out if there are still samples in the resampler */
					if (resampler_get_sample_count(resampler[0]) > 0) break;
				}
				OGG_Rewind();
				stage = iStage;
				break;
			default:
				return;
			}
		}
		iStage = stage;
	}
}

#endif