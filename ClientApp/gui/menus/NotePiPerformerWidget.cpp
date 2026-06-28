#include "NotePiPerformerWidget.h"
#include <QPainter>
#include <QPainterPath>

NotePiPerformerWidget::NotePiPerformerWidget(QWidget *parent)
    : BaseMenu(parent)
{
    showIdle();
}

void NotePiPerformerWidget::clearCentral()
{
    for (int i = 0; i < 5; ++i) {
        if (slotNoteLabels_[i])   { slotNoteLabels_[i]->deleteLater();   slotNoteLabels_[i]   = nullptr; }
        if (slotNameLabels_[i])   { slotNameLabels_[i]->deleteLater();   slotNameLabels_[i]   = nullptr; }
        if (slotHeaderLabels_[i]) { slotHeaderLabels_[i]->deleteLater(); slotHeaderLabels_[i] = nullptr; }
    }
    if (titleLabel_)   { titleLabel_->deleteLater();   titleLabel_   = nullptr; }
    if (recordBtn_)    { recordBtn_->deleteLater();    recordBtn_    = nullptr; }
}

void NotePiPerformerWidget::buildIdleLayout()
{
    QWidget *central = centralWidget();

    titleLabel_ = new QLabel("Start Recording", central);
    titleLabel_->setGeometry(0, 200, 800, 50);
    titleLabel_->setAlignment(Qt::AlignCenter);
    titleLabel_->setStyleSheet(
        "font-size: 28px;"
        "color: #1a237e;"
        "background: transparent;"
    );
    titleLabel_->show();

    recordBtn_ = new QPushButton("▶", central);
    recordBtn_->setGeometry(340, 270, 120, 120);
    recordBtn_->setStyleSheet(
        "QPushButton {"
        "  border: none;"
        "  border-radius: 60px;"
        "  background-color: #000000;"
        "  color: white;"
        "  font-size: 56px;"
        "  padding-left: 12px;"
        "}"
        "QPushButton:pressed {"
        "  background-color: #333333;"
        "}"
    );
    connect(recordBtn_, &QPushButton::clicked, this, [this]() {
        emit recordingStarted();
    });
    recordBtn_->show();
}

void NotePiPerformerWidget::buildRecordingLayout()
{
    QWidget *central = centralWidget();

    int slotWidth   = 130;
    int spacing     = (800 - 5 * slotWidth) / 6;
    int startY      = 60;

    for (int i = 0; i < 5; ++i) {
        int x = spacing + i * (slotWidth + spacing);

        slotHeaderLabels_[i] = new QLabel(QString("Note %1").arg(i + 1), central);
        slotHeaderLabels_[i]->setGeometry(x, startY, slotWidth, 30);
        slotHeaderLabels_[i]->setAlignment(Qt::AlignCenter);
        slotHeaderLabels_[i]->setStyleSheet(
            "font-size: 18px;"
            "color: #1a237e;"
            "background: transparent;"
        );
        slotHeaderLabels_[i]->show();

        slotNoteLabels_[i] = new QLabel("♪", central);
        slotNoteLabels_[i]->setGeometry(x, startY + 40, slotWidth, 100);
        slotNoteLabels_[i]->setAlignment(Qt::AlignCenter);
        slotNoteLabels_[i]->setStyleSheet(
            "font-size: 80px;"
            "color: #1a237e;"
            "background: transparent;"
        );
        slotNoteLabels_[i]->show();

        slotNameLabels_[i] = new QLabel("—", central);
        slotNameLabels_[i]->setGeometry(x, startY + 145, slotWidth, 30);
        slotNameLabels_[i]->setAlignment(Qt::AlignCenter);
        slotNameLabels_[i]->setStyleSheet(
            "font-size: 18px;"
            "color: #1a237e;"
            "background: transparent;"
        );
        slotNameLabels_[i]->show();
    }

    QLabel *hint = new QLabel("Play 5 notes…", central);
    hint->setGeometry(0, 380, 800, 40);
    hint->setAlignment(Qt::AlignCenter);
    hint->setStyleSheet(
        "font-size: 20px;"
        "color: #1a237e;"
        "background: transparent;"
    );
    hint->show();
}

void NotePiPerformerWidget::showIdle()
{
    clearCentral();
    buildIdleLayout();
}

void NotePiPerformerWidget::showRecording()
{
    clearCentral();
    buildRecordingLayout();
}

void NotePiPerformerWidget::onNoteDetected(QString noteName, int slotIndex)
{
    if (slotIndex < 0 || slotIndex >= 5) return;
    if (!slotNameLabels_[slotIndex]) return;
    slotNameLabels_[slotIndex]->setText(noteName);
    slotNoteLabels_[slotIndex]->setStyleSheet(
        "font-size: 80px;"
        "color: #1a237e;"
        "background: transparent;"
        "font-weight: bold;"
    );
}

