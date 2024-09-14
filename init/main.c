#include <common.h>
#include <asm.h>
#include <asm/unistd.h>
#include <os/loader.h>
#include <os/irq.h>
#include <os/sched.h>
#include <os/lock.h>
#include <os/kernel.h>
#include <os/task.h>
#include <os/string.h>
#include <os/mm.h>
#include <os/time.h>
#include <sys/syscall.h>
#include <screen.h>
#include <printk.h>
#include <assert.h>
#include <os/pipe.h>		//约定了存放字符串的位置和大小，以及指向存放队列的两个指针
#include <os/batch.h>		//batch_sh函数声明，以及task_queue的定义
#include <type.h>
#include <csr.h>

extern void ret_from_exception();

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
    jmptab[CONSOLE_PUTSTR]  = (long (*)())port_write;
    jmptab[CONSOLE_PUTCHAR] = (long (*)())port_write_ch;
    jmptab[CONSOLE_GETCHAR] = (long (*)())port_read_ch;
    jmptab[SD_READ]         = (long (*)())sd_read;
    jmptab[SD_WRITE]        = (long (*)())sd_write;
    jmptab[QEMU_LOGGING]    = (long (*)())qemu_logging;
    jmptab[SET_TIMER]       = (long (*)())set_timer;
    jmptab[READ_FDT]        = (long (*)())read_fdt;
    jmptab[MOVE_CURSOR]     = (long (*)())screen_move_cursor;
    jmptab[PRINT]           = (long (*)())printk;
    jmptab[YIELD]           = (long (*)())do_scheduler;
    jmptab[MUTEX_INIT]      = (long (*)())do_mutex_lock_init;
    jmptab[MUTEX_ACQ]       = (long (*)())do_mutex_lock_acquire;
    jmptab[MUTEX_RELEASE]   = (long (*)())do_mutex_lock_release;

    // TODO: [p2-task1] (S-core) initialize system call table.

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

//(for p1-t5)
static void init_pipe_info(void)
{
	char** pipe_ptr;
	pipe_ptr = (char **)PIPE_HEAD_ADDR;
	*pipe_ptr = (char *)PIPE_BUFFER_ADDR;
	**pipe_ptr = 0;
	pipe_ptr = (char **)PIPE_TAIL_ADDR;
	*pipe_ptr = (char *)PIPE_BUFFER_ADDR;
}
//测试用批处理
void batch_sh(task_queue *array, int *taskhead)
{
	int taskid = 0;
	//加载四个程序，并将相关信息存入array
	while (taskid < tasknum) {
		if (strcmp(tasks[taskid].filename, "printstr") == 0)
		{
			array[*taskhead].taskid = taskid;
			array[*taskhead].object_file = (char)(TERMINAL_OUT | PIPE_IN);
			array[*taskhead].p = (int (*)(void))load_task_img(taskid);
			(*taskhead)++;
			if (*taskhead == TASK_MAXNUM)
				*taskhead = 0;
			break;
		}
		taskid++;
	}
	taskid = 0;

	while (taskid < tasknum) {
		if (strcmp(tasks[taskid].filename, "strsort") == 0)
		{
			array[*taskhead].taskid = taskid;
			array[*taskhead].object_file = (char)(PIPE_IN | PIPE_OUT);
			array[*taskhead].p = (int (*)(void))load_task_img(taskid);
			(*taskhead)++;
			if (*taskhead == TASK_MAXNUM)
				*taskhead = 0;
			break;
		}
		taskid++;
	}
	taskid = 0;

	while (taskid < tasknum) {
		if (strcmp(tasks[taskid].filename, "duplication") == 0)
		{
			array[*taskhead].taskid = taskid;
			array[*taskhead].object_file = (char)(PIPE_IN | PIPE_OUT);
			array[*taskhead].p = (int (*)(void))load_task_img(taskid);
			(*taskhead)++;
			if (*taskhead == TASK_MAXNUM)
				*taskhead = 0;
			break;
		}
		taskid++;
	}
	taskid = 0;

	while (taskid < tasknum) {
		if (strcmp(tasks[taskid].filename, "format") == 0)
		{
			array[*taskhead].taskid = taskid;
			array[*taskhead].object_file = (char)(TERMINAL_IN | PIPE_OUT);
			array[*taskhead].p = (int (*)(void))load_task_img(taskid);
			(*taskhead)++;
			if (*taskhead == TASK_MAXNUM)
				*taskhead = 0;
			break;
		}
		taskid++;
	}
	taskid = 0;
}

/************************************************************/
static void init_pcb_stack(
    ptr_t kernel_stack, ptr_t user_stack, ptr_t entry_point,
    pcb_t *pcb)
{
     /* TODO: [p2-task3] initialization of registers on kernel stack
      * HINT: sp, ra, sepc, sstatus
      * NOTE: To run the task in user mode, you should set corresponding bits
      *     of sstatus(SPP, SPIE, etc.).
      */
    regs_context_t *pt_regs =
        (regs_context_t *)(kernel_stack - sizeof(regs_context_t));


    /* TODO: [p2-task1] set sp to simulate just returning from switch_to
     * NOTE: you should prepare a stack, and push some values to
     * simulate a callee-saved context.
     */
    switchto_context_t *pt_switchto =
        (switchto_context_t *)((ptr_t)pt_regs - sizeof(switchto_context_t));

}

static void init_pcb(void)
{
    /* TODO: [p2-task1] load needed tasks and init their corresponding PCB */


    /* TODO: [p2-task1] remember to initialize 'current_running' */

}

static void init_syscall(void)
{
    // TODO: [p2-task3] initialize system call table.
}
/************************************************************/

int main(void)
{
	// Check whether .bss section is set to zero
	int check = bss_check();

	// Init jump table provided by kernel and bios(ΦωΦ)
	init_jmptab();

	// Init task information (〃'▽'〃)
	init_task_info();

	// Init pipe information ○|￣|_
	init_pipe_info();

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
	
    // Init Process Control Blocks |•'-'•) ✧
    init_pcb();
    printk("> [INIT] PCB initialization succeeded.\n");

    // Read CPU frequency (｡•ᴗ-)_
    time_base = bios_read_fdt(TIMEBASE);

    // Init lock mechanism o(´^｀)o
    init_locks();
    printk("> [INIT] Lock mechanism initialization succeeded.\n");

    // Init interrupt (^_^)
    init_exception();
    printk("> [INIT] Interrupt processing initialization succeeded.\n");

    // Init system call table (0_0)
    init_syscall();
    printk("> [INIT] System call initialized successfully.\n");

    // Init screen (QAQ)
    init_screen();
    printk("> [INIT] SCREEN initialization succeeded.\n");

    // TODO: [p2-task4] Setup timer interrupt and enable all interrupt globally
    // NOTE: The function of sstatus.sie is different from sie's
    


	// TODO: Load tasks by either task id [p1-task3] or task name [p1-task4],
	//   and then execute them.
	int taskid = 0;
	int c;
	task_queue taskqueue[TASK_MAXNUM];
	int taskhead = 0, tasktail = 0;
	task_crtl_info* ctrl_adr = (task_crtl_info *)PROGRAM_CONTROL_ADDR;
	i = 0;

 
	while(1)
	{
		if(i < 49)
		{
			if((c=bios_getchar())==13)	// press Enter
			{
				// refresh buff
				buf[i] = '\0';
				i = 0;

				bios_putchar(10);	// move the cursor to the next line
				
				if(strcmp(buf,"batch")==0)	// p1-task5: batch
					batch_sh(taskqueue, &taskhead);
				else
				{
					while(taskid < tasknum)
					{
						// if buf[0] == '\0' while (tasks[taskid].filename)[0] == '\0'
						// the taskqueue should not be updated
						if(buf[0] && (strcmp(tasks[taskid].filename, buf)==0))
						{
							// update the queue
							taskqueue[taskhead].taskid = taskid;
							// The targets of both input and output are terminals
							taskqueue[taskhead].object_file = (char)(TERMINAL_IN | TERMINAL_OUT);
							// load program from SD card
							taskqueue[taskhead].p = (int (*)(void))load_task_img(taskid);
							taskhead++;
							if(taskhead == TASK_MAXNUM)
								taskhead = 0;
							break;
						}
						taskid++;
					}
					if(buf[0] && (taskid >= tasknum))
						bios_putstr("\033[31mERROR:\033[0m no such task\n\r");
					taskid = 0;
				}
			}
			else if(c != -1)
			{
				if(c != 127)
				{
					bios_putchar(c);
					buf[i++] = c;
				}
				else
				{
					// after pressing Backspace
					if(i > 0)
						buf[--i] = '\0';
					bios_putstr("\b \b");
				}
			}
		}
		else
		{
			bios_putstr("\n\r\033[33mWARNING:\033[0m the name is too long\n\r");
			i = 0;
		}

		// check the queue
		// if there are pending tasks, excute in turn
		while(taskhead != tasktail)
		{
			// Based on the value of the contents of the address
			// function in pipe.c determines the targets of both input and output
			(*ctrl_adr).io_d = taskqueue[tasktail].object_file;
			// jump to the entry point of the program
			(taskqueue[tasktail].p)();
			tasktail++;
			if(tasktail == TASK_MAXNUM)
				tasktail = 0;
		}

	}

	// Infinite while loop, where CPU stays in a low-power state (QAQQQQQQQQQQQ)
	while (1)
	{
		asm volatile("wfi");
	}
    // Infinite while loop, where CPU stays in a low-power state (QAQQQQQQQQQQQ)
    while (1)
    {
        // If you do non-preemptive scheduling, it's used to surrender control
        do_scheduler();

        // If you do preemptive scheduling, they're used to enable CSR_SIE and wfi
        // enable_preempt();
        // asm volatile("wfi");
    }

	return 0;
}
