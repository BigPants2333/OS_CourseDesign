#include "resmem.h"
#include "ui_resmem.h"
#include "sys/statfs.h"
#include <unistd.h>

resMem::resMem(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::resMem),
    m_timer(new QTimer(this))
{
    ui->setupUi(this);
    this->setFixedSize(this->width(), this->height());
    this->move(100, 600);

    int pid = getpid();
    ui->mem_pid->setText(QString::number(pid));

    connect(m_timer, &QTimer::timeout, this, &resMem::updateMem);
    m_timer->start(m_interval);
}

void resMem::updateMem()
{
    QProcess process;
    process.start("free", QStringList() << "-m");
    process.waitForFinished();
    process.readLine();
    QString str = process.readLine();
    str.replace("\n", "");
    str.replace(QRegExp("( ){1,}"), " ");
    auto lst = str.split(" ");
    if(lst.size() > 6)
    {
        ui->mem_total->setText(QString::number(lst[1].toDouble(), 'f', 0));
        ui->mem_used->setText(QString::number(lst[2].toDouble(), 'f', 0));
        ui->mem_free->setText(QString::number(lst[6].toDouble(), 'f', 0));
        // qDebug("Total:%.01lfMB,Used:%.01lfMB,Free:%.01lfMB", lst[1].toDouble(), lst[2].toDouble(), lst[6].toDouble());
    }
}

resMem::~resMem()
{
    delete ui;
}
