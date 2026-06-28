#include "UsernameMenu.h"
#include "../base/BaseMenu.h"
#include <QLabel>
#include <QPushButton>
#include <QRegularExpression>

UsernameMenu::UsernameMenu(const QString &prefill, QWidget *parent): BaseMenu(parent){
    QWidget *central = centralWidget();

    QLabel *title = new QLabel("Enter Username", central);
    title->setGeometry(0, 150, 800, 50);
    title->setAlignment(Qt::AlignCenter);
    title->setStyleSheet(BaseMenu::titleStyle());

    usernameInput_ = new QLineEdit(central);
    usernameInput_->setGeometry(250, 215, 300, 45);
    usernameInput_->setPlaceholderText("Username");
    usernameInput_->setInputMethodHints(Qt::ImhNoPredictiveText);
    usernameInput_->setStyleSheet(BaseMenu::inputStyle());
    if (!prefill.isEmpty())
        usernameInput_->setText(prefill);

    errorLabel_ = new QLabel("", central);
    errorLabel_->setGeometry(250, 268, 300, 22);
    errorLabel_->setAlignment(Qt::AlignCenter);
    errorLabel_->setStyleSheet("color: red; font-size: 13px;");
    errorLabel_->hide();

    QPushButton *confirmBtn = new QPushButton("Save", central);
    confirmBtn->setGeometry(325, 280, 150, 45);
    confirmBtn->setStyleSheet(BaseMenu::btnStyle());
    connect(usernameInput_, &QLineEdit::returnPressed, this, &UsernameMenu::onConfirm);
    connect(confirmBtn, &QPushButton::clicked, this, &UsernameMenu::onConfirm);
}

void UsernameMenu::onConfirm()
{
    QString text = usernameInput_->text().trimmed();

    static const QString errorInputStyle =
        "QLineEdit {"
        "  border: 2px solid red;"
        "  border-radius: 8px;"
        "  padding: 0 12px;"
        "  font-size: 16px;"
        "  color: #1a237e;"
        "  background: white;"
        "}";

    if (text.isEmpty()) {
        usernameInput_->setStyleSheet(errorInputStyle);
        errorLabel_->setText("Username cannot be empty");
        errorLabel_->show();
        return;
    }

    if (text.length() > 12) {
        usernameInput_->setStyleSheet(errorInputStyle);
        errorLabel_->setText("Username max 12 characters");
        errorLabel_->show();
        usernameInput_->clear();
        return;
    }

    static const QRegularExpression validPattern("^[A-Za-z0-9]+$");
    if (!validPattern.match(text).hasMatch()) {
        usernameInput_->setStyleSheet(errorInputStyle);
        errorLabel_->setText("Invalid Username");
        errorLabel_->show();
        usernameInput_->clear();
        return;
    }

    std::string username = text.toStdString();
    usernameInput_->setStyleSheet(BaseMenu::inputStyle());
    errorLabel_->hide();
    emit usernameConfirmed(username);
    this->close();
}

void UsernameMenu::onUsernameValidated(bool approved)
{
    if (approved) {
        this->close();
    } else {
        usernameInput_->setStyleSheet(
            "QLineEdit {"
            "  border: 2px solid red;"
            "  border-radius: 8px;"
            "  padding: 0 12px;"
            "  font-size: 16px;"
            "  color: #1a237e;"
            "  background: white;"
            "}"
            );
        errorLabel_->setText("Invalid Username");
        errorLabel_->show();
    }
}
