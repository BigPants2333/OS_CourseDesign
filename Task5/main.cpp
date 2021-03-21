#include "widget.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Widget w;
    w.setWindowTitle("Filesystem Simulation");
    w.setWindowIcon(QIcon(":/icons/3000.png"));
    w.show();
    return a.exec();
}
