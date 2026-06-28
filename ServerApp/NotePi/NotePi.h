#pragma once
#include <string>
#include <vector>

struct NoteSequence {
    std::vector<std::string> notes;
};

enum class NoteFeedback {
    CORRECT,
    WRONG_PITCH,
    WRONG_ORDER
};

