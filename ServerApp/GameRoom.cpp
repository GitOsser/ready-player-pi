#include <iostream>
#include <string>
#include <memory>
#include <nlohmann/json.hpp>

#include "GameRoom.h"

#include "../Shared/JSONCommunication.h"
#include "Game.h"
#include "GyroPi/GyroPiGame.h"
#include "NotePi/NotePiGame.h"

using json = nlohmann::json;

GameRoom::GameRoom(int         hostSocket,
                   int         guestSocket,
                   const std::string &hostName,
                   const std::string &guestName,
                   const std::string &gameName)
    : hostSocket_(hostSocket),
      guestSocket_(guestSocket),
      hostName_(hostName),
      guestName_(guestName),
      gameName_(gameName)
{}

void GameRoom::run() {
    std::cout << "[GameRoom] Started: " << hostName_ << " & " << guestName_
              << " | Game: " << gameName_ << "\n";

    while (true) {
        assignRoles();
        sendGameStart();

        std::string playerNames = hostName_ + " & " + guestName_;
        std::unique_ptr<Game> game = createGame();
        game->run(performerSocket_, communicatorSocket_, playerNames);

        json message = receiveJson(hostSocket_);
        if (message.empty() || message.value("command", "") != "start_game") {
            std::cout << "[GameRoom] Host did not request a new game. Ending.\n";
            break;
        }
        gameName_ = message["data"]["game"];
        std::cout << "[GameRoom] Starting new game: " << gameName_ << "\n";
    }

    close(hostSocket_);
    close(guestSocket_);
    std::cout << "[GameRoom] Ended.\n";
}

void GameRoom::assignRoles() {
    srand(time(nullptr) ^ getpid());
    bool hostIsPerformer = (rand() % 2 == 0);

    performerSocket_    = hostIsPerformer ? hostSocket_  : guestSocket_;
    communicatorSocket_ = hostIsPerformer ? guestSocket_ : hostSocket_;
    performerName_      = hostIsPerformer ? hostName_    : guestName_;
    communicatorName_   = hostIsPerformer ? guestName_   : hostName_;

    std::cout << "[GameRoom] " << performerName_   << " = PERFORMER, "
                               << communicatorName_ << " = COMMUNICATOR\n";
}

void GameRoom::sendGameStart() const {
    sendJson(hostSocket_, {
        {"event", "game_start"},
        {"data",  {{"game",    gameName_},
                   {"role",    (performerSocket_ == hostSocket_) ? "PERFORMER" : "COMMUNICATOR"},
                   {"partner", guestName_}}}
    });
    sendJson(guestSocket_, {
        {"event", "game_start"},
        {"data",  {{"game",    gameName_},
                   {"role",    (performerSocket_ == guestSocket_) ? "PERFORMER" : "COMMUNICATOR"},
                   {"partner", hostName_}}}
    });
}

std::unique_ptr<Game> GameRoom::createGame() const {
    if (gameName_ == "GYROPI")  return std::make_unique<GyroPiGame>();
    if (gameName_ == "NOTEPI")  return std::make_unique<NotePiGame>();

    std::cout << "[GameRoom] Unknown game '" << gameName_ << "' — defaulting to GyroPi.\n";
    return std::make_unique<GyroPiGame>();
}

