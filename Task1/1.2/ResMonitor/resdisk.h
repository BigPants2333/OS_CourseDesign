#ifndef RESDISK_H
#define RESDISK_H

#include <QWidget>
#include <QProcess>
#include <QTimer>
#include <QDebug>

namespace Ui {
class resDisk;
}

class resDisk : public QWidget
{
    Q_OBJECT

public:
    explicit resDisk(QWidget *parent = nullptr);
    ~resDisk();
    void updateDisk();

private:
    Ui::resDisk *ui;
    const int m_interval = 1000;
    QTimer *m_timer;
};

#endif // RESDISK_H
