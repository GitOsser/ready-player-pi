#pragma once

#include <optional>
#include <vector>

class SampleProcessor {
public:
    SampleProcessor(int frameSize, int hopSize);

    std::optional<std::vector<float>> prepareFrame(const std::vector<float>& samples);

    void reset();

private:
    int frameSize_;
    int hopSize_;
    std::vector<float> buffer_;
};

