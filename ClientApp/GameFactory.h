#pragma once
#include <memory>
#include <string>
#include <iostream>

#include "GameSession.h"
#include "GyroPi/GyroPiSession.h"
#include "NotePi/NotePiSession.h"

inline std::unique_ptr<GameSession> createGameSession(const std::string &gameName) {
    if (gameName == "GYROPI") return std::make_unique<GyroPiSession>();
    if (gameName == "NOTEPI") return std::make_unique<NotePiSession>();

    std::cout << "[GameFactory] Unknown game '" << gameName
              << "' — defaulting to GyroPi.\n";
    return std::make_unique<GyroPiSession>();
}

