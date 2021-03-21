#include "filesystem.h"

FileSystem::FileSystem(): systemStartAddr((char *)malloc(fs_size * sizeof(char)))
  , bitmapStartAddr(systemStartAddr)
  , blockStartAddr(systemStartAddr + bit_map_size)
{
    memset(systemStartAddr, 0, fs_size);
    FILE *fs = fopen(FILESYSNAME, "r");
    if(fs == 0)
    {
        qDebug() << "Filesystem first opened";
        for(unsigned int i = 0; i < bit_map_size; ++i)
            bitmapStartAddr[i] = '0';
        qDebug() << "Creating root directory...";
        iNode &inode = iTable.i[iTable.num++];
        inode.i_uid = 0;
        inode.i_type = MY_DIR;
        strncpy(inode.i_mode, "rwxr--r--", 9);
        inode.i_ilink = 1;
        inode.i_size = 0;
        inode.i_addr[0] = getBlock(1);
        curDir = rootDir = &inode;
        path.cur = 0;
        qDebug() << "Done";
    }
    else
    {
        qDebug() << "Filesystem initialized...";
        fscanf(fs, "%d", &iTable.num);
        // qDebug() << iTable.num;
        for(int i = 0; i < iTable.num; ++i)
        {
            iNode &inode = iTable.i[i];
            fscanf(fs, "%hd%hd%s%hd%ld", &inode.i_uid, &inode.i_type, inode.i_mode, &inode.i_ilink, &inode.i_size);
            for(int j = 0; j < MY_ADDR_NUM; ++j) fscanf(fs, "%d", &inode.i_addr[j]);
        }
        fseek(fs, 1, SEEK_CUR);
        for(unsigned int i = 0; i < bit_map_size; ++i)
            fscanf(fs, "%c", bitmapStartAddr + i);
        fread(blockStartAddr, block_size, block_num, fs);
        fclose(fs);
        curDir = rootDir = &iTable.i[0];
        path.cur = 0;
        qDebug() << "Done";
    }
}

FileSystem::~FileSystem()
{
    FILE *fs = fopen(FILESYSNAME, "w");
    qDebug() << "Filesystem saving changes...";
    fprintf(fs, "%d ", iTable.num);
    for(int i = 0; i < iTable.num; ++i)
    {
        iNode &inode = iTable.i[i];
        fprintf(fs, "%hd %hd %s %hd %ld ", inode.i_uid, inode.i_type, inode.i_mode, inode.i_ilink, inode.i_size);
        for(int j = 0; j < MY_ADDR_NUM; ++j) fprintf(fs, "%d ", inode.i_addr[j]);
    }
    for(unsigned int i = 0; i < bit_map_size; ++i)
        fprintf(fs, "%c", bitmapStartAddr[i]);
    fwrite(blockStartAddr, block_size, block_num, fs);
    fclose(fs);
    qDebug() << "Done";
    qDebug() << "Filesystem closed";
    free(systemStartAddr);
}

int FileSystem::getBlock(int blockSize)
{
    int start = 0;
    int counter = 0;
    for(unsigned int i = 1; i <= block_num; ++i)
    {
        if(bitmapStartAddr[i - 1] == '0')
        {
            if(counter == 0)
                start = i;
            counter++;
            if(counter == blockSize)
            {
                for(int j = start; j < start + blockSize; ++j)
                {
                    bitmapStartAddr[j - 1] = '1';
                    memset(getBlockAddr(j), 0, block_size);
                }
                // qDebug() << start;
                return start;
            }
        }
    }
    qDebug() << "Disk storage is not enough";
    return -1;
}

char *FileSystem::getBlockAddr(int blockNum)
{
    return blockStartAddr + (blockNum - 1) * block_size;
}

int FileSystem::getAddrBlock(const char *addr)
{
    return (addr - blockStartAddr) / block_size + 1;
}

int FileSystem::releaseBlock(int blockNum, int blockSize)
{
    int endBlock = blockNum + blockSize;
    for(int i = blockNum; i < endBlock; ++i)
    {
        bitmapStartAddr[i - 1] = '0';
        // memset(getBlockAddr(i), 0, block_size);
    }
    return 0;
}

int FileSystem::createFile(const char *fileName)
{
    iNode &inode = iTable.i[iTable.num++];
    inode.i_uid = 0;
    inode.i_type = MY_FLIE;
    strncpy(inode.i_mode, "rwxr-xr-x", 9);
    inode.i_ilink = 1;
    inode.i_size = 0;
    inode.i_addr[0] = getBlock(1);
    for(int i = 0; i < MY_ADDR_NUM; ++i)
    {
        dirEntry *pe = (dirEntry *)getBlockAddr(curDir->i_addr[i]);
        for(int j = 0; j < search_range; ++j)
        {
            if(pe->inode_num == 0)
            {
                strncpy(pe->fileName, fileName, my_file_length);
                pe->inode_num = iTable.num - 1;
                return 0;
            }
            pe++;
        }
    }
    return -1;
}

int FileSystem::removeFile(const char *fileName)
{
    qDebug() << "Removing " << fileName;
    dirEntry *pe, *pl, *p;
    pe = pl = p = 0;
    bool done = false;
    for(int i = 0; i < MY_ADDR_NUM; ++i)
    {
        p = (dirEntry *)getBlockAddr(curDir->i_addr[i]);
        for(int j = 0; j < search_range; ++j)
        {
            if(p->inode_num == 0)
            {
                pl = p - 1;
                done = true;
                break;
            }
            if(strcmp(p->fileName, fileName) == 0)
                pe = p;
            p++;
        }
        if(done) break;
    }
    if(pe && pl)
    {
        iNode *inode = &iTable.i[pe->inode_num];
        removeINode(inode);
        pe->inode_num = pl->inode_num;
        strncpy(pe->fileName, pl->fileName, my_file_length);
        pl->inode_num = 0;
        return 0;
    }
    else
    {
        qDebug() << fileName << " does not exist";
        return -1;
    }
}

int FileSystem::removeINode(iNode *inode)
{
    bool isDir = false;
    if(inode->i_type == MY_DIR)
    {
        dirEntry *p;
        for(int i = 0; i < MY_ADDR_NUM; ++i)
        {
            p = (dirEntry *)getBlockAddr(inode->i_addr[i]);
            for(int j = 0; j < search_range; ++j)
            {
                if(p->inode_num == 0)
                {
                    isDir = true;
                    break;
                }
                iNode *itmp = &iTable.i[p->inode_num];
                removeINode(itmp);
                p++;
            }
            if(isDir) break;
        }
    }
    for(int i = 0; i < MY_ADDR_NUM; ++i)
    {
        if(inode->i_addr[i] > 0)
            releaseBlock(inode->i_addr[i], 1);
        else break;
    }
    return 0;
}

int FileSystem::createDir(const char *dirName)
{
    QString dirtmp = dirName;
    if(dirtmp.at(dirtmp.length() - 1) == '/') dirtmp = dirtmp.left(dirtmp.length() - 1) + '\0';
    dirName = dirtmp.toLatin1().data();
    iNode &inode = iTable.i[iTable.num++];
    inode.i_uid = 0;
    inode.i_type = MY_DIR;
    strncpy(inode.i_mode, "rwxr-xr-x", 9);
    inode.i_ilink = 1;
    inode.i_size = 0;
    inode.i_addr[0] = getBlock(1);
    for(int i = 0; i < MY_ADDR_NUM; ++i)
    {
        dirEntry *pe = (dirEntry *)getBlockAddr(curDir->i_addr[i]);
        for(int j = 0; j < search_range; ++j)
        {
            if(pe->inode_num == 0)
            {
                strncpy(pe->fileName, dirName, my_file_length);
                pe->inode_num = iTable.num - 1;
                return 0;
            }
            pe++;
        }
    }
    return -1;
}

int FileSystem::changeDir(const char *dirName)
{
    qDebug() << dirName;
    const char *p = dirName;
    const char *q;
    if(strcmp(dirName, ".") == 0)
        return 0;
    if(strcmp(dirName, "..") == 0)
    {
        path.cur--;
        curDir = &iTable.i[path.pe[path.cur - 1]->inode_num];
        return 0;
    }
    if(dirName[0] == '/')
    {
        curDir = rootDir;
        path.cur = 0;
        p++;
        if(*p == '\0') return 0;
        goto changeDirLabel;
    }
    else
    {
        // if(strcmp(dirName, ".") == 0) return 0;
        if(strncmp(dirName, "./", 2) == 0) p += 2;

changeDirLabel:
        q = strchr(p, '/');
        if(q == 0)
            q = dirName + strlen(dirName);
        for(int i = 0; i < MY_ADDR_NUM; ++i)
        {
            dirEntry *pe = (dirEntry *)getBlockAddr(curDir->i_addr[i]);
            for(int j = 0; j < search_range; ++j)
            {
                if(pe->inode_num == 0) return -1;
                if(strncmp(pe->fileName, p, q-p) == 0)
                {
                    path.pe[path.cur++] = pe;
                    curDir = &iTable.i[pe->inode_num];
                    p = *q ? q + 1 : q;
                    if(*p == '\0') return 0;
                    goto changeDirLabel;
                }
            }
            pe++;
        }
        return -1;
    }
}

int FileSystem::rename(const char *oldName, const char *newName)
{
    for(int i = 0; i < MY_ADDR_NUM; ++i)
    {
        dirEntry *pe = (dirEntry *)getBlockAddr(curDir->i_addr[i]);
        for(int j = 0; j < search_range; ++j)
        {
            if(pe->inode_num == 0)
                return -1;
            if(strcmp(pe->fileName, oldName) == 0)
            {
                strncpy(pe->fileName, newName, my_file_length);
                return 0;
            }
            pe++;
        }
    }
    return 1;
}

int FileSystem::move(const char *fileName, const char *path)
{
    dirEntry *pe, *pl, *p;
    pe = pl = p = 0;
    bool find = false;
    for(int i = 0; i < MY_ADDR_NUM; ++i)
    {
        p = (dirEntry *)getBlockAddr(curDir->i_addr[i]);
        for(int j = 0; j < search_range; ++j)
        {
            if(p->inode_num == 0)
            {
                pl = p - 1;
                find = true;
                break;
            }
            if(strcmp(p->fileName, fileName) == 0)
                pe = p;
            p++;
        }
        if(find) break;
    }
    if(pe && pl)
    {
        int i_num = pe->inode_num;
        pe->inode_num = pl->inode_num;
        strncpy(pe->fileName, pl->fileName, my_file_length);
        pl->inode_num = 0;
        changeDir(path);
        for(int i = 0; i < MY_ADDR_NUM; ++i)
        {
            dirEntry *q = (dirEntry *)getBlockAddr(curDir->i_addr[i]);
            for(int j = 0; j < search_range; ++j)
            {
                if(q->inode_num == 0)
                {
                    strncpy(q->fileName, fileName, my_file_length);
                    q->inode_num = i_num;
                    return 0;
                }
                q++;
            }
        }
        return 1;
    }
    else
    {
        qDebug() << fileName << " does not exist";
        return -1;
    }
}

int FileSystem::open(const char *fileName)
{
    iNode *inode;
    bool find = false;
    for(int i = 0; i < MY_ADDR_NUM; ++i)
    {
        dirEntry *pe = (dirEntry *)getBlockAddr(curDir->i_addr[i]);
        for(int j = 0; j < search_range; ++j)
        {
            if(pe->inode_num == 0) return -1;
            if(strcmp(pe->fileName, fileName) == 0)
            {
                inode = &iTable.i[pe->inode_num];
                find = true;
                break;
            }
            pe++;
        }
        if(find) break;
    }
    long left = inode->i_size;
    // qDebug() << "left:" << left;
    char buf[block_size + 1];
    memset(buf, 0, block_size + 1);
    myEdit = new Edit;
    myEdit->setWindowTitle(fileName);
    if(inode->i_size <= MY_ADDR_NUM * block_size)
    {
        for(int i = 0; i < MY_ADDR_NUM; ++i)
        {
            if(left == 0) break;
            if(left >= block_size)
            {
                strncpy(buf, getBlockAddr(inode->i_addr[i]), block_size);
                myEdit->setTextEdit(buf);
                memset(buf, 0, block_size + 1);
                left -= block_size;
            }
            else
            {
                strncpy(buf, getBlockAddr(inode->i_addr[i]), left);
                myEdit->setTextEdit(buf);
                memset(buf, 0, block_size + 1);
                left = 0;
            }
        }
    }

    if(myEdit->exec() == QDialog::Accepted)
    {
        left = myEdit->getText().length();
        inode->i_size = left;
        if(left <= MY_ADDR_NUM * block_size)
        {
            for(int i = 0; i < MY_ADDR_NUM; ++i)
            {
                if(left == 0) inode->i_addr[i] = 0;
                if(left >= block_size)
                {
                    if(inode->i_addr[i] == 0)
                        inode->i_addr[i] = getBlock(1);
                    strncpy(getBlockAddr(inode->i_addr[i]), myEdit->getText().toLatin1().data(), block_size);
                    left -= block_size;
                }
                else
                {
                    if(inode->i_addr[i] == 0)
                        inode->i_addr[i] = getBlock(1);
                    strncpy(getBlockAddr(inode->i_addr[i]), myEdit->getText().toLatin1().data(), left);
                    left = 0;
                }
            }
        }
        myEdit->freeText();
    }
    return 0;
}

int FileSystem::copy(const char *oldFile, const char *newFile)
{
    bool findOld = false;
    bool findNew = false;
    iNode *inodeOld, *inodeNew;
    for(int i = 0; i < MY_ADDR_NUM; ++i)
    {
        dirEntry *pe = (dirEntry *)getBlockAddr(curDir->i_addr[i]);
        for(int j = 0; j < search_range; ++j)
        {
            if(pe->inode_num == 0)
                return -1;
            if(strcmp(pe->fileName, oldFile) == 0)
            {
                findOld = true;
                inodeOld = &iTable.i[pe->inode_num];
            }
            if(strcmp(pe->fileName, newFile) == 0)
            {
                findNew = true;
                inodeNew = &iTable.i[pe->inode_num];
            }
            pe++;
            if(findOld && findNew) break;
        }
        if(findOld && findNew) break;
    }

    long left = inodeOld->i_size;
    if(inodeOld->i_size <= MY_ADDR_NUM * block_size)
    {
        for(int i = 0; i < MY_ADDR_NUM; ++i)
        {
            if(left == 0) break;
            if(left >= block_size)
            {
                strncpy(getBlockAddr(inodeNew->i_addr[i]), getBlockAddr(inodeOld->i_addr[i]), block_size);
                left -= block_size;
            }
            else
            {
                strncpy(getBlockAddr(inodeNew->i_addr[i]), getBlockAddr(inodeOld->i_addr[i]), left);
                left = 0;
            }
        }
    }
    return 0;
}

QString FileSystem::checkPath()
{
    QString pathStr = "/";
    for(int i = 0; i < path.cur; ++i)
    {
        QString tmp = path.pe[i]->fileName;
        tmp += "/";
        pathStr += tmp;
    }
    return pathStr;
}
