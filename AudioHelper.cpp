#include "AudioHelper.h"

namespace Audio
{
	

	float GetLength(std::uint32_t bufSize, std::uint32_t sampleRate, std::uint32_t bytesPerSample)
	{
    	if (bufSize == 0 || sampleRate == 0)
			return 0.0f;

        const float ssInv = 1.0f / bytesPerSample;
        const float freqInv = 1.0f / sampleRate;

		return bufSize * ssInv * freqInv;
	}

	std::uint32_t GetBufferSize(std::uint32_t freq, std::uint32_t bytesPerSample, float time)
	{
		const float result = freq * bytesPerSample * time;
		return static_cast<std::uint32_t>(result);
	}

    std::uint32_t GetNumberOfSamples(std::uint32_t numBytes, const AudioConfig& format)
    {
        const auto bps = format.getBytesPerSample();
        return numBytes / bps;
    }

}