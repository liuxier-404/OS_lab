#include<stdio.h>
#include<sys/wait.h>
#include<sys/time.h>
#include<unistd.h>
#include<math.h>
#include<stdlib.h>
#include<sys/types.h>
int main(int argc, char **argv)
{
    struct timeval start, end;
    pid_t pid;
    pid=fork();
    // 创建子进程
    if (pid < 0)//此时pid为-1，则创建失败
    {
        printf("fork error\n");
    }
    else if (pid == 0)
    {//此时pid为0，则位于子进程中
        //在子进程中将程序装入
        gettimeofday(&start, NULL);
        printf("Create Subprogress!\n");
        execv(argv[1], &argv[1]);
    }
    else
    {
    	gettimeofday(&start, NULL);
        //父进程等待子进程完成
        wait(NULL);
        gettimeofday(&end, NULL);
        int sec=end.tv_sec-start.tv_sec;
        
	long usec=end.tv_usec-start.tv_usec;
		while(usec<0){
		    usec+=1000000;
		    sec--;
		}
	
		int hours=sec/3600;					
		int minutes=(sec/60)%60;			
		int seconds=sec%60;					
		int milliseconds=usec/1000;			
		int microseconds=usec%1000;	
        //格式化输出时间
        printf("运行程序:%s\n", argv[1]);
        printf("使用时间: %d小时%d分%d秒%d毫秒%d微秒\n", hours, minutes, seconds, milliseconds, microseconds);
    }
    return 0;
}
