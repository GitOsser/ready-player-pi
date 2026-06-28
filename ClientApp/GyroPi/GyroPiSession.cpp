#include "GyroPiSession.h"

#include "GyroscopeReader.h"

#include "../gui/menus/GyroPiPerformerWindow.h"
#include "../gui/menus/GyroPiCommunicatorWindow.h"
#include "../gui/menus/ScoreboardScreen.h"

#include "../../Shared/JSONCommunication.h"
#include "../../Shared/Protocol.h"

#include <QApplication>
#include <QEventLoop>
#include <QMetaObject>
#include <QString>
#include <QVector>

#include <atomic>
#include <chrono>
#include <iostream>
#include <thread>

#include <nlohmann/json.hpp>

using json = nlohmann::json;

static constexpr int GYROPI_TICK_MS = 33;

namespace {
    struct TiltCommand { float tiltAxisX, tiltAxisY; int durationMs; };

    const TiltCommand kHardcodedTiltPath[] = {
        { 1.5f,  2.5f, 1500 },
        { 2.5f,  0.5f, 1500 },
        { 2.0f, -1.0f, 1000 },
        { 0.0f,  0.0f,    0 },
    };
}

static bool showGyroScoreboard(const json        &gameOverData,
                                const std::string &logTag,
                                int                socketFd,
                                const std::string &gameName)
{
    int yourScore = gameOverData.value(Protocol::SCORE, 0);
    int yourRank  = gameOverData.value(Protocol::RANK, 0);

    QVector<ScoreboardEntry> top5;
    if (gameOverData.contains(Protocol::TOP5)) {
        for (auto &entry : gameOverData[Protocol::TOP5]) {
            ScoreboardEntry e;
            e.names = QString::fromStdString(entry.value(Protocol::NAMES, ""));
            e.score = entry.value(Protocol::SCORE, 0);
            top5.push_back(e);
        }
    }

    const bool isHost = (logTag == "host");

    QEventLoop loop;
    bool playAgainResult = false;

    ScoreboardScreen *screen = new ScoreboardScreen(
        yourScore, yourRank, top5, "s", isHost);

    if (isHost) {

        QObject::connect(screen, &ScoreboardScreen::playAgain, &loop, [&]() {
            playAgainResult = true;
            sendJson(socketFd, {{"command", "start_game"},
                                {"data",    {{"game", gameName}}}});
            loop.quit();
        });
    }

    QObject::connect(screen, &ScoreboardScreen::back,
                     &loop, &QEventLoop::quit);
    QObject::connect(screen, &BaseMenu::mainWindowRequested,
                     &loop, &QEventLoop::quit);
    screen->show();

    std::atomic<bool> peekRunning{!isHost};
    std::thread peekThread;
    if (!isHost) {
        peekThread = std::thread([&]() {
            while (peekRunning) {
                fd_set readSet;
                FD_ZERO(&readSet);
                FD_SET(socketFd, &readSet);
                struct timeval tv{0, 100000};
                if (select(socketFd + 1, &readSet, nullptr, nullptr, &tv) > 0
                    && peekRunning)
                {
                    playAgainResult = true;
                    QMetaObject::invokeMethod(&loop, "quit", Qt::QueuedConnection);
                    break;
                }
            }
        });
    }

    loop.exec();
    peekRunning = false;
    if (peekThread.joinable()) peekThread.join();
    screen->deleteLater();
    return playAgainResult;
}

static GyroCourseInfo buildCourseInfo(const json &courseData)
{
    GyroCourseInfo info;

    if (courseData.contains("start")) {
        info.startX = courseData["start"].value("x", 0.0f);
        info.startY = courseData["start"].value("y", 0.0f);
    }
    if (courseData.contains("goal")) {
        info.goalX = courseData["goal"].value("x", 0.0f);
        info.goalY = courseData["goal"].value("y", 0.0f);
    }
    if (courseData.contains("holes")) {
        for (auto &hole : courseData["holes"]) {
            GyroHoleInfo h;
            h.x      = hole.value("x", 0.0f);
            h.y      = hole.value("y", 0.0f);
            h.radius = hole.value("r", 0.0f);
            info.holes.push_back(h);
        }
    }
    return info;
}

bool GyroPiSession::playAsPerformer(int socketFileDescriptor,
                                    const std::string &logTag,
                                    const std::string &gameName)
{
    std::cout << "[" << logTag << "] Role: PERFORMER\n";

    GyroPiPerformerWindow *window = new GyroPiPerformerWindow();
    window->show();

    QEventLoop loop;
    std::atomic<bool> isRunning{true};
    json gameOverData;
    bool gameOverReceived = false;

    auto sendExitAndQuit = [&]() {
        isRunning = false;
        sendJson(socketFileDescriptor, {{"command", "exit_game"}});
        QMetaObject::invokeMethod(&loop, "quit", Qt::QueuedConnection);
    };
    QObject::connect(window, &GyroPiPerformerWindow::exitRequested,
                     &loop, sendExitAndQuit);
    QObject::connect(window, &BaseMenu::mainWindowRequested,
                     &loop, sendExitAndQuit);

    std::thread receiver([&]() {
        try {
            while (isRunning) {
                json message = receiveJson(socketFileDescriptor);
                if (message.empty()) { isRunning = false; break; }

                std::string event = message.value(Protocol::EVENT, "");

                if (event == "tilt_feedback") {

                } else if (event == "blackhole_hit") {
                    QMetaObject::invokeMethod(window, "onBlackHoleHit",
                                              Qt::QueuedConnection);

                } else if (event == Protocol::GAME_OVER) {
                    if (message.contains(Protocol::DATA))
                        gameOverData = message[Protocol::DATA];
                    gameOverReceived = true;
                    isRunning        = false;
                    QMetaObject::invokeMethod(&loop, "quit", Qt::QueuedConnection);
                    break;

                } else if (event == "game_exit") {
                    isRunning = false;
                    QMetaObject::invokeMethod(&loop, "quit", Qt::QueuedConnection);
                    break;

                }
            }
        } catch (const std::exception &e) {
            std::cerr << "[" << logTag << "] Performer receiver exception: " << e.what() << "\n";
        }
        QMetaObject::invokeMethod(&loop, "quit", Qt::QueuedConnection);
    });

    std::thread sender([&]() {
#ifdef USE_HARDWARE_SENSOR
        std::cout << "[" << logTag << "] Using GyroscopeReader (live IMU)\n";
        GyroscopeReader gyroscopeReader;
        while (isRunning) {
            TiltReading tilt = gyroscopeReader.read();
            sendJson(socketFileDescriptor, {
                {"command", "tilt"},
                {"data",    {{"tiltX", tilt.tiltX},
                             {"tiltY", tilt.tiltY}}}
            });
            QMetaObject::invokeMethod(window, "setTilt",
                                      Qt::QueuedConnection,
                                      Q_ARG(float, tilt.tiltX),
                                      Q_ARG(float, tilt.tiltY));
            std::this_thread::sleep_for(std::chrono::milliseconds(GYROPI_TICK_MS));
        }
#else
        std::cout << "[" << logTag << "] Using hardcoded tilt path (no hardware)\n";
        std::this_thread::sleep_for(std::chrono::seconds(1));

        for (int stepIdx = 0;
             kHardcodedTiltPath[stepIdx].durationMs > 0 && isRunning;
             stepIdx++)
        {
            const TiltCommand &step = kHardcodedTiltPath[stepIdx];
            int tickCount = step.durationMs / GYROPI_TICK_MS;

            for (int t = 0; t < tickCount && isRunning; t++) {
                sendJson(socketFileDescriptor, {
                    {"command", "tilt"},
                    {"data",    {{"tiltX", step.tiltAxisX},
                                 {"tiltY", step.tiltAxisY}}}
                });
                QMetaObject::invokeMethod(window, "setTilt",
                                          Qt::QueuedConnection,
                                          Q_ARG(float, step.tiltAxisX),
                                          Q_ARG(float, step.tiltAxisY));
                std::this_thread::sleep_for(std::chrono::milliseconds(GYROPI_TICK_MS));
            }
        }

        if (isRunning) {
            sendJson(socketFileDescriptor, {{"command","tilt"},
                                            {"data",   {{"tiltX", 0.0f},
                                                        {"tiltY", 0.0f}}}});
            QMetaObject::invokeMethod(window, "setTilt",
                                      Qt::QueuedConnection,
                                      Q_ARG(float, 0.0f),
                                      Q_ARG(float, 0.0f));
        }
#endif
    });

    loop.exec();
    isRunning = false;

    sender.join();
    receiver.join();

    window->close();
    window->deleteLater();

    if (gameOverReceived)
        return showGyroScoreboard(gameOverData, logTag,
                                   socketFileDescriptor, gameName);
    return false;
}

bool GyroPiSession::playAsCommunicator(int socketFileDescriptor,
                                       const std::string &logTag,
                                       const std::string &gameName)
{
    std::cout << "[" << logTag << "] Role: COMMUNICATOR\n";

    GyroPiCommunicatorWindow *window = new GyroPiCommunicatorWindow();
    window->show();

    QEventLoop loop;
    std::atomic<bool> isRunning{true};
    json gameOverData;
    bool gameOverReceived = false;

    auto sendExitAndQuit = [&]() {
        isRunning = false;
        sendJson(socketFileDescriptor, {{"command", "exit_game"}});
        QMetaObject::invokeMethod(&loop, "quit", Qt::QueuedConnection);
    };
    QObject::connect(window, &GyroPiCommunicatorWindow::exitRequested,
                     &loop, sendExitAndQuit);
    QObject::connect(window, &BaseMenu::mainWindowRequested,
                     &loop, sendExitAndQuit);

    std::thread receiver([&]() {
        try {
            while (isRunning) {
                json message = receiveJson(socketFileDescriptor);
                if (message.empty()) { isRunning = false; break; }

                std::string event = message.value(Protocol::EVENT, "");

                if (event == "course_info") {
                    if (message.contains(Protocol::DATA)) {
                        GyroCourseInfo info = buildCourseInfo(message[Protocol::DATA]);
                        QMetaObject::invokeMethod(window, "onCourseLoaded",
                                                  Qt::QueuedConnection,
                                                  Q_ARG(GyroCourseInfo, info));
                    }

                } else if (event == "ball_update") {
                    if (message.contains(Protocol::DATA)) {
                        const json &d = message[Protocol::DATA];
                        float x     = d.value("x",     0.0f);
                        float y     = d.value("y",     0.0f);
                        int   score = d.value("score", 0);
                        QMetaObject::invokeMethod(window, "onBallUpdate",
                                                  Qt::QueuedConnection,
                                                  Q_ARG(float, x),
                                                  Q_ARG(float, y));
                        QMetaObject::invokeMethod(window, "onScoreUpdate",
                                                  Qt::QueuedConnection,
                                                  Q_ARG(int, score));
                    }

                } else if (event == "blackhole_hit") {
                    QMetaObject::invokeMethod(window, "onBlackHoleHit",
                                              Qt::QueuedConnection);

                } else if (event == Protocol::GAME_OVER) {
                    if (message.contains(Protocol::DATA))
                        gameOverData = message[Protocol::DATA];
                    gameOverReceived = true;
                    isRunning        = false;
                    QMetaObject::invokeMethod(&loop, "quit", Qt::QueuedConnection);
                    break;

                } else if (event == "game_exit") {
                    isRunning = false;
                    QMetaObject::invokeMethod(&loop, "quit", Qt::QueuedConnection);
                    break;

                } else {
                    std::cout << "[" << logTag << "] Unknown event: " << event << "\n";
                }
            }
        } catch (const std::exception &e) {
            std::cerr << "[" << logTag << "] Communicator receiver exception: " << e.what() << "\n";
        }
        QMetaObject::invokeMethod(&loop, "quit", Qt::QueuedConnection);
    });

    loop.exec();
    isRunning = false;
    receiver.join();

    window->close();
    window->deleteLater();

    if (gameOverReceived)
        return showGyroScoreboard(gameOverData, logTag,
                                   socketFileDescriptor, gameName);
    return false;
}

