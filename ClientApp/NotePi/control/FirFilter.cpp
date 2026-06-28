#include "control/FirFilter.h"
#include "domain/FilterCoeffs.h"

FirFilter::FirFilter()
    : coefficients_(coeffs::FIR_BANDPASS_PIANO_B.begin(), coeffs::FIR_BANDPASS_PIANO_B.end())
    , delayLine_(coefficients_.size(), 0.0) {}

void FirFilter::applyFilter(const std::vector<float>& rawFrame, std::vector<float>& filteredFrame) {
    filteredFrame.resize(rawFrame.size());
    const std::size_t M = coefficients_.size();

    for (std::size_t n = 0; n < rawFrame.size(); ++n) {
        for (std::size_t k = M - 1; k > 0; --k) {
            delayLine_[k] = delayLine_[k - 1];
        }
        delayLine_[0] = static_cast<double>(rawFrame[n]);

        double y = 0.0;
        for (std::size_t k = 0; k < M; ++k) {
            y += coefficients_[k] * delayLine_[k];
        }
        filteredFrame[n] = static_cast<float>(y);
    }
}

void FirFilter::reset() {
    std::fill(delayLine_.begin(), delayLine_.end(), 0.0);
}

