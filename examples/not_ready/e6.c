#include "utask.h"
#include "leds.h"
#include "timer.h"
#include "uart.h"

#define SECOND1000

//called 1000 times a second
void timer_isr()
{
	utask_sleep_process();
}


#define WT_INIT0
#define WT_WORKING1
#define WT_READY2

#define LONG_LONG_TIME 0xFFFFFFFF

void worker_task(utask_t * t)
{
	switch (t->istate)
	{
		case 0:
//working task is alive!
			t->arg = WT_WORKING;
			t->istate = 1;
			break;

		case 1:
/*
Do the heavy work here.
It can take some time.
.
.
.
*/
			break;

/*
.
.
.
/*

case N-1:
/*
...
*/
			break;

		case N:
//set ready flag for parent process
//and wait to be killed
			t->data[0] = WT_READY;
			utask_sleep(t, LONG_LONG_TIME);
			break;
	}
}


void main_task(utask_t * t)
{
	static utask_t *worker = NULL;
	switch (t->istate)
	{
		case 0:
			uart_puts("Hello :)\n");
			t->istate = 1;
			break;

		case 1:
//create new task
			utask * worker = utask_add(worker_task);
//assign a new state
			worker->arg = WT_INIT;
//wait for worker state to be set to WT_READY
			t->istate = 2;
			utask_wait_eq(t, &worker->arg, WT_READY, 5 * SECOND) break;

		case 2:
			if (t->sleep)
			{
/*
collect results here
*/
			}
//kill worker task
			utask_exit(worker);
//go to the first state again
			t->istate = 1;
			utask_sleep(t, 5 * SECOND);
			break;
	}
}


int main()
{
	timer_init();
	utask_init();

	utask_add(main_task);

	while (1)
	{
		utask_schedule();
	}
	return 0;
}
