#pragma once

#include "AudioConfig.h"

namespace Audio
{
    class AudioMixerBase
    {
    public:
        virtual ~AudioMixerBase() = default;
        
        /*
            @brief: Initializes mixer
        */
        virtual bool            initialize(const AudioConfig&) = 0;        

        /*
            @brief: Update incoming sounds, e.g. adjust panning, or gain
        */
        virtual bool            updateActiveSounds( const ActiveAudioVector& ) = 0;

        /*
            @brief: Mix all sounds together into a output buffer
        */
        virtual std::uint32_t   mixIncomingSounds(  const ActiveAudioVector&, std::uint32_t numSamples, void* data ) = 0;
    };
}