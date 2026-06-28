#ifndef GUESTGAMEROOM_H
#define GUESTGAMEROOM_H

#include "../base/BaseMenu.h"
#include <QLabel>
#include <string>
#include <vector>

class GuestGameRoom : public BaseMenu
{
    Q_OBJECT

public:

    GuestGameRoom(const std::string &roomCode, QWidget *parent = nullptr);

public slots:

    void updatePlayers(const std::vector<std::string> &players);

private:
    QLabel *playersLabel_;
};

#endif
