#include <common.h>
#include <asm.h>
#include <os/kernel.h>
#include <os/task.h>
#include <os/string.h>
#include <os/loader.h>
#include <type.h>

#define VERSION_BUF 50

int version = 2; // version must between 0 and 9
char buf[VERSION_BUF];

// Task info array
task_info_t tasks[TASK_MAXNUM];
int tasknum;

static int bss_check(void)
{
	for (int i = 0; i < VERSION_BUF; ++i)
	{
		if (buf[i] != 0)
		{
		return 0;
		}
	}
	return 1;
}

static void init_jmptab(void)
{
	volatile long (*(*jmptab))() = (volatile long (*(*))())KERNEL_JMPTAB_BASE;

	jmptab[CONSOLE_PUTSTR]  = (long (*)())port_write;
	jmptab[CONSOLE_PUTCHAR] = (long (*)())port_write_ch;
	jmptab[CONSOLE_GETCHAR] = (long (*)())port_read_ch;
	jmptab[SD_READ]         = (long (*)())sd_read;
}

static void init_task_info(void)
{
	// TODO: [p1-task4] Init 'tasks' array via reading app-info sector
	// NOTE: You need to get some related arguments from bootblock first
	task_info_t * taskinfo_ptr = (task_info_t *)0x50200200;
	for(int i = 0; i < TASK_MAXNUM; i++, taskinfo_ptr++)
		tasks[i] = *taskinfo_ptr;
	tasknum = *((int *)0x502001fe);

}

/************************************************************/
/* Do not touch this comment. Reserved for future projects. */
/************************************************************/

int main(void)
{
	// Check whether .bss section is set to zero
	int check = bss_check();

	// Init jump table provided by kernel and bios(ΦωΦ)
	init_jmptab();

	// Init task information (〃'▽'〃)
	init_task_info();

	// Output 'Hello OS!', bss check result and OS version
	char output_str[] = "bss check: _ version: _\n\r";
	char output_val[2] = {0};
	int i, output_val_pos = 0;

	output_val[0] = check ? 't' : 'f';
	output_val[1] = version + '0';
	for (i = 0; i < sizeof(output_str); ++i)
	{
		buf[i] = output_str[i];
		if (buf[i] == '_')
		{
		buf[i] = output_val[output_val_pos++];
		}
	}

	bios_putstr("Hello OS!\n\r");
	bios_putstr(buf);
	

	// TODO: Load tasks by either task id [p1-task3] or task name [p1-task4],
	//   and then execute them.
	int taskid = 0;
	int c;
	int (*funpt)(void);
	i = 0;
	while(1)
	{
		if(i < 49)
		{
			if((c=bios_getchar())==13)	//press enter
			{
				buf[i] = '\0';
				i = 0;

				bios_putchar(10);
				
				while(taskid < tasknum)
				{
					if(strcmp(tasks[taskid].filename, buf)==0)
					{
						funpt = (int (*)(void))load_task_img(taskid);
						funpt();
						break;
					}
					taskid++;
				}
				if(taskid >= tasknum)
					bios_putstr("ERROR:no such task\n\r");
				taskid = 0;
			}
			else if(c != -1)
			{
				bios_putchar(c);
				buf[i++] = c;
			}
		}
		else
		{
			bios_putstr("\n\rWARNING: the name is too long\n\r");
			i = 0;
		}
	}

	// Infinite while loop, where CPU stays in a low-power state (QAQQQQQQQQQQQ)
	while (1)
	{
		asm volatile("wfi");
	}

	return 0;
}
