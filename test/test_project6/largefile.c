#include <stdio.h>
#include <string.h>
#include <unistd.h>

static char buff[64];

#define TEST_POS 0x8000000

int main(void)
{
	sys_move_cursor(0, 0);
	
	int fd = sys_open("1.txt", O_RDWR);

	if(fd == -1)
		return 1;

	sys_write(fd, "hello world!\n", 13);

	sys_lseek(fd, TEST_POS, SEEK_SET);

	sys_write(fd, "hello os!\n", 10);
	
	sys_lseek(fd, 0, SEEK_SET);
	
	sys_read(fd, buff, 13);
	for (int j = 0; j < 13; j++)
	{
		printf("%c", buff[j]);
	}

	sys_lseek(fd, TEST_POS, SEEK_SET);
	
	sys_read(fd, buff, 10);
	for (int j = 0; j < 10; j++)
	{
		printf("%c", buff[j]);
	}

	sys_close(fd);

	return 0;
}