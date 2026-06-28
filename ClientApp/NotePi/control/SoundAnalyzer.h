#pragma once

#include <vector>

#include "control/FftAnalyzer.h"
#include "control/FirFilter.h"
#include "control/NoteDetector.h"
#include "control/SampleProcessor.h"

class NoteSequence;

class SoundAnalyzer {
public:
    SoundAnalyzer();

    void analyze(const std::vector<float>& samples, NoteSequence& sequence);

    void reset();

private:
    SampleProcessor sampleProcessor_;
    FirFilter       firFilter_;
    FftAnalyzer     fftAnalyzer_;
    NoteDetector    noteDetector_;

    std::vector<float> rawFrame_;
    std::vector<float> filteredFrame_;
    std::vector<float> magnitudes_;
};

