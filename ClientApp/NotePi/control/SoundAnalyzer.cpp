#include "control/SoundAnalyzer.h"

#include "domain/NotePiConfig.h"
#include "domain/NoteSequence.h"

SoundAnalyzer::SoundAnalyzer()
    : sampleProcessor_(notepi::config::FRAME_SIZE, notepi::config::HOP_SIZE)
    , firFilter_()
    , fftAnalyzer_(notepi::config::FRAME_SIZE, notepi::config::SAMPLE_RATE_HZ)
    , noteDetector_(notepi::config::SAMPLE_RATE_HZ, notepi::config::FRAME_SIZE) {}

void SoundAnalyzer::analyze(const std::vector<float>& samples, NoteSequence& sequence) {
    auto frame = sampleProcessor_.prepareFrame(samples);
    if (!frame) {
        return;
    }
    rawFrame_ = std::move(*frame);

    firFilter_.applyFilter(rawFrame_, filteredFrame_);
    fftAnalyzer_.computeSpectrum(filteredFrame_, magnitudes_);

    auto note = noteDetector_.detectNote(magnitudes_);
    if (note) {
        sequence.addNote(*note);
    }
}

void SoundAnalyzer::reset() {
    sampleProcessor_.reset();
    firFilter_.reset();
    noteDetector_.reset();

}

