#pragma once
#include <QObject>
#include <QEventLoop>
#include <QString>
#include <string>
#include <vector>

#include "menus/MainWindow.h"
#include "menus/UsernameMenu.h"
#include "menus/HostGameRoom.h"
#include "menus/JoinGameRoom.h"
#include "menus/GuestGameRoom.h"
#include "menus/GameSelectMenu.h"

class QtGUI : public QObject
{
    Q_OBJECT

public:
    explicit QtGUI(QObject *parent = nullptr);

    std::string promptUsername(const std::string &prefill = "");
    int         promptMainMenu();

    void showConnecting(const char *serverIp, int port);
    void showConnected();
    void showDisconnected();

    void showRoomCreationFailed();
    void showRoomCreated(const std::string &roomCode);
    void showWaitingForGuest();
    int  promptGameChoice();
    void showStartingGame(const std::string &gameName);

    std::string promptRoomCode();
    void showJoinedRoom(const std::string &roomCode,
                        const std::vector<std::string> &players);
    void showWaitingForHost();

    void showPlayersJoined(const std::vector<std::string> &players);
    void showError(const std::string &detail);
    void showGameStartFailed();
    void showGameStartHeader(const std::string &gameName,
                             const std::string &role,
                             const std::string &partner);
    int  promptPlayAgain();

    void showNotePiRole(const std::string &role, const std::string &logTag);

    void showGyroPiRole(const std::string &role,
                        const std::string &partner,
                        const std::string &logTag);

signals:
    void cancelRequested();

private:
    std::string    username_;
    MainWindow    *mainWindow_    = nullptr;
    BaseMenu      *activeRoom_    = nullptr;
    HostGameRoom  *hostGameRoom_  = nullptr;
    GuestGameRoom *guestGameRoom_ = nullptr;

    void showStatus(const QString &msg);
};

