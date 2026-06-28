#include "NotePiCommunicatorWidget.h"

namespace {
QString colorForFeedback(const QString &fb) {
    if (fb == "GREEN")  return "#4caf50";
    if (fb == "YELLOW") return "#fbc02d";
    if (fb == "RED")    return "#e53935";
    return "#1a237e";
}
}

NotePiCommunicatorWidget::NotePiCommunicatorWidget(QWidget *parent)
    : BaseMenu(parent)
{
    onWaiting();
}

void NotePiCommunicatorWidget::clearCentral()
{
    if (waitingLabel_) { waitingLabel_->deleteLater(); waitingLabel_ = nullptr; }
    for (int i = 0; i < 5; ++i) {
        if (noteIcons_[i])   { noteIcons_[i]->deleteLater();   noteIcons_[i]   = nullptr; }
        if (noteLetters_[i]) { noteLetters_[i]->deleteLater(); noteLetters_[i] = nullptr; }
    }
}

void NotePiCommunicatorWidget::buildWaitingLayout()
{
    QWidget *central = centralWidget();

    waitingLabel_ = new QLabel("WAITING FOR PERFORMER", central);
    waitingLabel_->setGeometry(0, 200, 800, 60);
    waitingLabel_->setAlignment(Qt::AlignCenter);
    waitingLabel_->setStyleSheet(
        "font-size: 36px;"
        "color: #1a237e;"
        "background: transparent;"
    );
    waitingLabel_->show();
}

void NotePiCommunicatorWidget::buildFeedbackLayout()
{
    QWidget *central = centralWidget();

    int slotWidth = 130;
    int spacing   = (800 - 5 * slotWidth) / 6;
    int startY    = 180;

    for (int i = 0; i < 5; ++i) {
        int x = spacing + i * (slotWidth + spacing);

        noteIcons_[i] = new QLabel("♪", central);
        noteIcons_[i]->setGeometry(x, startY, slotWidth, 140);
        noteIcons_[i]->setAlignment(Qt::AlignCenter);
        noteIcons_[i]->setStyleSheet(
            "font-size: 100px;"
            "color: #1a237e;"
            "background: transparent;"
        );
        noteIcons_[i]->show();

        noteLetters_[i] = new QLabel("", central);
        noteLetters_[i]->setGeometry(x, startY + 140, slotWidth, 40);
        noteLetters_[i]->setAlignment(Qt::AlignCenter);
        noteLetters_[i]->setStyleSheet(
            "font-size: 32px;"
            "color: #1a237e;"
            "background: transparent;"
            "font-weight: bold;"
        );
        noteLetters_[i]->show();
    }

}

void NotePiCommunicatorWidget::onWaiting()
{
    clearCentral();
    buildWaitingLayout();
}

void NotePiCommunicatorWidget::onAttempt(QVector<QString> feedback,
                                         QVector<QString> notes,
                                         int )
{
    if (waitingLabel_ || !noteIcons_[0]) {
        clearCentral();
        buildFeedbackLayout();
    }

    for (int i = 0; i < 5 && i < feedback.size(); ++i) {
        QString hex = colorForFeedback(feedback[i]);
        noteIcons_[i]->setStyleSheet(
            QString("font-size: 100px; color: %1; background: transparent;").arg(hex)
        );
        if (noteLetters_[i]) {
            QString letter = (i < notes.size()) ? notes[i] : QString();
            noteLetters_[i]->setText(letter);
            noteLetters_[i]->setStyleSheet(
                QString("font-size: 32px; color: %1; background: transparent; font-weight: bold;").arg(hex)
            );
        }
    }

}

