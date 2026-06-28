#pragma once

#include <string>
#include <vector>

class NoteSequence {
public:
    void addNote(const std::string& noteName);
    const std::vector<std::string>& getNotes() const;
    void reset();

private:
    std::vector<std::string> notes_;
};

