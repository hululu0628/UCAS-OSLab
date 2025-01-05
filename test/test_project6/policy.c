#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main(int argc, char *argv[])
{
	sys_move_cursor(0, 0);
	if(argc < 2)
	{
		printf("Illegal input\n");
		return -1;
	}

	int fd = sys_open("proc/sys/vm", O_RDWR);
	if(strcmp(argv[1], "-t") == 0 && argc == 2)
	{
		sys_lseek(fd, 0, SEEK_SET);
		sys_write(fd, "page_cache_policy = write through\n", 34);
		sys_cache_change(WRITE_THROUGH, 0);
		printf("cache policy: write through\n");
	}
	else if(strcmp(argv[1], "-b") == 0 && argc == 3)
	{
		sys_lseek(fd, 0, SEEK_SET);
		sys_write(fd, "page_cache_policy = write back   \n", 34);
		sys_write(fd, "write_back_freq = ", 18);
		sys_write(fd, argv[2], strlen(argv[2]));
		sys_write(fd, "\n", 1);
		sys_cache_change(WRITE_BACK, atoi(argv[2]));

		printf("cache policy: write back, update frequency: %ds\n", atoi(argv[2]));
	}
	else
	{
		printf("Illegal input\n");
		return -1;
	}

	sys_close(fd);

	return 0;
}