#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QTimer>
#include <QFile>
#include <QDir>
#include <QDateTime>
#include <QMessageBox>
#include <QStandardItemModel>
#include <QTableWidget>
#include <QDebug>

#include "cpuinfo.h"

QT_BEGIN_NAMESPACE
namespace Ui { class Widget; }
QT_END_NAMESPACE

class Widget : public QWidget
{
    Q_OBJECT

public:
    Widget(QWidget *parent = nullptr);
    ~Widget();

private:
    void updateInfo();

    void initTable();
    void sysInfo();
    void proInfo();
    void timeInfo();
    void memInfo();

    void on_proRefBtn_clicked();
    void on_shutdownBtn_clicked();
    void on_rebootBtn_clicked();
    void on_exitBtn_clicked();

    void findPro();
    void killPro();


private:
    Ui::Widget *ui;
    const int m_interval = 1000;
    QTimer *m_timer;

    CpuInfo *cpu;

    QStandardItemModel *model_pro;
};
#endif // WIDGET_H
