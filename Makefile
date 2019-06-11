X86CC=gcc
ARMCC=arm-elf-gcc
CFLAGS=-Os -Wall -pedantic -std=c99

all:
#	$(ARMCC) $(CFLAGS) -c utask.c -o utask_arm.o
	$(X86CC) $(CFLAGS) -c utask.c -o utask_x86.o
#	$(ARMCC) $(CFLAGS) -S utask.c -o utask_arm.asm
	$(X86CC) $(CFLAGS) -S utask.c -o utask_x86.asm
	size utask_x86.o
	nm --size-sort -S utask_x86.o

test:
	gcc -lpthread utask.c utest.c -o utest