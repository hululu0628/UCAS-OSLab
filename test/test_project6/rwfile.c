#include <stdio.h>
#include <string.h>
#include <unistd.h>

static char buff[64];

int main(void)
{
	sys_move_cursor(0, 0);
	
	int fd = sys_open("1.txt", O_RDWR);

	if(fd == -1)
		return -1;

	// write 'hello world!' * 10
	for (int i = 0; i < 10; i++)
	{
		sys_write(fd, "hello world!\n", 13);
	}
	
	sys_lseek(fd, 0, SEEK_SET);
	
	// read
	for (int i = 0; i < 10; i++)
	{
		sys_read(fd, buff, 13);
		for (int j = 0; j < 13; j++)
		{
		printf("%c", buff[j]);
		}
	}

	sys_close(fd);

	return 0;
}