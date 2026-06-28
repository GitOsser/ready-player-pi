#include "GameSelectMenu.h"
#include <QLabel>

GameSelectMenu::GameSelectMenu(QWidget *parent)
    : BaseMenu(parent)
{
    QWidget *central = centralWidget();

    QLabel *title = new QLabel("Select Game", central);
    title->setGeometry(0, 100, 800, 50);
    title->setAlignment(Qt::AlignCenter);
    title->setStyleSheet(BaseMenu::titleStyle());

    QLabel *subtitle = new QLabel("Choose which game to play", central);
    subtitle->setGeometry(0, 160, 800, 30);
    subtitle->setAlignment(Qt::AlignCenter);
    subtitle->setStyleSheet(
        "font-size: 16px; color: #1a237e; background: transparent;");

    int btnWidth = 160, btnHeight = 80, spacing = 60;
    int startX = (800 - (2 * btnWidth + spacing)) / 2;

    QPushButton *gyroBtn = new QPushButton("GyroPi", central);
    gyroBtn->setGeometry(startX, 230, btnWidth, btnHeight);
    gyroBtn->setStyleSheet(BaseMenu::btnStyle());
    connect(gyroBtn, &QPushButton::clicked, this, [this]() {
        emit gameSelected(1);
    });

    QPushButton *noteBtn = new QPushButton("NotePi", central);
    noteBtn->setGeometry(startX + btnWidth + spacing, 230, btnWidth, btnHeight);
    noteBtn->setStyleSheet(BaseMenu::btnStyle());
    connect(noteBtn, &QPushButton::clicked, this, [this]() {
        emit gameSelected(2);
    });
}
