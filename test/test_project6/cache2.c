#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

static char buff[32];

int main()
{
	sys_move_cursor(0, 0);
	int fd = sys_open("cache_test", O_RDWR);
	if(fd == -1)
		return -1;

	clock_t start, end;

	start = clock();
	for(int i = 0; i < 10; i++)
	{
		sys_lseek(fd, i << 12, SEEK_SET);
		sys_read(fd,buff, 13);
	}
	end = clock();

	printf("time: %ld\n", (end - start));


	start = clock();
	for(int i = 0; i < 10; i++)
	{
		sys_lseek(fd, i << 12, SEEK_SET);
		sys_read(fd,buff, 13);
	}
	end = clock();

	printf("time: %ld\n", (end - start));

	return 0;
}