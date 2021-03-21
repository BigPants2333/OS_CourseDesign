#include "head.h"

int main(int argc, char *argv[]){

    /* 检查参数 */
    if(argc != 3){
        printf("Please type [command] [srcfile] [desfile]\n");
        exit(0);
    }
    //printf("PROCESS MAIN->start\n");

    /* 创建信号灯 */
    int semid = semget(semkey, 4, IPC_CREAT|0666);
    union semun sem0, sem1, sem2, sem3;
    sem0.val = BUFNUM, sem1.val = 0;
    sem2.val = BUFNUM, sem3.val = 0;
    semctl(semid, 0, SETVAL, sem0);     // sbuf1可写
    semctl(semid, 1, SETVAL, sem1);     // sbuf1可读
    semctl(semid, 2, SETVAL, sem2);     // sbuf2可写
    semctl(semid, 3, SETVAL, sem3);     // sbuf2可读

    /* 创建共享缓冲区 */
    // 进程read与copy之间的共享缓冲区sbuf1
    int shmid1 = shmget(bufkey1, sizeof(ShareBuf), IPC_CREAT|0666);
    ShareBuf *sbuf1 = (ShareBuf*)shmat(shmid1, NULL, 0);
    sbuf1->read = 0, sbuf1->write = 0;
    // 进程copy与write之间的共享缓冲区sbuf2
    int shmid2 = shmget(bufkey2, sizeof(ShareBuf), IPC_CREAT|0666);
    ShareBuf *sbuf2 = (ShareBuf*)shmat(shmid2, NULL, 0);
    sbuf2->read = 0, sbuf2->write = 0;
    //printf("semid = %d, shmid1 = %d, shmid2 = %d\n", semid, shmid1, shmid2);
    
    /* 创建子进程 */
    pid_t p1, p2, p3;
    if((p1 = fork()) == -1){        // 创建子进程read
        perror("Failed to create process READ");
        exit(1);
    }
    else if(p1 == 0){               // 进入子进程read
        //printf("PROCESS READ->start\n");
        execv("./read", argv);
    }
    else if((p2 = fork()) == -1){   // 创建子进程copy
        perror("Failed to create process COPY");
        kill(p1, SIGKILL);
        exit(1);
    }
    else if(p2 == 0){               // 进入子进程copy
        //printf("PROCESS COPY->start\n");
        execv("./copy", argv);
    }
    else if((p3 = fork()) == -1){   // 创建子进程write
        perror("Failed to create process WRITE");
        kill(p1, SIGKILL);
        kill(p2, SIGKILL);
        exit(1);
    }
    else if(p3 == 0){               // 进入子进程write
        //printf("PROCESS WRITE->start\n");
        execv("./write", argv);
    }

    /* 主进程的收尾工作 */
    else{
        wait(&p1);wait(&p2);wait(&p3);  // 等待进程结束
        semctl(semid, 0, IPC_RMID);     // 销毁信号灯
        semctl(shmid1, 0, IPC_RMID);    // 销毁共享缓冲区1
        semctl(shmid2, 0, IPC_RMID);    // 销毁共享缓冲区2
        //printf("PROCESS MAIN->end\n");
    }
    return 0;
}
