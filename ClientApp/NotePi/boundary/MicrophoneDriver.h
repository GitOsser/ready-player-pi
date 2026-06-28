#pragma once

#include <cstddef>
#include <string>
#include <vector>

struct _snd_pcm;

class MicrophoneDriver {
public:
    MicrophoneDriver(std::string device, unsigned int sampleRateHz, std::size_t blockSizeFrames);
    ~MicrophoneDriver();

    MicrophoneDriver(const MicrophoneDriver&) = delete;
    MicrophoneDriver& operator=(const MicrophoneDriver&) = delete;

    bool start();
    void stop();
    std::size_t readBlock(std::vector<float>& out);

private:
    std::string device_;
    unsigned int sampleRateHz_ = 0;
    std::size_t blockSizeFrames_ = 0;
    _snd_pcm* handle_ = nullptr;
};

