#include<stdio.h>
#include<unistd.h>
#include<sys/syscall.h>
#include<linux/kernel.h>
int main(int argc, char *argv[]){
	long int a = syscall(335, 233);
	printf("System call sys_hello return %ld\n", a);
	return 0;
}
