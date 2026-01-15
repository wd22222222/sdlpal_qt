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

#include "sdl2_compat.h"

#if USING_SDL3

static bool
SDL_IsSupportedAudioFormat(const SDL_AudioFormat fmt)
{
    switch (fmt) {
    case SDL_AUDIO_U8:
    case SDL_AUDIO_S8:
    case SDL_AUDIO_S16LE:
    case SDL_AUDIO_S16BE:
    case SDL_AUDIO_S32LE:
    case SDL_AUDIO_S32BE:
    case SDL_AUDIO_F32LE:
    case SDL_AUDIO_F32BE:
        return true; /* supported. */

    default:
        break;
    }

    return false; /* unsupported. */
}

static bool SDL_IsSupportedChannelCount(const int channels)
{
    return ((channels >= 1) && (channels <= 8));
}
SDL_Surface* SDL_CreateRGBSurface(Uint32 flags, int width, int height, int depth, Uint32 Rmask, Uint32 Gmask, Uint32 Bmask, Uint32 Amask)
{
    return SDL_CreateSurface(width, height,
        SDL_GetPixelFormatForMasks(depth, Rmask, Gmask, Bmask, Amask));
}

#define RESAMPLER_BITS_PER_SAMPLE           16
#define RESAMPLER_SAMPLES_PER_ZERO_CROSSING (1 << ((RESAMPLER_BITS_PER_SAMPLE / 2) + 1))
typedef struct {
    /* src_format is read directly from the AudioCVT in real SDL */
    Uint8 src_channels;
    int src_rate;
    SDL_AudioFormat dst_format;
    Uint8 dst_channels;
    int dst_rate;
} AudioParam;

static void SDLCALL AudioCVTFilter(SDL_AudioCVT* cvt, SDL_AudioFormat src_format)
{
    SDL_AudioStream* stream2;
    int src_len, dst_len, real_dst_len;
    int src_samplesize;
    SDL_AudioSpec src_spec, dst_spec;

    { /* Fetch from the end of filters[], aligned */
        AudioParam ap;

        SDL_memcpy(
            &ap,
            (Uint8*)&cvt->filters[SDL_AUDIOCVT_MAX_FILTERS + 1] - (sizeof(AudioParam) & ~3),
            sizeof(ap));

        src_spec.format = src_format;
        src_spec.channels = ap.src_channels;
        src_spec.freq = ap.src_rate;
        dst_spec.format = ap.dst_format;
        dst_spec.channels = ap.dst_channels;
        dst_spec.freq = ap.dst_rate;
    }

    /* don't use the SDL stream directly or even SDL_ConvertAudioSamples; we want the U16 support in the SDL-compat layer */
    stream2 = SDL_CreateAudioStream(&src_spec, &dst_spec);
    if (stream2 == NULL) {
        goto exit;
    }

    src_samplesize = (SDL_AUDIO_BITSIZE(src_format) / 8) * src_spec.channels;

    src_len = cvt->len_cvt & ~(src_samplesize - 1);
    dst_len = cvt->len * cvt->len_mult;

    /* Run the audio converter */
    if (SDL_PutAudioStreamData(stream2, cvt->buf, src_len) != SDL_OK ||
        SDL_FlushAudioStream(stream2) != SDL_OK) {
        goto exit;
    }

    /* Get back in the same buffer */
    real_dst_len = SDL_GetAudioStreamData(stream2, cvt->buf, dst_len);
    if (real_dst_len < 0) {
        goto exit;
    }

    cvt->len_cvt = real_dst_len;

exit:
    SDL_DestroyAudioStream(stream2);

    /* Call the next filter in the chain */
    if (cvt->filters[++cvt->filter_index]) {
        cvt->filters[cvt->filter_index](cvt, dst_spec.format);
    }
}
/// <summary>
/// ///
/// </summary>

// 键盘事件访问
SDL_Keycode SDL_GetKeyboardKeycode(const SDL_Event* event) {
    return event->key.key;
}

// 鼠标事件访问
Uint8 SDL_GetMouseButton(const SDL_Event* event) {
    return event->button.button;
}
// 鼠标事件访问
int SDL_GetMouseX(const SDL_Event* event) {
    return event->button.x;
}
// 鼠标事件访问
int SDL_GetMouseY(const SDL_Event* event) {
    return event->button.y;
}
// 事件类型访问包装
Uint8 SDL_GetWindowEventType(const SDL_Event* event) {
    return event->type; // SDL3 中窗口事件就是主类型
}


// 简单的包装函数
int SDL_BuildAudioCVT(SDL_AudioCVT* cvt,
    SDL_AudioFormat src_format, Uint8 src_channels, int src_rate,
    SDL_AudioFormat dst_format, Uint8 dst_channels, int dst_rate)
{
    /* Sanity check target pointer */
    if (cvt == NULL) {
        SDL_InvalidParamError("cvt");
        return -1;
    }

    /* Make sure we zero out the audio conversion before error checking */
    SDL_zerop(cvt);

    if (!SDL_IsSupportedAudioFormat(src_format)) {
        SDL_SetError("Invalid source format");
        return -1;
    }
    if (!SDL_IsSupportedAudioFormat(dst_format)) {
        SDL_SetError("Invalid destination format");
        return -1;
    }
    if (!SDL_IsSupportedChannelCount(src_channels)) {
        SDL_SetError("Invalid source channels");
        return -1;
    }
    if (!SDL_IsSupportedChannelCount(dst_channels)) {
        SDL_SetError("Invalid destination channels");
        return -1;
    }
    if (src_rate <= 0) {
        SDL_SetError("Source rate is equal to or less than zero");
        return -1;
    }
    if (dst_rate <= 0) {
        SDL_SetError("Destination rate is equal to or less than zero");
        return -1;
    }
    if (src_rate >= SDL_MAX_SINT32 / RESAMPLER_SAMPLES_PER_ZERO_CROSSING) {
        SDL_SetError("Source rate is too high");
        return -1;
    }
    if (dst_rate >= SDL_MAX_SINT32 / RESAMPLER_SAMPLES_PER_ZERO_CROSSING) {
        SDL_SetError("Destination rate is too high");
        return -1;
    }

#ifdef DEBUG_CONVERT
    SDL_Log("SDL_AUDIO_CONVERT: Build format %04x->%04x, channels %u->%u, rate %d->%d\n",
        src_format, dst_format, src_channels, dst_channels, src_rate, dst_rate);
#endif

    /* Start off with no conversion necessary */
    cvt->src_format = src_format;
    cvt->dst_format = dst_format;
    cvt->needed = 0;
    cvt->filter_index = 0;
    SDL_zeroa(cvt->filters);
    cvt->len_mult = 1;
    cvt->len_ratio = 1.0;
    cvt->rate_incr = ((double)dst_rate) / ((double)src_rate);

    { /* Use the filters[] to store some data ... */
        AudioParam ap;
        ap.src_channels = src_channels;
        ap.src_rate = src_rate;
        ap.dst_format = dst_format;
        ap.dst_channels = dst_channels;
        ap.dst_rate = dst_rate;

        /* Store at the end of filters[], aligned */
        SDL_memcpy(
            (Uint8*)&cvt->filters[SDL_AUDIOCVT_MAX_FILTERS + 1] - (sizeof(AudioParam) & ~3),
            &ap,
            sizeof(ap));

        cvt->needed = 1;
        if (src_format == dst_format && src_rate == dst_rate && src_channels == dst_channels) {
            cvt->needed = 0;
        }

        if (src_format != dst_format) {
            const Uint16 src_bitsize = SDL_AUDIO_BITSIZE(src_format);
            const Uint16 dst_bitsize = SDL_AUDIO_BITSIZE(dst_format);

            if (src_bitsize < dst_bitsize) {
                const int mult = (dst_bitsize / src_bitsize);
                cvt->len_mult *= mult;
                cvt->len_ratio *= mult;
            }
            else if (src_bitsize > dst_bitsize) {
                const int div = (src_bitsize / dst_bitsize);
                cvt->len_ratio /= div;
            }
        }

        if (src_channels < dst_channels) {
            cvt->len_mult = ((cvt->len_mult * dst_channels) + (src_channels - 1)) / src_channels;
        }

        if (src_rate < dst_rate) {
            const double mult = ((double)dst_rate / (double)src_rate);
            cvt->len_mult *= (int)SDL_ceil(mult);
            cvt->len_ratio *= mult;
        }
        else {
            const double divisor = ((double)src_rate / (double)dst_rate);
            cvt->len_ratio /= divisor;
        }
    }

    if (cvt->needed) {
        /* Insert a single filter to perform all necessary audio conversion.
         * Some apps may examine or modify the filter chain, so we use a real
         * SDL-style audio filter function to keep those apps happy. */
        cvt->filters[0] = AudioCVTFilter;
        cvt->filters[1] = NULL;
        cvt->filter_index = 1;
    }

    return cvt->needed;
}

SDL_Surface* SDL_ConvertSurfaceFormat(SDL_Surface* src,
    const SDL_PixelFormat fmt, Uint32 flags)
{

    return SDL_ConvertSurface(src, fmt);
};

SDL_DECLSPEC int SDLCALL
SDL_ConvertAudio(SDL_AudioCVT* cvt)
{
    /* Make sure there's data to convert */
    if (!cvt->buf) {
        return SDL_SetError("No buffer allocated for conversion");
    }

    /* Return okay if no conversion is necessary */
    cvt->len_cvt = cvt->len;
    if (cvt->filters[0] == NULL) {
        return 0;
    }

    /* Set up the conversion and go! */
    cvt->filter_index = 0;
    cvt->filters[0](cvt, cvt->src_format);
    return 0;
}

#endif

