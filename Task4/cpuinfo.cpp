#include "cpuinfo.h"

CpuInfo::CpuInfo(QWidget *parent) : QWidget(parent), m_widget(parent)
{

}

CpuInfo::~CpuInfo()
{

}

void CpuInfo::updateCpuInfo()
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
            char *cpubuf = new char[8];
            sprintf(cpubuf, "%.2lf", m_cpu_rate);
            cpuUsage = QString(cpubuf);
            // qDebug() << cpuUse;

            cpuVec.push_front(m_cpu_rate);
            if(cpuVec.count() > MY_SECONDS) cpuVec.pop_back();
            // qDebug() << cpuVec;
        }
    }
    update();
}

QString CpuInfo::getCpuUsage()
{
    return cpuUsage;
}

void CpuInfo::paintEvent(QPaintEvent *)
{
    QPainter painter;
    painter.begin(this);
    painter.drawImage(this->x(), this->y(), drawImage(m_widget));
}

QImage CpuInfo::drawImage(QWidget *widget)
{
    QImage image = QImage(widget->width(), widget->height(), QImage::Format_RGB32);
    // qDebug() << widget->x() << widget->y() << widget->width() << widget->height();
    QColor backColor = qRgb(255, 255, 255);
    image.fill(backColor);

    // qDebug() << "cpuVec.count() > 0";
    QPainter *painter = new QPainter(&image);
    painter->setRenderHint(QPainter::Antialiasing, true);

    int pointx = widget->width() - MY_SPACE_X;
    int pointy = widget->height() - MY_SPACE_Y;
    int width = widget->width() - MY_SPACE_X * 2;
    int height = widget->height() - MY_SPACE_Y * 2;
    double increment = width / (MY_SECONDS / 10);

    QPen penRect;
    penRect.setColor(Qt::gray);
    penRect.setWidth(1);
    painter->setPen(penRect);
    painter->drawRect(MY_SPACE_X, MY_SPACE_Y, width , height);

    QPen penSplit;
    penSplit.setColor(Qt::gray);
    penSplit.setWidth(1);
    penSplit.setStyle(Qt::DotLine);
    painter->setPen(penSplit);
    painter->drawLine(pointx, pointy - height / 2, pointx - width, pointy - height / 2);
    for(int i = 1; i < (int)(MY_SECONDS / 10); i++)
        painter->drawLine(pointx - increment * i, pointy, pointx - increment * i, pointy - height);

    double kx = (double)width / (MY_SECONDS - 1);
    double ky = (double)height / 100;

    QPen penText;
    penText.setColor(Qt::black);
    penText.setWidth(2);
    painter->setPen(penText);
    for(int i = 0; i < 3; i++)
        painter->drawText(pointx + 5, pointy - ky * 50 * i + 5, QString::number(50 * i));
    painter->drawText(pointx - width - 20, pointy - height + 5, QString("%"));
    for(int i = 1; i <= (int)(MY_SECONDS / 10); i++)
        painter->drawText(pointx - i * increment - 10, pointy + 15, QString::number(10 * i));
    painter->drawText(pointx - width + 15, pointy + 15, QString("Seconds"));

    if(cpuVec.count() > 0)
    {
        QPen penLink, penPoint;
        penLink.setColor(Qt::black);
        penLink.setWidth(2);
        penPoint.setColor(Qt::red);
        penPoint.setWidth(0);
        for(int i = 0; i < cpuVec.count(); i++)
        {
            painter->setPen(penPoint);
            painter->drawPoint(pointx - kx * i, pointy - cpuVec.at(i) * ky);
            if(i > 0)
            {
                painter->setPen(penLink);
                painter->drawLine(pointx - kx * i, pointy - cpuVec.at(i) * ky,
                                  pointx - kx * (i - 1), pointy - cpuVec.at(i - 1) * ky);
            }

        }
        painter->drawPoint(pointx - kx * (cpuVec.count() - 1),
                           pointy - cpuVec.at(cpuVec.count() - 1) * ky);
    }
    delete painter;
    return image;
}
