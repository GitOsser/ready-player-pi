#pragma once
#include <string>

struct Room {
    std::string code;
    int         hostSocket  = -1;
    int         guestSocket = -1;
    std::string hostName;
    std::string guestName;
};

