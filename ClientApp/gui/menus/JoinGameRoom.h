#ifndef JOINGAMEROOM_H
#define JOINGAMEROOM_H

#include "../base/BaseMenu.h"
#include <QLineEdit>
#include <string>

class JoinGameRoom : public BaseMenu
{
    Q_OBJECT

public:
    JoinGameRoom(QWidget *parent = nullptr);

signals:
    void pinEntered(std::string pin);

private slots:
    void onPinChanged(const QString &text);

private:
    QLineEdit *pinInput_;
};

#endif
