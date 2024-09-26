#include <common.h>
#include <asm.h>
#include <asm/unistd.h>
#include <asm/regs.h>
#include <os/loader.h>
#include <os/irq.h>
#include <os/sched.h>
#include <os/list.h>
#include <os/lock.h>
#include <os/kernel.h>
#include <os/task.h>
#include <os/string.h>
#include <os/mm.h>
#include <os/time.h>
#include <os/workload.h>	// for p2-task5
#include <sys/syscall.h>
#include <screen.h>
#include <printk.h>
#include <assert.h>
#include <os/pipe.h>		// for p1-task5, 约定了存放字符串的位置和大小，以及指向存放队列的两个指针
#include <os/batch.h>		// for p1-task5, batch_sh函数声明，以及task_queue的定义
#include <type.h>
#include <csr.h>

extern void ret_from_exception();

// Task info array
task_info_t tasks[TASK_MAXNUM];

char buf[50];

int tasknum;


static void init_jmptab(void)
{
	volatile long (*(*jmptab))() = (volatile long (*(*))())KERNEL_JMPTAB_BASE;

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
	// 
	jmptab[WRITE]		= (long (*)())screen_write;
	jmptab[FLUSH]		= (long (*)())screen_reflush;

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
/*
static void init_pipe_info(void)
{
	char** pipe_ptr;
	pipe_ptr = (char **)PIPE_HEAD_ADDR;
	*pipe_ptr = (char *)PIPE_BUFFER_ADDR;
	**pipe_ptr = 0;
	pipe_ptr = (char **)PIPE_TAIL_ADDR;
	*pipe_ptr = (char *)PIPE_BUFFER_ADDR;
}
*/


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
	regs_context_t *pt_regs = (regs_context_t *)(kernel_stack - sizeof(regs_context_t));
	
	pt_regs->sstatus = SR_SPIE;
	pt_regs->sepc = entry_point;
	pt_regs->regs[SP] = user_stack;
	pt_regs->regs[TP] = pcb;


	/* TODO: [p2-task1] set sp to simulate just returning from switch_to
	* NOTE: you should prepare a stack, and push some values to
	* simulate a callee-saved context.
	*/
	switchto_context_t *pt_switchto = (switchto_context_t *)((ptr_t)pt_regs - sizeof(switchto_context_t));

	pcb->kernel_sp = (reg_t)pt_switchto;
	pcb->user_sp = (reg_t)user_stack;

	pt_switchto->regs[0] = (reg_t)ret_from_exception;
	pt_switchto->regs[1] = pcb->kernel_sp;
	pt_switchto->regs[2] = 0;
	pt_switchto->regs[3] = 0;
	pt_switchto->regs[4] = 0;
	pt_switchto->regs[5] = 0;
	pt_switchto->regs[6] = 0;
	pt_switchto->regs[7] = 0;
	pt_switchto->regs[8] = 0;
	pt_switchto->regs[9] = 0;
	pt_switchto->regs[10] = 0;
	pt_switchto->regs[11] = 0;
	pt_switchto->regs[12] = 0;
	pt_switchto->regs[13] = 0;


}

static inline int add_new_task(char * str,int pid)
{
	ptr_t entrypoint;
	ptr_t kernel_stack,usr_stack;

	if((entrypoint = load_task_img_by_name(str)) != 0)
	{
		kernel_stack = allocKernelPage(1);
		usr_stack = allocUserPage(1);
		init_pcb_stack(kernel_stack, usr_stack, entrypoint, &pcb[pid-1]);
		pcb[pid-1].status = TASK_READY;
		pcb[pid-1].time_chunk = TIMER_INTERVAL;
		return 0;
	}
	else  
		return 1;
}


static void init_pcb(void)
{
	/* TODO: [p2-task1] load needed tasks and init their corresponding PCB */
	int i;
	for(i = 0; i < NUM_MAX_TASK; i++)
	{
		pcb[i].pid = i + 1;
		pcb[i].list.prev = NULL;
		pcb[i].list.next = NULL;
		pcb[i].list.pcb_ptr = (ptr_t)&pcb[i];
		pcb[i].status = TASK_EXITED;
		pcb[i].total_running = 0;
	}

	pid0_pcb.status = TASK_RUNNING;


	add_new_task("print1",1);

	add_new_task("print2",2);

	add_new_task("lock1",3);

	add_new_task("lock2",4);

	add_new_task("sleep",5);

	add_new_task("timer",6);

	add_new_task("fly",7);

	//add_new_task("fly1", 8);

	//add_new_task("fly2", 9);

	//add_new_task("fly3", 10);

	//add_new_task("fly4", 11);

	//add_new_task("fly5", 12);


	/* TODO: [p2-task1] remember to initialize 'current_running' */

	current_running = &pid0_pcb;
	process_id = pid0_pcb.pid;

	allocReadyProcess();

}

static void init_syscall(void)
{
	// TODO: [p2-task3] initialize system call table.
	syscall[SYSCALL_SLEEP] 		= (long (*)())do_sleep;
	syscall[SYSCALL_YIELD] 		= (long (*)())do_scheduler;
	syscall[SYSCALL_WRITE] 		= (long (*)())screen_write;
	syscall[SYSCALL_CURSOR] 	= (long (*)())screen_move_cursor;
	syscall[SYSCALL_REFLUSH] 	= (long (*)())screen_reflush;
	syscall[SYSCALL_GET_TIMEBASE] 	= (long (*)())get_time_base;
	syscall[SYSCALL_GET_TICK] 	= (long (*)())get_ticks;
	syscall[SYSCALL_LOCK_INIT] 	= (long (*)())do_mutex_lock_init;
	syscall[SYSCALL_LOCK_ACQ] 	= (long (*)())do_mutex_lock_acquire;
	syscall[SYSCALL_LOCK_RELEASE]	= (long (*)())do_mutex_lock_release;
	syscall[SYSCALL_WORKLOAD]	= (long (*)())do_workload_schedule;
}

static inline void getTask()
{
	char buf[64];
	int error;
	int c;
	int i = 0;
	int pid = 1;
	while(1)
	{
		if(((c=bios_getchar())==13 && i == 0) || pid > 16)
		{
			break;
		}
		else if(i < 49)
		{
			if(c == 13)	// press Enter
			{
				// refresh buff
				buf[i] = '\0';
				i = 0;

				bios_putchar(10);	// move the cursor to the next line

				if(strcmp(buf,"task1") == 0)
				{
					add_new_task("print1", pid++);
					add_new_task("print2", pid++);
					add_new_task("fly", pid++);
				}
				else if(strcmp(buf, "task2") == 0)
				{
					add_new_task("lock1", pid++);
					add_new_task("lock2", pid++);
					add_new_task("fly", pid++);
				}
				else if(strcmp(buf, "task3") == 0 || strcmp(buf, "task4") == 0)
				{
					add_new_task("print1", pid++);
					add_new_task("print2", pid++);
					add_new_task("lock1", pid++);
					add_new_task("lock2", pid++);
					add_new_task("sleep", pid++);
					add_new_task("timer", pid++);
					add_new_task("fly", pid++);
				}
				else
				{
					error = add_new_task(buf, pid);
					if(error)
						bios_putstr("\033[31mERROR:\033[0m no such task\n\r");
					else  
						pid++;
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
	}
}
/************************************************************/

int main(void)
{
	
	// Init jump table provided by kernel and bios(ΦωΦ)
	init_jmptab();

	// Init task information (〃'▽'〃)
	init_task_info();

	// Output 'Hello OS!'
	bios_putstr("Hello OS!\n\r");
	

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


	/*
	do_sleep(2);

	screen_clear();

	getTask();

	allocReadyProcess();

	do_sleep(2);

	screen_clear();
	*/


	// TODO: [p2-task4] Setup timer interrupt and 
	// enable all interrupt globally
	// NOTE: The function of sstatus.sie is different from sie's

	//enable_interrupt();

	bios_set_timer(get_ticks() + pid0_pcb.time_chunk);


	// Infinite while loop, where CPU stays in a low-power state (QAQQQQQQQQQQQ)
	while (1)
	{
		// If you do non-preemptive scheduling, it's used to surrender control
		// do_scheduler();

		// If you do preemptive scheduling, they're used to enable CSR_SIE and wfi
		enable_preempt();
		asm volatile("wfi");
	}

	return 0;
}
