#pragma once
#include <iostream>
#include <string>
#include <memory>
#include <nlohmann/json.hpp>

#include "../Shared/JSONCommunication.h"
#include "Game.h"
#include "GyroPi/GyroPiGame.h"
#include "NotePi/NotePiGame.h"

using json = nlohmann::json;

class GameRoom {
public:
    GameRoom(int hostSocket, int guestSocket,
             const std::string &hostName, const std::string &guestName,
             const std::string &gameName);

    void run();

private:

    int         hostSocket_;
    int         guestSocket_;
    std::string hostName_;
    std::string guestName_;
    std::string gameName_;

    int         performerSocket_    = -1;
    int         communicatorSocket_ = -1;
    std::string performerName_;
    std::string communicatorName_;

    void assignRoles();
    void sendGameStart() const;
    std::unique_ptr<Game> createGame() const;
};

