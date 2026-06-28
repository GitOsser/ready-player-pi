#pragma once
#include <string>
#include <vector>
#include <cstdlib>
#include "NotePi.h"

class SecretSequence {
public:

    explicit SecretSequence(int length = 5) {
        static const std::vector<std::string> availableNotes =
            {"C", "C#", "D", "D#", "E", "F",
             "F#", "G", "G#", "A", "A#", "B"};

        for (int i = 0; i < length; i++)
            sequence.notes.push_back(
                availableNotes[rand() % availableNotes.size()]);
    }

    std::vector<NoteFeedback> evaluate(const NoteSequence &attempt) const {
        const auto &secret = sequence.notes;
        const auto &guess = attempt.notes;
        const int n = static_cast<int>(std::min(secret.size(), guess.size()));

        std::vector<NoteFeedback> feedback(n, NoteFeedback::WRONG_PITCH);
        std::vector<bool> secretUsed(n, false);

        for (int i = 0; i < n; i++) {
            if (guess[i] == secret[i]) {
                feedback[i]   = NoteFeedback::CORRECT;
                secretUsed[i] = true;
            }
        }

        for (int i = 0; i < n; i++) {
            if (feedback[i] == NoteFeedback::CORRECT) continue;
            for (int j = 0; j < n; j++) {
                if (!secretUsed[j] && guess[i] == secret[j]) {
                    feedback[i]   = NoteFeedback::WRONG_ORDER;
                    secretUsed[j] = true;
                    break;
                }
            }
        }

        return feedback;
    }

    bool isSolved(const NoteSequence &attempt) const {
        return attempt.notes == sequence.notes;
    }

    const NoteSequence &getSequence() const { return sequence; }

private:
    NoteSequence sequence;
};

