#include<asm/unistd.h>
#include<stdio.h>
#include<sys/errno.h>
#define smunch(arg1,arg2) syscall(324,arg1,arg2)
int main(int argc, char** argv)
{
	unsigned long bitpattern=0;
	int result;
	int pid;
	bitpattern |= 256;
	printf("About to make smunch call\n");
	pid=atoi(argv[1]);
	result = smunch(pid,bitpattern);
	printf("the result is %d\n",result);
	return 0;
} 
 
