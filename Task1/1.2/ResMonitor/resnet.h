#ifndef RESNET_H
#define RESNET_H

#include <QWidget>
#include <QTimer>
#include <QProcess>
#include <QDebug>

namespace Ui {
class resNet;
}

class resNet : public QWidget
{
    Q_OBJECT

public:
    explicit resNet(QWidget *parent = nullptr);
    ~resNet();
    void updateNet();

private:
    Ui::resNet *ui;
    const int m_interval = 1000;
    QTimer *m_timer;
    double m_recv_bytes = 0;
    double m_send_bytes = 0;
};

#endif // RESNET_H
