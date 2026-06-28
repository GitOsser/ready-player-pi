#pragma once

#include "../base/BaseMenu.h"
#include <QPaintEvent>
#include <QPainter>
#include <QPushButton>
#include <QWidget>

class GyroPiTiltIndicator : public QWidget
{
    Q_OBJECT

public:
    explicit GyroPiTiltIndicator(QWidget *parent = nullptr);

public slots:

    void setTilt(float tiltX, float tiltY);

    void onBlackHoleHit();

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    float tiltX_ = 0.0f;
    float tiltY_ = 0.0f;
    bool  flash_ = false;

    void drawDial(QPainter &painter, const QPointF &centre, qreal radius);
    void drawBall(QPainter &painter, const QPointF &centre, qreal radius);
};

class GyroPiPerformerWindow : public BaseMenu
{
    Q_OBJECT

public:
    explicit GyroPiPerformerWindow(QWidget *parent = nullptr);

public slots:
    void setTilt(float tiltX, float tiltY);
    void onBlackHoleHit();

signals:
    void exitRequested();

private:
    GyroPiTiltIndicator *indicator_ = nullptr;
    QPushButton         *exitBtn_   = nullptr;
};

