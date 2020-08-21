#include <IO/FileInputStream.h>
#include <IO/FileSystem.h>
#include "AudioBuffer.h"
#include "AudioHelper.h"
#include "AudioException.h"
#include "WavFile.h"

using namespace IO;

namespace Audio
{

    WavFile::WavFile(const std::string& fileName)
    {
        if (!read(fileName))
            throw AudioException( "Not A Wave File" );
    }

    WavFile::WavFile()
    {
        std::memset(this, 0, sizeof(*this));
    }

    bool WavFile::read(const std::string& fileName)
    {
        auto fileSize = FileSystem::GetFileSize(fileName);
        if (fileSize < sizeof(WaveHeader))
            return false;

        bool succeed = false;
        FileInputStream fis(fileName);
        if (!fis.isOpen())
            return false;
        m_header = fis.read <WaveHeader>(&succeed);
        if (!succeed)
            return false;
        //verify header
        if ( memcmp("RIFF", m_header.m_riffText, 4) != 0 || 
             memcmp("WAVE", m_header.m_waveText, 4) != 0 )
            return false;

       m_waveData = std::make_shared<std::vector<std::int8_t>>();
       m_waveData->resize(m_header.m_dataLength);       
       succeed = fis.readData(m_waveData->data(), m_waveData->size()) == m_header.m_dataLength;       
       m_totalLength = GetLength( std::uint32_t(m_waveData->size()), m_header.m_frequency, m_header.m_bits / 8);     

       return succeed;

    }

}

