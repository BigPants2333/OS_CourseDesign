#include "head.h"

int main(int argc, char *argv[]){
    int semid = semget(semkey, 4, IPC_CREAT|0666);                  // 获取信号灯
    int shmid = shmget(bufkey2, sizeof(ShareBuf), IPC_CREAT|0666);  // 获取共享缓冲区并附接
    ShareBuf *sbuf=(ShareBuf*)shmat(shmid, NULL, 0);

    int fileid;
    if((fileid = open(argv[2], O_WRONLY|O_CREAT, S_IRWXG|S_IRWXO|S_IRWXU)) == -1){  // 打开或创建目标文件
        perror("Can't open or create destination file");
        exit(1);
    }
    //printf("PROCESS WRITE:Open or create destination file successfully.semid = %d, shmid = %d\n", semid, shmid);

    int flag = 0;
    while(flag != 1){   // 将共享缓冲区2的内容写入目标文件
        if(sbuf->size[sbuf->write] < BUFSIZE) flag = 1;     // 读完缓冲区内容
        P(semid, 3);
        write(fileid, sbuf->buf[sbuf->write], sizeof(char) * sbuf->size[sbuf->write]);
        sbuf->write = (sbuf->write + 1) % BUFNUM;
        V(semid, 2);
    }
    close(fileid);
    //printf("PROCESS WRITE->end\n");
    return 0;
}