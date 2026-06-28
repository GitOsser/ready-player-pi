#include "boundary/MicrophoneDriver.h"

#include <alsa/asoundlib.h>

#include <cstdint>
#include <iostream>
#include <vector>

MicrophoneDriver::MicrophoneDriver(std::string device, unsigned int sampleRateHz, std::size_t blockSizeFrames)
    : device_(std::move(device)), sampleRateHz_(sampleRateHz), blockSizeFrames_(blockSizeFrames) {}

MicrophoneDriver::~MicrophoneDriver() {
    stop();
}

bool MicrophoneDriver::start() {
    if (handle_ != nullptr) {
        return true;
    }

    if (snd_pcm_open(&handle_, device_.c_str(), SND_PCM_STREAM_CAPTURE, 0) < 0) {
        std::cerr << "Failed to open ALSA capture device: " << device_ << "\n";
        handle_ = nullptr;
        return false;
    }

    snd_pcm_hw_params_t* params = nullptr;
    snd_pcm_hw_params_alloca(&params);
    snd_pcm_hw_params_any(handle_, params);
    snd_pcm_hw_params_set_access(handle_, params, SND_PCM_ACCESS_RW_INTERLEAVED);
    snd_pcm_hw_params_set_format(handle_, params, SND_PCM_FORMAT_S16_LE);
    snd_pcm_hw_params_set_channels(handle_, params, 1);

    unsigned int configuredRate = sampleRateHz_;
    snd_pcm_hw_params_set_rate_near(handle_, params, &configuredRate, nullptr);

    snd_pcm_uframes_t periodSize = static_cast<snd_pcm_uframes_t>(blockSizeFrames_);
    snd_pcm_hw_params_set_period_size_near(handle_, params, &periodSize, nullptr);

    if (snd_pcm_hw_params(handle_, params) < 0) {
        std::cerr << "Failed to configure ALSA hardware parameters\n";
        snd_pcm_close(handle_);
        handle_ = nullptr;
        return false;
    }

    sampleRateHz_ = configuredRate;
    blockSizeFrames_ = static_cast<std::size_t>(periodSize);
    snd_pcm_prepare(handle_);
    return true;
}

void MicrophoneDriver::stop() {
    if (handle_ == nullptr) {
        return;
    }

    snd_pcm_drop(handle_);
    snd_pcm_close(handle_);
    handle_ = nullptr;
}

std::size_t MicrophoneDriver::readBlock(std::vector<float>& out) {
    out.assign(blockSizeFrames_, 0.0f);
    if (handle_ == nullptr) {
        return 0;
    }

    std::vector<std::int16_t> pcm(blockSizeFrames_, 0);
    snd_pcm_sframes_t frames = snd_pcm_readi(handle_, pcm.data(), blockSizeFrames_);
    if (frames == -EPIPE) {

        snd_pcm_prepare(handle_);
        return 0;
    }

    if (frames < 0) {
        frames = snd_pcm_recover(handle_, static_cast<int>(frames), 0);
        if (frames < 0) {
            std::cerr << "ALSA capture failed: " << snd_strerror(static_cast<int>(frames)) << "\n";
            return 0;
        }
    }

    const std::size_t count = static_cast<std::size_t>(frames);
    out.resize(count);
    for (std::size_t i = 0; i < count; ++i) {
        out[i] = static_cast<float>(pcm[i]) / 32768.0f;
    }

    return count;
}

