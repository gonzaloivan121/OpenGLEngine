#include "AudioEngine.h"

#include "Core/Log/Log.h"

#include "Core/Settings/Manager/SettingsManager.h"

void AudioEngine::Init() {
	PaError err = Pa_Initialize();

    if (err != paNoError) {
        Log::Error("AudioEngine::Init - Error while initializing PortAudio: " + std::string(Pa_GetErrorText(err)));
        return;
    }

    int numDevices = Pa_GetDeviceCount();
    if (numDevices < 0) {
        Log::Error("AudioEngine::Init - Error while obtaining the devices list: " + std::string(Pa_GetErrorText(numDevices)));
        Pa_Terminate();
        return;
    }

    for (int i = 0; i < numDevices; ++i) {
        const PaDeviceInfo* deviceInfo = Pa_GetDeviceInfo(i);

        if (!deviceInfo) {
            continue;
        }

        if (deviceInfo->maxInputChannels > 0) {
            s_InputDevices.push_back({ i, std::string(deviceInfo->name) });
        }

        if (deviceInfo->maxOutputChannels > 0) {
            s_OutputDevices.push_back({ i, std::string(deviceInfo->name) });
        }
    }

    const auto& settings = SettingsManager::Get().Audio;
    int outputDeviceId = settings.Device.OutputDeviceID;
    int inputDeviceId = settings.Device.InputDeviceID;

    if (outputDeviceId == -1) {
        outputDeviceId = GetDefaultOutputDeviceID();
    }

    if (inputDeviceId == -1) {
        inputDeviceId = GetDefaultInputDeviceID();
    }

    // Get the information from this device to know it's recommended configuration
    const PaDeviceInfo* outputDeviceInfo = Pa_GetDeviceInfo(outputDeviceId);
    Log::Trace("AudioEngine::Init - Using Output Audio Device: " + std::string(outputDeviceInfo->name));

    const PaDeviceInfo* inputDeviceInfo = Pa_GetDeviceInfo(inputDeviceId);
    Log::Trace("AudioEngine::Init - Using Input Audio Device: " + std::string(inputDeviceInfo->name));

    // Configure the output stream parameters using the device's info
    PaStreamParameters outputParameters;
    outputParameters.device = outputDeviceId;
    outputParameters.channelCount = 2;
    outputParameters.sampleFormat = paFloat32;
    outputParameters.suggestedLatency = outputDeviceInfo->defaultLowOutputLatency;
    outputParameters.hostApiSpecificStreamInfo = nullptr;

    // Configure the input stream parameters using the device's info
    PaStreamParameters inputParameters;
    inputParameters.device = inputDeviceId;
    inputParameters.channelCount = 1;
    inputParameters.sampleFormat = paFloat32;
    inputParameters.suggestedLatency = inputDeviceInfo->defaultLowInputLatency;
    inputParameters.hostApiSpecificStreamInfo = nullptr;

    err = Pa_OpenStream(
        &s_Stream,
        &inputParameters,
        &outputParameters,
        (double)settings.Device.SampleRate,
        paFramesPerBufferUnspecified,
        paClipOff,
        PortAudioCallback,
        nullptr
    );

    if (err != paNoError) {
        Log::Error("AudioEngine::Init - Error while opening the PortAudio Stream: " + std::string(Pa_GetErrorText(err)));
        Pa_Terminate();
        return;
    }

    err = Pa_StartStream(s_Stream);

    if (err != paNoError) {
        Log::Error("AudioEngine::Init - Error while initializing the PortAudio Stream: " + std::string(Pa_GetErrorText(err)));
        Pa_Terminate();
        return;
    }

    Log::Trace("AudioEngine::Init - Audio Engine initialized");
}

void AudioEngine::Shutdown() {
    Log::Trace("AudioEngine::Shutdown - Shutting down the Audio Engine");
    PaError err;

    // Lock the mutex during the restart process to avoid the
    // callback being executed while we modify the stream
    std::lock_guard<std::mutex> lock(s_AudioMutex);

    if (s_Stream) {
        err = Pa_StopStream(s_Stream);

        if (err != paNoError) {
            Log::Error("AudioEngine::Shutdown - Error while stopping the PortAudio Stream: " + std::string(Pa_GetErrorText(err)));
        }

        err = Pa_CloseStream(s_Stream);

        if (err != paNoError) {
            Log::Error("AudioEngine::Shutdown - Error while closing the PortAudio Stream: " + std::string(Pa_GetErrorText(err)));
        }

        s_Stream = nullptr;
    }

    Log::Trace("AudioEngine::Shutdown - Terminating PortAudio");

    err = Pa_Terminate();

    if (err != paNoError) {
        Log::Error("AudioEngine::Shutdown - Error while terminating PortAudio: " + std::string(Pa_GetErrorText(err)));
    }

    Log::Trace("AudioEngine::Shutdown - PortAudio has been terminated");

    Log::Trace("AudioEngine::Shutdown - Clearing Input and Output Devices");
    s_InputDevices.clear();
    s_OutputDevices.clear();

    Log::Trace("AudioEngine::Shutdown - Audio Engine has been shutdown");
}

void AudioEngine::Restart() {
    Log::Trace("AudioEngine::Restart - Restarting the Audio Engine");

    Shutdown();
    Init();
}

Ref<AudioClip> AudioEngine::LoadAudioClip(const std::filesystem::path& filepath) {
    Log::Trace("AudioEngine::LoadAudioClip - Loading Audio Clip: " + filepath.string());

    return AudioClip::LoadFromFile(filepath);
}

void AudioEngine::PlayAudioClip(const Ref<AudioClip>& audioClip, AudioChannel channel, float volume, float pitch) {
    if (!audioClip) {
        Log::Warning("AudioEngine::PlayAudioClip - Audio Clip not found");
        return;
    }

    Log::Warning("AudioEngine::PlayAudioClip - Playing Audio Clip: " + audioClip->Filepath.string());

    std::lock_guard<std::mutex> lock(s_AudioMutex);
    s_ActiveAudioClips.push_back({ audioClip, channel, 0, volume, pitch });
}

int AudioEngine::PortAudioCallback(
    const void* inputBuffer,
    void* outputBuffer,
    unsigned long framesPerBuffer,
    const PaStreamCallbackTimeInfo* timeInfo,
    PaStreamCallbackFlags statusFlags,
    void* userData
) {
    const auto& settings = SettingsManager::Get().Audio;
    float* out = (float*)outputBuffer;
    const float* in = (const float*)inputBuffer;

    if (settings.Device.PassThrough && in != nullptr) {
        // Copy the input channel to both output channels
        for (unsigned long i = 0; i < framesPerBuffer; ++i) {
            out[i * 2 + 0] = in[i]; // Left channel
            out[i * 2 + 1] = in[i]; // Right channel
        }
    } else {
        // Clean the output buffer to silence (0.0f)
        for (unsigned int i = 0; i < framesPerBuffer * 2; ++i) { // * 2 for stereo
            out[i] = 0.0f;
        }
    }

    const float masterVolume = settings.Volume.MuteAll ? 0.0f : settings.Volume.Master;

    // Lock the mutex to access the active audio clip list securely
    std::lock_guard<std::mutex> lock(s_AudioMutex);

    // Use an iterator to be able to delete elements securely while looping the vector
    auto it = s_ActiveAudioClips.begin();
    while (it != s_ActiveAudioClips.end()) {
        ActiveAudioClip& activeAudioClip = *it;

        // Determine the volume for the category depending on the audio clip type
        float categoryVolume = 1.0f;
        switch (activeAudioClip.Channel) {
            case AudioChannel::Ambience:   categoryVolume = settings.Volume.Ambience;  break;
            case AudioChannel::Music:      categoryVolume = settings.Volume.Music;     break;
            case AudioChannel::Effects:    categoryVolume = settings.Volume.Effects;   break;
            case AudioChannel::Voices:     categoryVolume = settings.Volume.Voices;    break;
            default:                       categoryVolume = masterVolume;              break;
        }

        // Calculate the final volume that will be applied to the sample
        const float finalVolume = masterVolume * categoryVolume * activeAudioClip.Volume;

        size_t framesToMix = framesPerBuffer;
        size_t totalFramesInAudioClip = activeAudioClip.AudioClipData->Data.size() / activeAudioClip.AudioClipData->NumChannels;
        size_t remainingFrames = totalFramesInAudioClip - activeAudioClip.CurrentFrame;

        if (framesToMix > remainingFrames) {
            framesToMix = remainingFrames;
        }

        for (size_t i = 0; i < framesToMix; ++i) {
            size_t frameIndex = activeAudioClip.CurrentFrame + i;

            if (activeAudioClip.AudioClipData->NumChannels == 1) { // Mono
                // Copy the mono signal to both output channels
                const float sample = activeAudioClip.AudioClipData->Data[frameIndex] * finalVolume;
                out[i * 2 + 0] += sample; // Left channel
                out[i * 2 + 1] += sample; // Right channel
            } else if (activeAudioClip.AudioClipData->NumChannels == 2) {
                // Copy each channel to it's corresponding output channel
                const float leftSample = activeAudioClip.AudioClipData->Data[frameIndex * 2 + 0] * finalVolume;
                const float rightSample = activeAudioClip.AudioClipData->Data[frameIndex * 2 + 1] * finalVolume;
                out[i * 2 + 0] += leftSample; // Left channel
                out[i * 2 + 1] += rightSample; // Right channel
            }
        }

        activeAudioClip.CurrentFrame += framesToMix;

        // If the audio clip has finished playing, we remove it from the list and increment the iterator
        if (activeAudioClip.CurrentFrame >= totalFramesInAudioClip) {
            it = s_ActiveAudioClips.erase(it);
        } else {
            ++it;
        }
    }

    // Limit the output buffer data to avoid clipping
    for (unsigned int i = 0; i < framesPerBuffer * 2; ++i) {
        if (out[i] > 1.0f) out[i] = 1.0f;
        if (out[i] < -1.0f) out[i] = -1.0f;
    }

    return paContinue;
}
