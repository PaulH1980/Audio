#pragma once

#include <string>
#include <vector>
#include "AudioFileBase.h"

namespace Audio
{
    
    struct OggFile : public AudioFileBase
    {
        OggFile() 
            : m_numChannels( 0 )
            , m_frequency( 0 )           
        {

        }

        OggFile( const std::string& fileName );

        bool                read(const std::string& fileName);
        
        std::uint32_t       m_numChannels;
        std::uint32_t       m_frequency;              
    };
    
  
}
