#include <stdio.h>
#include <stdint.h>
#include <mm.h>
#include <unistd.h>

int main(void)
{
	uintptr_t mem1 = 0x300000;
	// uintptr_t mem2 = 0x301000;
	// long a;
	*(long *)mem1 = 34;
	mprotect((void *)mem1, 4096, PROT_NONE);
	*(long *)mem1 = 25;
	return 0;
}