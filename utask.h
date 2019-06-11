/*
	 Copyright (c) 2010 Bartosz Bielawski, Marcin Zapolski
	 Copyright (c) 2011 Bartosz Bielawski
	This software is licensed under MIT License, see LICENSE file.
*/


#ifndef UTASK_H
#define UTASK_H

/**
 @file utask.h
 @authors B.Bielawski - InventLab, Marcin Zapolski - InventLab
 @date 2010.02.28
 @mainpage
 @version 0.3.3

 No long introductions. Just short Q&A list. Feel free to ask questions. The newest version of uTask can be downloaded at https://sourceforge.net/projects/utask/.

 @b Q&A
 
	@b Q: What is uTask?
	
	@b A: uTask is a simple library providing simple coroutines (http://en.wikipedia.org/wiki/Coroutine).
 
 	@b Q: Why was uTask created?
 	
 	@b A: The library's main goal is to make writing firmware for 32bit uCs easier by providing simple framework for running multiple task in non-colliding manner. 
 	
 	@b Q: Can I use uTask on other devices? @b [new]
 	
 	@b A: I have used uTask on ATmega8 uC, for better performance tune the structure a little, reduce members' sizes.
 	
 	@b Q: How does it work?
 	
 	@b A: uTask manages user's FSMs (http://en.wikipedia.org/wiki/Finite-state_machine) implemented as functions. uTask provides functions to put tasks to sleep, suspend them or make them wait for specific resource to be free. It also can spawn new tasks at runtime and end them if they are no longer needed.
 	
 	@b Q: How big is uTask?
 	
 	@b A: Library's footprint can be reduced to about 600 B of flash and 200 B of RAM, depending on selected features and maximal number of tasks that can be run concurrently. Full blown version occupies about 1.5 kB of flash and about 40B of RAM per task (again, depending on user choices).
 	
 	@b Q: Was uTask tested?
 	
 	@b A: Yes, it was tested for a few months now, it is used in production equipment.
 	
 	@b Q: Is uTask free software?
 	
 	@b A: Yes, uTask is a free software, it is licensed under MIT License (see LICENSE file).
 	
 	@b Q: Why is it free?
 	
 	@b A: It is free because I like it that way and my boss agreed to release it to the public. This is just a small tool of mine, it's not a commercial product. Enjoy!
 	
 	@b Q: Who wrote uTask?
 	
 	@b Q: uTask was written by Bartosz Bielawski with help from Marcin Zapolski. We are both Embedded System Engineers working at InventLab (http://inventlab.eu) company. This software was created in our worktime.
 	
 
*/

//------------------------ USER CONFIG STARTS HERE ----------------------------------

#include <stdint.h>

///defines utask version string
#define UTASK_VER_STR	"0.3.3"
///major version
#define UTASK_VER_MAJOR 0
///minor version
#define UTASK_VER_MINOR 3
///revision
#define UTASK_VER_REV   3

///enables simplified utask_add (for fixed number of tasks)
#define UTASK_SIMPLE_ADD

///enables utask_exit for ending tasks
#define UTASK_EXIT

///enables task finding and counting
#define UTASK_STATS

///enables utask_suspend and utask_resume functions
#define UTASK_SUSPEND_RESUME

///emables task names
#define UTASK_NAME

///enables task indentifiers (tids)
#define UTASK_TID

///enables arg field in utask type
#define UTASK_ARG

///enables semaphores
#define UTASK_SEM

///enables waiting for specific semaphore values (without only for 0 and non zero)
#define UTASK_SEM_CHANGE

///enables data field in utask type (for free use)
#define UTASK_DATA		1

///defines how many tasks can be run in one time
#define UTASKS		10

///enables structure packing
#define UTASK_PACKED_STRUCT

///enables putting device to sleep
#define UTASK_CHECK_MCU_SLEEP

///This typedef defines size of utask_t->sleep timer member
typedef uint32_t utask_timer_t;
///This typedef defines size of utask_t->state and utask_t->istate member
typedef uint32_t utask_state_t;
///This typedef defines size of utask_t->sem member
typedef uint32_t utask_sem_t;
///This typedef defines size of utask_t->data member
typedef uint32_t utask_data_t;

#define UTASK_SLEEP_MAX 0xFFFFFFFF

/*
//suggested sizes for 16-bit uCs (MSP430, PIC24...)
typedef uint32_t utask_timer_t;
typedef uint16_t utask_state_t;
typedef uint16_t utask_istate_t;
typedef uint16_t utask_sem_t;
typedef uint16_t utask_data_t;	
#define UTASK_SLEEP_MAX 0xFFFFFFFF

//suggested sizes for 8-bit uCs (ATmega, AT90, PIC10/12/14/16/18)
typedef uint16_t utask_timer_t;
typedef uint8_t  utask_state_t;
typedef uint8_t  utask_istate_t;
typedef uint8_t  utask_sem_t;
typedef uint8_t  utask_data_t;
#define UTASK_SLEEP_MAX 0xFFFF
*/


//------------------------ END OF USER CONFIG ----------------------------------


#ifdef UTASK_SEM_CHANGE
	#ifndef UTASK_SEM
		#define UTASK_SEM
	#endif
#endif

///unsused
#define	UTS_NONE	0
///suspended
#define	UTS_SUSP	1
///ready for execution
#define UTS_READY	2
///asleep
#define UTS_SLEEP	3

#ifdef UTASK_SEM
#ifndef UTASK_SEM_CHANGE
	///waiting for semaphore == 0
	#define UTS_WAIT_Z	4
	///waiting for semaphore != 0
	#define UTS_WAIT_NZ	5
#else
	///waiting for semaphore == val (val set in function)
	#define UTS_WAIT_E 4
	///waiting for semaphore != val (val set in function)
	#define UTS_WAIT_NE 5
#endif
#endif

#ifdef UTASK_PACKED_STRUCT
	#pragma pack(1)
#endif

#ifdef UTASK_CHECK_MCU_SLEEP
	extern void (*utask_put_mcu_to_sleep)(utask_data_t d);
#endif


/**
 * @struct utask
 * @brief Structure describing single task
 *
 * This structure describes single task. Pointer to it is passed to many utask_* functions. Structures are allocated internally by uTask.
 */
typedef struct utask
{
	///task internal state (UTS_* defines) - internal data, do not modify manually
    volatile	utask_state_t	state;
	///internal task state (for programmers use)
    volatile	utask_state_t	istate;
	#ifdef UTASK_SEM
		///semaphore pointer - internal data, do not modify manually
		volatile 	utask_sem_t*	sem;
		#ifdef UTASK_SEM_CHANGE
			///samaphore conditional value - internal data, do not modify manually
			volatile utask_sem_t 	sem_val;
		#endif
	#endif
	///sleep counter (in ticks) - internal data, do not modify manually
    volatile	utask_timer_t	sleep;
	///function to be run
    void		(*fun)(struct utask* t);

    #ifdef UTASK_DATA
    	#if UTASK_DATA > 0
		///user's general purpose per task data storage
		utask_data_t	data[UTASK_DATA];
		#endif
	#endif

	#ifdef UTASK_ARG
		///user's data passed as param for the task
		utask_data_t	arg;
	#endif

	#ifdef UTASK_TID
		///task id
		utask_data_t	tid;
	#endif

	#ifdef UTASK_NAME
		///task name
        const char*		name;
    #endif
} utask_t;

///task function typedef
typedef void (*utask_fun)(utask_t* t);

/**
@brief Intializes internal variables. Must be run before all other utask functions.

Intializes internal variables. @b Must be run before all other utask functions.
@return nothing
*/
void	utask_init(void);


/**
@brief Scheduling function, must be called forever.

Scheduling function, @b must be called forever (e.g. in main() in while(1) {} loop) or utask will @b not work!
@return nothing
 */
void 	utask_schedule(void);


/**
@brief Timekeeping function, should be called from ISR.

Function decreasing tasks' sleep timers. @b Must be called periodically (e.g. from timer ISR) or utask_sleep() and utask_wait_*() will @b not work properly! 
Calling frequency selects timeout tick resolution. Calling this function at 1kHz rate gives you tick equal to 1 ms. Call this function as often (or as rarely) as you need.
@return nothing
*/
void	utask_sleep_process(void);


/**
@brief Add new task with function @a function for execution.

Add new task with function @a function for execution.
Can be one of two versions:
- simple version allowing adding tasks only before first call to utask_schedule(), enabled when @ref UTASK_SIMPLE_ADD is defined.
- version allowing spawning new task any time - when @ref UTASK_SIMPLE_ADD is undefined.

After add is called following structure members are set to as follows:
- state is set to UTS_READY and task will be executed as soon as possible,
- istate is set to zero,
- sleep is set to zero,
- arg (if enabled) is set to zero,
- tid (if enabled) is last task's id + 1,
- not mentioned members are in undefined state.

@param	fun			pointer to function to be executed
@return pointer to task structure describing task of NULL in case of error
*/
utask_t*	utask_add(utask_fun fun);

#ifdef UTASK_NAME
/**
@brief Add new task with function @a function and name @a name for execution.

Add new task with function @a function and name @a name for execution.
This is simple wrapper. See @ref utask_add() for details.
@param	fun			pointer to function to be executed
@param name		name of the task
@return pointer to task structure describing task of NULL in case of error
*/
utask_t*	utask_add_name(utask_fun fun, const char *name);
#endif


#ifdef UTASK_EXIT
/**
@brief Terminate task @a t.

Ends task @a t. utask_exit() can terminate other tasks. Do @b not reference @a t after it has been freed with utask_exit().
@param	t			pointer to utask structure
 */
void	utask_exit(utask_t* t);
#endif


/**
@brief Sleep task @a t for @a ticks ticks of clock.

Sleep task @a t for @a ticks ticks of clock. This function can put to sleep another task.
@param t pointer to utask structure
@param ticks number of ticks of clock for task to sleep
*/
void 	utask_sleep	(utask_t* t,utask_timer_t ticks);


#ifdef UTASK_SUSPEND_RESUME
/**
@brief Suspend task exectuion.

Suspend task exectuion. This function can suspend another task.
@param	t			pointer to utask structure
 */
void	utask_suspend(utask_t* t);


/**
@brief Resume task @a t.

Resume task @a t. It works on both on tasks put to sleep and suspended.
@param t	pointer to utask structure
*/
void utask_resume(utask_t* t);
#endif

/**
@brief Wait for the semaphore \a sem to be cleared with \a timeout ticks of timeout.

Wait for the semahore \a sem to be cleared with \a timeout ticks of timeout. If condition was met t->sleep != 0.
@param	t			pointer to utask structure
@param	sem			pointer to semaphore
@param	timeout	waiting timeout
*/
void	utask_wait_zero(utask_t* t, utask_sem_t* sem, utask_timer_t timeout);

/**
@brief Wait for the semahore \a sem to be set with \a timeout ticks of timeout.

Wait for the semahore \a sem to be set with \a timeout ticks of timeout. If condition was met t->sleep != 0.
@param	t			pointer to utask structure
@param	sem			pointer to semaphore
@param	timeout	waiting timeout
@return nothing
*/
void	utask_wait_nzero(utask_t* t,utask_sem_t* sem, utask_timer_t timeout);

#ifdef UTASK_SEM_CHANGE
/**
@brief Wait for the semaphore \a sem value to be set to \a sem_value with \a timeout ticks of timeout.

Wait for the semaphore \a sem value to be set to \a sem_value with \a timeout ticks of timeout. If contidion was met t->sleep != 0.
@param	t			pointer to utask structure
@param	sem			pointer to semaphore
@param	sem_val	value waited for
@param	timeout	waiting timeout
@return nothing
 */
void	utask_wait_eq(utask_t* t, utask_sem_t* sem, utask_sem_t sem_val, utask_timer_t timeout);

/**
@brief Wait for the semaphore \a sem value to be changed from \a sem_value with \a timeout ticks of timeout.

Wait for the semaphore \a sem value to be changed from \a sem_value with \a timeout ticks of timeout. If contidion was met t->sleep != 0.
@param	t			pointer to utask structure
@param	sem			pointer to semaphore
@param	sem_val 	value waited for
@param	timeout	waiting timeout
@return nothing
 */
void	utask_wait_neq(utask_t* t, utask_sem_t* sem, utask_sem_t sem_val, utask_timer_t timeout);
#endif


#ifdef UTASK_STATS
/**
@brief Returns free task slots.

Returns free task slots.
@return	free task slots
 */
utask_data_t	utask_get_free_slots(void);


/**
@brief Function returns number of tasks executing @a fun function.

Function returns number of tasks executing @a fun function.
@param   fun	function
@return	numbers of tasks executing @a fun function.
*/
utask_data_t	utask_get_task_cnt(utask_fun fun);
#endif

#ifdef UTASK_TID
	/**
	 @brief Returns task structure pointer by task id.
	 
	 Returns task structure pointer by task id.
	 @param	tid		task id
	 @return	pointer to the task structure or NULL if not found
	*/
	utask_t*	utask_get_by_tid(utask_data_t tid);
#endif


#endif					//UTASK_H


/**
@example utask_example_1.c
This simple example shows how to make a LED blink at 1 Hz rate using utasks.

@example utask_example_2.c
This example shows how to delay task execution by using utask_sleep().

@example utask_example_3.c
This is simple example of semaphor usage. See docs for more details.

@example utask_example_4.c
Example of spawning new tasks from running ones.
*/
