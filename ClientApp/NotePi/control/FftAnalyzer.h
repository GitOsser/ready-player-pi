#pragma once

#include <vector>
#include "kiss_fftr.h"

class FftAnalyzer {
public:
    FftAnalyzer(int frameSize, double fs);
    ~FftAnalyzer();

    FftAnalyzer(const FftAnalyzer&) = delete;
    FftAnalyzer& operator=(const FftAnalyzer&) = delete;

    void computeSpectrum(const std::vector<float>& filteredFrame, std::vector<float>& magnitudes);

private:
    int frameSize_;
    double fs_;
    std::vector<float> hannWindow_;
    kiss_fftr_state* cfg_ = nullptr;
};

