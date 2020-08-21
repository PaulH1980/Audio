#pragma once
#include <cstdint>

#include <IO/FileTypes.h>

#include "AudioConfig.h"
#include "AudioBuffer.h"

namespace Audio
{
   
    
    class AudioStreamBase
    {
    public:
        AudioStreamBase();        
        AudioStreamBase( const AudioBufferPtr& buffer, const AudioFormat& format );
        virtual ~AudioStreamBase() = default;
        
        virtual bool            seek( std::uint32_t sample );
        virtual std::uint32_t   getData( void* dest, std::uint32_t numBytes );
        virtual std::uint32_t   getSamplePos() const;              


        const AudioBufferPtr&   getBuffer() const;
        const AudioFormat&      getInternalFormat() const;
       
        std::uint32_t           getTotalSamples() const;   

        bool                    isLooping() const;
        void                    setLooping( bool val);

        std::uint32_t           numBytesAvailable() const
        {
            return static_cast<std::uint32_t>(m_bufferPtr->size()) - m_bufPos;
        }


        

    protected:
       bool                     m_looping; //current loop iter

    private:       
        AudioBufferPtr          m_bufferPtr; //shared with a sound resource
        AudioFormat             m_format;
        std::uint32_t           m_bufPos;          
       
    };


}