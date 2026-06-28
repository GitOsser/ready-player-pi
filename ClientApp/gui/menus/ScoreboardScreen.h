#ifndef SCOREBOARDSCREEN_H
#define SCOREBOARDSCREEN_H

#include "../base/BaseMenu.h"
#include <QString>
#include <QVector>

struct ScoreboardEntry {
    QString names;
    int     score;
};

class ScoreboardScreen : public BaseMenu
{
    Q_OBJECT
public:

    ScoreboardScreen(int yourScore,
                      int yourRank,
                      const QVector<ScoreboardEntry> &top5,
                      const QString &scoreUnit = "",
                      bool showPlayAgain = true,
                      QWidget *parent = nullptr);

signals:
    void playAgain();
    void back();
};

#endif

