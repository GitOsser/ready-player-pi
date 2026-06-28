#include "NotePiSession.h"

#include "boundary/MicrophoneDriver.h"
#include "control/SoundAnalyzer.h"
#include "domain/NotePiConfig.h"
#include "domain/NoteSequence.h"

#include "../gui/menus/NotePiPerformerWidget.h"
#include "../gui/menus/NotePiCommunicatorWidget.h"
#include "../gui/menus/ScoreboardScreen.h"

#include "../../Shared/JSONCommunication.h"
#include "../../Shared/NotePiProtocol.h"

#include <QApplication>
#include <QEventLoop>
#include <QMetaObject>
#include <QString>
#include <QVector>

#include <atomic>
#include <iostream>
#include <string>
#include <thread>
#include <vector>
#include <sys/select.h>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

static bool showNotePiScoreboard(const json        &gameOverData,
                                  const std::string &logTag,
                                  int                socketFd,
                                  const std::string &gameName)
{
    int yourScore = gameOverData.value(Protocol::SCORE, 0);
    int yourRank  = gameOverData.value(Protocol::RANK,  0);

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
        yourScore, yourRank, top5, "", isHost);

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

bool NotePiSession::playAsPerformer(int socketFileDescriptor,
                                    const std::string &logTag,
                                    const std::string &gameName)
{
    namespace cfg = notepi::config;

    performerWidget_ = new NotePiPerformerWidget();
    performerWidget_->show();

    {
        QEventLoop loop;
        QObject::connect(performerWidget_, &NotePiPerformerWidget::recordingStarted,
                         &loop, &QEventLoop::quit);
        loop.exec();
    }
    QMetaObject::invokeMethod(performerWidget_, "showRecording",
                              Qt::QueuedConnection);

    const char *envDevice = std::getenv("RPP_MIC_DEVICE");
    std::string micDevice = (envDevice && *envDevice) ? envDevice : cfg::MIC_DEVICE;
    std::cout << "[" << logTag << "] Mic device: " << micDevice << std::endl;

    MicrophoneDriver mic(micDevice,
                         static_cast<unsigned int>(cfg::SAMPLE_RATE_HZ),
                         static_cast<std::size_t>(cfg::BLOCK_SIZE));
    if (!mic.start()) {
        std::cerr << "[" << logTag << "] Failed to start microphone.\n";
        performerWidget_->deleteLater();
        return false;
    }

    SoundAnalyzer analyzer;
    NoteSequence  sequence;

    bool gameRunning = true;
    bool scoreboardResult = false;

    while (gameRunning) {

        QEventLoop recordingLoop;

        std::thread micThread([&]() {
            std::size_t lastReportedCount = 0;
            while (sequence.getNotes().size()
                   < static_cast<std::size_t>(cfg::SEQUENCE_LENGTH)) {
                std::vector<float> audioBlock;
                mic.readBlock(audioBlock);
                analyzer.analyze(audioBlock, sequence);

                if (sequence.getNotes().size() != lastReportedCount) {
                    lastReportedCount = sequence.getNotes().size();
                    QString name = QString::fromStdString(sequence.getNotes().back());
                    int slotIdx = static_cast<int>(lastReportedCount) - 1;
                    QMetaObject::invokeMethod(performerWidget_, "onNoteDetected",
                                              Qt::QueuedConnection,
                                              Q_ARG(QString, name),
                                              Q_ARG(int, slotIdx));
                }
            }

            QMetaObject::invokeMethod(&recordingLoop, "quit",
                                      Qt::QueuedConnection);
        });

        recordingLoop.exec();
        micThread.join();

        sendJson(socketFileDescriptor, {
            {Protocol::COMMAND, Protocol::NotePi::NOTE_SEQUENCE},
            {Protocol::DATA, {{Protocol::NotePi::NOTES, sequence.getNotes()}}}
        });

        analyzer.reset();
        sequence.reset();

        json response = receiveJson(socketFileDescriptor);
        if (response.empty()) {
            std::cout << "[" << logTag << "] Server disconnected.\n";
            break;
        }

        std::string event = response.value(Protocol::EVENT, "");

        if (event == Protocol::NotePi::NOTE_RESULT) {
            if (!response.contains(Protocol::DATA)) continue;
            const json &d = response[Protocol::DATA];
            bool solved  = d.value(Protocol::NotePi::SOLVED, false);

            if (solved) {
                gameRunning = false;
            } else {
                QMetaObject::invokeMethod(performerWidget_, "showIdle",
                                          Qt::QueuedConnection);
                QEventLoop loop;
                QObject::connect(performerWidget_, &NotePiPerformerWidget::recordingStarted,
                                 &loop, &QEventLoop::quit);
                loop.exec();
                QMetaObject::invokeMethod(performerWidget_, "showRecording",
                                          Qt::QueuedConnection);
            }

        } else {
            std::cout << "[" << logTag << "] Unknown event: " << event << "\n";
        }
    }

    if (performerWidget_) {
        json gameOver = receiveJson(socketFileDescriptor);
        mic.stop();
        performerWidget_->close();
        performerWidget_->deleteLater();
        performerWidget_ = nullptr;

        if (!gameOver.empty() && gameOver.value(Protocol::EVENT, "") == Protocol::GAME_OVER
            && gameOver.contains(Protocol::DATA))
        {
            scoreboardResult = showNotePiScoreboard(
                gameOver[Protocol::DATA], logTag, socketFileDescriptor, gameName);
        }
    }
    return scoreboardResult;
}

bool NotePiSession::playAsCommunicator(int socketFileDescriptor,
                                       const std::string &logTag,
                                       const std::string &gameName)
{
    communicatorWidget_ = new NotePiCommunicatorWidget();
    communicatorWidget_->show();

    QEventLoop loop;
    json gameOverData;
    bool gameOverReceived = false;

    std::thread receiver([&]() {
        try {
            while (true) {
                json message = receiveJson(socketFileDescriptor);
                if (message.empty()) {
                    std::cout << "[" << logTag << "] Server disconnected.\n";
                    QMetaObject::invokeMethod(&loop, "quit", Qt::QueuedConnection);
                    return;
                }

                std::string event = message.value(Protocol::EVENT, "");

                if (event == Protocol::NotePi::NOTE_RESULT) {
                    if (!message.contains(Protocol::DATA)) continue;
                    const json &d = message[Protocol::DATA];
                    int attempt = d.value(Protocol::NotePi::ATTEMPT, 0);

                    QVector<QString> feedback;
                    if (d.contains(Protocol::NotePi::FEEDBACK)) {
                        for (auto &fb : d[Protocol::NotePi::FEEDBACK])
                            feedback.push_back(QString::fromStdString(fb.get<std::string>()));
                    }

                    QVector<QString> notes;
                    if (d.contains(Protocol::NotePi::NOTES)) {
                        for (auto &n : d[Protocol::NotePi::NOTES])
                            notes.push_back(QString::fromStdString(n.get<std::string>()));
                    }

                    QMetaObject::invokeMethod(communicatorWidget_, "onAttempt",
                                              Qt::QueuedConnection,
                                              Q_ARG(QVector<QString>, feedback),
                                              Q_ARG(QVector<QString>, notes),
                                              Q_ARG(int, attempt));

                } else if (event == Protocol::GAME_OVER) {
                    if (message.contains(Protocol::DATA))
                        gameOverData = message[Protocol::DATA];
                    gameOverReceived = true;
                    QMetaObject::invokeMethod(&loop, "quit", Qt::QueuedConnection);
                    return;

                } else {
                    std::cout << "[" << logTag << "] Unknown event: " << event << "\n";
                }
            }
        } catch (const std::exception &e) {
            std::cerr << "[" << logTag << "] Communicator receiver exception: " << e.what() << "\n";
            QMetaObject::invokeMethod(&loop, "quit", Qt::QueuedConnection);
        }
    });

    loop.exec();
    receiver.join();

    communicatorWidget_->close();
    communicatorWidget_->deleteLater();
    communicatorWidget_ = nullptr;

    if (gameOverReceived)
        return showNotePiScoreboard(gameOverData, logTag, socketFileDescriptor, gameName);
    return false;
}

