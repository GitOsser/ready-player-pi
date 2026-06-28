#include <string>
#include <memory>
#include <nlohmann/json.hpp>

#include "ClientController.h"

#include "ClientSocket.h"
#include "gui/QtGui.h"
#include "GameSession.h"
#include "GameFactory.h"
#include "../Shared/JSONCommunication.h"

#include <QEventLoop>
#include <QMetaObject>
#include <thread>

using json = nlohmann::json;

static json receiveEventPumpingGui(ClientSocket &socket) {
    QEventLoop loop;
    json result;

    std::thread receiver([&]() {
        result = socket.receiveEvent();
        QMetaObject::invokeMethod(&loop, "quit", Qt::QueuedConnection);
    });

    loop.exec();
    receiver.join();
    return result;
}

    ClientController::ClientController(const char *serverIp, int port)
        : serverIp_(serverIp), port_(port), credentials_() {}

    int ClientController::run() {

        {
            std::string prefill = credentials_.exists() ? credentials_.load() : "";
            username_ = gui_.promptUsername(prefill);
            if (username_.empty()) return 0;
            credentials_.save(username_);
        }

        QObject::connect(&gui_, &QtGUI::cancelRequested, &gui_, [this]() {
            cancelRequested_ = true;
            socket_.disconnect();
        });

        while (true) {
            cancelRequested_ = false;
            int choice = gui_.promptMainMenu();
            if (choice <= 0) break;

            gui_.showConnecting(serverIp_, port_);
            if (!socket_.connectToServer(serverIp_, port_)) {
                gui_.showError("Kunne ikke forbinde til server");
                continue;
            }
            gui_.showConnected();

            bool isHost = (choice == 1);
            bool lobbyOk = false;
            if (isHost) {
                logTag_ = "host";
                lobbyOk = !hostFlow().empty();
            } else {
                logTag_ = "guest";
                lobbyOk = guestFlow();
            }
            if (!lobbyOk) {
                socket_.disconnect();
                continue;
            }

            while (true) {
                if (!awaitGameStart()) break;
                bool playAgainHandled = runGameSession();

                if (playAgainHandled) {

                    continue;
                }

                if (isHost) {
                    int gameChoice = gui_.promptPlayAgain();
                    if (gameChoice == 0) break;

                    std::string newGame = (gameChoice == 2) ? "NOTEPI" : "GYROPI";
                    socket_.sendCommand({{"command", "start_game"},
                                         {"data",    {{"game", newGame}}}});
                }

            }

            socket_.disconnect();

        }

        gui_.showDisconnected();
        return 0;
    }

    std::string ClientController::hostFlow() {
        socket_.sendCommand({{"command", "host_room"},
                             {"data",    {{"username", username_}}}});

        json serverReply = socket_.receiveEvent();
        if (serverReply.value("event", "") != "room_created") {
            gui_.showRoomCreationFailed();
            return "";
        }

        std::string roomCode = serverReply["data"]["roomCode"];
        gui_.showRoomCreated(roomCode);
        gui_.showWaitingForGuest();

        json playerJoinedMessage = receiveEventPumpingGui(socket_);
        if (cancelRequested_ || playerJoinedMessage.empty()) return "";
        if (playerJoinedMessage.value("event", "") == "player_joined") {
            std::vector<std::string> players;
            for (auto &player : playerJoinedMessage["data"]["players"])
                players.push_back(player.get<std::string>());
            gui_.showPlayersJoined(players);
        }

        int gameChoice = gui_.promptGameChoice();
        if (cancelRequested_ || gameChoice <= 0) return "";
        std::string gameName = (gameChoice == 2) ? "NOTEPI" : "GYROPI";
        gui_.showStartingGame(gameName);

        socket_.sendCommand({{"command", "start_game"},
                             {"data",    {{"roomCode", roomCode},
                                          {"game",     gameName}}}});
        return gameName;
    }

    bool ClientController::guestFlow() {
        std::string roomCode = gui_.promptRoomCode();
        if (roomCode.empty()) return false;

        socket_.sendCommand({{"command", "join_room"},
                             {"data",    {{"username", username_},
                                          {"roomCode", roomCode}}}});

        json serverReply = socket_.receiveEvent();
        if (serverReply.value("event", "") == "error") {
            gui_.showError(serverReply["data"]["detail"]);
            return false;
        }
        if (serverReply.value("event", "") == "player_joined") {
            std::vector<std::string> players;
            for (auto &player : serverReply["data"]["players"])
                players.push_back(player.get<std::string>());
            gui_.showJoinedRoom(roomCode, players);
        }
        gui_.showWaitingForHost();
        return true;
    }

    bool ClientController::awaitGameStart() {

        json gameStartMessage = receiveEventPumpingGui(socket_);
        if (cancelRequested_ || gameStartMessage.empty() ||
            gameStartMessage.value("event", "") != "game_start") {
            if (!cancelRequested_) gui_.showGameStartFailed();
            return false;
        }

        role_     = gameStartMessage["data"]["role"];
        partner_  = gameStartMessage["data"]["partner"];
        gameName_ = gameStartMessage["data"]["game"];

        gui_.showGameStartHeader(gameName_, role_, partner_);
        return true;
    }

    bool ClientController::runGameSession() {
        std::unique_ptr<GameSession> gameSession = createGameSession(gameName_);

        if (gameName_ == "NOTEPI") {
            gui_.showNotePiRole(role_, logTag_);
        } else if (gameName_ == "GYROPI") {
            gui_.showGyroPiRole(role_, partner_, logTag_);
        }

        if (role_ == "PERFORMER")
            return gameSession->playAsPerformer   (socket_.getSocketFd(), logTag_, gameName_);
        else
            return gameSession->playAsCommunicator(socket_.getSocketFd(), logTag_, gameName_);
    }

