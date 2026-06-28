#pragma once
#include <iostream>
#include <string>
#include <nlohmann/json.hpp>
#include "SecretSequence.h"

#include "../../Shared/JSONCommunication.h"
#include "../../Shared/NotePiProtocol.h"
#include "NotePi.h"
#include "../Scoreboard.h"
#include "../Game.h"

using json = nlohmann::json;

class NotePiGame : public Game {
public:
    void run(int performerSocket, int communicatorSocket,
             const std::string &playerNames) override;
};

inline void NotePiGame::run(int performerSocket, int communicatorSocket,
                             const std::string &playerNames)
{
    SecretSequence secret_sequence;
    int attempt = 0;
    bool solved = false;

    while (!solved) {
        json cmd = receiveJson(performerSocket);
        if (cmd.empty()) break;

        NoteSequence guess;
        for (auto &n : cmd[Protocol::DATA][Protocol::NotePi::NOTES])
            guess.notes.push_back(n.get<std::string>());

        attempt++;
        solved = secret_sequence.isSolved(guess);

        auto feedback = secret_sequence.evaluate(guess);
        json feedbackArray = json::array();
        for (auto &f : feedback) {
            if (f == NoteFeedback::CORRECT) feedbackArray.push_back(Protocol::NotePi::GREEN);
            else if (f == NoteFeedback::WRONG_PITCH) feedbackArray.push_back(Protocol::NotePi::RED);
            else feedbackArray.push_back(Protocol::NotePi::YELLOW);
        }

        json result = {
            {Protocol::EVENT, Protocol::NotePi::NOTE_RESULT},
            {Protocol::DATA, {
                {Protocol::NotePi::FEEDBACK, feedbackArray},
                {Protocol::NotePi::NOTES, guess.notes},
                {Protocol::NotePi::ATTEMPT, attempt},
                {Protocol::NotePi::SOLVED, solved},
            }}
        };
        sendJson(performerSocket, result);
        sendJson(communicatorSocket, result);
    }

    int score = solved ? attempt : 0;

    if (solved)
        writeScore("scoreboard_notepi.txt", playerNames, score);

    json scoreboard = readScoreboard("scoreboard_notepi.txt");

    int rank = -1;
    if (solved) {
        for (int i = 0; i < (int)scoreboard.size(); i++) {
            if (scoreboard[i]["score"] == score) { rank = i + 1; break; }
        }
    }

    json topFiveScores = json::array();
    for (int i = 0; i < std::min(5, (int)scoreboard.size()); i++)
        topFiveScores.push_back(scoreboard[i]);

    json gameOver = {
        {Protocol::EVENT, Protocol::GAME_OVER},
        {Protocol::DATA, {
            {Protocol::SCORE, score},
            {Protocol::RANK,  rank},
            {Protocol::TOP5,  topFiveScores}
        }}
    };
    sendJson(performerSocket,    gameOver);
    sendJson(communicatorSocket, gameOver);
}

