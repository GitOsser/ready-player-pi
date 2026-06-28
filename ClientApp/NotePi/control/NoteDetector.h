#pragma once

#include <optional>
#include <string>
#include <vector>

class NoteDetector {
public:
    NoteDetector(double fs, int frameSize);

    std::optional<std::string> detectNote(const std::vector<float>& magnitudes);

    void reset();

private:
    struct NoteBin {
        std::string name;
        int         midiPitch;
        double      frequencyHz;
        int         fundamentalBin;
        int         overtone1Bin;
        int         overtone2Bin;
    };

    std::vector<NoteBin> buildNoteTable(int frameSize) const;

    double fs_;
    std::vector<NoteBin> noteBins_;
    std::vector<NoteBin> recentDetections_;
    bool silenceDetected_ = true;

    bool requireSilenceBeforeNext_ = false;
};

