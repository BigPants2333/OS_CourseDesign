#ifndef HEAD_H
#define HEAD_H

#include<sys/types.h>
#include<sys/ipc.h>
#include<sys/shm.h>
#include<sys/sem.h>
#include<sys/stat.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<pthread.h>
#include<unistd.h>
#include<wait.h>
#include<fcntl.h>

#define BUFSIZE 256         // 缓冲区大小
#define BUFNUM 10           // 缓冲区个数

key_t semkey = 200;         // 创建信号灯的key
key_t bufkey1 = 0xa0a0;     // 创建共享缓冲区1的key
key_t bufkey2 = 0xa0d0;     // 创建共享缓冲区2的key

union semun{
    int val;
    struct semid_ds *buf;
    unsigned short *array;
    struct seminfo *__buf;
};

typedef struct sharebuf{    // 共享缓冲区的结构
    char buf[BUFNUM][BUFSIZE];  // 数组环形缓冲区
    int size[BUFNUM];           // 每个缓冲区的对应大小
    int read;                   // 当前可读缓冲区的序号
    int write;                  // 当前可写缓冲区的序号
}ShareBuf;

void P(int semid, int index){   // P操作
    struct sembuf sem = {index, -1, 0};
    semop(semid, &sem, 1);
}

void V(int semid, int index){   // V操作
    struct sembuf sem = {index, +1, 0};
    semop(semid, &sem, 1);
}

#endif