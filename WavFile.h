#pragma once
#include <string>
#include <vector>
#include "AudioFileBase.h"


namespace Audio
{
   
    struct WaveHeader
    {
        std::uint8_t       m_riffText[4];
        std::uint32_t      m_totalLength;
        std::uint8_t       m_waveText[4];
        std::uint8_t       m_formatText[4];
        std::uint32_t      m_formatLength;
        std::uint16_t      m_format;
        std::uint16_t      m_channels;
        std::uint32_t      m_frequency;
        std::uint32_t      m_avgBytes;
        std::uint16_t      m_blockAlign;
        std::uint16_t      m_bits;
        std::uint8_t       m_dataText[4];
        std::uint32_t      m_dataLength;
    };
    
    
    struct WavFile : public AudioFileBase
    {
        WavFile();

        WavFile( const std::string& fileName );

        bool                read(const std::string& fileName);     


       
        WaveHeader          m_header;   
    };
}