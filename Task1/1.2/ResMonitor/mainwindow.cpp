#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <unistd.h>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_timer(new QTimer(this))
{
    ui->setupUi(this);
    this->setFixedSize(this->width(), this->height());
    this->move(600, 250);

    int pid = getpid();
    ui->time_pid->setText(QString::number(pid));

    connect(m_timer, &QTimer::timeout, this, [=](){
       QDateTime time = QDateTime::currentDateTime();
       // qDebug() << time;
       QString str = time.toString("yyyy MM dd hh:mm:ss dddd");
       ui->time->setText(str);
       // qDebug() << str;
    });
    m_timer->start(m_interval);
}

MainWindow::~MainWindow()
{
    delete ui;
}

