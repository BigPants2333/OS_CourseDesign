#include "mainwindow.h"
#include "rescpu.h"
#include "resmem.h"
#include "resdisk.h"
#include "resnet.h"
#include <QApplication>
#include <unistd.h>

int main(int argc, char *argv[])
{
    int pid;
    if((pid = fork()) == 0)
    {
        QApplication a(argc, argv);
        resCpu cpu;
        cpu.setWindowTitle("CPU Monitor");
        cpu.setWindowIcon(QIcon(":/icon/3000.png"));
        cpu.show();
        return a.exec();
    }
    else if((pid = fork()) == 0)
    {
        QApplication a(argc, argv);
        resMem mem;
        mem.setWindowTitle("Mem Monitor");
        mem.setWindowIcon(QIcon(":/icon/5012.png"));
        mem.show();
        return a.exec();
    }
    else if((pid = fork()) == 0)
    {
        QApplication a(argc, argv);
        resDisk disk;
        disk.setWindowTitle("Disk Monitor");
        disk.setWindowIcon(QIcon(":/icon/7004.png"));
        disk.show();
        return a.exec();
    }
    else if((pid = fork()) == 0)
    {
        QApplication a(argc, argv);
        resNet net;
        net.setWindowTitle("Net Monitor");
        net.setWindowIcon(QIcon(":/icon/1007.png"));
        net.show();
        return a.exec();
    }
    QApplication a(argc, argv);
    MainWindow w;
    w.setWindowTitle("Time Monitor");
    w.setWindowIcon(QIcon(":/icon/8006.png"));
    w.show();
    return a.exec();
}
