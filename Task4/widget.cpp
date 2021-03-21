#include "widget.h"
#include "ui_widget.h"

Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
    , m_timer(new QTimer(this))
    , model_pro(new QStandardItemModel())
{
    ui->setupUi(this);
    this->setFixedSize(this->width(), this->height());

    initTable();

    sysInfo();
    cpu = new CpuInfo(ui->cpu_widget);
    cpu->resize(ui->cpu_widget->width(),ui->cpu_widget->height());

    connect(ui->proRefBtn, &QPushButton::clicked, this, &Widget::on_proRefBtn_clicked);
    connect(ui->shutdownBtn, &QPushButton::clicked, this, &Widget::on_shutdownBtn_clicked);
    connect(ui->rebootBtn, &QPushButton::clicked, this, &Widget::on_rebootBtn_clicked);
    connect(ui->exitBtn, &QPushButton::clicked, this, &Widget::on_exitBtn_clicked);
    connect(ui->findBtn, &QPushButton::clicked, this, &Widget::findPro);
    connect(ui->killBtn, &QPushButton::clicked, this, &Widget::killPro);


    connect(m_timer, &QTimer::timeout, this, &Widget::updateInfo);
    m_timer->start(m_interval);
}

Widget::~Widget()
{
    delete ui;
}

void Widget::updateInfo()
{
    proInfo();
    timeInfo();
    memInfo();
    cpu->updateCpuInfo();
    cpu->show();
    ui->label_cpu->setText("CPU: " + cpu->getCpuUsage() + "%");
    ui->cpu_usage->setText("CPU: " + cpu->getCpuUsage() + "%");
    // qDebug() << cpu->getCpuUsage();
}

void Widget::on_proRefBtn_clicked()
{
    proInfo();
}

void Widget::on_shutdownBtn_clicked()
{
    QMessageBox:: StandardButton result;
    result = QMessageBox::warning(this, tr("Shutdown"), tr("The computer will shutdown immediately!"), QMessageBox::Yes | QMessageBox::No);
    switch (result) {
    case QMessageBox::Yes:
        system("shutdown -h now");
        break;
    default:
        break;
    }
}

void Widget::on_rebootBtn_clicked()
{
    QMessageBox:: StandardButton result;
    result = QMessageBox::warning(this, tr("Reboot"), tr("The computer will reboot immediately!"), QMessageBox::Yes | QMessageBox::No);
    switch (result) {
    case QMessageBox::Yes:
        system("shutdown -r now");
        break;
    default:
        break;
    }
}

void Widget::on_exitBtn_clicked()
{
    this->close();
}

void Widget::initTable()
{
    model_pro->setColumnCount(6);
    model_pro->setHeaderData(0, Qt::Horizontal, QString::fromLocal8Bit("PID"));
    model_pro->setHeaderData(1, Qt::Horizontal, QString::fromLocal8Bit("Name"));
    model_pro->setHeaderData(2, Qt::Horizontal, QString::fromLocal8Bit("Status"));
    model_pro->setHeaderData(3, Qt::Horizontal, QString::fromLocal8Bit("PPID"));
    model_pro->setHeaderData(4, Qt::Horizontal, QString::fromLocal8Bit("Priority"));
    model_pro->setHeaderData(5, Qt::Horizontal, QString::fromLocal8Bit("Memory"));
    ui->tableView_pro->setModel(model_pro);
    ui->tableView_pro->horizontalHeader()->setDefaultAlignment(Qt::AlignLeft);
    ui->tableView_pro->verticalHeader()->setVisible(false);
}

void Widget::sysInfo()
{
    QFile procFile;
    QString str;
    QStringList strlist;
    procFile.setFileName("/proc/cpuinfo");
    if(!procFile.open(QIODevice::ReadOnly))
    {
        QMessageBox::warning(this, tr("warning"), tr("/proc/cpuinfo can not open!"), QMessageBox::Yes);
        return;
    }
    int i = 9;
    while(i--)
    {
        str = procFile.readLine();
        str.remove("\t:");
        str.remove("\n");
        strlist << str;
    }
    // qDebug() << strlist;
    ui->cputype->setText(strlist[1].remove("vendor_id "));
    ui->cpuname->setText(strlist[4].remove("model name "));
    ui->cpufre->setText(strlist[7].remove("cpu MHz\t ") + "MHz");
    ui->cpucache->setText(strlist[8].remove("cache size "));
    procFile.close();
    strlist.clear();

    procFile.setFileName("/proc/sys/kernel/hostname");
    if(!procFile.open(QIODevice::ReadOnly))
    {
        QMessageBox::warning(this, tr("warning"), tr("/proc/sys/kernel/hostname can not open!"), QMessageBox::Yes);
        return;
    }
    str = procFile.readLine();
    str.remove("\n");
    // qDebug() << str;
    ui->hostname->setText(str);
    procFile.close();

    procFile.setFileName("/proc/version");
    if(!procFile.open(QIODevice::ReadOnly))
    {
        QMessageBox::warning(this, tr("warning"), tr("/proc/version can not open!"), QMessageBox::Yes);
        return;
    }
    str = procFile.readLine();
    strlist = str.split(" ");
    // qDebug() << strlist;
    ui->ostype->setText(strlist[0]);
    ui->osversion->setText(strlist[2]);
    ui->gccversion->setText(strlist[7].replace(",", " ") + strlist[5] + strlist[6]);
    procFile.close();
}

void Widget::proInfo()
{
    QDir qd("/proc");
    QStringList qslist = qd.entryList();
    QString qs = qslist.join("\n");
    QString proPID;
    int total, run, sleep, io, zombie;
    total = run = sleep = io = zombie = 0;
    bool ok = true;         // if proPID can not turn to number based 10 then break the loop
    int id_begin, id_end;
    int find_start = 3;     // ignore substring "\n..."
    while(1)
    {
        id_begin = qs.indexOf("\n", find_start);
        id_end = qs.indexOf("\n", id_begin + 1);
        find_start = id_end;
        proPID = qs.mid(id_begin + 1, id_end - id_begin - 1);
        proPID.toInt(&ok);
        if(!ok) break;
        // qDebug() << proPID;

        QFile procFile;
        procFile.setFileName("/proc/" + proPID + "/stat");
        // QString errorInfo = "/proc/" + proPID + "/stat" + " can not open";
        if(!procFile.open(QIODevice::ReadOnly))
        {
            // QMessageBox::warning(this, tr("warning"), errorInfo, QMessageBox::Yes);
            continue;
        }
        ++total;

        QString proName, proSta, proPPID, proPri, proMem;
        QString nametmp, memtmp, tmpstr;
        tmpstr = procFile.readLine();
        procFile.close();
        if(tmpstr.length() == 0) break;
        // qDebug() << tmpstr;

        nametmp = tmpstr.section(' ', 1, 1);
        proName = nametmp.mid(1, nametmp.length() - 2);
        // qDebug() << proName;

        proSta = tmpstr.section(' ', 2, 2);
        // qDebug() << proSta;
        if(proSta == "R") ++run;
        else if(proSta == "S") ++sleep;
        else if(proSta == "I") ++io;
        else if(proSta == "Z") ++zombie;

        proPPID = tmpstr.section(' ', 3, 3);
        // qDebug() << proPPID;

        proPri = tmpstr.section(' ', 17, 17);
        // qDebug() << proPri;

        memtmp = tmpstr.section(' ', 22, 22);
        proMem = QString::number(memtmp.toUInt() / 1024) + "KB";
        // qDebug() << proMem;

        model_pro->setItem(total - 1, 0, new QStandardItem(QString::fromLocal8Bit(proPID.toLatin1().data())));
        model_pro->setItem(total - 1, 1, new QStandardItem(QString::fromLocal8Bit(proName.toLatin1().data())));
        model_pro->setItem(total - 1, 2, new QStandardItem(QString::fromLocal8Bit(proSta.toLatin1().data())));
        model_pro->setItem(total - 1, 3, new QStandardItem(QString::fromLocal8Bit(proPPID.toLatin1().data())));
        model_pro->setItem(total - 1, 4, new QStandardItem(QString::fromLocal8Bit(proPri.toLatin1().data())));
        model_pro->setItem(total - 1, 5, new QStandardItem(QString::fromLocal8Bit(proMem.toLatin1().data())));

    }
    // ui->tableView_pro->resizeColumnsToContents();
    ui->tableView_pro->setShowGrid(false);
    ui->tableView_pro->setEditTriggers(QTableWidget::NoEditTriggers);
    ui->tableView_pro->setSelectionBehavior(QTableWidget::SelectRows);
    ui->tableView_pro->setSelectionMode(QAbstractItemView::SingleSelection);

    // qDebug("total:%d, run:%d, sleep:%d, io:%d, zombie:%d", total, run, sleep, io, zombie);
    ui->pro_total->setText(QString::number(total));
    ui->pro_run->setText(QString::number(run));
    ui->pro_sleep->setText(QString::number(sleep));
    ui->pro_io->setText(QString::number(io));
    ui->pro_zom->setText(QString::number(zombie));
}

void Widget::timeInfo()
{
    QString str;
    QFile procFile;
    procFile.setFileName("/proc/uptime");
    if(!procFile.open(QIODevice::ReadOnly))
    {
        QMessageBox::warning(this, tr("warning"), tr("/proc/uptime can not open!"), QMessageBox::Yes);
        return;
    }
    str = procFile.readLine();
    procFile.close();
    auto strlist = str.split(" ");
    strlist[0].chop(3);
    int run_secs = strlist[0].toUInt();
    int day = run_secs / 86400;
    int hour = (run_secs % 86400) / 3600;
    int min = (run_secs % 3600) / 60;
    int sec = run_secs % 60;
    char *run_buf = new char[16];
    sprintf(run_buf, "%02d %02d:%02d:%02d", day, hour, min, sec);
    QString runTime = QString(run_buf);
    delete []run_buf;
    // qDebug() <<runTime;
    ui->run_time->setText(runTime);

    QDateTime time = QDateTime::currentDateTime();
    // qDebug() << curTime;
    QString curTime = time.toString("yyyy MM dd hh:mm:ss");
    ui->cur_time->setText(curTime);
    // qDebug() << curTime;

    /* statusbar current time */
    ui->label_time->setText("Time: " + curTime);

    strlist = curTime.split(" ");
    // qDebug() << strlist;
    auto timelist = strlist[3].split(":");
    // qDebug() << timelist;
    int cur_secs = timelist[0].toUInt() * 3600 + timelist[1].toUInt() * 60 + timelist[2].toUInt();
    int boot_secs = cur_secs - run_secs;
    hour = (boot_secs % 86400) / 3600;
    min = (boot_secs % 3600) / 60;
    sec = boot_secs % 60;
    int boot_day = strlist[2].toUInt() - day;
    char *boot_buf = new char[16];
    sprintf(boot_buf, "%02d %02d:%02d:%02d", boot_day, hour, min, sec);
    QString bootTime = strlist[0] + " " + strlist[1] + " " + QString(boot_buf);
    // qDebug() << bootTime;
    delete []boot_buf;
    ui->boot_time->setText(bootTime);
}

void Widget::memInfo()
{
    QProcess process;
    process.start("free", QStringList() << "-m");
    process.waitForFinished();
    process.readLine();
    QString str = process.readLine();
    str.replace("\n", "");
    str.replace(QRegExp("( ){1,}"), " ");
    auto lst = str.split(" ");
    QString memInfo, memInfo_status;
    if(lst.size() > 6)
    {
        char *membuf = new char[64];
        sprintf(membuf, "Memory: %.01lfMB(total), %.01lfMB(used), %.01lfMB(free)", lst[1].toDouble(), lst[2].toDouble(), lst[6].toDouble());
        memInfo_status = QString(membuf);
        sprintf(membuf, "Total: %.01lfMB, Used: %.01lfMB, Free: %.01lfMB", lst[1].toDouble(), lst[2].toDouble(), lst[6].toDouble());
        memInfo = QString(membuf);
        delete []membuf;
        // qDebug() << memInfo;
    }
    ui->progressBar->setValue(lst[2].toDouble() * 100 / lst[1].toDouble());
    ui->memoryInfo->setText(memInfo);
    ui->label_mem->setText(memInfo_status);
}

void Widget::findPro()
{
    QString strPid = ui->lineEditPid->text();
    ui->lineEditPid->clear();
    QString strName = ui->lineEditName->text();
    ui->lineEditName->clear();
    if((strPid.length() == 0) && (strName.length() == 0))
    {
        QMessageBox::about(this, tr("Find"), "Please input something!");
        return;
    }
    if((strPid.length() > 0) && (strName.length() > 0))
    {
        QMessageBox::about(this, tr("Find"), "Please just input one editline!");
        return;
    }

    QList<QStandardItem *> itemlist;
    if(strPid.length() > 0) itemlist = model_pro->findItems(strPid);
    else itemlist = model_pro->findItems(strName, Qt::MatchExactly, 1);
    if(itemlist.length() == 0)
    {
        QMessageBox::warning(this, tr("Find"), "The process does not exist!");
        return;
    }
    // qDebug() << itemlist;

    QStringList listFind;
    listFind.append("PID\tName\tStatus\tPPID\tPriority\tMemory");
    for(int i = 0; i < itemlist.length(); i++)
    {
        // qDebug() << itemlist.at(i)->text();
        int row = itemlist.at(i)->row();
        if(i == 0) ui->tableView_pro->selectRow(row);
        QString strFind = "";
        for (int j = 0; j < 6; j++)
            strFind += model_pro->item(row, j)->text() + "\t";
        listFind.append(strFind);
    }
    // qDebug() << listFind;
    QString qs = listFind.join("\n");
    // qDebug() << qs;
    QMessageBox::information(this, tr("Find"), qs);
}

void Widget::killPro()
{
    QString strPid = ui->lineEditPid->text();
    ui->lineEditPid->clear();
    QString strName = ui->lineEditName->text();
    ui->lineEditName->clear();
    if((strPid.length() == 0) && (strName.length() == 0))
    {
        QMessageBox::about(this, tr("Kill"), "Please input something!");
        return;
    }
    if((strPid.length() > 0) && (strName.length() > 0))
    {
        QMessageBox::about(this, tr("Kill"), "Please just input one editline!");
        return;
    }

    QList<QStandardItem *> itemlist;
    if(strPid.length() > 0) itemlist = model_pro->findItems(strPid);
    else itemlist = model_pro->findItems(strName, Qt::MatchExactly, 1);
    if(itemlist.length() == 0)
    {
        QMessageBox::warning(this, tr("Kill"), "The process does not exist!");
        return;
    }
    // qDebug() << itemlist;

    if(itemlist.length() > 1)
    {
        QStringList listKill;
        listKill.append("PID\tName\tStatus\tPPID\tPriority\tMemory");
        for(int i = 0; i < itemlist.length(); i++)
        {
            // qDebug() << itemlist.at(i)->text();
            int row = itemlist.at(i)->row();
            if(i == 0) ui->tableView_pro->selectRow(row);
            QString strKill = "";
            for (int j = 0; j < 6; j++)
                strKill += model_pro->item(row, j)->text() + "\t";
            listKill.append(strKill);
        }
        listKill.append("Please input the PID to kill the process!");
        // qDebug() << listFind;
        QString qs = listKill.join("\n");
        // qDebug() << qs;
        QMessageBox::information(this, tr("Kill"), qs);
    }
    else
    {
        QString killPid = model_pro->item(itemlist.at(0)->row(), 0)->text();
        QString killSta = model_pro->item(itemlist.at(0)->row(), 2)->text();
        if(killSta == "Z")
        {
            QString killPpid = model_pro->item(itemlist.at(0)->row(), 3)->text();
            QMessageBox::warning(this, tr("Kill"), "PID " + killPid +
                                 " is a zoombie process\nkill its father process " +
                                 killPpid + " please!");
        }
        else
        {
            QProcess process;
            process.start("kill", QStringList() << killPid);
            process.waitForFinished();
            QMessageBox::about(this, tr("Kill"), "process with PID " + killPid + " has been killed!");
        }
    }
}
