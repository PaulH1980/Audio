#pragma once

#include "AudioBuffer.h"

namespace Audio
{
    struct AudioFileBase
    {
        AudioFileBase() : m_totalLength(0.0f) {}
        
        AudioBufferPtr      m_waveData;
        float               m_totalLength;
    };
}