#pragma once
#include <string>
#include <unistd.h>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

inline std::string readLine(int fileDescriptor) {
    std::string line;
    char        character;
    while (true) {
        int bytesRead = read(fileDescriptor, &character, 1);
        if (bytesRead <= 0) return "";
        if (character == '\n') break;
        line += character;
    }
    return line;
}

inline bool sendLine(int fileDescriptor, const std::string &message) {
    std::string framedMessage   = message + "\n";
    int         bytesSentSoFar  = 0;
    while (bytesSentSoFar < (int)framedMessage.size()) {
        int bytesWritten = write(fileDescriptor,
                                 framedMessage.c_str() + bytesSentSoFar,
                                 framedMessage.size()  - bytesSentSoFar);
        if (bytesWritten <= 0) return false;
        bytesSentSoFar += bytesWritten;
    }
    return true;
}

inline bool sendJson(int fileDescriptor, const json &message) {
    return sendLine(fileDescriptor, message.dump());
}

inline json receiveJson(int fileDescriptor) {
    std::string line = readLine(fileDescriptor);
    if (line.empty()) return json();
    try   { return json::parse(line); }
    catch (...) { return json(); }
}

