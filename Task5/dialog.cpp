#include "dialog.h"
#include "ui_dialog.h"

Dialog::Dialog(QString windowText, QString prompt, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Dialog)
{
    ui->setupUi(this);
    setWindowTitle(windowText);
    ui->label->setText(prompt);
    this->setFixedSize(this->width(), this->height());

    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &Dialog::on_buttonBox_accepted);
}

Dialog::~Dialog()
{
    delete ui;
}

void Dialog::on_buttonBox_accepted()
{
    name = ui->lineEdit->text();
}

QString Dialog::getName()
{
    return name;
}

void Dialog::freeName()
{
    name.clear();
}
