#pragma once
#include <string>

#include "../GameSession.h"

class GyroPiSession : public GameSession {
public:
    bool playAsPerformer   (int socketFileDescriptor, const std::string &logTag,
                            const std::string &gameName) override;
    bool playAsCommunicator(int socketFileDescriptor, const std::string &logTag,
                            const std::string &gameName) override;
};

