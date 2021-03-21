#ifndef CPUINFO_H
#define CPUINFO_H

#include <QWidget>
#include <QProcess>
#include <QPainter>
#include <QLabel>
#include <QDebug>

#ifndef MY_SECONDS
#define MY_SECONDS 60
#endif

#ifndef MY_SPACE_X
#define MY_SPACE_X 34
#endif

#ifndef MY_SPACE_Y
#define MY_SPACE_Y 15
#endif

class CpuInfo : public QWidget
{
    Q_OBJECT
public:
    explicit CpuInfo(QWidget *parent = nullptr);
    ~CpuInfo();

    void updateCpuInfo();
    QString getCpuUsage();

protected:
    void paintEvent(QPaintEvent *);

private:
    QImage drawImage(QWidget *widget);

signals:

private:
    QWidget *m_widget;
    QString cpuUsage;
    double m_cpu_rate = 0;
    double m_cpu_total = 0;
    double m_cpu_use = 0;

    QVector<qreal> cpuVec;

};

#endif // CPUINFO_H
