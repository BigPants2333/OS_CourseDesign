#ifndef FILESYSTEM_H
#define FILESYSTEM_H

/*
 * Apply for a file sized 100MB
 * Block size is 512B
 * Manage disk with bitmap
 * Continuous distribution
 */

#include <string.h>
#include <bits/stdc++.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <QDebug>
#include "edit.h"

#ifndef MY_ADDR_NUM
#define MY_ADDR_NUM 8
#endif

#ifndef MY_DIR
#define MY_DIR 0
#endif

#ifndef MY_FILE
#define MY_FLIE 1
#endif

#ifndef MY_FILE_HEADER
#define MY_FILE_HEADER 16
#endif

#ifndef MY_PATH_NUM
#define MY_PATH_NUM 100
#endif

#ifndef FILESYSNAME
#define FILESYSNAME	"./test.txt"
#endif

using namespace std;

const unsigned int max_file_size = 100 * 0x100000;
const unsigned int block_size = 512;
const unsigned int block_num = max_file_size / block_size;
const unsigned int bit_map_size = block_num;
const unsigned int fs_size = max_file_size + bit_map_size;
const int my_file_length = MY_FILE_HEADER - sizeof(short);
const int search_range = block_size / MY_FILE_HEADER;

/* inode structure */
typedef struct
{
    short i_uid;                    // mark different users
    short i_type;                   // mark file type a common file or a dir
    char i_mode[9];                 // stores string like "rwxr--r--"
    short i_ilink;                  // link nums to this inode
    long i_size;                    // up to MY_ADDR_NUM * block_size
    int i_addr[MY_ADDR_NUM];        // stores block nums

}iNode;

/* inode table structure */
typedef struct iTbl
{
    int num;
    iNode i[block_num];
    iTbl(){ num = 0; }
}iNodeTable;

/* stores file info */
typedef struct
{
    short inode_num;                    // in inode table
    char fileName[my_file_length];
}dirEntry;

typedef struct
{
    dirEntry *pe[MY_PATH_NUM];          // a path can hold dir num up to MY_PATH_NUM
    int cur;                            // mark cur dir
}myPath;

class FileSystem
{
public:
    FileSystem();
    virtual ~FileSystem();

    int getBlock(int blockSize);                    // block distribution
    char *getBlockAddr(int blockNum);               // return blockNum's address
    int getAddrBlock(const char *addr);             // return addr's blocknum
    int releaseBlock(int blockNum, int blockSize);  // set the block's bit '0'

    int createFile(const char *fileName);
    int removeFile(const char *fileName);
    int removeINode(iNode *inode);

    int createDir(const char *dirName);
    int changeDir(const char *dirName);

    int rename(const char *oldName, const char *newName);
    int move(const char *fileName, const char *path);
    int open(const char *fileName);
    int copy(const char *oldFile, const char *newFile);

    QString checkPath();

public:
    iNodeTable iTable;
    iNode *rootDir;
    iNode *curDir;
    myPath path;

private:
    char *const systemStartAddr;
    char *const bitmapStartAddr;
    char *const blockStartAddr;
    Edit *myEdit;
};

#endif // FILESYSTEM_H
