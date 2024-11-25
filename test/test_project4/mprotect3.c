#include <stdio.h>
#include <stdint.h>
#include <mm.h>
#include <unistd.h>
// test function(can not execute)
int main(void)
{
	uintptr_t mem1 = 0x300000;
	long (*fun)(void) = mem1;
	brk(0x303000);
	// uintptr_t mem2 = 0x301000;
	// long a;
	*(long *)mem1 = 34;
	mprotect((void *)mem1, 4096, PROT_NONE | PROT_READ);
	fun();
	return 0;
}