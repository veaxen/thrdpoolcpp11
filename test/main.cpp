#include "thread_pool.h"
#include <time.h>
#include <unistd.h>
#include <stdio.h>

void func(int n)
{
	printf("Test:%d \r\n",n);//在Linux下是线程安全的
	usleep(2000);
}

int main()
{
	clock_t start,end;
	double seconds;
	start = clock();
	{
	    vv::thread_pool thrpool(4,8);
	    for(int i=0; i < 10000; ++i)
	        while(!thrpool.add_task(func,i));
        sleep(3);//等待线程池里的任务都执行完
    }
	printf("\r\n\r\n\r\n");
    end = clock();
	seconds = (double)(end - start)/CLOCKS_PER_SEC;
	printf("耗时 %lf 秒\r\n", seconds);
	return 0;
}
