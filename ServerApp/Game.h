#pragma once
#include <string>

class Game {
public:
    virtual void run(int         performerSocket,
                     int         communicatorSocket,
                     const std::string &playerNames) = 0;
    virtual ~Game() = default;
};

