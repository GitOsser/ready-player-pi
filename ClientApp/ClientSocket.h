#pragma once
#include <string>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

class ClientSocket {
public:
    ClientSocket();
    ~ClientSocket();

    ClientSocket(const ClientSocket &)            = delete;
    ClientSocket &operator=(const ClientSocket &) = delete;

    bool connectToServer(const char *serverIp, int port);

    bool sendCommand(const json &message);

    json receiveEvent();

    void disconnect();

    int  getSocketFd() const;
    bool isConnected() const;

private:
    int socketFd_ = -1;
};

