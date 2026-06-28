#pragma once
#include <string>

class GameSession {
public:

    virtual bool playAsPerformer   (int socketFileDescriptor, const std::string &logTag,
                                    const std::string &gameName) = 0;
    virtual bool playAsCommunicator(int socketFileDescriptor, const std::string &logTag,
                                    const std::string &gameName) = 0;
    virtual ~GameSession() = default;
};

