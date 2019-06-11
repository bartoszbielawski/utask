#include "utask.h"
#include "leds.h"
#include "timer.h"

void timer_isr ()			//called 1000 times a second
{
  utask_sleep_process();
}

void task_delayed_run(utask * t)
{
	turn_led_on(t->arg);		//turn led on
	utask_exit(t);		//end task
}

int main ()
{
	timer_init();
	utask_init();
	
	utask* new_task;
	
	new_task = utask_add (task_delayed_run);
	utask_sleep(new_task,5000);		//delay execution, it will run in 5 seconds
	new_task->arg = RLED;			//turn on red led
	
	new_task = utask_add (task_delayed_run);
	utask_sleep(new_task,10000);		//delay execution, it will run in 10 seconds
	new_task->arg = GLED;			//turn on green led
	
	new_task = utask_add (task_delayed_run);
	utask_sleep(new_task,15000);		//delay execution, it will run in 15 seconds
	new_task->arg = BLED;			//turn on blue led
	
	while (1)
	{
		utask_schedule();
	}
	return 0;
}
