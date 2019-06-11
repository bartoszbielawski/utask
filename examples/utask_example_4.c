#include "utask.h"
#include "leds.h"
#include "timer.h"
#include "uart.h"

void timer_isr()			//called 1000 times a second
{
  utask_sleep_process();
}

int btn_press_counter = 0;

void button_isr()
{
	btn_press_counter++;
}

void task_do_something(utask* t)
{
	//do yer stuff here
}

void task_spawn_on_button_press(utask* t)
{
	switch (t->istate)
	{
		case 0:
			utask_wait_nzero(t,&btn_press_counter,1000000);		//wait looooong time
			t->istate = 1;
			break;
			
		case 1:
			if (t->sleep != 0)	//the task has been awakened -> btn_press_counter != 0
			{
				utask_add(task_do_something);		//spawn new task
				btn_press_counter--;
			}
			t->istate = 0;
			break;
	}
}


int main ()
{
	timer_init();
	uart_init();
	utask_init();

	utask_add(task_spawn_on_button_press);
	
	while (1)
	{
		utask_schedule();
	}
	return 0;
}
