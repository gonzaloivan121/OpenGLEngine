#pragma once

#include "Core/Core.h"

#include <vector>
#include <filesystem>

struct AudioClip {
    std::vector<float> Data;
    int NumChannels = 2;
    int SampleRate = 44100;
    std::filesystem::path Filepath;

    static Ref<AudioClip> LoadFromFile(const std::filesystem::path& filepath);

    const std::filesystem::path& GetFilepath() const { return Filepath; }
};