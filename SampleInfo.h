#pragma once
#include "eAudioFormat.h"

namespace Audio
{
   struct SampleInfo
    {
        SampleInfo() : m_format(0), m_bytesPerSample(0) {}
        SampleInfo(std::uint32_t format, std::uint32_t bytes)
            : m_format(format)
            , m_bytesPerSample(bytes)
        {

        }
        std::uint32_t m_format,
            m_bytesPerSample;
    };

    static SampleInfo SampleInformation[audio_format_count] =
    {
        { audio_format_unknown, 0 },
        { audio_format_u8,      1 },
        { audio_format_s16,     2 },
        { audio_format_s24,     3 },
        { audio_format_s32,     4 },
        { audio_format_f32,     4 },
    };
}