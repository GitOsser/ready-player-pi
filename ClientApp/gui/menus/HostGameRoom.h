#ifndef HOSTGAMEROOM_H
#define HOSTGAMEROOM_H

#include "../base/BaseMenu.h"
#include <QLabel>
#include <QPushButton>
#include <string>
#include <vector>

class HostGameRoom : public BaseMenu
{
    Q_OBJECT

public:
    HostGameRoom(const std::string &pin, QWidget *parent = nullptr);

public slots:

    void updatePlayers(const std::vector<std::string> &players);

signals:

    void allPlayersConnected();

private:
    QLabel      *playersLabel_;
    QPushButton *startBtn_;
};

#endif
