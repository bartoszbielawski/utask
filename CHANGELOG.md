Version 0.3.0 [2010-8 - not released]:
- Changed several things since 0.2.2 version
	
Version 0.3.1 [2010-10-15]:
- Prepared for release
- Added examples
	
Version 0.3.2 [2011-02-28]:
- [**fix**] Fixed bug in utask_add() with UTASK_SIMPLE_ADD defined. Only the first task would run.
- [**new**] Included typedefs for easy change of utask's member size. Code is prepared for 8 and 16-bit uCs.
- [**new architecture**] uTasks have been confirmed to work on Atmel ATmega8 device.

Version 0.3.3 [2011-06-04]:
- [**fix**] Fixed bug in utask_schedule(), task could be woken prematurely,
- [**thanx**] for Mateusz Kondej for finding utask_schedule() bug,
- [**new**] Added function to check if CPU can be put to sleep, user needs to implement it's own function to put CPU to sleep.
	