#ifndef EDIT_H
#define EDIT_H

#include <QDialog>
#include <QPushButton>
#include <QDebug>
#include <string.h>

namespace Ui {
class Edit;
}

class Edit : public QDialog
{
    Q_OBJECT

public:
    explicit Edit(QWidget *parent = nullptr);
    ~Edit();
    void setTextEdit(const char *buf);
    QString getText();
    void freeText();

private:
    void on_buttonBox_accepted();

private:
    Ui::Edit *ui;
    QString text;
};

#endif // EDIT_H
