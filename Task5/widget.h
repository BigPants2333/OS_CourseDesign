#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QMenu>
#include <QMessageBox>
#include <QTreeWidgetItem>
#include <QDebug>
#include <string.h>

#include "filesystem.h"
#include "dialog.h"

QT_BEGIN_NAMESPACE
namespace Ui { class Widget; }
QT_END_NAMESPACE

class Widget : public QWidget
{
    Q_OBJECT

public:
    Widget(QWidget *parent = nullptr);
    ~Widget();

    void showMenu(const QPoint &pos);
    void showTableMenu(const QPoint &pos);
    void tableDoubleClicked(QModelIndex index);

    void newDir();
    void newFile();
    void newDirTable();
    void newFileTable();

    void deleteFile();
    void rename();
    void move();
    void copy();

private:
    void buildTree(iNode *curDir, QString fileName, QTreeWidgetItem *parent);
    void showCurDir();
    void showFile(QString &fileName, short uid, QString mode, short type, long size);

private:
    Ui::Widget *ui;
    FileSystem *fs;
    Dialog *dialog;
    QMenu *menu;
    QMenu *tableMenu;
    QString m_fileName;
    QString m_curDir;
};
#endif // WIDGET_H
