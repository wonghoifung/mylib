#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#define __USE_GNU		//CPU_ZERO
//#define _GNU_SOURCE
#include <sched.h>
#include <pthread.h>            

void* new_test_thread(void* arg)
{
	cpu_set_t mask;
	int i = 0;
	int num = sysconf(_SC_NPROCESSORS_CONF);    
	pthread_detach(pthread_self());
	
	CPU_ZERO(&mask);	
	CPU_SET(1, &mask);      //cpu 1
	if(sched_setaffinity(0, sizeof(mask), &mask) == -1)      //0 this thread 
	{
		printf("set affinity failed..\n");
	}
	while(1)
	{
		CPU_ZERO(&mask);
		if(sched_getaffinity(0, sizeof(mask), &mask) == -1)	
		{
			printf("get failed..\n");
		}

		for(i = 0; i < num; i++)
		{
			if(CPU_ISSET(i, &mask))
			printf("new thread %d run on processor %d\n", getpid(), i);
		}
		while(1);
		sleep (1);
	}
}      

void* child_test_thread(void* arg)
{
	cpu_set_t mask;
	int i = 0;
	int num = sysconf(_SC_NPROCESSORS_CONF);
	pthread_detach(pthread_self());
	
	while(1)
	{
		CPU_ZERO(&mask);
		if(sched_getaffinity(0, sizeof(mask), &mask) == -1)	
		{
			printf("get failed..\n");
		}

		for(i = 0; i < num; i++)
		{
			if(CPU_ISSET(i, &mask))
			printf("child thread %d run on processor %d\n", getpid(), i);
		}
		while(1);
	}

}

int
main(int argc, char* argv[])
{
	int num = sysconf(_SC_NPROCESSORS_CONF);
	int created_thread = 0;
	int myid;
	int i;
	int j = 0;
	pthread_t ptid = 0;

	cpu_set_t mask;
	cpu_set_t get;

	if(argc != 2)
	{
		printf("usage: ./cpu num\n");
		return -1;
	}
	myid = atoi(argv[1]);
	printf("system has %i processor(s).\n", num);

	CPU_ZERO(&mask);
	CPU_SET(myid, &mask);
	if(sched_setaffinity(0, sizeof(mask), &mask) == -1)
	{
		printf("warning: set CPU affinity failed...");
	}

	int ret = pthread_create(&ptid, NULL, new_test_thread, NULL);
	if(ret)
	{
		return -1;
	}
	ret = pthread_create(&ptid, NULL, child_test_thread, NULL);
	if(ret)
	{
		return -1;
	}


	while(1)
	{
		CPU_ZERO(&get);
		if(sched_getaffinity(0, sizeof(get), &get) == -1)
		{
			printf("can't get cpu affinity...\n");
		}

		for(i = 0; i < num; i++)
		{
			if(CPU_ISSET(i, &get))
			{
				printf("this process %d is runing on procesor:%d\n", getpid(), i);
			}
		}
		
		sleep(1);
	}
	return 0;
}


