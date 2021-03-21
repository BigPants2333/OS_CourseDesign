#ifndef DIALOG_H
#define DIALOG_H

#include <QDialog>

namespace Ui {
class Dialog;
}

class Dialog : public QDialog
{
    Q_OBJECT

public:
    explicit Dialog(QString windowText, QString prompt, QWidget *parent = nullptr);
    ~Dialog();
    QString getName();
    void freeName();

private:
    void on_buttonBox_accepted();

private:
    Ui::Dialog *ui;
    QString name;
};

#endif // DIALOG_H
