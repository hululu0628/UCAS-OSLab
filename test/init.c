#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
int main()
{
	// init vm
	char buff[35];
	int fd = sys_open("proc/sys/vm", O_RDWR);
	int temp1, temp2;
	if(fd == -1)
	{
		sys_mkdir("proc");
		sys_cd("proc");
		sys_mkdir("sys");
		sys_cd("sys");
		sys_touch("vm");
		sys_cd("../..");
		fd = sys_open("proc/sys/vm", O_RDWR);
		sys_write(fd, "page_cache_policy = write through\n", 34);
		sys_write(fd, "write_back_freq = 30\n", 21);
	}
	sys_lseek(fd, 0, SEEK_SET);
	sys_read(fd, (char *)buff, 34);
	temp1 = strchr(buff, '=');
	if(strncmp(&buff[temp1 + 2], "write back", strlen("write back")) == 0)
	{
		sys_read(fd, (char *)buff, 35);
		temp1 = strchr(buff, '=');
		temp2 = strchr(&buff[temp1+2], '\n');
		buff[temp2] = '\0';
		temp2 = atoi(&buff[temp1+2]);
		sys_cache_change(WRITE_BACK, temp2);
	}
	else if(strncmp(&buff[temp1 + 2], "write through", strlen("write through")) == 0)
	{
		sys_cache_change(WRITE_THROUGH, 0);
	}
	else
	{
		printf("ERROR: invalid format in /proc/sys/vm\n");
	}
	sys_close(fd);

	int shell_argc = 1;
	char *shell_argv[shell_argc];
	shell_argv[0] = "shell";
	if(sys_fork() == 0)
	{
		sys_exec("shell",shell_argc,shell_argv);
	}
	while(1)
	{
		// restart shell
		if(sys_wait((int *) 0) == 1)
		{
			if(sys_fork() == 0)
				sys_exec("shell",shell_argc,shell_argv);
		}
	}
	return 0;
}