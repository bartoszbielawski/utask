#include "utask.h"
#include "leds.h"
#include "timer.h"
#include "uart.h"

void timer_isr()			//called 1000 times a second
{
  utask_sleep_process();
}

int uart_busy = 0;

void task_send_msg(utask* t)
{
	switch (t->istate)
	{
		case 0:
			utask_wait_zero(t,&uart_busy,1000);	//wait for uart to be free, timeout 1000ms
			t->istate = 1;
			break;
			
		case 1:
			if (t->sleep != 0)		//task has been awakened, uart_busy == 0
			{
				uart_busy = 1;	//claim resource
				t->istate = 2;
			}
			else
				t->istate = 0;		//go to waiting loop again
			break
			
		case 2:
			//do yer stuff with uart
			/*
			.
			.
			.
			*/
			break;
			
		case N-1:
			//and here
			/*
			.
			.
			.
			*/
			break;
		
		case N:
			uart_busy =  0;		//free resource
			utask_sleep(t,5000);
			t->istate = 0;		//do that over and over again
			break;
	}
}

const char* strings[] = 
{
	"Oh hello!",
	"Howdy!",
	"Fhtagn?"
};


int main ()
{
	timer_init();
	uart_init();
	utask_init();
	
	utask* t;
	
	t = utask_add(task_send_msg);		//add task
	t->arg = 0;		//use 1st string
	
	t = utask_add(task_send_msg);		//and another one
	t->arg = 1;		//use 2nd string
	
	t = utask_add(task_send_msg);		//and another one
	t->arg = 2;		//use 3rd string
	
	while (1)
	{
		utask_schedule();
	}
	return 0;
}
