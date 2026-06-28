#include "control/FftAnalyzer.h"

#include <cmath>
#include <stdexcept>
#include <vector>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

FftAnalyzer::FftAnalyzer(int frameSize, double fs)
    : frameSize_(frameSize), fs_(fs) {
    if (frameSize_ <= 0 || (frameSize_ % 2) != 0) {
        throw std::invalid_argument("FftAnalyzer: frameSize must be positive and even");
    }

    hannWindow_.resize(static_cast<std::size_t>(frameSize_));
    for (int k = 0; k < frameSize_; ++k) {
        hannWindow_[static_cast<std::size_t>(k)] =
            0.5f - 0.5f * static_cast<float>(
                std::cos(2.0 * M_PI * k / static_cast<double>(frameSize_ - 1)));
    }

    cfg_ = kiss_fftr_alloc(frameSize_, 0, nullptr, nullptr);
    if (!cfg_) {
        throw std::runtime_error("FftAnalyzer: kiss_fftr_alloc failed");
    }
}

FftAnalyzer::~FftAnalyzer() {
    kiss_fftr_free(cfg_);
}

void FftAnalyzer::computeSpectrum(const std::vector<float>& filteredFrame, std::vector<float>& magnitudes) {
    if (static_cast<int>(filteredFrame.size()) != frameSize_) {
        throw std::invalid_argument("FftAnalyzer::computeSpectrum: frame size mismatch");
    }

    std::vector<kiss_fft_scalar> timeDomain(static_cast<std::size_t>(frameSize_));
    for (int k = 0; k < frameSize_; ++k) {
        timeDomain[static_cast<std::size_t>(k)] =
            filteredFrame[static_cast<std::size_t>(k)] * hannWindow_[static_cast<std::size_t>(k)];
    }

    const std::size_t bins = static_cast<std::size_t>(frameSize_ / 2 + 1);
    std::vector<kiss_fft_cpx> freqDomain(bins);
    kiss_fftr(cfg_, timeDomain.data(), freqDomain.data());

    magnitudes.resize(bins);
    for (std::size_t k = 0; k < bins; ++k) {
        const float re = freqDomain[k].r;
        const float im = freqDomain[k].i;
        magnitudes[k] = re * re + im * im;
    }
}

