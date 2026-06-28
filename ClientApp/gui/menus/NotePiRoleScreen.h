#ifndef NOTEPIROLESCREEN_H
#define NOTEPIROLESCREEN_H

#include "../base/BaseMenu.h"
#include <QString>

class NotePiRoleScreen : public BaseMenu
{
    Q_OBJECT
public:
    explicit NotePiRoleScreen(const QString &role,
                              const QString &rpiTag,
                              int autoCloseMs = 3000,
                              QWidget *parent = nullptr);

signals:
    void done();
};

#endif

