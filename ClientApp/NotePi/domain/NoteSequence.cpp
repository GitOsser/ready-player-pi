#include "domain/NoteSequence.h"

#include <cctype>

void NoteSequence::addNote(const std::string& noteName) {
    std::string letter = noteName;
    while (!letter.empty()
           && std::isdigit(static_cast<unsigned char>(letter.back()))) {
        letter.pop_back();
    }
    notes_.push_back(letter);
}

const std::vector<std::string>& NoteSequence::getNotes() const {
    return notes_;
}

void NoteSequence::reset() {
    notes_.clear();
}

