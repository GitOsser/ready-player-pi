#include "control/NoteDetector.h"

#include "domain/NotePiConfig.h"
#include "domain/PianoFrequencies.h"

#include <cmath>
#include <unordered_map>

NoteDetector::NoteDetector(double fs, int frameSize)
    : fs_(fs), noteBins_(buildNoteTable(frameSize)) {}

std::vector<NoteDetector::NoteBin> NoteDetector::buildNoteTable(int frameSize) const {
    std::vector<NoteBin> table;
    table.reserve(piano::NOTES.size());

    for (const auto& note : piano::NOTES) {
        NoteBin b;
        b.name           = note.name;
        b.midiPitch      = note.midiPitch;
        b.frequencyHz    = note.frequencyHz;
        b.fundamentalBin = static_cast<int>(std::round(note.frequencyHz * 1.0 * frameSize / fs_));
        b.overtone1Bin   = static_cast<int>(std::round(note.frequencyHz * 2.0 * frameSize / fs_));
        b.overtone2Bin   = static_cast<int>(std::round(note.frequencyHz * 3.0 * frameSize / fs_));
        table.push_back(b);
    }
    return table;
}

std::optional<std::string> NoteDetector::detectNote(const std::vector<float>& magnitudes) {
    namespace cfg = notepi::config;

    float totalEnergy = 0.0f;
    for (float m : magnitudes) {
        totalEnergy += m;
    }
    if (totalEnergy < cfg::MIN_ENERGY) {

        if (!silenceDetected_) {
            recentDetections_.clear();
        }
        silenceDetected_ = true;
        requireSilenceBeforeNext_ = false;
        return std::nullopt;
    }

    if (requireSilenceBeforeNext_) {
        return std::nullopt;
    }

    const int maxBin = static_cast<int>(magnitudes.size()) - 1;
    int bestIndex = -1;
    float bestScore = 0.0f;

    for (std::size_t i = 0; i < noteBins_.size(); ++i) {
        const NoteBin& b = noteBins_[i];
        float score = 0.0f;
        if (b.fundamentalBin <= maxBin) {
            score += cfg::FUNDAMENTAL_WEIGHT * magnitudes[static_cast<std::size_t>(b.fundamentalBin)];
        }
        if (b.overtone1Bin <= maxBin) {
            score += cfg::OVERTONE1_WEIGHT * magnitudes[static_cast<std::size_t>(b.overtone1Bin)];
        }
        if (b.overtone2Bin <= maxBin) {
            score += cfg::OVERTONE2_WEIGHT * magnitudes[static_cast<std::size_t>(b.overtone2Bin)];
        }
        if (score > bestScore) {
            bestScore = score;
            bestIndex = static_cast<int>(i);
        }
    }

    if (bestIndex < 0 || bestScore < cfg::DOMINANCE_THRESHOLD * totalEnergy) {
        return std::nullopt;
    }

    recentDetections_.push_back(noteBins_[static_cast<std::size_t>(bestIndex)]);
    silenceDetected_ = false;

    if (recentDetections_.size() < cfg::RECENT_DETECTIONS_SIZE) {
        return std::nullopt;
    }

    std::unordered_map<int, int> counts;
    int winnerMidi = recentDetections_.front().midiPitch;
    int winnerCount = 0;
    for (const NoteBin& d : recentDetections_) {
        const int c = ++counts[d.midiPitch];
        if (c > winnerCount) {
            winnerCount = c;
            winnerMidi = d.midiPitch;
        }
    }

    std::string winnerName;
    for (const NoteBin& d : recentDetections_) {
        if (d.midiPitch == winnerMidi) {
            winnerName = d.name;
            break;
        }
    }

    recentDetections_.clear();
    requireSilenceBeforeNext_ = true;
    return winnerName;
}

void NoteDetector::reset() {
    recentDetections_.clear();
    silenceDetected_ = true;
    requireSilenceBeforeNext_ = false;
}

