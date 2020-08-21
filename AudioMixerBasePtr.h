#pragma once
#include <memory>

namespace Audio
{
    class AudioMixerBase;
    using AudioMixerBasePtr = std::shared_ptr<AudioMixerBase>;
}