#pragma once

#include "Core/Core.h"

#include "Core/Audio/AudioClip.h"
#include "Core/Audio/AudioChannel.h"

#include <portaudio.h>

#include <vector>
#include <string>
#include <mutex>
#include <filesystem>

struct AudioDevice {
    int ID;
    std::string Name;
};

struct ActiveAudioClip {
    Ref<AudioClip> AudioClipData;
    AudioChannel Channel = AudioChannel::Effects;
    size_t CurrentFrame = 0;
    float Volume = 1.0f;
    float Pitch = 0.0f;
};

class AudioEngine {
public:
    static void Init();
    static void Shutdown();
    static void Restart();

    static Ref<AudioClip> LoadAudioClip(const std::filesystem::path& filepath);
    static void PlayAudioClip(const Ref<AudioClip>& audioClip, AudioChannel channel = AudioChannel::Effects, float volume = 1.0f, float pitch = 0.0f);

    static const std::vector<AudioDevice>& GetInputDevices() { return s_InputDevices; }
    static const std::vector<AudioDevice>& GetOutputDevices() { return s_OutputDevices; }

    static int GetDefaultInputDeviceID() { return Pa_GetDefaultInputDevice(); }
    static int GetDefaultOutputDeviceID() { return Pa_GetDefaultOutputDevice(); }
private:
    static int PortAudioCallback(
        const void* inputBuffer, 
        void* outputBuffer,
        unsigned long framesPerBuffer,
        const PaStreamCallbackTimeInfo* timeInfo,
        PaStreamCallbackFlags statusFlags,
        void* userData
    );
private:
    inline static PaStream* s_Stream = nullptr;
    inline static std::mutex s_AudioMutex;

    inline static std::vector<ActiveAudioClip> s_ActiveAudioClips;

    inline static std::vector<AudioDevice> s_InputDevices;
    inline static std::vector<AudioDevice> s_OutputDevices;
};