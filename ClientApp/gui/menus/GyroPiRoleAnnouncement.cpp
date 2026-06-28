#include "GyroPiRoleAnnouncement.h"
#include <QString>
#include <QTimer>

GyroPiRoleAnnouncement::GyroPiRoleAnnouncement(const std::string &gameName,
                                               const std::string &role,
                                               const std::string &partner,
                                               int autoCloseMs,
                                               QWidget *parent)
    : BaseMenu(parent)
{
    QTimer::singleShot(autoCloseMs, this, [this]() {
        emit done();
        close();
    });
    QWidget *central = centralWidget();

    gameLabel_ = new QLabel(QString::fromStdString(gameName), central);
    gameLabel_->setGeometry(0, 130, 800, 70);
    gameLabel_->setAlignment(Qt::AlignCenter);
    gameLabel_->setStyleSheet(
        "font-size: 56px;"
        "font-weight: bold;"
        "letter-spacing: 6px;"
        "color: #1a237e;"
        "background: transparent;"
    );

    QString roleText = "ROLE: " + QString::fromStdString(role);
    roleLabel_ = new QLabel(roleText, central);
    roleLabel_->setGeometry(0, 220, 800, 60);
    roleLabel_->setAlignment(Qt::AlignCenter);
    roleLabel_->setStyleSheet(
        "font-size: 36px;"
        "font-weight: bold;"
        "letter-spacing: 3px;"
        "color: #1a237e;"
        "background: transparent;"
    );

    QString partnerText = "Partner: " + QString::fromStdString(partner);
    partnerLabel_ = new QLabel(partnerText, central);
    partnerLabel_->setGeometry(0, 310, 800, 35);
    partnerLabel_->setAlignment(Qt::AlignCenter);
    partnerLabel_->setStyleSheet(
        "font-size: 20px;"
        "color: #1a237e;"
        "background: transparent;"
    );
}

