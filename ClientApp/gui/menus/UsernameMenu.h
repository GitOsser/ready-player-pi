#ifndef USERNAMEMENU_H
#define USERNAMEMENU_H

#include "../base/BaseMenu.h"
#include <QLineEdit>
#include <QLabel>
#include <string>

class UsernameMenu : public BaseMenu
{
    Q_OBJECT

public:
    explicit UsernameMenu(const QString &prefill = "", QWidget *parent = nullptr);

signals:
    void usernameConfirmed(std::string username);

public slots:
    void onUsernameValidated(bool approved);

private slots:
    void onConfirm();

private:
    QLineEdit *usernameInput_;
    QLabel *errorLabel_;
};

#endif
