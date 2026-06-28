#include "MainWindow.h"
#include "UsernameMenu.h"
#include <QLabel>
#include <QPushButton>

MainWindow::MainWindow(const std::string &username, QWidget *parent)
    : BaseMenu(parent), currentUsername_(username)
{
    QWidget *central = centralWidget();

    usernameBtn_ = new QPushButton(QString::fromStdString(username), central);
    usernameBtn_->setGeometry(30, 20, 110, 35);
    usernameBtn_->setStyleSheet(BaseMenu::btnStyle());
    connect(usernameBtn_, &QPushButton::clicked, this, [this]() {
        UsernameMenu *menu = new UsernameMenu();
        menu->setAttribute(Qt::WA_DeleteOnClose);
        connect(menu, &UsernameMenu::usernameConfirmed, this,
                [this](std::string username) {
                    currentUsername_ = username;
                    usernameBtn_->setText(QString::fromStdString(username));
                });
        menu->show();
    });

    QLabel *title = new QLabel("ReadyPlayerPi", central);
    title->setGeometry(0, 150, 800, 60);
    title->setAlignment(Qt::AlignCenter);
    title->setStyleSheet(
        "font-size: 32px; font-weight: bold;"
        "color: #1a237e; background: transparent;");

    int btnWidth = 140, btnHeight = 50, spacing = 30, startX = 245;

    QPushButton *hostBtn = new QPushButton("Host Game", central);
    hostBtn->setGeometry(startX, 260, btnWidth, btnHeight);
    hostBtn->setStyleSheet(BaseMenu::btnStyle());
    connect(hostBtn, &QPushButton::clicked, this, [this]() { emit hostRequested(); });

    QPushButton *joinBtn = new QPushButton("Join Game", central);
    joinBtn->setGeometry(startX + btnWidth + spacing, 260, btnWidth, btnHeight);
    joinBtn->setStyleSheet(BaseMenu::btnStyle());
    connect(joinBtn, &QPushButton::clicked, this, [this]() { emit joinRequested(); });
}
