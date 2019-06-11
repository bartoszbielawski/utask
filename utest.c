#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include "utask.h"
#include <signal.h>

pthread_t timer_thread;

volatile int run = 1;

#define TIPS 10

#define D1S		TIPS
#define D3S		(3*D1S)
#define D500MS	(D1S/2)
#define D100MS  (D1S/10)

void* timer_thread_fun(void* arg)
{
	while (run)
	{
		utask_sleep_process();
		usleep(1e6 / TIPS);
	}
	return;
}


int semaphore = 0;

void task_print(utask_t* t)
{
	switch (t->istate)
	{
		case 0:
			t->istate = 1;
			utask_wait_eq(t,&semaphore,0,D3S);
			break;
		
		case 1:
			if (t->sleep == 0)
			{
				t->istate = 0;
				return;
			}
		
			semaphore = 1;
			
			printf("%d\n",t->arg);
			utask_sleep(t,D1S);
			t->istate = 2;
			break;
		case 2:
		
			semaphore = 0;
			t->istate = 0;
			utask_sleep(t,D100MS);
			break;
	}
}

void task_timedout(utask_t* t)
{
	switch (t->istate)
	{
		case 0:
			utask_wait_eq(t,&semaphore,0,D500MS);
			t->istate = 1;
			break;
			
		case 1:
			if (t->sleep == 0)
			{
				printf("T\n");
				t->istate = 0;
			}
			else
			{
				t->istate = 2;
				semaphore = 1;
				utask_sleep(t,D500MS);
			}
				

			break;
		case 2:
			printf("OK\n");
			semaphore = 0;
			t->istate = 0;
			utask_sleep(t,D1S);
			break;
	}
}

void signal_handler(int sig)
{
	fprintf(stderr,"Got signal!\n");
	run = 0;
}

void mcu_sleep(utask_timer_t s)
{
	if (s == 0)
		return;
	printf("%dT\n",s);
	usleep((1e6 * s / TIPS) / 2 + 1000);
}

int main()
{
	utask_init();

	signal(SIGINT,signal_handler);
	signal(SIGQUIT, signal_handler);
	signal(SIGTERM, signal_handler);

	pthread_create(&timer_thread,NULL,timer_thread_fun,NULL);
	
	utask_t* t;
	t = utask_add(task_print);
	t->arg = 1;
	t = utask_add(task_print);
	t->arg = 2;
	t = utask_add(task_print);
	t->arg = 3;
	
	utask_add(task_timedout);
	
	utask_put_mcu_to_sleep = mcu_sleep;
	
	while (run)
	{
		utask_schedule();
	}
	
	pthread_join(timer_thread,NULL);
	return 0;
}

