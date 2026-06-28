#include "GyroPiPerformerWindow.h"

#include <QBrush>
#include <QColor>
#include <QFont>
#include <QPen>
#include <QPointF>
#include <QRadialGradient>
#include <QtMath>
#include <algorithm>
#include <cmath>

namespace {

    constexpr int   NUM_SEGMENTS         = 24;

    constexpr qreal SEGMENT_ARC_DEGREES  = 11.0;

    constexpr qreal SEGMENT_INNER_FRAC   = 0.78;
    constexpr qreal SEGMENT_OUTER_FRAC   = 1.00;

    constexpr qreal BALL_DRIFT_FRAC      = 0.35;

    constexpr qreal FULL_DARK_MAGNITUDE  = 1.0;
}

GyroPiTiltIndicator::GyroPiTiltIndicator(QWidget *parent)
    : QWidget(parent)
{
    setAttribute(Qt::WA_OpaquePaintEvent, false);
}

void GyroPiTiltIndicator::setTilt(float tiltX, float tiltY)
{

    tiltX_ = std::clamp(-tiltX, -1.0f, 1.0f);
    tiltY_ = std::clamp(-tiltY, -1.0f, 1.0f);
    update();
}

void GyroPiTiltIndicator::onBlackHoleHit()
{
    flash_ = true;
    update();

}

void GyroPiTiltIndicator::paintEvent(QPaintEvent * )
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);

    QColor fieldColour = flash_ ? QColor(255, 235, 235) : QColor(255, 255, 255);
    painter.fillRect(rect(), fieldColour);
    flash_ = false;

    QPen borderPen(QColor("#1a237e"), 3);
    painter.setPen(borderPen);
    painter.setBrush(Qt::NoBrush);
    painter.drawRect(rect().adjusted(1, 1, -2, -2));

    const QPointF centre(width() / 2.0, height() / 2.0);
    const qreal   radius = std::min(width(), height()) * 0.40;

    drawDial(painter, centre, radius);
    drawBall(painter, centre, radius);
}

void GyroPiTiltIndicator::drawDial(QPainter &painter,
                                    const QPointF &centre,
                                    qreal radius)
{

    const qreal magnitude = std::min(qreal(1.0), qreal(std::hypot(tiltX_, tiltY_)));
    const bool  hasTilt   = magnitude > 0.02;

    qreal directionRad = 0.0;
    if (hasTilt) {
        directionRad = std::atan2(tiltX_, -tiltY_);
    }

    const qreal innerR = radius * SEGMENT_INNER_FRAC;
    const qreal outerR = radius * SEGMENT_OUTER_FRAC;

    for (int i = 0; i < NUM_SEGMENTS; ++i) {

        const qreal segAngleRad =
            (qreal(i) / NUM_SEGMENTS) * 2.0 * M_PI;

        qreal delta = segAngleRad - directionRad;

        while (delta >  M_PI) delta -= 2.0 * M_PI;
        while (delta < -M_PI) delta += 2.0 * M_PI;
        const qreal absDelta = std::abs(delta);

        constexpr qreal HALF_BEAM = M_PI / 7.0;
        qreal proximity = 0.0;
        if (hasTilt && absDelta < HALF_BEAM) {
            proximity = 1.0 - (absDelta / HALF_BEAM);
        }
        const qreal intensity = std::clamp(
            proximity * (magnitude / FULL_DARK_MAGNITUDE), 0.0, 1.0);

        const int base = 210;
        const QColor baseColour(base, base, base);
        const QColor highlightColour("#1a237e");
        const QColor segColour(
            int(baseColour.red()   * (1.0 - intensity) + highlightColour.red()   * intensity),
            int(baseColour.green() * (1.0 - intensity) + highlightColour.green() * intensity),
            int(baseColour.blue()  * (1.0 - intensity) + highlightColour.blue()  * intensity)
        );

        const qreal halfArc = qDegreesToRadians(SEGMENT_ARC_DEGREES) / 2.0;

        auto unit = [](qreal angRad) {
            return QPointF(std::sin(angRad), -std::cos(angRad));
        };

        const QPointF dirA = unit(segAngleRad - halfArc);
        const QPointF dirB = unit(segAngleRad + halfArc);

        QPolygonF poly;
        poly << centre + dirA * innerR
             << centre + dirA * outerR
             << centre + dirB * outerR
             << centre + dirB * innerR;

        painter.setPen(Qt::NoPen);
        painter.setBrush(segColour);
        painter.drawPolygon(poly);
    }
}

void GyroPiTiltIndicator::drawBall(QPainter &painter,
                                    const QPointF &centre,
                                    qreal radius)
{

    const qreal driftLimit = radius * BALL_DRIFT_FRAC;
    const qreal magnitude  = std::min(qreal(1.0), qreal(std::hypot(tiltX_, tiltY_)));

    QPointF offset(0, 0);
    if (magnitude > 0.0001) {
        offset = QPointF(tiltX_, tiltY_) * driftLimit;
    }

    const QPointF ballCentre = centre + offset;
    const qreal   ballRadius = radius * 0.30;

    QRadialGradient gradient(
        ballCentre + QPointF(-ballRadius * 0.30, -ballRadius * 0.30),
        ballRadius * 1.4);
    gradient.setColorAt(0.00, QColor(180, 220, 255));
    gradient.setColorAt(0.55, QColor( 60, 130, 220));
    gradient.setColorAt(1.00, QColor( 20,  50, 130));

    painter.setBrush(gradient);
    painter.setPen(QPen(QColor("#0d1554"), 1));
    painter.drawEllipse(ballCentre, ballRadius, ballRadius);
}

GyroPiPerformerWindow::GyroPiPerformerWindow(QWidget *parent)
    : BaseMenu(parent)
{
    QWidget *central = centralWidget();

    indicator_ = new GyroPiTiltIndicator(central);
    indicator_->setGeometry(40, 60, 720, 360);

    exitBtn_ = new QPushButton("EXIT", central);
    exitBtn_->setGeometry(800 - 110, 480 - 50, 80, 35);
    exitBtn_->setStyleSheet(BaseMenu::btnStyle());
    connect(exitBtn_, &QPushButton::clicked,
            this, &GyroPiPerformerWindow::exitRequested);
}

void GyroPiPerformerWindow::setTilt(float tiltX, float tiltY)
{
    indicator_->setTilt(tiltX, tiltY);
}

void GyroPiPerformerWindow::onBlackHoleHit()
{
    indicator_->onBlackHoleHit();
}

