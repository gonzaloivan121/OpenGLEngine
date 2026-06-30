#include "AudioClip.h"

#include "Core/Log/Log.h"

#include "dr_wav.h"

Ref<AudioClip> AudioClip::LoadFromFile(const std::filesystem::path& filepath) {
    if (filepath.empty()) {
        Log::Error("AudioClip::LoadFromFile - Filepath is empty");
        return nullptr;
    }

    Log::Trace("AudioClip::LoadFromFile - Loading WAV file: " + filepath.string());

    drwav wav;

    if (!drwav_init_file(&wav, filepath.string().c_str(), NULL)) {
        Log::Error("AudioClip::LoadWAV - Couldn't load or find the WAV file: " + filepath.string());
        return nullptr;
    }

    Log::Trace("AudioClip::LoadWAV - Loading WAV file with " + std::to_string(wav.channels) + " Channels and a Sample Rate of " + std::to_string(wav.sampleRate));

    Ref<AudioClip> audioClip = CreateRef<AudioClip>();
    audioClip->NumChannels = wav.channels;
    audioClip->SampleRate = wav.sampleRate;
    audioClip->Data.resize(wav.totalPCMFrameCount * wav.channels);
    audioClip->Filepath = filepath;

    drwav_read_pcm_frames_f32(&wav, wav.totalPCMFrameCount, audioClip->Data.data());

    drwav_uninit(&wav);

    Log::Trace("AudioClip::LoadWAV - WAV file loaded: " + filepath.string());

    return audioClip;
}