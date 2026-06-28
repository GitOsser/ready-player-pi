#include "NotePiRoleScreen.h"
#include <QLabel>
#include <QTimer>

NotePiRoleScreen::NotePiRoleScreen(const QString &role,
                                   const QString &rpiTag,
                                   int autoCloseMs,
                                   QWidget *parent)
    : BaseMenu(parent)
{
    QWidget *central = centralWidget();

    QLabel *rpiLabel = new QLabel(rpiTag, central);
    rpiLabel->setGeometry(615, 20, 120, 35);
    rpiLabel->setAlignment(Qt::AlignCenter);
    rpiLabel->setStyleSheet(
        "border: 2px solid #1a237e;"
        "color: #1a237e;"
        "font-size: 14px;"
        "background: transparent;"
    );

    QLabel *titleLabel = new QLabel("NOTE-PI", central);
    titleLabel->setGeometry(0, 160, 800, 70);
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setStyleSheet(
        "font-size: 56px;"
        "color: #1a237e;"
        "background: transparent;"
    );

    QLabel *roleLabel = new QLabel("ROLE: " + role, central);
    roleLabel->setGeometry(0, 240, 800, 70);
    roleLabel->setAlignment(Qt::AlignCenter);
    roleLabel->setStyleSheet(
        "font-size: 48px;"
        "color: #1a237e;"
        "background: transparent;"
    );

    QTimer::singleShot(autoCloseMs, this, [this]() {
        emit done();
        close();
    });
}

