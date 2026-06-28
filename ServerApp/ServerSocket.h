#pragma once
#include <string>
#include <map>
#include <algorithm>

#include <unistd.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <nlohmann/json.hpp>

#include "../Shared/JSONCommunication.h"

using json = nlohmann::json;

class ServerSocket {
public:
    ServerSocket() = default;
    ~ServerSocket() { closeListenSocket(); }

    ServerSocket(const ServerSocket &)            = delete;
    ServerSocket &operator=(const ServerSocket &) = delete;

    bool startListening(int port, int backlog = 10);

    bool waitForActivity(const std::map<int, std::string> &hostPendingMap,
                         bool                              &listenReadable,
                         std::map<int, std::string>        &hostReadable);

    int acceptClient();

    bool sendEvent(int clientFd, const json &message);

    json receiveCommand(int clientFd);

    void closeClient(int clientFd);

    void closeListenSocket();

    int getListenSocket() const;

private:
    int listenSocket_ = -1;
};

