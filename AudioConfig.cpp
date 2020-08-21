#include <Common/ReflectionRegister.h>
#include <exception>
#include "SampleInfo.h"
#include "AudioConfig.h"


RTTR_REGISTRATION
{
    using namespace Audio;
    using namespace rttr;

    {
        registration::enumeration<eAudioUsage>("eAudioUsage");
        registration::enumeration<eAudioType>("eAudioType");
        registration::enumeration<eConfigFlags>("eConfigFlags");
    }

    {
        using Type = AudioFormat;
        auto val = registration::class_<Type>("AudioFormat");
        val.method("setAudioGroup", &Type::setAudioGroup);
        val.method("setLoopCount", &Type::setLoopCount);
        val.method("getAudioGroup", &Type::getAudioGroup);
    }  
    {
        using Type = AudioConfig;
        auto val = registration::class_<Type>("AudioConfig");
        val.method("getBytesPerSample", &Type::getBytesPerSample);
        val.method("getBytesPerSecond", &Type::getBytesPerSecond);
        val.method("getNumSamples", &Type::getNumSamples);
        val.method("getNumBytes", &Type::getNumBytes);
        val.method("getSampleRate", &Type::getSampleRate);
        val.method("getNumChannels", &Type::getNumChannels);
    }
    
};


namespace Audio
{
    void AudioFormat::setAudioGroup(const std::string& name)
    {
        if (name == "AMBIENT")
            m_usage = AUDIO_USAGE_AMBIENT;
        else if (name == "FX")
            m_usage = AUDIO_USAGE_FX;
        else if (name == "MUSIC")
            m_usage = AUDIO_USAGE_MUSIC;
        else {
            m_usage = AUDIO_USAGE_UNDEFINED;
        }
    }

    void AudioFormat::setLoopCount(int val)
    {
        m_loopCount = val;
    }

   
    std::string AudioFormat::getAudioGroup() const
    {
        if (m_usage == AUDIO_USAGE_AMBIENT)
            return "AMBIENT";
        if( m_usage == AUDIO_USAGE_FX )
            return "FX";
        if (m_usage == AUDIO_USAGE_MUSIC)
            return "MUSIC";
        return "UNDEFINED";
    }

    std::uint32_t AudioConfig::getBytesPerSample() const
    {
        std::uint32_t bytes = SampleInformation[m_format].m_bytesPerSample;
        bytes *= m_channels;
        return bytes;
    }

    std::uint32_t AudioConfig::getBytesPerSecond() const
    {
        return getBytesPerSample() * getSampleRate();
    }

    std::uint32_t AudioConfig::getNumSamples(const float period) const
    {
        return (std::uint32_t)(getSampleRate() * period);
    }

    std::uint32_t AudioConfig::getNumBytes(const float period) const
    {
        return getNumSamples(period) * getBytesPerSample();
    }

    std::uint32_t AudioConfig::getSampleRate() const
    {
        return m_sampleRate;
    }

    std::uint32_t AudioConfig::getNumChannels() const
    {
        return m_channels;
    }

}

