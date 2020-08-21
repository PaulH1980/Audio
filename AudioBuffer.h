#pragma once
#include <vector>
#include <memory>

namespace Audio
{
	using AudioBuffer = std::vector<std::int8_t>;
	using AudioBufferPtr = std::shared_ptr<AudioBuffer>;
}

 