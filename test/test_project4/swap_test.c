#include <stdio.h>
#include <stdint.h>
#include <stdint.h>
#include <time.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>

#define NUM	100
#define TEST_ADDR 0x500000
#define PAGE_SHIFT 12

int main(void)
{
	srand(clock());
	long mem2[NUM];
	uintptr_t mem1 = 0;
	int curs = 0;
	int i;
	sys_move_cursor(2, 2);
	for (i = 0; i < NUM; i++)
	{
		
		mem1 = TEST_ADDR + (i << PAGE_SHIFT);
		// sys_move_cursor(2, curs+i);
		mem2[i] = rand();
		*(long*)mem1 = mem2[i];
		//printf("0x%lx, %ld\n", mem1, mem2[i]);
		if (*(long*)mem1 != mem2[i]) {
			printf("Error!\n");
		}
	}

	for(i = 0; i < NUM; i++)
	{
		mem1 = TEST_ADDR + (i << PAGE_SHIFT);
		if(i % 10 == 0)
			printf("0x%lx, %ld\n", mem1, mem2[i]);
		if (*(long*)mem1 != mem2[i]) {
			printf("Error!\n");
		}
	}
	//Only input address.
	//Achieving input r/w command is recommended but not required.
	printf("Success!\n");
	while(1);
	return 0;
}
