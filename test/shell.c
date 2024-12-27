/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * * * * * *
 *            Copyright (C) 2018 Institute of Computing Technology, CAS
 *               Author : Han Shukai (email : hanshukai@ict.ac.cn)
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * * * * * *
 *                  The shell acts as a task running in user mode.
 *       The main function is to make system calls through the user's output.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * * * * * *
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this
 * software and associated documentation files (the "Software"), to deal in the Software
 * without restriction, including without limitation the rights to use, copy, modify,
 * merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit
 * persons to whom the Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * * * * * */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>

#define SHELL_BEGIN 20
#define SHELL_END 50

#define MAX_ARGS 10
#define ARG_NAME 20

enum cmdtype
{
	UNKNOWN,
	PS,
	EXEC,
	KILL,
	CLEAR,
	TASKSET,
	MKFS,
	STATFS,
	MKDIR,
	RMDIR,
	CD,
	LS,
	TOUCH,
	RM,
	CAT,
	LN
};

typedef struct CMD
{
	enum cmdtype type;
	int argc;
	char *argv[MAX_ARGS];
}CMD;

char cmdstr[256];
char args[MAX_ARGS][ARG_NAME];
CMD cmd; 

void getcmdline();
void gettoken();

int main(void)
{
	sys_move_cursor(0, SHELL_BEGIN);
	printf("------------------- COMMAND -------------------\n");
	for(int i = 0; i < MAX_ARGS; i++)
	{
		cmd.argv[i] = args[i];
	}
	// printf("> root@UCAS_OS: ");
	while (1)
	{
		// TODO [P3-task1]: call syscall to read UART port
		getcmdline();
		if(cmdstr[0] != '\n' && cmdstr[0] != '\0')
		{
			gettoken();
			switch (cmd.type)
			{
			case PS:
				if(cmd.argc != 0)
					printf("ERROR\n");
				else
					sys_ps();
				break;
			case KILL:
				if(cmd.argc != 1)
					printf("ERROR\n");
				else
					sys_kill(atoi(args[0]));
				break;
			case CLEAR:
				if(cmd.argc != 0)
					printf("ERROR\n");
				else
				{
					sys_clear();
					sys_move_cursor(0, SHELL_BEGIN);
					printf("------------------- COMMAND -------------------\n");
				}
				break;
			case EXEC:
				if(cmd.argc == 0)
					printf("ERROR\n");
				else
				{
					if(cmd.argc != 1 && strcmp(args[cmd.argc - 1],"&") == 0)
					{
						if(sys_fork() == 0)
						{
							//while(1);
							sys_exec(args[0], cmd.argc - 1, (char **)cmd.argv);
						}
					}
					else
					{
						// unfinished
						int pid;
						if((pid = sys_fork()) == 0)
							sys_exec(args[0], cmd.argc, (char **)cmd.argv);
						else
							sys_waitpid(pid);
					}
				}
				break;
			case TASKSET:
				if(cmd.argc < 2)
					printf("ERROR\n");
				else
				{
					if(cmd.argc != 1 && strcmp(args[cmd.argc - 1],"&") == 0)
					{
						if(strcmp(args[0], "-p") != 0)
						{
							if(sys_fork() == 0)
								sys_taskset(cmd.argc - 1, (char **)cmd.argv);
						}
						else
							sys_taskset(cmd.argc - 1, (char **)cmd.argv);
					}
					else
					{
						int pid;
						pid = sys_taskset(cmd.argc, (char **)cmd.argv);
						sys_waitpid(pid);
					}
				}
				break;
			case MKFS:
				if(cmd.argc != 0)
					printf("ERROR\n");
				else
				{
					sys_mkfs();
				}
				break;
			case STATFS:
				if(cmd.argc != 0)
					printf("ERROR\n");
				else
				{
					sys_statfs();
				}
				break;
			case MKDIR:
				if(cmd.argc != 1)
					printf("ERROR\n");
				else
				{
					sys_mkdir(cmd.argv[0]);
				}
				break;
			case RMDIR:
				if(cmd.argc != 1)
					printf("ERROR\n");
				else
				{
					sys_rmdir(cmd.argv[0]);
				}
				break;
			case CD:
				if(cmd.argc != 1)
					printf("ERROR\n");
				else
				{
					sys_cd(cmd.argv[0]);
				}
				break;
			case LS:
				if(cmd.argc == 0)
					sys_ls(".", NORMAL_LIST);
				else if(cmd.argc == 1)
				{
					if(strcmp(cmd.argv[0], "-l") == 0)
						sys_ls(".", LONG_LIST);
					else
						sys_ls(cmd.argv[0], NORMAL_LIST);
				}
				else if(cmd.argc == 2)
				{
					if(strcmp(cmd.argv[0], "-l") == 0)
						sys_ls(cmd.argv[1], LONG_LIST);
					else if(strcmp(cmd.argv[1], "-l") == 0)
						sys_ls(cmd.argv[0], LONG_LIST);
				}
				else
					printf("ERROR\n");
				break;
			case TOUCH:
				if(cmd.argc != 1)
					printf("ERROR\n");
				else
				{
					sys_touch(cmd.argv[0]);
				}
				break;
			case RM:
				if(cmd.argc != 1)
					printf("ERROR\n");
				else
				{
					sys_rm(cmd.argv[0]);
				}
				break;
			case CAT:
				if(cmd.argc != 1)
					printf("ERROR\n");
				else
				{
					sys_cat(cmd.argv[0]);
				}
				break;
			case LN:
				if(cmd.argc != 2)
					printf("ERROR\n");
				else
				{
					sys_ln(cmd.argv[0], cmd.argv[1]);
				}
				break;
			default: printf("Unknown command\n");break;
			}
		}

		// TODO [P3-task1]: parse input
		// note: backspace maybe 8('\b') or 127(delete)

		// TODO [P3-task1]: ps, exec, kill, clear    

        /************************************************************/
        // TODO [P6-task1]: mkfs, statfs, cd, mkdir, rmdir, ls

        // TODO [P6-task2]: touch, cat, ln, ls -l, rm
        /************************************************************/
    }

	return 0;
}

void getcmdline()
{
	char buff[256];
	sys_getcwd(buff);
	printf("> hululu@UCAS_OS:%s#",buff);
	gets(cmdstr);
}

void gettoken(void)
{
	char token[32];
	int start,end;
	start = 0;
	end = strchr(cmdstr,' ');
	cmd.argc = 0;

	// get command name
	strncpy(token, &cmdstr[start], end - start);
	token[end - start] = '\0';
	if(strcmp(token,"ps") == 0)
		cmd.type = PS;
	else if(strcmp(token,"exec") == 0)
		cmd.type = EXEC;
	else if(strcmp(token,"kill") == 0)
		cmd.type = KILL;
	else if(strcmp(token,"clear") == 0)
		cmd.type = CLEAR;
	else if(strcmp(token,"taskset") == 0)
		cmd.type = TASKSET;
	else if(strcmp(token,"mkfs") == 0)
		cmd.type = MKFS;
	else if(strcmp(token,"statfs") == 0)
		cmd.type = STATFS;
	else if(strcmp(token,"mkdir") == 0)
		cmd.type = MKDIR;
	else if(strcmp(token,"rmdir") == 0)
		cmd.type = RMDIR;
	else if(strcmp(token,"cd") == 0)
		cmd.type = CD;
	else if(strcmp(token,"ls") == 0)
		cmd.type = LS;
	else if(strcmp(token,"touch") == 0)
		cmd.type = TOUCH;
	else if(strcmp(token,"rm") == 0)
		cmd.type = RM;
	else if(strcmp(token,"cat") == 0)
		cmd.type = CAT;
	else if(strcmp(token,"ln") == 0)
		cmd.type = LN;
	else
		cmd.type = UNKNOWN;

	// get parameters
	if(cmdstr[end] != '\0')
	{
		strncpy(token, &cmdstr[start], end - start);
		token[end - start] = '\0';
		if(strcmp(token,"ps") == 0)
			cmd.type = PS;
		else if(strcmp(token,"exec") == 0)
			cmd.type = EXEC;
		else if(strcmp(token,"kill") == 0)
			cmd.type = KILL;
		else if(strcmp(token,"clear") == 0)
			cmd.type = CLEAR;
		else if(strcmp(token,"taskset") == 0)
			cmd.type = TASKSET;
		else if(strcmp(token,"mkfs") == 0)
			cmd.type = MKFS;
		else if(strcmp(token,"statfs") == 0)
			cmd.type = STATFS;
		else if(strcmp(token,"mkdir") == 0)
			cmd.type = MKDIR;
		else if(strcmp(token,"rmdir") == 0)
			cmd.type = RMDIR;
		else if(strcmp(token,"cd") == 0)
			cmd.type = CD;
		else if(strcmp(token,"ls") == 0)
			cmd.type = LS;
		else if(strcmp(token,"touch") == 0)
			cmd.type = TOUCH;
		else if(strcmp(token,"rm") == 0)
			cmd.type = RM;
		else if(strcmp(token,"cat") == 0)
			cmd.type = CAT;
		else if(strcmp(token,"ln") == 0)
			cmd.type = LN;
		else
			cmd.type = UNKNOWN;

		start = end + 1;
		while(1)
		{
			end = strchr(cmdstr + start,' ') + start;
			if(cmdstr[end] != '\0')
			{
				if(start != end)
				{
					cmd.argc++;
					strncpy(token, &cmdstr[start],end - start);
					token[end - start] = '\0';
					strcpy(args[cmd.argc - 1], token);
				}
				start = end + 1;
			}
			else
			{
				if(start != end)
				{
					cmd.argc++;
					strncpy(token, &cmdstr[start],end - start);
					token[end - start] = '\0';
					strcpy(args[cmd.argc - 1], token);
					break;
				}
			}
		}
	}
}