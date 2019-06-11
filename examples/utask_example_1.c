#include "utask.h"
#include "leds.h"
#include "timer.h"

void timer_isr ()			//called 1000 times a second
{
  utask_sleep_process ();
}

void my_fun (utask * t)
{
  if (t->istate % 2)
    turn_led_on(RLED);
  else
    turn_led_off(RLED);
  t->istate++;
  utask_sleep(t, 500);		//sleep for 500ms
}

int main ()
{
	timer_init();
	utask_init();
	utask_add(my_fun);
	while (1)
	{
		utask_schedule();
	}
	return 0;
}
