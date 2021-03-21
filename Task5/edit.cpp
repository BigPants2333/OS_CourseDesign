#include "edit.h"
#include "ui_edit.h"

Edit::Edit(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Edit)
{
    ui->setupUi(this);
    this->setFixedSize(this->width(), this->height());
    ui->buttonBox->button(QDialogButtonBox::Ok)->setText("&Save");
    ui->buttonBox->button(QDialogButtonBox::Cancel)->setText("&Close");

    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &Edit::on_buttonBox_accepted);
}

Edit::~Edit()
{
    delete ui;
}

void Edit::on_buttonBox_accepted()
{
    text = ui->textEdit->toPlainText();
}

void Edit::setTextEdit(const char *buf)
{
    ui->textEdit->append(buf);
}

QString Edit::getText()
{
    return text;
}

void Edit::freeText()
{
    text.clear();
}
