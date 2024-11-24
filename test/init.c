#include <unistd.h>
int main()
{
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