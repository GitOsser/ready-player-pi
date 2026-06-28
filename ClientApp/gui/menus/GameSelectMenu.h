#ifndef GAMESELECTMENU_H
#define GAMESELECTMENU_H

#include "../base/BaseMenu.h"
#include <QPushButton>

class GameSelectMenu : public BaseMenu
{
    Q_OBJECT

public:
    explicit GameSelectMenu(QWidget *parent = nullptr);

signals:
    void gameSelected(int choice);
};

#endif
