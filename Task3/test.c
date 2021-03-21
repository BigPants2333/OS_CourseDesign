#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>

#ifndef TXGDEV_SIZE
#define TXGDEV_SIZE 4096
#endif

int main(int argc, char *argv[])
{
    /* 检查参数 */
    if(argc != 2)
    {
        printf("Please type [command] [string]\n");
        return 1; 
    }

    /* 获取输入字符长度 */
    int length = 0;
    while(argv[1][length] != '\0') ++length;
    // printf("argv[1]: %s, sizeof: %ld, length: %d\n", argv[1], sizeof(argv[1]), length);
    
    /* 打开设备文件 */
    FILE *fp = fopen("/dev/txgdev", "r+");
    if (fp == NULL)
    {
        printf("open txgdev error!\n");
        return -1;
    }

    /* 写入设备*/
    fwrite(argv[1], length, 1, fp);
    
    /* 重新定位文件位置*/
    fseek(fp, 0, SEEK_SET);
    
    /* 读出设备 */
    char buffer[TXGDEV_SIZE];
    fread(buffer, sizeof(buffer), 1, fp);
    printf("txgdev: %s\n",buffer);
    
    /* 检测结果 */
    execl("/bin/cat", "cat", "/dev/txgdev", NULL);
    
    return 0;    

}
