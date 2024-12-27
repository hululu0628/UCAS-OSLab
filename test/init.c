#include <stdio.h>
#include <unistd.h>
int main()
{
	// init vm
	int fd = sys_open("proc/sys/vm", O_RDWR);
	if(fd == -1)
		return -1;
	sys_write(fd, "page_cache_policy = write through\n", 34);
	sys_write(fd, "write_back_freq = 30\n", 21);
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