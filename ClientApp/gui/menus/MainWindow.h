#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "../base/BaseMenu.h"
#include <string>

class MainWindow : public BaseMenu
{
    Q_OBJECT

public:
    MainWindow(const std::string &username, QWidget *parent = nullptr);

signals:
    void hostRequested();
    void joinRequested();

private:
    std::string currentUsername_;
    QPushButton *usernameBtn_;
};

#endif
