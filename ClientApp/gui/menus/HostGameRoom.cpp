#include "HostGameRoom.h"
#include <QLabel>

HostGameRoom::HostGameRoom(const std::string &pin, QWidget *parent)
    : BaseMenu(parent)
{
    QWidget *central = centralWidget();

    QLabel *title = new QLabel("Game Room", central);
    title->setGeometry(0, 30, 800, 45);
    title->setAlignment(Qt::AlignCenter);
    title->setStyleSheet(BaseMenu::titleStyle());

    QLabel *pinLabel = new QLabel("Room PIN", central);
    pinLabel->setGeometry(0, 85, 800, 25);
    pinLabel->setAlignment(Qt::AlignCenter);
    pinLabel->setStyleSheet(
        "font-size: 15px; color: #1a237e; background: transparent;");

    QLabel *pinDisplay = new QLabel(QString::fromStdString(pin), central);
    pinDisplay->setGeometry(250, 115, 300, 75);
    pinDisplay->setAlignment(Qt::AlignCenter);
    pinDisplay->setStyleSheet(
        "font-size: 44px;"
        "font-weight: bold;"
        "color: #1a237e;"
        "background: white;"
        "border: 2px solid #1a237e;"
        "border-radius: 12px;"
        "letter-spacing: 10px;"
        );

    QLabel *playersTitle = new QLabel("Players", central);
    playersTitle->setGeometry(0, 210, 800, 28);
    playersTitle->setAlignment(Qt::AlignCenter);
    playersTitle->setStyleSheet(
        "font-size: 16px; font-weight: bold; color: #1a237e; background: transparent;");

    playersLabel_ = new QLabel("No other players yet", central);
    playersLabel_->setGeometry(150, 245, 500, 130);
    playersLabel_->setAlignment(Qt::AlignTop | Qt::AlignHCenter);
    playersLabel_->setWordWrap(true);
    playersLabel_->setStyleSheet(
        "font-size: 17px;"
        "color: #1a237e;"
        "background: white;"
        "border: 2px solid #1a237e;"
        "border-radius: 10px;"
        "padding: 10px;"
        );

    startBtn_ = new QPushButton("All Players Connected", central);
    startBtn_->setGeometry(275, 400, 250, 45);
    startBtn_->setStyleSheet(BaseMenu::btnStyle());
    connect(startBtn_, &QPushButton::clicked,
            this, &HostGameRoom::allPlayersConnected);
}

void HostGameRoom::updatePlayers(const std::vector<std::string> &players)
{
    if (players.empty()) {
        playersLabel_->setText("No other players yet");
        return;
    }

    QString text;
    for (const auto &p : players)
        text += QString::fromStdString(p) + "\n";
    playersLabel_->setText(text.trimmed());
}
