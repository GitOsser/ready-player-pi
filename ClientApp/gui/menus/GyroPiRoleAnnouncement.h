#pragma once

#include "../base/BaseMenu.h"
#include <QLabel>
#include <QString>
#include <string>

class GyroPiRoleAnnouncement : public BaseMenu
{
    Q_OBJECT

public:

    GyroPiRoleAnnouncement(const std::string &gameName,
                           const std::string &role,
                           const std::string &partner,
                           int autoCloseMs = 3000,
                           QWidget *parent = nullptr);

signals:
    void done();

private:
    QLabel *gameLabel_    = nullptr;
    QLabel *roleLabel_    = nullptr;
    QLabel *partnerLabel_ = nullptr;
};

