#ifndef RESMEM_H
#define RESMEM_H

#include <QWidget>
#include <QTimer>
#include <QProcess>
#include <QDebug>

namespace Ui {
class resMem;
}

class resMem : public QWidget
{
    Q_OBJECT

public:
    explicit resMem(QWidget *parent = nullptr);
    ~resMem();
    void updateMem();

private:
    Ui::resMem *ui;
    const int m_interval = 1000;
    QTimer *m_timer;
};

#endif // RESMEM_H
