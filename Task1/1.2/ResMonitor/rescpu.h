#ifndef RESCPU_H
#define RESCPU_H

#include <QWidget>
#include <QProcess>
#include <QTimer>
#include <QDebug>

namespace Ui {
class resCpu;
}

class resCpu : public QWidget
{
    Q_OBJECT

public:
    explicit resCpu(QWidget *parent = nullptr);
    ~resCpu();
    void updateCpu();

private:
    Ui::resCpu *ui;
    const int m_interval = 1000;
    QTimer *m_timer;
    double m_cpu_rate = 0;
    double m_cpu_total = 0;
    double m_cpu_use = 0;
};

#endif // RESCPU_H
