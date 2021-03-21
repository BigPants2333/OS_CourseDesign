#include "head.h"

int main(int argc, char *argv[]){
    int semid = semget(semkey, 4, IPC_CREAT|0666);                  // 获取信号灯
    int shmid = shmget(bufkey1, sizeof(ShareBuf), IPC_CREAT|0666);  // 获取共享缓冲区并附接
    ShareBuf *sbuf=(ShareBuf*)shmat(shmid, NULL, 0);

    int fileid;
    if((fileid = open(argv[1], O_RDONLY)) == -1){   // 打开源文件
        perror("Can't open source file");
        exit(1);
    }
    //printf("PROCESS READ:Open source file successfully.semid = %d, shmid = %d\n", semid, shmid);

    int flag = 0;
    while(flag != 1){   // 将源文件写入共享缓冲区1
        P(semid, 0);
        int size = read(fileid, sbuf->buf[sbuf->read], sizeof(char) * BUFSIZE);
        sbuf->size[sbuf->read] = size;
        if(size < BUFSIZE) flag = 1;    // 文件尾
        sbuf->read = (sbuf->read + 1) % BUFNUM;
        V(semid, 1); 
    }
    close(fileid);
    //printf("PROCESS READ->end\n");
    return 0;
}