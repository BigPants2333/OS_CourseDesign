#include "widget.h"
#include "ui_widget.h"

Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
    , dialog(0)
    , menu(0)
    , tableMenu(0)
{
    ui->setupUi(this);
    this->setFixedSize(this->width(), this->height());

    ui->treeWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->treeWidget, &QTreeWidget::customContextMenuRequested, this, &Widget::showMenu);

    ui->tableWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->tableWidget, &QTableWidget::customContextMenuRequested, this, &Widget::showTableMenu);
    connect(ui->tableWidget, &QTableWidget::doubleClicked, this, &Widget::tableDoubleClicked);

    QTableWidget &tablewidget = *ui->tableWidget;
    tablewidget.setColumnCount(5);
    tablewidget.horizontalHeader()->setStretchLastSection(true);
    tablewidget.verticalHeader()->setDefaultSectionSize(25);

    tablewidget.setShowGrid(false);
    tablewidget.verticalHeader()->setVisible(false);
    tablewidget.setSelectionMode(QAbstractItemView::ExtendedSelection);
    tablewidget.setSelectionBehavior(QAbstractItemView::SelectRows);
    tablewidget.setEditTriggers(QAbstractItemView::NoEditTriggers);
    tablewidget.setStyleSheet("selection-background-color:lightblue;");

    QStringList header;
    header << QString("Name") << QString("User") << QString("Mode") << QString("Type") << QString("Size");
    tablewidget.setHorizontalHeaderLabels(header);
    tablewidget.horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    tablewidget.horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    tablewidget.horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);
    tablewidget.horizontalHeader()->setSectionResizeMode(3, QHeaderView::Stretch);

    fs = new FileSystem;
    buildTree(fs->rootDir, "/", 0);
    showCurDir();
}

Widget::~Widget()
{
    delete ui;
    delete fs;
}

void Widget::buildTree(iNode *curDir, QString fileName, QTreeWidgetItem *parent)
{
    QTreeWidgetItem *treeItem;
    if(parent == 0)
    {
        ui->treeWidget->clear();
        treeItem = new QTreeWidgetItem(ui->treeWidget);
        treeItem->setText(0, "/");
        // treeItem->setText(1, "root");
    }
    else
    {
        if(curDir->i_type == MY_DIR && fileName[fileName.length() - 1] != '/')
        {
            fileName += "/";
        }
        treeItem = new QTreeWidgetItem();
        treeItem->setText(0, fileName);
        parent->addChild(treeItem);
    }
    if(curDir->i_type == MY_DIR)
    {
        for(int i = 0; i < MY_ADDR_NUM; ++i)
        {
            unsigned int blocknum = curDir->i_addr[i];
            if(blocknum >= 1 && blocknum <= block_num)
            {
                dirEntry *pe = (dirEntry *)fs->getBlockAddr(blocknum);
                for(int j = 0; j < search_range; ++j)
                {
                    int inode_num = pe->inode_num;
                    char *filename = pe->fileName;
                    if(inode_num > 0)
                    {
                        buildTree(&fs->iTable.i[inode_num], filename, treeItem);
                        pe++;
                    }
                    else return;
                }
            }
        }
    }
    ui->treeWidget->expandAll();
}

void Widget::showCurDir()
{
    for(int i = ui->tableWidget->rowCount() - 1; i >= 0; --i)
        ui->tableWidget->removeRow(i);
    m_curDir = fs->checkPath();
    // qDebug() << m_curDir;
    ui->curDir->setText(m_curDir);
    iNode *curDir = fs->curDir;
    QString fileName = " . .";
    showFile(fileName, 0, "", -1, 0);
    for(int i = 0; i < MY_ADDR_NUM; ++i)
    {
        unsigned int blocknum = curDir->i_addr[i];
        if(blocknum >= 1 && blocknum <= block_num)
        {
            dirEntry *pe = (dirEntry *)fs->getBlockAddr(blocknum);
            for(int j = 0; j < search_range; ++j)
            {
                int inode_num = pe->inode_num;
                fileName = pe->fileName;
                if(inode_num > 0)
                {
                    iNode *inode = &fs->iTable.i[inode_num];
                    showFile(fileName, inode->i_uid, inode->i_mode, inode->i_type, inode->i_size);
                    pe++;
                }
                else return;
            }
        }
    }
}

void Widget::showFile(QString &fileName, short uid, QString mode, short type, long size)
{
    QTableWidget *tablewidget = ui->tableWidget;
    int row_count = tablewidget->rowCount();
    tablewidget->insertRow(row_count);
    if(type == -1)
    {
        QTableWidgetItem *item = new QTableWidgetItem();
        item->setText(fileName);
        item->setTextAlignment(Qt::AlignCenter);
        tablewidget->setItem(row_count, 0, item);
        return;
    }
    QTableWidgetItem *item0 = new QTableWidgetItem();
    QTableWidgetItem *item1 = new QTableWidgetItem();
    QTableWidgetItem *item2 = new QTableWidgetItem();
    QTableWidgetItem *item3 = new QTableWidgetItem();
    QTableWidgetItem *item4 = new QTableWidgetItem();
    item0->setText(fileName);
    item1->setText("root");
    item2->setText(mode);
    if(type == MY_DIR)
        item3->setText("folder");
    else
        item3->setText("file");
    item4->setText(QString::number(size) + "B");
    tablewidget->setItem(row_count, 0, item0);
    tablewidget->setItem(row_count, 1, item1);
    tablewidget->setItem(row_count, 2, item2);
    tablewidget->setItem(row_count, 3, item3);
    tablewidget->setItem(row_count, 4, item4);

    item0->setTextAlignment(Qt::AlignCenter);
    item1->setTextAlignment(Qt::AlignCenter);
    item2->setTextAlignment(Qt::AlignCenter);
    item3->setTextAlignment(Qt::AlignCenter);
    item4->setTextAlignment(Qt::AlignCenter);

    QBrush brush("gray");
    item1->setForeground(brush);
    item2->setForeground(brush);
    item3->setForeground(brush);
    item4->setForeground(brush);
}

void Widget::showMenu(const QPoint &pos)
{
    if(menu)
    {
        delete menu;
        menu = 0;
    }
    menu = new QMenu(ui->treeWidget);
    QAction *createDir = menu->addAction("New fol&der");
    QAction *createFile = menu->addAction("New &file");
    connect(createDir, &QAction::triggered, this, &Widget::newDir);
    connect(createFile, &QAction::triggered, this, &Widget::newFile);
    menu->exec(QCursor::pos());
}

void Widget::showTableMenu(const QPoint &pos)
{
    if(tableMenu)
    {
        delete tableMenu;
        tableMenu = 0;
    }
    tableMenu = new QMenu(ui->tableWidget);
    QAction *actionCreateFolder = new QAction(ui->tableWidget);
    QAction *actionCreateFile = new QAction(ui->tableWidget);
    QAction *actionRename = new QAction(ui->tableWidget);
    QAction *actionDelete = new QAction(ui->tableWidget);
    QAction *actionMove = new QAction(ui->tableWidget);
    QAction *actionCopy = new QAction(ui->tableWidget);

    actionCreateFolder->setText("New fol&der");
    actionCreateFile->setText("New &file");
    actionRename->setText("&Rename");
    actionDelete->setText("&Delete");
    actionMove->setText("&Move to...");
    actionCopy->setText("&Copy to...");

    connect(actionCreateFolder, &QAction::triggered, this, &Widget::newDirTable);
    connect(actionCreateFile, &QAction::triggered, this, &Widget::newFileTable);
    connect(actionRename, &QAction::triggered, this, &Widget::rename);
    connect(actionDelete, &QAction::triggered, this, &Widget::deleteFile);
    connect(actionMove, &QAction::triggered, this, &Widget::move);
    connect(actionCopy, &QAction::triggered, this, &Widget::copy);

    QTableWidgetItem *item = ui->tableWidget->itemAt(pos);
    if(item != 0)
    {
        tableMenu->addAction(actionRename);
        tableMenu->addAction(actionDelete);
        tableMenu->addAction(actionMove);
        tableMenu->addAction(actionCopy);
        m_fileName = ui->tableWidget->item(item->row(), 0)->text();
    }
    else
    {
        tableMenu->addAction(actionCreateFolder);
        tableMenu->addAction(actionCreateFile);
    }
    tableMenu->exec(QCursor::pos());
}

void Widget::move()
{
    dialog = new Dialog("Move to...", "Please input the new (absolute) path:");
    if(dialog->exec() == QDialog::Accepted)
    {
        QString pathstr = dialog->getName();
        if(pathstr.at(pathstr.length() - 1) == '/') pathstr = pathstr.left(pathstr.length() - 1);
        // qDebug() << pathstr << m_curDir;
        int pos1 = pathstr.lastIndexOf('/');
        int pos2 = m_curDir.lastIndexOf('/');
        if(pos1 > pos2 && pathstr.left(m_curDir.length()) == m_curDir)
            QMessageBox::warning(this, tr("Move to..."), tr("Failed!Please try again!"), QMessageBox::Yes | QMessageBox::No);
        else if(fs->move(m_fileName.toLatin1().data(), dialog->getName().toLatin1().data()) == 0)
        {
            dialog->freeName();
            buildTree(fs->rootDir, "/", 0);
            showCurDir();
        }
    }
}

void Widget::copy()
{
    dialog = new Dialog("Copy to...", "Please input an absolute path:");
    if(dialog->exec() == QDialog::Accepted)
    {
        QString dirNew = dialog->getName();
        // fs->changeDir(dialog->getName().toLatin1().data()) == 0
        dialog->freeName();
        delete dialog;
        dialog = new Dialog("Copy to...", "Please input the name:");
        if(dialog->exec() == QDialog::Accepted &&
                fs->createFile(dialog->getName().toLatin1().data()) == 0)
        {
            if(fs->copy(m_fileName.toLatin1().data(), dialog->getName().toLatin1().data()) == 0)
            {
                dialog->freeName();
                if(fs->changeDir(dirNew.toLatin1().data()) == 0)
                {
                    buildTree(fs->rootDir, "/", 0);
                    showCurDir();
                }
            }
        }
    }
}

void Widget::rename()
{
    dialog = new Dialog("Rename", "Please input the new name:");
    if(dialog->exec() == QDialog::Accepted &&
                fs->rename(m_fileName.toLatin1().data(), dialog->getName().toLatin1().data()) == 0)
        {
            dialog->freeName();
            buildTree(fs->rootDir, "/", 0);
            showCurDir();
        }
}

void Widget::deleteFile()
{
    // qDebug() << m_fileName;
    if(fs->removeFile(m_fileName.toLatin1().data()) == 0)
    {
        showCurDir();
        buildTree(fs->rootDir, "/", 0);
    }
}

void Widget::newDirTable()
{
    dialog = new Dialog("New folder", "Please input the folder name:");
    if(dialog->exec() == QDialog::Accepted &&
            fs->createDir(dialog->getName().toLatin1().data()) == 0)
    {
        dialog->freeName();
        buildTree(fs->rootDir, "/", 0);
        showCurDir();
        ui->treeWidget->expandAll();
    }
}

void Widget::newFileTable()
{
    dialog = new Dialog("New file", "Please input the file name:");
    if(dialog->exec() == QDialog::Accepted &&
            fs->createFile(dialog->getName().toLatin1().data()) == 0)
    {
        qDebug() << dialog->getName();
        dialog->freeName();
        buildTree(fs->rootDir, "/", 0);
        showCurDir();
    }
}

void Widget::newDir()
{
    dialog = new Dialog("New folder", "Please input an absolute path:");
    if(dialog->exec() == QDialog::Accepted &&
            fs->changeDir(dialog->getName().toLatin1().data()) == 0)
    {
        dialog->freeName();
        delete dialog;
        dialog = new Dialog("New folder", "Please input the name:");
        if(dialog->exec() == QDialog::Accepted &&
                fs->createDir(dialog->getName().toLatin1().data()) == 0)
        {
            dialog->freeName();
            buildTree(fs->rootDir, "/", 0);
            showCurDir();
            ui->treeWidget->expandAll();
        }
    }
}

void Widget::newFile()
{
    dialog = new Dialog("New file", "Please input an absolute path:");
    if(dialog->exec() == QDialog::Accepted &&
            fs->changeDir(dialog->getName().toLatin1().data()) == 0)
    {
        dialog->freeName();
        delete dialog;
        dialog = new Dialog("New file", "Please input the name:");
        if(dialog->exec() == QDialog::Accepted &&
                fs->createFile(dialog->getName().toLatin1().data()) == 0)
        {
            dialog->freeName();
            buildTree(fs->rootDir, "/", 0);
            showCurDir();
        }
    }
}

void Widget::tableDoubleClicked(QModelIndex index)
{
    int row = index.row();
    QString fileName = ui->tableWidget->item(row, 0)->text();
    // char *fileName = file_name.toLatin1().data();
    // qDebug() << fileName;
    if(fileName == " . ." && fs->curDir == fs->rootDir)
    {
        return;
    }
    if(fileName == " . ." && fs->changeDir("..") == 0)
    {
        showCurDir();
        return;
    }

    for(int i = 0; i < MY_ADDR_NUM; ++i)
    {
        dirEntry *pe = (dirEntry *)fs->getBlockAddr(fs->curDir->i_addr[i]);
        for(int j = 0; j < search_range; ++j)
        {
            if(pe->inode_num == 0) return;
            if(strcmp(pe->fileName, fileName.toLatin1().data()) == 0)
            {
                iNode &inode = fs->iTable.i[pe->inode_num];
                if(inode.i_type == MY_DIR)
                {
                    if(fs->changeDir(fileName.toLatin1().data()) == 0)
                    {
                        showCurDir();
                        // qDebug() << fs->checkPath();
                    }
                }
                else
                {
                    fs->open(fileName.toLatin1().data());
                    showCurDir();
                }
                return;
            }
            pe++;
        }
    }
}

