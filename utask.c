/*
	 Copyright (c) 2010 Bartosz Bielawski, Marcin Zapolski
	 Copyright (c) 2011 Bartosz Bielawski
	This software is licensed under MIT License, see LICENSE file.
*/


#include <stdint.h>
#include "utask.h"


/**
 @file utask.c
 @authors B.Bielawski - InventLab, Marcin Zapolski - InventLab
 @date 2010.02.28
 @version 0.3.3
*/


static utask_data_t utask_free_slots = UTASKS;
static utask_data_t utask_free_tid = 0;
static utask_data_t utask_last_used = 0;

#ifdef UTASK_TID
	static utask_data_t utask_last_tid = 0;
#endif

static utask_t 	tasks[UTASKS];

#ifdef UTASK_CHECK_MCU_SLEEP
	void (*utask_put_mcu_to_sleep)(utask_data_t d) = (void*)0;
#endif

void utask_init()
{
    int i;
    for (i=0;i<UTASKS;i++)
		tasks[i].state = UTS_NONE;
}

void utask_sleep(utask_t* t,utask_timer_t ticks)
{
    t->sleep = ticks;
    t->state = UTS_SLEEP;
    #ifdef UTASK_SEM
		t->sem = 0;
	#endif
}


#ifdef UTASK_NAME
utask_t*	utask_add_name(utask_fun fun, const char* name)
{
	utask_t* n = utask_add(fun);
	if (n)
		n->name = name;
	return n;
}
#endif

utask_t*	utask_add(utask_fun fun)
{
	utask_t* t = &tasks[utask_free_tid];

	if (!utask_free_slots)
		return 0;			//NULL

   	t->state = UTS_READY;
    t->istate = 0;
    t->sleep = 0;
	t->fun = fun;
	
	#ifdef UTASK_ARG
		t->arg = 0;
	#endif

	#ifdef UTASK_NAME
		t->name = "";
	#endif

	#ifdef UTASK_TID
		t->tid = utask_last_tid++;
	#endif

	#ifdef UTASK_SIMPLE_ADD
		utask_free_tid++;
		utask_free_slots--;
		utask_last_used++;
	#else

	if (utask_last_used < utask_free_tid)
		utask_last_used = utask_free_tid;		//nowy ostatni element

	int i;

    for (i=0;i<UTASKS;i++)			//wyznacz nowy pierwszy pusty element
		if (tasks[i].state == UTS_NONE)
		{
			utask_free_tid = i;
			break;
		}
	utask_free_slots--;
	#endif
	return t;
}


#ifdef UTASK_CHECK_MCU_SLEEP
utask_data_t utask_check_sleep()
{
	utask_t* t = tasks;
	utask_data_t i;
	utask_timer_t s = UTASK_SLEEP_MAX;
	for (i=0;i<=utask_last_used;++i,++t)	//iterate over used tasks
	{
		if (t->state == UTS_READY)
			return 0;
		if (t->state >= UTS_SLEEP)
			if (t->sleep < s)
				s = t->sleep;
	}	
	return s;
}
#endif

void utask_schedule()
{
	int i;
#ifdef UTASK_SEM
	register utask_sem_t  cond = 0;
#endif
	register utask_t* t = tasks;
	for (i=0;i<=utask_last_used;++i,++t)	//iterate over used tasks
	{
		switch (t->state)
		{
			case UTS_NONE:
				continue;
			case UTS_SUSP:
				continue;
				
			case UTS_READY:			//this one is ready for execution
				t->fun(t);
				break;

			case UTS_SLEEP:
				continue;

#ifdef UTASK_SEM
	#ifdef UTASK_SEM_CHANGE
			case  UTS_WAIT_E:		//check if semaphore's value is equal to the one we are waiting for
				cond = (*t->sem == t->sem_val);
				break;
			case  UTS_WAIT_NE:		//inverted condition
				cond = (*t->sem != t->sem_val);
				break;
	#else
			case UTS_WAIT_Z:		//check if semaphore's value is zero
				cond = !(*t->sem);
				break;
			case UTS_WAIT_NZ:		//check if semaphore's value is non-zero
				cond = (*t->sem);
				break;
	#endif
#endif
			//just in case
			default:			//this catches UTS_NONE, UTS_SUSP, UTS_SLEEP	
				break;
		}
	
		
#ifdef UTASK_SEM
		if (cond)
		{
			t->state = UTS_READY;
			t->fun(t);
		}
#endif	

	}
	
#ifdef UTASK_CHECK_MCU_SLEEP
	if (!utask_put_mcu_to_sleep)		//if no callback has been defined return
		return;		
	
	utask_timer_t s = utask_check_sleep();
	utask_put_mcu_to_sleep(s);
#endif
}


void utask_sleep_process()
{
	int i;
	for (i=0;i<=utask_last_used;i++)
	{
		if (tasks[i].state < UTS_SLEEP)			//nie zyje, do wykonania, zawieszony
			continue;

		if (tasks[i].sleep > 0)
			tasks[i].sleep--;
		else
			tasks[i].state = UTS_READY;
	}
}


#ifdef UTASK_EXIT
void utask_exit(utask_t* t)
{
	int i;
	t->state = UTS_NONE;
	utask_free_slots++;
	utask_free_tid = UTASKS;
	utask_last_used = 0;
	for (i=0;i<UTASKS;i++)
	{
		if (tasks[i].state == UTS_NONE)
		{
			if (utask_free_tid > i)
				utask_free_tid = i;
		}
		else
			utask_last_used = i;
	}
}
#endif

#ifdef UTASK_SEM 
	#ifdef UTASK_SEM_CHANGE

//czekanie na jakas wartosc semafora
void utask_wait_eq(utask_t* t, utask_sem_t* sem, utask_sem_t sem_val, utask_timer_t timeout)
{
	t->sem = sem;
	t->sleep = timeout;
	t->state = UTS_WAIT_E;
	t->sem_val = sem_val;
}

//czekanie na zmiane z jakiejs wartosci semafora
void utask_wait_neq(utask_t* t, utask_sem_t* sem, utask_sem_t sem_val, utask_timer_t timeout)
{
	t->sem = sem;
	t->sleep = timeout;
	t->state = UTS_WAIT_NE;
	t->sem_val = sem_val;
}

//owrapowanie dla zera
#define utask_wait_nzero(utask, sem, timeout)	utask_wait_neq(utask, sem, 0, timeout)
#define utask_wait_zero(utask, sem, timeout)	utask_wait_eq(utask, sem, 0, timeout)
	#else
	
//to samo, czekanie dla zera i niezera
void utask_wait_nzero(utask_t* t, utask_sem_t* sem, utask_timer_t timeout)
{
	t->sem = sem;				//semafor
	t->sleep = timeout;
	t->state = UTS_WAIT_NZ;
}

void utask_wait_zero(utask_t* t, utask_sem_t* sem, utask_timer_t timeout)
{
	t->sem = sem;				//semafor
	t->sleep = timeout;
	t->state = UTS_WAIT_Z;
}	
	
	#endif
#endif

#ifdef UTASK_SUSPEND_RESUME
void	utask_suspend(utask_t* t)
{
	t->state = UTS_SUSP;
}

void	utask_resume(utask_t* t)
{
	t->state = UTS_READY;
	t->sleep = 0;
}
#endif

#ifdef UTASK_STATS
utask_data_t	utask_get_free_slots(void)
{
	return utask_free_slots;
}

utask_data_t    utask_get_task_cnt(utask_fun fun)
{
    int i;
    int cnt = 0;
	for (i=0;i<utask_last_used;i++)
		if (tasks[i].fun == fun)
			cnt++;
	return cnt;
}
#endif


#ifdef UTASK_TID
utask_t* utask_get_by_tid(utask_data_t tid)
{
	int i;
	for (i=0;i<utask_last_used;i++)
		if (tasks[i].tid == tid)
			return tasks+i;
	return 0;
}
#endif
