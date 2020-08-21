#define STB_VORBIS_HEADER_ONLY
#include "LibVorbis.h"
#include "AudioException.h"
#include "VorbisAudioStream.h"

namespace Audio
{

    VorbisAudioStream::VorbisAudioStream(const AudioBufferPtr& buffer, const  AudioFormat& format)
        : AudioStreamBase( buffer, format )
        , m_decoder( nullptr )
    {
        int error;
        m_decoder = stb_vorbis_open_memory( reinterpret_cast<const std::uint8_t*>( buffer->data()), (int)buffer->size(), &error, nullptr);
        if (!m_decoder)
            throw AudioException("Unable To Create Ogg Decoder");
    }

    VorbisAudioStream::~VorbisAudioStream()
    {
        if (m_decoder) {
            auto* vorbis = static_cast<stb_vorbis*>(m_decoder);
            stb_vorbis_close(vorbis);
            m_decoder = nullptr;
        }
    }

    bool VorbisAudioStream::seek(std::uint32_t sample)
    {
        auto* vorbis = static_cast<stb_vorbis*>(m_decoder);
        return stb_vorbis_seek(vorbis, sample) == 1;
    }

    std::uint32_t VorbisAudioStream::getData( void* dest, std::uint32_t numBytes )
    {
        const auto& getSamples  = stb_vorbis_get_samples_short_interleaved;    
        auto* vorbis        = static_cast<stb_vorbis*>(m_decoder);
        auto* destPtr       = reinterpret_cast<short*>( dest );
        auto offset         = 0;       
        auto numChannels    = getInternalFormat().m_channels;
        auto samples        = getSamples( vorbis, numChannels, &destPtr[offset], numBytes >> 1u);
        auto result         = static_cast<std::uint32_t>(samples * numChannels) << 1u;
        if ( result < numBytes ) 
        {
            if ( isLooping() ) 
            {
                stb_vorbis_seek_start(vorbis);
                offset   += result >> 1u;
                numBytes -= result;         
                
                samples   = getSamples(vorbis, numChannels, &destPtr[offset], numBytes >> 1u);
                result   += static_cast<std::uint32_t>(samples * numChannels) << 1u;             
            }
        }        
        return result;
    }

}