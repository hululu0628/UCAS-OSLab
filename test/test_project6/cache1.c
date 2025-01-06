#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main()
{
	sys_move_cursor(0, 0);
	int fd = sys_open("cache_test", O_RDWR);
	if(fd == -1)
		return -1;

	for(int i = 0; i < 16; i++)
	{
		sys_lseek(fd, i << 20, SEEK_SET);
		sys_write(fd,"hello world!\n", 13);
		sys_move_cursor(0, 0);
		printf("%d\n",i);
	}

	printf("Success\n");

	return 0;
}