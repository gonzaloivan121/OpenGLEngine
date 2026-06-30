#pragma once

#include "Core/Audio/AudioChannel.h"

#include "ECS/Component.h"

#include "Utilities/Utilities.h"

#include <filesystem>

struct AudioComponent : public Component {
    std::filesystem::path AudioFilepath;
    AudioChannel Channel = AudioChannel::Effects;

    bool PlayOnAwake = true;
    bool Loop = false;
    float Volume = 1.0f;
    float Pitch = 0.0f;
};