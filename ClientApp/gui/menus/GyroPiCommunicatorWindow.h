#pragma once

#include "../base/BaseMenu.h"
#include <QLabel>
#include <QMetaType>
#include <QPaintEvent>
#include <QPainter>
#include <QPointF>
#include <QPushButton>
#include <QTimer>
#include <QVector>
#include <QWidget>

struct GyroHoleInfo {
    float x;
    float y;
    float radius;
};

struct GyroCourseInfo {
    float startX;
    float startY;
    float goalX;
    float goalY;
    QVector<GyroHoleInfo> holes;
};

class GyroPiCourseView : public QWidget
{
    Q_OBJECT

public:
    explicit GyroPiCourseView(QWidget *parent = nullptr);

public slots:
    void onCourseLoaded(GyroCourseInfo info);
    void onBallUpdate(float x, float y);
    void onBlackHoleHit();
    void onScoreUpdate(int score);

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    GyroCourseInfo courseInfo_;
    QPointF        ballPosition_;
    bool           courseLoaded_   = false;
    bool           blackHoleFlash_ = false;
    int            currentScore_   = 0;

    QTimer *flashTimer_ = nullptr;

    void drawCourse(QPainter &painter);
    void drawHoles(QPainter &painter);
    void drawGoal(QPainter &painter);
    void drawBall(QPainter &painter);
    void drawHud(QPainter &painter);
    void drawBlackHoleMessage(QPainter &painter);
    void drawWaitingMessage(QPainter &painter);
};

class GyroPiCommunicatorWindow : public BaseMenu
{
    Q_OBJECT

public:
    explicit GyroPiCommunicatorWindow(QWidget *parent = nullptr);

public slots:
    void onCourseLoaded(GyroCourseInfo info);
    void onBallUpdate(float x, float y);
    void onBlackHoleHit();
    void onScoreUpdate(int score);

signals:

    void exitRequested();

private:
    GyroPiCourseView *courseView_ = nullptr;
    QPushButton      *exitBtn_    = nullptr;
};

Q_DECLARE_METATYPE(GyroCourseInfo)

