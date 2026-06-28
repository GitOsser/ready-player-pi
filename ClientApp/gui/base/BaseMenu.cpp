#include "BaseMenu.h"
#include <QWidget>

BaseMenu::BaseMenu(QWidget *parent)
        : QMainWindow(parent)
    {
        QWidget *central = new QWidget(this);
        central->setStyleSheet("background-color: #d6eaf8;");
        setCentralWidget(central);

        resize(800, 480);

        closeBtn_ = new QPushButton("\u2716", central);
        closeBtn_->setGeometry(800 - 55, 20, 35, 35);
        closeBtn_->setStyleSheet(
            "QPushButton {"
            "  border: 2px solid #1a237e;"
            "  border-radius: 6px;"
            "  color: #1a237e;"
            "  background: transparent;"
            "  font-size: 14px;"
            "}"
            "QPushButton:pressed {"
            "  color: red;"
            "  background-color: #ffdddd;"
            "}"
            );
        connect(closeBtn_, &QPushButton::clicked, this, [this]() {
            emit mainWindowRequested();
            close();
        });

        statusLabel_ = new QLabel("", central);
        statusLabel_->setGeometry(0, 445, 800, 25);
        statusLabel_->setAlignment(Qt::AlignLeft);
        statusLabel_->setStyleSheet(
            "font-size: 13px;"
            "color: #1a237e;"
            "background: transparent;"
            );
    }

    void BaseMenu::showStatus(const QString &msg){
        statusLabel_->setText(msg);
    }

    QString BaseMenu::btnStyle()
    {
        return
            "QPushButton {"
            "  border: 2px solid #1a237e;"
            "  border-radius: 8px;"
            "  color: #1a237e;"
            "  background: transparent;"
            "  font-size: 14px;"
            "}"
            "QPushButton:pressed {"
            "  background-color: rgba(26, 35, 126, 0.15);"
            "  color: #1a237e;"
            "}";
    }

    QString BaseMenu::inputStyle(int fontSize, int letterSpacing)
    {
        return
            "QLineEdit {"
            "  border: 2px solid #1a237e;"
            "  border-radius: 8px;"
            "  padding: 0 12px;"
            "  font-size: " + QString::number(fontSize) + "px;"
                                          "  font-weight: bold;"
                                          "  letter-spacing: " + QString::number(letterSpacing) + "px;"
                                               "  color: #1a237e;"
                                               "  background: white;"
                                               "}"
                                               "QLineEdit:focus {"
                                               "  border: 2px solid rgba(26, 35, 126, 0.5);"
                                               "}";
    }

    QString BaseMenu::titleStyle()
    {
        return
            "font-size: 24px;"
            "font-weight: bold;"
            "color: #1a237e;"
            "background: transparent;";
    }

