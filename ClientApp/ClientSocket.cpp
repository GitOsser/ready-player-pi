#include "ClientSocket.h"
#include "../Shared/JSONCommunication.h"

#include <cstdio>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

ClientSocket::ClientSocket() = default;

ClientSocket::~ClientSocket() { disconnect(); }

bool ClientSocket::connectToServer(const char *serverIp, int port) {
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) { perror("socket"); return false; }

    sockaddr_in serverAddress{};
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port   = htons(port);
    inet_pton(AF_INET, serverIp, &serverAddress.sin_addr);

    if (connect(fd, (sockaddr *)&serverAddress, sizeof(serverAddress)) < 0) {
        perror("connect");
        close(fd);
        return false;
    }
    socketFd_ = fd;
    return true;
}

bool ClientSocket::sendCommand(const json &message) {
    return sendJson(socketFd_, message);
}

json ClientSocket::receiveEvent() {
    return receiveJson(socketFd_);
}

void ClientSocket::disconnect() {
    if (socketFd_ >= 0) {
        close(socketFd_);
        socketFd_ = -1;
    }
}

int  ClientSocket::getSocketFd() const { return socketFd_;      }
bool ClientSocket::isConnected() const { return socketFd_ >= 0; }

