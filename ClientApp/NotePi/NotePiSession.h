#pragma once
#include <string>

#include "../GameSession.h"

class NotePiPerformerWidget;
class NotePiCommunicatorWidget;

class NotePiSession : public GameSession {
public:
    bool playAsPerformer   (int socketFileDescriptor, const std::string &logTag,
                            const std::string &gameName) override;
    bool playAsCommunicator(int socketFileDescriptor, const std::string &logTag,
                            const std::string &gameName) override;

private:
    NotePiPerformerWidget    *performerWidget_    = nullptr;
    NotePiCommunicatorWidget *communicatorWidget_ = nullptr;
};

