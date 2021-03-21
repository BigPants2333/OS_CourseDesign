#include "head.h"

int main(int argc, char *argv[]){
    int semid = semget(semkey, 4, IPC_CREAT|0666);                      // 获取信号灯
    int shmid1 = shmget(bufkey1, sizeof(ShareBuf), IPC_CREAT|0666);     // 获取共享缓冲区1并附接
    ShareBuf *sbuf1=(ShareBuf*)shmat(shmid1, NULL, 0);
    int shmid2 = shmget(bufkey2, sizeof(ShareBuf), IPC_CREAT|0666);     // 获取共享缓冲区2并附接
    ShareBuf *sbuf2=(ShareBuf*)shmat(shmid2, NULL, 0);

    int flag = 0;
    while(flag != 1){   // 将共享缓冲区1的内容放入共享缓冲区2中
        P(semid, 1);
        P(semid, 2);
        int size = sizeof(char) * sbuf1->size[sbuf1->write];
        memcpy(sbuf2->buf[sbuf2->read], sbuf1->buf[sbuf1->write], size);
        sbuf2->size[sbuf2->read] = size;
        if(size < BUFSIZE) flag = 1;
        sbuf1->write = (sbuf1->write + 1) % BUFNUM;
        sbuf2->read = (sbuf2->read + 1) % BUFNUM;
        V(semid, 0);
        V(semid, 3);
    }
    //printf("PROCESS COPY->end\n");
    return 0;
}