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

#include "ServerController.h"

#include "ServerSocket.h"
#include "Scoreboard.h"
#include "Room.h"
#include "GameRoom.h"

using json = nlohmann::json;

    ServerController::ServerController(int port) : port_(port) {}

    int ServerController::run() {
        srand(time(nullptr));
        installSigchldHandler();

        if (!socket_.startListening(port_)) return 1;
        std::cout << "[Lobby] Listening on port " << port_ << "\n";

        while (true) {
            bool                       listenReadable = false;
            std::map<int, std::string> hostReadable;

            if (!socket_.waitForActivity(hostPendingMap_,
                                          listenReadable,
                                          hostReadable))
                break;

            if (listenReadable)
                handleIncomingConnection();

            for (auto pendingEntry = hostPendingMap_.begin();
                 pendingEntry != hostPendingMap_.end(); )
            {
                int         hostSocket = pendingEntry->first;
                std::string roomCode   = pendingEntry->second;

                if (hostReadable.find(hostSocket) == hostReadable.end()) {
                    ++pendingEntry;
                    continue;
                }

                json message = socket_.receiveCommand(hostSocket);
                if (message.empty()) {
                    handleHostDisconnect(hostSocket, roomCode, pendingEntry);
                    continue;
                }

                if (message.value("command", "") == "start_game") {
                    handleStartGame(hostSocket, roomCode, message, pendingEntry);
                } else {
                    ++pendingEntry;
                }
            }
        }

        socket_.closeListenSocket();
        return 0;
    }

    void ServerController::installSigchldHandler() {
        struct sigaction signalAction{};
        signalAction.sa_handler = serverSigchldHandler;
        signalAction.sa_flags   = SA_RESTART | SA_NOCLDSTOP;
        sigaction(SIGCHLD, &signalAction, nullptr);
    }

    void ServerController::handleIncomingConnection() {
        int clientSocket = socket_.acceptClient();
        if (clientSocket < 0) return;

        json message = socket_.receiveCommand(clientSocket);
        if (message.empty()) { socket_.closeClient(clientSocket); return; }

        std::string command = message.value("command", "");
        if      (command == "host_room")       handleHostRoom      (clientSocket, message);
        else if (command == "join_room")       handleJoinRoom      (clientSocket, message);
        else if (command == "get_scoreboard") handleGetScoreboard(clientSocket, message);
        else {
            socket_.sendEvent(clientSocket,
                {{"event","error"},{"data",{{"detail","Unknown command"}}}});
            socket_.closeClient(clientSocket);
        }
    }

    void ServerController::handleHostRoom(int clientSocket, const json &message) {
        std::string roomCode = std::to_string(100000 + rand() % 900000);
        Room &room       = rooms_[roomCode];
        room.code        = roomCode;
        room.hostSocket  = clientSocket;
        room.hostName    = message["data"]["username"];

        socket_.sendEvent(clientSocket,
            {{"event","room_created"},{"data",{{"roomCode",roomCode}}}});
        hostPendingMap_[clientSocket] = roomCode;
        std::cout << "[Lobby] Room " << roomCode
                  << " created by " << room.hostName << "\n";
    }

    void ServerController::handleJoinRoom(int clientSocket, const json &message) {
        std::string roomCode = message["data"]["roomCode"];

        if (rooms_.find(roomCode) == rooms_.end() ||
            rooms_[roomCode].hostSocket < 0)
        {
            socket_.sendEvent(clientSocket,
                {{"event","error"},{"data",{{"detail","Room not found"}}}});
            socket_.closeClient(clientSocket);
            return;
        }

        Room &room       = rooms_[roomCode];
        room.guestSocket = clientSocket;
        room.guestName   = message["data"]["username"];

        json playerJoinedMessage = {
            {"event", "player_joined"},
            {"data",  {{"players", {room.hostName, room.guestName}}, {"count", 2}}}
        };
        socket_.sendEvent(room.hostSocket,  playerJoinedMessage);
        socket_.sendEvent(room.guestSocket, playerJoinedMessage);
        std::cout << "[Lobby] " << room.guestName
                  << " joined room " << roomCode << "\n";
    }

    void ServerController::handleGetScoreboard(int clientSocket, const json &message) {
        std::string game            = message["data"]["game"];
        std::string scoreboardFile = "scoreboard_" + game + ".txt";
        json        scoreboard     = readScoreboard(scoreboardFile);

        socket_.sendEvent(clientSocket,
            {{"event","scoreboard"},
             {"data", {{"game",game},{"scores",scoreboard}}}});
        socket_.closeClient(clientSocket);
    }

    void ServerController::handleStartGame(int                                  hostSocket,
                         const std::string                   &roomCode,
                         const json                          &message,
                         std::map<int, std::string>::iterator &pendingEntry)
    {
        std::string game = message["data"]["game"];

        if (rooms_.find(roomCode) == rooms_.end()) {
            socket_.sendEvent(hostSocket,
                {{"event","error"},{"data",{{"detail","Room not found"}}}});
            pendingEntry = hostPendingMap_.erase(pendingEntry);
            return;
        }

        Room &room = rooms_[roomCode];
        if (room.guestSocket < 0) {
            socket_.sendEvent(hostSocket,
                {{"event","error"},{"data",{{"detail","Waiting for guest"}}}});
            ++pendingEntry;
            return;
        }

        std::cout << "[Lobby] Starting " << game
                  << " in room " << roomCode << "\n";

        pid_t childProcessId = fork();
        if (childProcessId == 0) {

            socket_.closeListenSocket();
            GameRoom gameRoom(room.hostSocket, room.guestSocket,
                              room.hostName,   room.guestName, game);
            gameRoom.run();
            _exit(0);
        } else if (childProcessId > 0) {

            socket_.closeClient(room.hostSocket);
            socket_.closeClient(room.guestSocket);
            std::cout << "[Lobby] Room " << roomCode
                      << " forked to PID " << childProcessId << "\n";
        } else {
            perror("fork");
        }

        rooms_.erase(roomCode);
        pendingEntry = hostPendingMap_.erase(pendingEntry);
    }

    void ServerController::handleHostDisconnect(int                                  hostSocket,
                              const std::string                   &roomCode,
                              std::map<int, std::string>::iterator &pendingEntry)
    {
        std::cout << "[Lobby] Host disconnected from room " << roomCode << "\n";
        rooms_.erase(roomCode);
        socket_.closeClient(hostSocket);
        pendingEntry = hostPendingMap_.erase(pendingEntry);
    }

