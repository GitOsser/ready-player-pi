#ifndef NOTEPICOMMUNICATORWIDGET_H
#define NOTEPICOMMUNICATORWIDGET_H

#include "../base/BaseMenu.h"
#include <QLabel>
#include <QString>
#include <QVector>

class NotePiCommunicatorWidget : public BaseMenu
{
    Q_OBJECT
public:
    explicit NotePiCommunicatorWidget(QWidget *parent = nullptr);

public slots:
    void onWaiting();
    void onAttempt(QVector<QString> feedback, QVector<QString> notes, int attemptNum);

private:
    void buildWaitingLayout();
    void buildFeedbackLayout();
    void clearCentral();

    QLabel *waitingLabel_    = nullptr;
    QLabel *noteIcons_[5]    = {nullptr, nullptr, nullptr, nullptr, nullptr};
    QLabel *noteLetters_[5]  = {nullptr, nullptr, nullptr, nullptr, nullptr};
};

#endif

