#include "resnet.h"
#include "ui_resnet.h"
#include "sys/statfs.h"
#include <unistd.h>

resNet::resNet(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::resNet),
    m_timer(new QTimer(this))
{
    ui->setupUi(this);
    this->setFixedSize(this->width(), this->height());
    this->move(1500, 600);

    int pid = getpid();
    ui->net_pid->setText(QString::number(pid));

    connect(m_timer, &QTimer::timeout, this, &resNet::updateNet);
    m_timer->start(m_interval);
}

void resNet::updateNet()
{
    QProcess process;
    process.start("cat", QStringList() << "/proc/net/dev");
    process.waitForFinished();
    process.readLine();
    process.readLine();
    process.readLine();
    QString str = process.readLine();
    str.replace("\n", "");
    str.replace(QRegExp("( ){1,}"), " ");
    auto lst = str.split(" ");
    if(lst.size() > 10)
    {
        double recv = lst[2].toDouble();
        double send = lst[10].toDouble();
        lst[1].replace(":", "");
        ui->inter->setText(lst[1]);
        ui->recv->setText(QString::number((recv - m_recv_bytes) / (m_interval / 1000.0), 'f', 0));
        ui->send->setText(QString::number((send - m_send_bytes) / (m_interval / 1000.0), 'f', 0));
//        qDebug("Interface:%s Receive:%.0lfbyte/s Send:%.0lfbyte/s",
//               lst[1].toStdString().c_str(),
//               (recv - m_recv_bytes) / (m_interval / 1000.0),
//               (send - m_send_bytes) / (m_interval / 1000.0));
        m_recv_bytes = recv;
        m_send_bytes = send;
    }
}

resNet::~resNet()
{
    delete ui;
}
