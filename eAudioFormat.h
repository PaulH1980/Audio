#pragma once

#include <cstdint>

namespace Audio
{
    enum eAudioFormat : std::uint32_t
    {
        audio_format_unknown  = 0,     // Mainly used for indicating an error, but also used as the default for the output format for decoders.
        audio_format_u8       = 1,
        audio_format_s16      = 2,     // Seems to be the most widely supported format.
        audio_format_s24      = 3,     // Tightly packed. 3 bytes per sample.
        audio_format_s32      = 4,
        audio_format_f32      = 5,
        audio_format_count
    };
}