#pragma once
#include "AudioBuffer.h"
#include "AudioConfig.h"

namespace Audio
{	
	float GetLength(std::uint32_t bufSize, std::uint32_t sampleRate, std::uint32_t bytesPerSample);
	std::uint32_t GetBufferSize(std::uint32_t freq, std::uint32_t bytesPerSample, float time);
    std::uint32_t GetNumberOfSamples(std::uint32_t numBytes, const AudioConfig& format);
}