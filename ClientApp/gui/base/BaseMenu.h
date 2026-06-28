#ifndef BASEMENU_H
#define BASEMENU_H

#include <QMainWindow>
#include <QLabel>
#include <QPushButton>
#include <QString>

class BaseMenu : public QMainWindow
{
    Q_OBJECT

public:
    explicit BaseMenu(QWidget *parent = nullptr);
    void showStatus(const QString &msg);

    static QString btnStyle();
    static QString inputStyle(int fontSize = 16, int letterSpacing = 0);
    static QString titleStyle();

signals:
    void mainWindowRequested();

private:
    QPushButton *closeBtn_;
    QLabel *statusLabel_;
};

#endif

