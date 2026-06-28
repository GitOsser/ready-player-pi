#pragma once
#include <iostream>
#include <string>
#include <map>
#include <cstdlib>
#include <ctime>

#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <nlohmann/json.hpp>

#include "ServerSocket.h"
#include "Scoreboard.h"
#include "Room.h"
#include "GameRoom.h"

using json = nlohmann::json;

inline void serverSigchldHandler(int) {
    while (waitpid(-1, nullptr, WNOHANG) > 0);
}

class ServerController {
public:
    explicit ServerController(int port = 9000);

    int run();

private:

    void installSigchldHandler();

    void handleIncomingConnection();

    void handleHostRoom(int clientSocket, const json &message);

    void handleJoinRoom(int clientSocket, const json &message);

    void handleGetScoreboard(int clientSocket, const json &message);

    void handleStartGame(int                                  hostSocket,
                         const std::string                   &roomCode,
                         const json                          &message,
                         std::map<int, std::string>::iterator &pendingEntry);

    void handleHostDisconnect(int                                  hostSocket,
                              const std::string                   &roomCode,
                              std::map<int, std::string>::iterator &pendingEntry);

    int                               port_;
    ServerSocket                      socket_;
    std::map<std::string, Room>       rooms_;
    std::map<int, std::string>        hostPendingMap_;
};

