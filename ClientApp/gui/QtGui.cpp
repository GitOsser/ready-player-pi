#include "QtGui.h"
#include "menus/NotePiRoleScreen.h"
#include "menus/GyroPiRoleAnnouncement.h"
#include "base/BaseMenu.h"
#include <QApplication>
#include <QDialog>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>

QtGUI::QtGUI(QObject *parent) : QObject(parent) {}

void QtGUI::showStatus(const QString &msg)
{
    if (activeRoom_) { activeRoom_->showStatus(msg); return; }
    if (mainWindow_) { mainWindow_->showStatus(msg); return; }
}

std::string QtGUI::promptUsername(const std::string &prefill)
{
    QEventLoop loop;
    std::string result;

    UsernameMenu *menu = new UsernameMenu(QString::fromStdString(prefill));
    menu->setAttribute(Qt::WA_DeleteOnClose);

    connect(menu, &UsernameMenu::usernameConfirmed, &loop, [&](std::string username) {
        result = username;
        loop.quit();
    });
    connect(menu, &BaseMenu::mainWindowRequested, &loop, &QEventLoop::quit);

    menu->show();
    loop.exec();

    username_ = result;
    return result;
}

int QtGUI::promptMainMenu()
{
    QEventLoop loop;
    int choice = 0;

    if (!mainWindow_)
        mainWindow_ = new MainWindow(username_);

    activeRoom_    = nullptr;
    hostGameRoom_  = nullptr;
    guestGameRoom_ = nullptr;

    disconnect(mainWindow_, &MainWindow::hostRequested,    nullptr, nullptr);
    disconnect(mainWindow_, &MainWindow::joinRequested,    nullptr, nullptr);
    disconnect(mainWindow_, &BaseMenu::mainWindowRequested, nullptr, nullptr);

    connect(mainWindow_, &MainWindow::hostRequested, &loop, [&]() {
        choice = 1;
        loop.quit();
    });
    connect(mainWindow_, &MainWindow::joinRequested, &loop, [&]() {
        choice = 2;
        loop.quit();
    });

    connect(mainWindow_, &BaseMenu::mainWindowRequested, &loop, [&]() {
        choice = 0;
        loop.quit();
    });

    mainWindow_->show();
    loop.exec();

    return choice;
}

void QtGUI::showConnecting(const char *serverIp, int port)
{
    showStatus(QString("Connecting to %1:%2...").arg(serverIp).arg(port));
}

void QtGUI::showConnected()
{
    showStatus("Connected");
}

void QtGUI::showDisconnected()
{
    showStatus("Connection lost");
    hostGameRoom_  = nullptr;
    guestGameRoom_ = nullptr;
    if (activeRoom_) { activeRoom_->close(); activeRoom_ = nullptr; }
    if (mainWindow_) { mainWindow_->close(); mainWindow_ = nullptr; }
}

void QtGUI::showRoomCreationFailed()
{
    showStatus("Could not create room");
}

void QtGUI::showRoomCreated(const std::string &roomCode)
{
    if (mainWindow_) mainWindow_->hide();

    auto *room = new HostGameRoom(roomCode);
    room->setAttribute(Qt::WA_DeleteOnClose);
    connect(room, &QObject::destroyed, this, [this]() {
        hostGameRoom_ = nullptr;
        activeRoom_   = nullptr;
    });

    connect(room, &HostGameRoom::allPlayersConnected, this, [this]() {
        int gameChoice = promptGameChoice();
        std::string gameName = (gameChoice == 2) ? "NOTEPI" : "GYROPI";
        showStartingGame(gameName);
    });

    connect(room, &BaseMenu::mainWindowRequested, this, &QtGUI::cancelRequested);

    hostGameRoom_ = room;
    activeRoom_   = room;
    room->show();
}

void QtGUI::showWaitingForGuest()
{
    showStatus("Waiting for guest to join...");
}

int QtGUI::promptGameChoice()
{
    QEventLoop loop;
    int choice = 1;

    if (activeRoom_) { activeRoom_->close(); activeRoom_ = nullptr; }
    hostGameRoom_ = nullptr;

    auto *menu = new GameSelectMenu();
    menu->setAttribute(Qt::WA_DeleteOnClose);
    connect(menu, &QObject::destroyed, this, [this]() { activeRoom_ = nullptr; });
    activeRoom_ = menu;

    connect(menu, &GameSelectMenu::gameSelected, &loop, [&](int selected) {
        choice = selected;
        loop.quit();
    });
    connect(menu, &BaseMenu::mainWindowRequested, &loop, [&]() {
        choice = -1;
        loop.quit();
    });

    menu->show();
    loop.exec();

    if (activeRoom_) { activeRoom_->close(); activeRoom_ = nullptr; }

    return choice;
}

void QtGUI::showStartingGame(const std::string &gameName)
{
    showStatus(QString::fromStdString("Starting " + gameName + "..."));
}

std::string QtGUI::promptRoomCode()
{
    QEventLoop loop;
    std::string pin;

    if (mainWindow_) mainWindow_->hide();

    auto *room = new JoinGameRoom();
    room->setAttribute(Qt::WA_DeleteOnClose);
    connect(room, &QObject::destroyed, this, [this]() { activeRoom_ = nullptr; });
    activeRoom_ = room;
    room->show();

    connect(room, &JoinGameRoom::pinEntered, &loop, [&](std::string enteredPin) {
        pin = enteredPin;
        loop.quit();
    });
    connect(room, &BaseMenu::mainWindowRequested, &loop, &QEventLoop::quit);

    loop.exec();
    return pin;
}

void QtGUI::showJoinedRoom(const std::string &roomCode,
                           const std::vector<std::string> &players)
{
    if (activeRoom_) { activeRoom_->close(); activeRoom_ = nullptr; }

    auto *room = new GuestGameRoom(roomCode);
    room->setAttribute(Qt::WA_DeleteOnClose);
    connect(room, &QObject::destroyed, this, [this]() {
        guestGameRoom_ = nullptr;
        activeRoom_    = nullptr;
    });

    connect(room, &BaseMenu::mainWindowRequested, this, &QtGUI::cancelRequested);

    guestGameRoom_ = room;
    activeRoom_    = room;
    room->updatePlayers(players);
    room->show();
}

void QtGUI::showWaitingForHost()
{
    showStatus("Waiting for host to select game...");
}

void QtGUI::showPlayersJoined(const std::vector<std::string> &players)
{
    if (hostGameRoom_)  { hostGameRoom_->updatePlayers(players);  return; }
    if (guestGameRoom_) { guestGameRoom_->updatePlayers(players); return; }
}

void QtGUI::showError(const std::string &detail)
{
    showStatus(QString::fromStdString("Error: " + detail));
}

void QtGUI::showGameStartFailed()
{
    showStatus("Failed to start game");
}

void QtGUI::showGameStartHeader(const std::string &gameName,
                                const std::string &role,
                                const std::string &partner)
{
    showStatus(QString::fromStdString(gameName + " — " + role + " — " + partner));
}

int QtGUI::promptPlayAgain()
{
    QDialog dialog;
    dialog.setWindowTitle("Play again");
    dialog.setModal(true);
    dialog.resize(420, 180);

    QVBoxLayout *layout = new QVBoxLayout(&dialog);

    QLabel *label = new QLabel("Choose next action:", &dialog);
    layout->addWidget(label);

    QHBoxLayout *buttonRow = new QHBoxLayout();
    layout->addLayout(buttonRow);

    int choice = 0;

    QPushButton *exitButton = new QPushButton("Exit", &dialog);
    QPushButton *gyroButton = new QPushButton("GyroPi", &dialog);
    QPushButton *noteButton = new QPushButton("NotePi", &dialog);

    buttonRow->addWidget(exitButton);
    buttonRow->addWidget(gyroButton);
    buttonRow->addWidget(noteButton);

    QObject::connect(exitButton, &QPushButton::clicked, &dialog, [&]() {
        choice = 0;
        dialog.accept();
    });
    QObject::connect(gyroButton, &QPushButton::clicked, &dialog, [&]() {
        choice = 1;
        dialog.accept();
    });
    QObject::connect(noteButton, &QPushButton::clicked, &dialog, [&]() {
        choice = 2;
        dialog.accept();
    });

    dialog.exec();
    return choice;
}

void QtGUI::showNotePiRole(const std::string &role, const std::string &logTag)
{
    QString rpiTag = (logTag == "host") ? "HOST RPI" : "GUEST RPI";

    QEventLoop loop;
    NotePiRoleScreen *screen = new NotePiRoleScreen(
        QString::fromStdString(role), rpiTag);

    QObject::connect(screen, &NotePiRoleScreen::done, &loop, &QEventLoop::quit);

    screen->show();
    loop.exec();
    screen->deleteLater();

    if (activeRoom_) { activeRoom_->close(); activeRoom_ = nullptr; }
    hostGameRoom_  = nullptr;
    guestGameRoom_ = nullptr;
    if (mainWindow_) { mainWindow_->close(); mainWindow_ = nullptr; }
}

void QtGUI::showGyroPiRole(const std::string &role,
                           const std::string &partner,
                           const std::string &logTag)
{
    (void)logTag;

    QEventLoop loop;
    GyroPiRoleAnnouncement *screen =
        new GyroPiRoleAnnouncement("GYRO-PI", role, partner);

    QObject::connect(screen, &GyroPiRoleAnnouncement::done, &loop, &QEventLoop::quit);

    screen->show();
    loop.exec();
    screen->deleteLater();

    if (activeRoom_) { activeRoom_->close(); activeRoom_ = nullptr; }
    hostGameRoom_  = nullptr;
    guestGameRoom_ = nullptr;
    if (mainWindow_) { mainWindow_->close(); mainWindow_ = nullptr; }
}

