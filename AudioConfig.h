#pragma once
#include <cstdint>
#include <string>
#include <unordered_set>
#include <vector>

#include <Scene/AudioEmitterEntityPtr.h>
#include <Scene/AudioListenerEntityFwd.h>
#include <Components/AudioListenerComponentFwd.h>
#include <Components/AudioComponentFwd.h>

#include "eAudioFormat.h"






namespace Audio
{
    constexpr int AUDIO_LOOP_INFINITE = -1;
   
    enum eAudioUsage : std::uint32_t
    {
        AUDIO_USAGE_UNDEFINED,
        AUDIO_USAGE_AMBIENT,
        AUDIO_USAGE_FX,
        AUDIO_USAGE_MUSIC
    };

    enum eAudioType : std::uint32_t
    {
        AUDIO_TYPE_NONE = 0,
        AUDIO_TYPE_GEN,
        AUDIO_TYPE_WAV,
        AUDIO_TYPE_OGG
    };

    enum eConfigFlags : std::uint32_t
    {
        AUDIO_FLAGS_NONE = 0x0,
        AUDIO_UNIT_TEST  = 0x01
    };

    struct AudioConfig
    {
        
        std::uint32_t   getBytesPerSample() const;
        std::uint32_t   getSampleRate()   const;
        std::uint32_t   getNumChannels() const;
        
        std::uint32_t   getBytesPerSecond() const;
        std::uint32_t   getNumSamples(const float period) const;
        std::uint32_t   getNumBytes(const float period) const;
        
        
        
        std::uint32_t   m_format     = audio_format_s16;
        std::uint32_t   m_channels   = 2;
        std::uint32_t   m_sampleRate = 44100;
        std::uint32_t   m_flags      = AUDIO_FLAGS_NONE;        
    };


    struct AudioFormat : public AudioConfig
    {
        
        void            setAudioGroup( const std::string& name);
        void            setLoopCount( int val );    
        std::string     getAudioGroup() const;
        

        eAudioUsage     m_usage      = AUDIO_USAGE_UNDEFINED;
        eAudioType      m_type       = AUDIO_TYPE_NONE;
        int             m_loopCount  = AUDIO_LOOP_INFINITE;
    };
  
    inline AudioConfig GetDefaultAudioConfig()
    {
        AudioConfig result;
        result.m_format      = audio_format_f32;
        result.m_channels    = 2;
        result.m_sampleRate  = 44100;
        result.m_flags       = 0u;        
        return result;
    }


    inline AudioConfig     GetTestOutputAudioConfig()
    {
        AudioConfig result;
        result.m_format      = audio_format_f32;
        result.m_channels    = 1;
        result.m_sampleRate  = 22050;
        result.m_flags       = AUDIO_UNIT_TEST;
        return result;
    }

    inline AudioFormat     GetDefaultAudioFormat()
    {
        AudioFormat result;
        result.m_format     = audio_format_s16;
        result.m_channels   = 1;
        result.m_sampleRate = 44100;
        result.m_flags      = 0u;

        result.m_usage      = AUDIO_USAGE_UNDEFINED;
        result.m_type       = AUDIO_TYPE_NONE;
        result.m_loopCount  = AUDIO_LOOP_INFINITE;

        return result;
    }

    using AudioSource       = Components::AudioComponent;
    using AudioListener     = Components::AudioListenerComponent;
    using ActiveAudioSet    = std::unordered_set<AudioSource*>;
    using ActiveAudioVector = std::vector<AudioSource*>;

}