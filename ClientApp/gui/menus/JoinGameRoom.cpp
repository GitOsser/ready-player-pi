#include "JoinGameRoom.h"
#include <QRegularExpressionValidator>

JoinGameRoom::JoinGameRoom(QWidget *parent)
    : BaseMenu(parent)
{
    QWidget *central = centralWidget();

    QLabel *title = new QLabel("Enter room PIN", central);
    title->setGeometry(0, 100, 800, 50);
    title->setAlignment(Qt::AlignCenter);
    title->setStyleSheet(BaseMenu::titleStyle());

    pinInput_ = new QLineEdit(central);
    pinInput_->setGeometry(250, 170, 300, 55);
    pinInput_->setPlaceholderText("6-digit PIN...");
    pinInput_->setMaxLength(6);
    pinInput_->setAlignment(Qt::AlignCenter);
    pinInput_->setInputMethodHints(Qt::ImhDigitsOnly);
    pinInput_->setStyleSheet(BaseMenu::inputStyle(28,8));
    connect(pinInput_, &QLineEdit::returnPressed, this, [this]() { onPinChanged(pinInput_->text()); });
    connect(pinInput_, &QLineEdit::textChanged, this, &JoinGameRoom::onPinChanged);

}

void JoinGameRoom::onPinChanged(const QString &text){
    if(text.length()==6){emit pinEntered(text.toStdString());}
}
