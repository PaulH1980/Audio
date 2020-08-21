#include <IO/FileInputStream.h>
#include <IO/FileSystem.h>
#pragma warning(push, 3)
#include "LibVorbis.h"
#pragma warning( pop )
#include "AudioBuffer.h"
#include "AudioException.h"
#include "OggFile.h"

using namespace IO;

namespace Audio
{

    OggFile::OggFile(const std::string& fileName)
    {
        if (!read(fileName))
            throw AudioException("Not A Vorbis File");
    }

    bool OggFile::read(const std::string& fileName)
    {
        bool succeed = false;
        FileInputStream fis(fileName);
        if (!fis.isOpen())
            return false;

        auto fileSize = fis.getFileSize();
        if (!fileSize)
            return false;
        
        fs_data fileData(fileSize);
        succeed = fis.readData(fileData, fileSize) == fileSize;
        if (!succeed)
            return false;

        int errCode;
        auto* vorbis = stb_vorbis_open_memory( fileData.data(), (int)fileSize, &errCode, nullptr);
        if ( !vorbis || errCode )
            return false;

        auto info     = stb_vorbis_get_info(vorbis);
        m_totalLength = stb_vorbis_stream_length_in_seconds(vorbis);
        m_frequency   = info.sample_rate;
        m_numChannels = info.channels;       
        m_waveData    = std::make_shared<AudioBuffer>( fileSize );
        memcpy(m_waveData->data(), fileData.data(), fileSize);
        stb_vorbis_close(vorbis);
                
        return true;
    }

}

