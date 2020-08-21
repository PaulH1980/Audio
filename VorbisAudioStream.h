#pragma once

#include "AudioStream.h"


namespace Audio
{
    
    class VorbisAudioStream : public AudioStreamBase
    {
    public:
        VorbisAudioStream(const AudioBufferPtr& buffer, const  AudioFormat& format);
        virtual ~VorbisAudioStream();
               
        bool            seek( std::uint32_t sample ) final override;
        std::uint32_t   getData( void* dest, std::uint32_t numBytes )  final override;
        
    private:
        void* m_decoder;
    };


}

