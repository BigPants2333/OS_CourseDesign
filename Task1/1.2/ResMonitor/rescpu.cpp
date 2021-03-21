#include "rescpu.h"
#include "ui_rescpu.h"
#include "sys/statfs.h"
#include <unistd.h>

resCpu::resCpu(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::resCpu),
    m_timer(new QTimer(this))
{
    ui->setupUi(this);
    this->setFixedSize(this->width(), this->height());
    this->move(100, 100);

    int pid = getpid();
    ui->cpu_pid->setText(QString::number(pid));

    connect(m_timer, &QTimer::timeout, this, &resCpu::updateCpu);
    m_timer->start(m_interval);
}

void resCpu::updateCpu()
{
    QProcess process;
    process.start("cat", QStringList() << "/proc/stat");
    process.waitForFinished();
    QString str = process.readLine();
    str.replace("\n", "");
    str.replace(QRegExp("( ){1,}"), " ");
    auto lst = str.split(" ");
    if(lst.size() > 3)
    {
        double use = lst[1].toDouble() + lst[2].toDouble() + lst[3].toDouble();
        double total = 0;
        for(int i = 1; i < lst.size(); ++i)
            total += lst[i].toDouble();
        if(total - m_cpu_total > 0)
        {
            m_cpu_rate = (use - m_cpu_use) / (total - m_cpu_total) * 100.0;
            m_cpu_total = total;
            m_cpu_use = use;
            ui->cpu_rate->setText(QString::number(m_cpu_rate, 'f', 2));
            // qDebug("CPU:%.2lf%%", m_cpu_rate);
        }
    }
}

resCpu::~resCpu()
{
    delete ui;
}
