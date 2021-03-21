#include "resdisk.h"
#include "ui_resdisk.h"
#include <unistd.h>

resDisk::resDisk(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::resDisk),
    m_timer(new QTimer(this))
{
    ui->setupUi(this);
    this->setFixedSize(this->width(), this->height());
    this->move(1500, 100);

    int pid = getpid();
    ui->disk_pid->setText(QString::number(pid));

    connect(m_timer, &QTimer::timeout, this, &resDisk::updateDisk);
    m_timer->start(m_interval);
}

void resDisk::updateDisk()
{
    QProcess process;
    process.start("df", QStringList() << "-k");
    process.waitForFinished();
    process.readLine();
    while(!process.atEnd())
    {
        QString str = process.readLine();
        if(str.startsWith("/dev/sda5"))
        {
            str.replace("\n", "");
            str.replace(QRegExp("( ){1,}"), " ");
            auto lst = str.split(" ");
            if(lst.size() > 5)
            {
                ui->file->setText(lst[0]);
                ui->mount->setText(lst[5]);
                ui->used->setText(QString::number(lst[2].toDouble()/1024.0, 'f', 0));
                ui->free->setText(QString::number(lst[3].toDouble()/1024.0, 'f', 0));
//                qDebug("File system:%s Mount point:%s Used:%.0lfMB Free:%.0lfMB", lst[0].toStdString().c_str(),
//                       lst[5].toStdString().c_str(), lst[2].toDouble()/1024.0, lst[3].toDouble()/1024.0);
            }
        }
    }
}

resDisk::~resDisk()
{
    delete ui;
}
