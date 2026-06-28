#pragma once
#include <iostream>
#include <string>
#include <thread>
#include <atomic>
#include <chrono>
#include <algorithm>

#include <sys/select.h>
#include <nlohmann/json.hpp>

#include "../../Shared/JSONCommunication.h"
#include "GyroPi.h"
#include "Ball.h"
#include "Course.h"
#include "../Scoreboard.h"
#include "../Game.h"

using json = nlohmann::json;

class GyroPiGame : public Game {
public:
    void run(int performerSocket, int communicatorSocket,
             const std::string &playerNames) override;

private:
    void physicsLoop(int performerSocket, int communicatorSocket,
                     const std::string &playerNames);

    Course            course_;
    Ball              ball_{course_.getStart()};
    GameState         gameState_;
    std::atomic<bool> isRunning_{false};
    std::chrono::steady_clock::time_point startTime_;
};

inline void GyroPiGame::run(int performerSocket, int communicatorSocket,
                             const std::string &playerNames)
{
    gameState_.score  = 0;
    gameState_.active = true;
    isRunning_        = true;
    startTime_        = std::chrono::steady_clock::now();

    json holesArray = json::array();
    for (const auto &hole : course_.getHoles())
        holesArray.push_back({{"x", hole.getPosition().x},
                              {"y", hole.getPosition().y},
                              {"r", hole.getRadius()}});

    sendJson(communicatorSocket, {
        {"event", "course_info"},
        {"data",  {{"start", {{"x", course_.getStart().x}, {"y", course_.getStart().y}}},
                   {"goal",  {{"x", course_.getGoal().x},  {"y", course_.getGoal().y}}},
                   {"holes", holesArray}}}
    });

    std::thread physicsThread([this, performerSocket, communicatorSocket, playerNames]() {
        physicsLoop(performerSocket, communicatorSocket, playerNames);
    });

    while (isRunning_) {
        fd_set readSet;
        FD_ZERO(&readSet);
        FD_SET(performerSocket,    &readSet);
        FD_SET(communicatorSocket, &readSet);
        int maxSocketFd = std::max(performerSocket, communicatorSocket) + 1;

        struct timeval timeout;
        timeout.tv_sec  = 0;
        timeout.tv_usec = 100000;
        if (select(maxSocketFd, &readSet, nullptr, nullptr, &timeout) < 0) break;

        if (FD_ISSET(performerSocket, &readSet)) {
            json message = receiveJson(performerSocket);
            if (message.empty()) { isRunning_ = false; break; }

            const std::string cmd = message.value("command", "");
            if (cmd == "tilt") {
                std::lock_guard<std::mutex> stateLock(gameState_.mutex);
                gameState_.tiltX = message["data"].value("tiltX", 0.0f);
                gameState_.tiltY = message["data"].value("tiltY", 0.0f);
            } else if (cmd == "exit_game") {
                std::cout << "[GyroPiGame] EXIT from performer — ending game.\n";
                isRunning_ = false;
                json exitMsg = {{"event", "game_exit"}};
                sendJson(performerSocket,    exitMsg);
                sendJson(communicatorSocket, exitMsg);
                break;
            }
        }

        if (FD_ISSET(communicatorSocket, &readSet)) {
            json message = receiveJson(communicatorSocket);
            if (message.empty()) { isRunning_ = false; break; }

            if (message.value("command", "") == "exit_game") {
                std::cout << "[GyroPiGame] EXIT from communicator — ending game.\n";
                isRunning_ = false;
                json exitMsg = {{"event", "game_exit"}};
                sendJson(performerSocket,    exitMsg);
                sendJson(communicatorSocket, exitMsg);
                break;
            }
        }
    }

    physicsThread.join();
}

inline void GyroPiGame::physicsLoop(int performerSocket, int communicatorSocket,
                                     const std::string &playerNames)
{
    std::cout << "[GyroPiGame] Physics thread started (30 Hz)\n";

    while (isRunning_) {
        std::this_thread::sleep_for(std::chrono::milliseconds(TICK_MS));

        std::lock_guard<std::mutex> stateLock(gameState_.mutex);
        if (!gameState_.active) continue;

        ball_.applyTilt(gameState_.tiltX, gameState_.tiltY);

        gameState_.score = static_cast<int>(
            std::chrono::duration<float>(std::chrono::steady_clock::now() - startTime_).count());

        for (const auto &hole : course_.getHoles()) {
            if (hole.contains(ball_.getPosition())) {
                std::cout << "[GyroPiGame] BLACK HOLE HIT — ball reset.\n";
                ball_.resetToStart();
                startTime_ -= std::chrono::seconds(5);

                json blackHoleMessage = {{"event", "blackhole_hit"}};
                sendJson(performerSocket,    blackHoleMessage);
                sendJson(communicatorSocket, blackHoleMessage);
                break;
            }
        }

        if (course_.isGoalReached(ball_.getPosition())) {
            std::cout << "[GyroPiGame] GOAL! Final score: " << gameState_.score << "\n";
            gameState_.active = false;

            writeScore("scoreboard_gyropi.txt", playerNames, gameState_.score);
            json scoreboard = readScoreboard("scoreboard_gyropi.txt");

            int rank = -1;
            for (int i = 0; i < (int)scoreboard.size(); i++) {
                if (scoreboard[i]["score"] == gameState_.score) { rank = i + 1; break; }
            }

            json topFiveScores = json::array();
            for (int i = 0; i < std::min(5, (int)scoreboard.size()); i++)
                topFiveScores.push_back(scoreboard[i]);

            json gameOverMessage = {
                {"event", "game_over"},
                {"data",  {{"score", gameState_.score},
                           {"rank",  rank},
                           {"top5",  topFiveScores}}}
            };
            sendJson(performerSocket,    gameOverMessage);
            sendJson(communicatorSocket, gameOverMessage);

            isRunning_ = false;
            return;
        }

        sendJson(communicatorSocket, {
            {"event", "ball_update"},
            {"data",  {{"x",     ball_.getPosition().x},
                       {"y",     ball_.getPosition().y},
                       {"score", gameState_.score}}}
        });
        sendJson(performerSocket, {
            {"event", "tilt_feedback"},
            {"data",  {{"tiltX", gameState_.tiltX},
                       {"tiltY", gameState_.tiltY},
                       {"ballX", ball_.getPosition().x},
                       {"ballY", ball_.getPosition().y}}}
        });
    }
}

