#pragma once

#include <memory>

namespace Audio
{
    class AudioStreamBase;
    using AudioStreamBasePtr = std::shared_ptr<AudioStreamBase>;
}