#ifndef NOTEPIPERFORMERWIDGET_H
#define NOTEPIPERFORMERWIDGET_H

#include "../base/BaseMenu.h"
#include <QLabel>
#include <QPushButton>
#include <QString>
#include <QVector>

class NotePiPerformerWidget : public BaseMenu
{
    Q_OBJECT
public:
    explicit NotePiPerformerWidget(QWidget *parent = nullptr);

public slots:
    void showIdle();
    void showRecording();
    void onNoteDetected(QString noteName, int slotIndex);

signals:
    void recordingStarted();

private:
    void buildIdleLayout();
    void buildRecordingLayout();
    void clearCentral();

    QLabel  *titleLabel_          = nullptr;
    QPushButton *recordBtn_       = nullptr;
    QLabel  *slotNoteLabels_[5]   = {nullptr, nullptr, nullptr, nullptr, nullptr};
    QLabel  *slotNameLabels_[5]   = {nullptr, nullptr, nullptr, nullptr, nullptr};
    QLabel  *slotHeaderLabels_[5] = {nullptr, nullptr, nullptr, nullptr, nullptr};
};

#endif

