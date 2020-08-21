#pragma once


#include <Engine/EngineContext.h>

#include <Math/GenMath.h>

#include <Components/AudioListenerComponent.h>
#include <Components/AudioComponent.h>

#include "AudioMixerBase.h"
#include "AudioMixerHelper.h"
#include "AudioSystem.h"

using namespace Components;
using namespace Common;
using namespace Engine;
using namespace Math;


namespace Audio
{
    
    struct AudioMixInfo
    {
        AudioMixInfo() :AudioMixInfo(0.0f, -1.0f) {}
        AudioMixInfo(float distance, float angle)
            : m_distance(distance), m_angle(angle) {}

        float           m_distance; //distance to listener
        float           m_angle;    //[0..2PI] //angle w.r.t. listener
    };

    /*
        @brief: Returns additional mixing information, assumes 'z' is up vector
    */
    AudioMixInfo GetMixInfo( const Vector3f& dst, const Vector3f& pos )
    {
        auto dir = pos - dst;
        auto dist = dir.length();
        auto angle = static_cast<float>(HALF_PI);

        if ( dist > FLOAT_EPSILON )
        {
            const auto invLength = 1.0f / dist;
            dir *= invLength;
            auto xyDir = dir.toVector2();
            if (xyDir.normalizeSafe())
            {
                angle = Math::Atan2ToRads( xyDir.getY(), xyDir.getX() );
            }
        }
        return AudioMixInfo(dist, angle);
    }

    /*
        @brief: Return the panning ratio where -1 is left and 1 is right
    */
    inline float AngleToAudioPan(const float angle)
    {
        return Math::Cos(angle);
    }
    
    
    //////////////////////////////////////////////////////////////////////////
    //\Brief: Default audio mixer & sound update handler
    //////////////////////////////////////////////////////////////////////////
    class MixerDefault : public AudioMixerBase
    {
    public:

        MixerDefault(Engine::EngineContext* context)
            : m_context(context)           
        {
        }

        bool            initialize(const AudioConfig& format) override
        {
            m_outputFormat = format;
            return true;
        }

        bool            updateActiveSounds(const ActiveAudioVector& aav) override
        {
            const auto& as = m_context->getSystem<AudioSystem>();
            const auto& outFormat = m_outputFormat;
            const auto& listener  = as->getListener();
            const auto pos = listener ? listener->getPosition() : Vector3f(0.0f);

            for (const auto& iter : aav)
            {
                const auto info = GetMixInfo( pos, iter->getPosition() );
                const auto audioPan = outFormat.m_channels >= 2
                    ? AngleToAudioPan(info.m_angle) : 0.0f;

                iter->setPanning(audioPan);
            }
            return true;
        };


        std::uint32_t   mixIncomingSounds(const ActiveAudioVector& aav, std::uint32_t numSamples, void* data) override
        {
            AudioBlockInternal result;

            const auto& outFormat = m_outputFormat;
            const auto outChanCount = outFormat.getNumChannels();
            const auto numOutputSamples = numSamples * outChanCount;

            //mix all sources
            int numSoundSources = 0;
            for (const auto& sound : aav)
            {
                if (!sound->isPlaying())
                    continue;

                const bool isStereo = outChanCount == 2;
                const bool ignorePan = sound->hasAudioFlag(AUDIO_NO_PANNING);

                AudioBlockInternal  curAudioBlock; //working block
                const auto& inFormat = sound->getAudioFormat();

                //need to resample audio data?
                float sampleRatio = 1.0f;
                if (inFormat.m_sampleRate != outFormat.m_sampleRate)
                    sampleRatio = static_cast<float>(inFormat.m_sampleRate) / static_cast<float>(outFormat.m_sampleRate);


                //read data from audio source
                const auto bps = sound->getAudioFormat().getBytesPerSample();
                auto numBytes = std::uint32_t(bps * numSamples * sampleRatio);
                if (numBytes & 0x01) //don't read an odd number of from stream
                    numBytes--;
                sound->consume(curAudioBlock.getData(), numBytes);

                const auto inChanCount = inFormat.getNumChannels();

                //total amount of samples for input data
                const auto numInputSamples = std::uint32_t(numSamples * inChanCount * sampleRatio);
                //convert to input internal FP32 format            
                curAudioBlock = ConvertAudioBlock(curAudioBlock, inFormat, outFormat, numInputSamples);

                //Convert input stereo channel into mono, or mono to stereo
                if (inFormat.getNumChannels() != outFormat.getNumChannels())
                    curAudioBlock = ConvertChannel<float>(curAudioBlock, numInputSamples, inChanCount, outChanCount);

                //apply panning to source
                if (isStereo && !ignorePan)
                {
                    const float pan = sound->getPanning();
                    const float atten = sound->getAttenuation();
                    curAudioBlock = ApplyPanningInterleaved<float>(curAudioBlock, numSamples, pan, atten);
                }

                //when arriving here, assume curAudioBlock has already the correct interleaved channels 
                if (sampleRatio != 1.0f)  //resample audio format, depending on the input & output frequencies       
                    curAudioBlock = ResampleAudioBlock<float>(curAudioBlock, numSamples, outChanCount, sampleRatio);

                //add to output
                result = AddAudioBlock<float>(result, curAudioBlock, numOutputSamples);
                numSoundSources++; //increment # sources
            }

            if (numSoundSources)
            {
                const float invNumSounds = 1.0f / numSoundSources;
                //finalBlock = ScaleAudioBlock<float>(finalBlock, numOutputSamples, invNumSounds);
                memcpy(data, result.toPointer<float>(), sizeof(float) * numOutputSamples);
                return numSamples;
            }
            return 0;
        }

    private:

        EngineContext*  m_context;
        AudioConfig     m_outputFormat;

    };
}