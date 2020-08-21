#pragma once
#include <cstring> //memcpy
#include <limits>
#include <cstdint>
#include <Math/Int24.h>
#include "AudioBlock.h"
#include "AudioException.h"

namespace Audio
{
    /*
        Various mixing helpers, TODO implement SSE versions    
    */
    using AudioBlockInternal = AudioBlock16k;

    //Internal helper
    struct SampleFormat
    {
        SampleFormat( const AudioConfig& ac, AudioBlockInternal& samples )
            : m_audioConfig(ac)
            , m_audioSamples( samples )
        {
            m_audioSamples = samples;
        }
        AudioConfig                 m_audioConfig;
        AudioBlockInternal&         m_audioSamples;
    };


    //////////////////////////////////////////////////////////////////////////
    //\Brief: Convert a input audio block to the AudioSystem it's native
    // format( FP32 )
    //////////////////////////////////////////////////////////////////////////
    template<typename T>
    AudioBlockInternal ConvertToFP32( const  AudioBlockInternal& inSamples, std::uint32_t numSamples )
    {
        AudioBlockInternal result;
        
        constexpr auto MIN = static_cast<float>(std::numeric_limits<T>::min()) * -1.0f;
        constexpr auto RANGE = MIN + static_cast<float>(std::numeric_limits<T>::max());
        constexpr auto RANGE_INV = 1.0f / RANGE;

        const auto* srcPtr = inSamples.toConstPointer<T>();
        auto* dstPtr       = result.toPointer<float>();
        for (int i = 0; i < (int)numSamples; ++i)
        {
            auto val = MIN + static_cast<float>(srcPtr[i]); //convert to [0...MAX]
            val *= RANGE_INV;
            val -= 1.0f;
            dstPtr[i] = val; //convert to [-1...1]
        }
        return result;
    }

    //////////////////////////////////////////////////////////////////////////
    //\Brief: float specialization, just memcpy input to output
    //////////////////////////////////////////////////////////////////////////
    template<>
    AudioBlockInternal ConvertToFP32<float>(const AudioBlockInternal& inSamples, std::uint32_t numSamples)
    {
        AudioBlockInternal result;
        memcpy(result.toPointer<float>(), inSamples.toConstPointer<float>(), sizeof(float) * numSamples);
        return result;
    }
    

    //////////////////////////////////////////////////////////////////////////
    //\Brief: Adjust frequency for audio block
    //////////////////////////////////////////////////////////////////////////
    template<typename T>
    AudioBlockInternal ResampleAudioBlock( const AudioBlockInternal& input, const std::uint32_t numSamples, 
        const std::uint32_t numChannels = 1, const float sampleRatio = 1.0f )
    {
        const auto numOutputSamples = static_cast<int>( numSamples * sampleRatio );
        const int stride = numChannels;

        const int inIterEnd  = numOutputSamples - 1;
        const int outIterEnd = numSamples - 1;
        const float idxStep   = outIterEnd / float(inIterEnd);
               
        AudioBlockInternal result;
        //for each channel
        for (int curChannel = 0; curChannel < (int)numChannels; ++curChannel)
        {
            //offset arrays for interleaved access
            const auto* inData  = input.toConstPointer<T>() + curChannel;
            auto*		outData = result.toPointer<T>() + curChannel;
            //copy end data
            outData[outIterEnd * stride] = inData[inIterEnd * stride];
            //resample single channel
            for (int curIdx = 0; curIdx < (int)numOutputSamples - 1; ++curIdx)
            {
                const auto nextIdx      = curIdx + 1;
                const auto curOutIdx    = static_cast<int>(curIdx * idxStep);
                const auto nextOutIdx   = static_cast<int>(nextIdx * idxStep);
                const auto numSteps     = nextOutIdx - curOutIdx;
                
                outData[curOutIdx * stride] = inData[curIdx * stride];
                                
                if (numSteps > 1)	//interpolate between samples
                {
                    const float stepInc = 1.0f / static_cast<float>(numSteps);
                    const T start = inData[curIdx  * stride];
                    const T end   = inData[nextIdx * stride];
                    //interpolate sample values
                    for (int j = 1; j < numSteps; ++j) 
                    {
                        auto intOutIdx = static_cast<int>(curOutIdx + j);
                        outData[intOutIdx * stride] = Math::Lerp(start, end, j * stepInc);
                    }
                }
            }
        }
        return result;    
    }


    //////////////////////////////////////////////////////////////////////////
   //\Brief: Concatenates 2 audio blocks return a new block
   //////////////////////////////////////////////////////////////////////////
    template<typename T>
    AudioBlockInternal AddAudioBlock( const AudioBlockInternal& left, const AudioBlockInternal& right, 
        std::uint32_t numSamples, float scaleVal = 1.0f )
    {
        AudioBlockInternal result;
        const auto* leftPtr  = left.toConstPointer<T>();
        const auto* rightPtr = right.toConstPointer<T>();
        auto* dstPtr = result.toPointer<T>();
        for (int i = 0; i < (int)numSamples; i++) 
        {
            const auto val = static_cast<float>( leftPtr[i] + rightPtr[i] );
            *dstPtr++ = static_cast<T>( val * scaleVal );
        }
        return result;
    }



    //////////////////////////////////////////////////////////////////////////
    //\Brief: Scales an audio block by specified factor, input remains unchanged
    //////////////////////////////////////////////////////////////////////////
    template<typename T>
    AudioBlockInternal ScaleAudioBlock(const AudioBlockInternal& input, std::uint32_t numSamples, float scaleVal = 1.0f )
    {
        AudioBlockInternal result;
        const auto* srcPtr = input.toConstPointer<T>();
        auto* dstPtr = result.toPointer<T>();
        for (int i = 0; i < (int)numSamples; i++) {
            const auto val = static_cast<float>(srcPtr[i]) ;
            *dstPtr++ = static_cast<T>( val * scaleVal );
        }
        return result;
    }


    //////////////////////////////////////////////////////////////////////////
    //\Brief: Merge stereo channels into a mono one, input remains unchanged
    //////////////////////////////////////////////////////////////////////////
    template<typename T>
    AudioBlockInternal StereoToMono( const AudioBlockInternal& input, std::uint32_t numSamples, float scaleVal = 0.5f )
    {
        AudioBlockInternal result;
        const auto* srcPtr = input.toConstPointer<T>();
        auto* dstPtr       = result.toPointer<T>();

        for (int i = 0; i < (int)numSamples; i += 2 ) {
            const auto left  = static_cast<float>( srcPtr[i + 0] );
            const auto right = static_cast<float>( srcPtr[i + 1] );
            *dstPtr++ = static_cast<T>( ( left + right ) * scaleVal );
        }
        return result;
    }

    //////////////////////////////////////////////////////////////////////////
    //\Brief: Split audio block into 2 separate channels
    //////////////////////////////////////////////////////////////////////////
    template<typename T>
    std::uint32_t SplitStereo( const AudioBlockInternal& input, AudioBlockInternal& left, 
        AudioBlockInternal& right, std::uint32_t numSamples )
    {
        const auto* srcPtr = input.toConstPointer<T>();
        
        auto* dstLeft  = left.toPointer<T>();
        auto* dstRight = right.toPointer<T>();

        for (int i = 0; i < (int)numSamples * 2; i += 2 ) {
            *dstLeft++  = srcPtr[i + 0];
            *dstRight++ = srcPtr[i + 1];  
        }
        return numSamples;
    }

    template<typename T>
    AudioBlockInternal ApplyPanningInterleaved(const AudioBlockInternal& input, std::uint32_t numSamples,
        float panning, float attenuation = 1.0f)
    {
        const float biasLeft = Math::Clamp( 0.0f, 1.0f, panning * -0.5f + 0.5f );
        const float soundBias[2] = { biasLeft * attenuation, (1.0f - biasLeft) * attenuation };

        AudioBlockInternal result;
        const auto* srcPtr = input.toConstPointer<T>();
        auto* destPtr = result.toPointer<T>();
        for ( int i = 0; i < (int)numSamples * 2; i += 2 )
        {
            *destPtr++ = srcPtr[i + 0] * soundBias[0];
            *destPtr++ = srcPtr[i + 1] * soundBias[1];
        }
        return result;
    }



    //////////////////////////////////////////////////////////////////////////
    //\Brief: Convert mono into an interleaved stereo sample block
    //////////////////////////////////////////////////////////////////////////
    template<typename T>
    AudioBlockInternal MonoToStereoInterleaved(const AudioBlockInternal& input, std::uint32_t numSamples,
        float leftChannelVol = 1.0f, float rightChannelVol = 1.0f )
    {
        AudioBlockInternal result;        
        const auto* srcPtr = input.toConstPointer<T>();
        auto* destPtr = result.toPointer<T>();        

        for (int i = 0; i < (int)numSamples; ++i ) 
        {
            auto srcVal  = static_cast<float>( srcPtr[i] );
            *destPtr++ = static_cast<T>( srcVal * leftChannelVol  );
            *destPtr++ = static_cast<T>( srcVal * rightChannelVol );            
        }
        return result;
    }
       
    //////////////////////////////////////////////////////////////////////////
    //\Brief: Either converted mono to stereo( interleaved), or stereo to mono
    //////////////////////////////////////////////////////////////////////////
    template<typename T>
    AudioBlockInternal ConvertChannel( const AudioBlockInternal& input, std::uint32_t numSamples,
        std::uint32_t numSrcChannel, std::uint32_t numDstChannel )
    {
        if (numSrcChannel == 2 && numDstChannel == 1) //stereo to mono
            return StereoToMono<T>(input, numSamples); //assumes numSamples is already in stereo
        else if (numSrcChannel == 1 && numDstChannel == 2) //mono to stereo
            return MonoToStereoInterleaved<T>(input, numSamples);
        else //TODO: surround sound
            throw AudioException("Invalid Channel Parameters");
    }

    //////////////////////////////////////////////////////////////////////////
    //\Brief: Converts input format to a compatible output format( fp32 )
    //////////////////////////////////////////////////////////////////////////
    AudioBlockInternal ConvertAudioBlock( const AudioBlockInternal& input, const AudioConfig& inFormat,
        const AudioConfig& outFormat, std::uint32_t numSamples )
    {
        //internal format is always fp32
        (outFormat);
        
        AudioBlockInternal result;       
        switch (inFormat.m_format)
        {
            case audio_format_u8:
                result = ConvertToFP32<std::uint8_t>(input, numSamples);
                break;
            case audio_format_s16:
                result = ConvertToFP32<std::int16_t>(input, numSamples);
                break;
            case audio_format_s24:
                result = ConvertToFP32<Int24>(input, numSamples);
                break;
            case audio_format_s32:
                result = ConvertToFP32<std::int32_t>(input,numSamples);
                break;
            case audio_format_f32:
                result = ConvertToFP32<float>(input, numSamples);
                break;
            default:
                throw AudioException("Unsupported Sound Format");
        }
        return result;
    }


}


