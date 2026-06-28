#pragma once

#include <vector>

class FirFilter {
public:
    FirFilter();

    void applyFilter(const std::vector<float>& rawFrame, std::vector<float>& filteredFrame);

    void reset();

private:
    std::vector<double> coefficients_;
    std::vector<double> delayLine_;
};

