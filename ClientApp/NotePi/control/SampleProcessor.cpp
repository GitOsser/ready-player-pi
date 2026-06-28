#include "control/SampleProcessor.h"

SampleProcessor::SampleProcessor(int frameSize, int hopSize)
    : frameSize_(frameSize), hopSize_(hopSize) {}

std::optional<std::vector<float>> SampleProcessor::prepareFrame(const std::vector<float>& samples) {
    buffer_.insert(buffer_.end(), samples.begin(), samples.end());

    if (static_cast<int>(buffer_.size()) < frameSize_) {
        return std::nullopt;
    }

    std::vector<float> frame(buffer_.begin(), buffer_.begin() + frameSize_);
    buffer_.erase(buffer_.begin(), buffer_.begin() + hopSize_);
    return frame;
}

void SampleProcessor::reset() {
    buffer_.clear();
}

