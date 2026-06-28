#include "ScoreboardScreen.h"
#include <QLabel>
#include <QPushButton>

ScoreboardScreen::ScoreboardScreen(int yourScore,
                                     int yourRank,
                                     const QVector<ScoreboardEntry> &top5,
                                     const QString &scoreUnit,
                                     bool showPlayAgain,
                                     QWidget *parent)
    : BaseMenu(parent)
{
    QWidget *central = centralWidget();

    QPushButton *backBtn = new QPushButton("↩", central);
    backBtn->setGeometry(20, 20, 60, 60);
    backBtn->setStyleSheet(
        "QPushButton {"
        "  border: 2px solid #1a237e;"
        "  border-radius: 8px;"
        "  color: #1a237e;"
        "  background: #d6eaf8;"
        "  font-size: 28px;"
        "}"
        "QPushButton:pressed {"
        "  background-color: rgba(26, 35, 126, 0.15);"
        "}"
    );
    connect(backBtn, &QPushButton::clicked, this, [this]() {
        emit back();
        close();
    });

    QLabel *titleLabel = new QLabel("SCOREBOARD  🏆", central);
    titleLabel->setGeometry(0, 50, 800, 60);
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setStyleSheet(
        "font-size: 44px;"
        "color: #1a237e;"
        "background: transparent;"
    );

    QString scoreLabelText = scoreUnit.isEmpty()
        ? QString("YOUR SCORE: %1").arg(yourScore)
        : QString("YOUR TIME: %1%2").arg(yourScore).arg(scoreUnit);
    QLabel *scoreLabel = new QLabel(scoreLabelText, central);
    scoreLabel->setGeometry(0, 110, 800, 40);
    scoreLabel->setAlignment(Qt::AlignCenter);
    scoreLabel->setStyleSheet(
        "font-size: 26px;"
        "color: #1a237e;"
        "background: transparent;"
    );

    int panelX = 220, panelY = 160, panelW = 360, panelH = 200;
    QLabel *panel = new QLabel(central);
    panel->setGeometry(panelX, panelY, panelW, panelH);
    panel->setStyleSheet(
        "border: 2px solid #1a237e;"
        "background: #aed6f1;"
    );

    int rowHeight = panelH / 5;
    for (int i = 0; i < top5.size() && i < 5; ++i) {
        QLabel *row = new QLabel(
            QString("%1. %2 — %3%4").arg(i + 1).arg(top5[i].names).arg(top5[i].score).arg(scoreUnit),
            central);
        row->setGeometry(panelX + 20, panelY + i * rowHeight, panelW - 40, rowHeight);
        row->setAlignment(Qt::AlignCenter);
        row->setStyleSheet(
            "font-size: 18px;"
            "color: #1a237e;"
            "background: transparent;"
            "border: none;"
        );
    }

    if (yourRank > 5 && yourRank > 0) {
        QString playerNames = "You";

        for (const auto &entry : top5) {

            Q_UNUSED(entry);
        }

        QLabel *ownPanel = new QLabel(central);
        ownPanel->setGeometry(panelX, panelY + panelH + 5, panelW, 40);
        ownPanel->setStyleSheet(
            "border: 2px solid #1a237e;"
            "background: #aed6f1;"
        );

        QLabel *ownRow = new QLabel(
            QString("%1. %2 — %3%4").arg(yourRank).arg(playerNames).arg(yourScore).arg(scoreUnit),
            central);
        ownRow->setGeometry(panelX + 20, panelY + panelH + 5, panelW - 40, 40);
        ownRow->setAlignment(Qt::AlignCenter);
        ownRow->setStyleSheet(
            "font-size: 18px;"
            "color: #1a237e;"
            "background: transparent;"
            "border: none;"
        );
    }

    if (showPlayAgain) {
        QPushButton *playAgainBtn = new QPushButton("PLAY\nAGAIN", central);
        playAgainBtn->setGeometry(670, 400, 110, 60);
        playAgainBtn->setStyleSheet(
            "QPushButton {"
            "  border: 2px solid #1a237e;"
            "  border-radius: 8px;"
            "  background: #c8e6c9;"
            "  color: #1a237e;"
            "  font-size: 16px;"
            "}"
            "QPushButton:pressed {"
            "  background: #a5d6a7;"
            "}"
        );
        connect(playAgainBtn, &QPushButton::clicked, this, [this]() {
            emit playAgain();
            close();
        });
    }
}

