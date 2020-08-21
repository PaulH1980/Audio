#include <algorithm>
#include <assert.h>

#include "AudioException.h"
#include "AudioStream.h"



namespace Audio
{

    AudioStreamBase::AudioStreamBase(const AudioBufferPtr& buffer, const AudioFormat& format)
        : m_bufferPtr( buffer )
        , m_format( format )       
        , m_bufPos( 0 )
        , m_looping(true)
    {
        if ( buffer->empty() )
            throw AudioException("Empty Audio Buffer");
     
    }

    AudioStreamBase::AudioStreamBase() : m_bufPos(0), m_looping( true )
    {

    }

    bool AudioStreamBase::seek(std::uint32_t sample)
    {
        std::uint32_t  idx = m_format.m_sampleRate * sample;
        m_bufPos = idx;   
        return true;
    }

    

    std::uint32_t AudioStreamBase::getData( void* dest, std::uint32_t numBytes )
    {
        auto& audioBuf    = *m_bufferPtr;
        auto bufSize      = (std::uint32_t)( audioBuf.size() );
        const auto* start = &audioBuf[0];

        assert( numBytes <= bufSize );
        
        auto available   = bufSize - m_bufPos;
        auto bytesToRead = std::min( available, numBytes ); 
       
        if (bytesToRead) 
        {
            const auto* srcPtr = &audioBuf[m_bufPos];
            auto* destPtr      = reinterpret_cast<char*>( dest );
            memcpy( destPtr, srcPtr, bytesToRead );
            m_bufPos += bytesToRead; 
            
            if ( m_bufPos == bufSize && isLooping() ) //at end ?
            {
                m_bufPos = 0;
                if (bytesToRead < numBytes) //loop around
                {
                    srcPtr   = start;       //reset start ptr
                    destPtr += bytesToRead; //increment dest ptr
                    available = numBytes - bytesToRead;
                    memcpy( destPtr, srcPtr, available);

                    bytesToRead += available;
                    m_bufPos    += available;
                }                 
            }            
        }
        return bytesToRead;
    }

    std::uint32_t AudioStreamBase::getSamplePos() const
    {
        return m_bufPos / m_format.getBytesPerSample();
    }

    const AudioBufferPtr& AudioStreamBase::getBuffer() const
    {
        return m_bufferPtr;
    }

    const AudioFormat& AudioStreamBase::getInternalFormat() const
    {
        return m_format;
    }
        

    std::uint32_t AudioStreamBase::getTotalSamples() const
    {
        return static_cast<std::uint32_t>(m_bufferPtr->size()) / m_format.getBytesPerSample();
    }

    bool AudioStreamBase::isLooping() const
    {
        return m_looping;
    }

    void AudioStreamBase::setLooping(bool val)
    {
        m_looping = val;
    }

}