#include <pgtable.h>
#include <common.h>
#include <asm.h>
#include <asm/unistd.h>
#include <asm/regs.h>
#include <os/smp.h>
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
#include <sys/syscall.h>
#include <screen.h>
#include <printk.h>
#include <assert.h>
#include <type.h>
#include <csr.h>


// Task info array
task_info_t tasks[TASK_MAXNUM];

static void init_jmptab(void)
{
	volatile long (*(*jmptab))() = (volatile long (*(*))())KERNEL_JMPTAB_BASE;

    	jmptab[CONSOLE_PUTSTR]  = (volatile long (*)())port_write;
    	jmptab[CONSOLE_PUTCHAR] = (volatile long (*)())port_write_ch;
    	jmptab[CONSOLE_GETCHAR] = (volatile long (*)())port_read_ch;
    	jmptab[SD_READ]         = (volatile long (*)())sd_read;
    	jmptab[SD_WRITE]        = (volatile long (*)())sd_write;
    	jmptab[QEMU_LOGGING]    = (volatile long (*)())qemu_logging;
    	jmptab[SET_TIMER]       = (volatile long (*)())set_timer;
    	jmptab[READ_FDT]        = (volatile long (*)())read_fdt;
    	jmptab[MOVE_CURSOR]     = (volatile long (*)())screen_move_cursor;
    	jmptab[PRINT]           = (volatile long (*)())printk;
    	jmptab[YIELD]           = (volatile long (*)())do_scheduler;
    	jmptab[MUTEX_INIT]      = (volatile long (*)())do_mutex_lock_init;
    	jmptab[MUTEX_ACQ]       = (volatile long (*)())do_mutex_lock_acquire;
    	jmptab[MUTEX_RELEASE]   = (volatile long (*)())do_mutex_lock_release;

	// TODO: [p2-task1] (S-core) initialize system call table.
	jmptab[WRITE]		= (volatile long (*)())screen_write;
	jmptab[REFLUSH]		= (volatile long (*)())screen_reflush;

}

static void init_task_info(void)
{
	// TODO: [p1-task4] Init 'tasks' array via reading app-info sector
	// NOTE: You need to get some related arguments from bootblock first
	task_info_t * taskinfo_ptr = (task_info_t *)0x50200200;
	for(int i = 0; i < TASK_MAXNUM; i++, taskinfo_ptr++)
		tasks[i] = *taskinfo_ptr;
	// tasknum = *((int *)0x502001fe);

}


/************************************************************/

static inline void load_shell()
{
	int shell_argc = 2;
	char *shell_argv[shell_argc];
	shell_argv[0] = "0x3";
	shell_argv[1] = "shell";
	int pid = do_taskset(shell_argc, shell_argv);

	// there's a temp content in kernel pgdir 
	// (0x5000000~0x51000000 to 0x50000000~0x51000000)
	// the index of pgd is one 
	((PTE *)pcb[pid-1].pgdir)[1] = 0;
}


static void init_pcb(void)
{
	/* TODO: [p2-task1] load needed tasks and init their corresponding PCB */
	int i;
	// initialize pcb array (for user)
	for(i = 0; i < NUM_MAX_TASK; i++)
	{
		pcb[i].pid = i + 1;
		pcb[i].list.prev = NULL;
		pcb[i].list.next = NULL;
		pcb[i].list.pcb_ptr = (ptr_t)&pcb[i];
		pcb[i].wait_list.next = &pcb[i].wait_list;
		pcb[i].wait_list.prev = &pcb[i].wait_list;
		pcb[i].status = TASK_EXITED;			// useless?
		pcb[i].current_core_id = NO_CORE;
	}


}

static void init_syscall(void)
{
	// TODO: [p2-task3] initialize system call table.

	// process management
	syscall[SYSCALL_SLEEP] 		= (long (*)())do_sleep;
	syscall[SYSCALL_YIELD] 		= (long (*)())do_scheduler;
	syscall[SYSCALL_TASKSET]	= (long (*)())do_taskset;
	syscall[SYSCALL_PS]		= (long (*)())do_process_show;
	syscall[SYSCALL_EXEC] 		= (long (*)())exec;
	syscall[SYSCALL_EXIT]		= (long (*)())do_exit;
	syscall[SYSCALL_KILL]		= (long (*)())do_kill;
	syscall[SYSCALL_GETPID]		= (long (*)())do_getpid;
	syscall[SYSCALL_WAITPID] 	= (long (*)())do_waitpid;
	syscall[SYSCALL_FORK]		= (long (*)())do_fork;

	// get input from terminal
	syscall[SYSCALL_GETCHAR]	= (long (*)())bios_getchar;

	// screen
	syscall[SYSCALL_PUTCHAR]	= (long (*)())screen_putchar;
	syscall[SYSCALL_WRITE] 		= (long (*)())screen_write;
	syscall[SYSCALL_CURSOR] 	= (long (*)())screen_move_cursor;
	syscall[SYSCALL_REFLUSH] 	= (long (*)())screen_reflush;
	syscall[SYSCALL_CLEAR]		= (long (*)())screen_clear;

	// cpu time attribute
	syscall[SYSCALL_GET_TIMEBASE] 	= (long (*)())get_time_base;
	syscall[SYSCALL_GET_TICK] 	= (long (*)())get_ticks;

	// mutex
	syscall[SYSCALL_LOCK_INIT] 	= (long (*)())do_mutex_lock_init;
	syscall[SYSCALL_LOCK_ACQ] 	= (long (*)())do_mutex_lock_acquire;
	syscall[SYSCALL_LOCK_RELEASE]	= (long (*)())do_mutex_lock_release;
	
	// barrier
	syscall[SYSCALL_BARR_INIT]	= (long (*)())do_barrier_init;
	syscall[SYSCALL_BARR_WAIT]	= (long (*)())do_barrier_wait;
	syscall[SYSCALL_BARR_DESTROY]	= (long (*)())do_barrier_destroy;

	// condition
	syscall[SYSCALL_COND_INIT]	= (long (*)())do_condition_init;
	syscall[SYSCALL_COND_WAIT]	= (long (*)())do_condition_wait;
	syscall[SYSCALL_COND_SIGNAL]	= (long (*)())do_condition_signal;
	syscall[SYSCALL_COND_BROADCAST]	= (long (*)())do_condition_broadcast;
	syscall[SYSCALL_COND_DESTROY]	= (long (*)())do_condition_destroy;

	// mailbox
	syscall[SYSCALL_MBOX_OPEN]	= (long (*)())do_mbox_open;
	syscall[SYSCALL_MBOX_CLOSE]	= (long (*)())do_mbox_close;
	syscall[SYSCALL_MBOX_SEND]	= (long (*)())do_mbox_send;
	syscall[SYSCALL_MBOX_RECV]	= (long (*)())do_mbox_recv;
}

/*
 * Once a CPU core calls this function,
 * it will stop executing!
 */
static void kernel_brake(void)
{
    disable_interrupt();
    while (1)
        __asm__ volatile("wfi");
}


static void cleanTempPgtab()
{
	uint64_t vpn2 = 1lu;

	// freePage(pa2kva(get_pa(((PTE *)pa2kva(PGDIR_PA))[vpn2])));

	((PTE *)pa2kva(PGDIR_PA))[vpn2] = 0;

	local_flush_tlb_all();
}

static void inline init_sstatus(void)
{
	asm volatile(
		"li	t0,0x00040000\n\t"
		"csrs	sstatus,t0\n\t"
	);
}

/*********************************************************************/

int main(void)
{
	if(get_current_cpu_id() == 0)
	{
		init_page();

		// Init jump table provided by kernel and bios(ΦωΦ)
		init_jmptab();

		// Init task information (〃'▽'〃)
		init_task_info();

		// Output 'Hello OS!'
		bios_putstr("Hello OS!\n\r");
		

		// Init Process Control Blocks |•'-'•) ✧
		init_pcb();

		init_pcb0(0);
		
		printk("> [INIT] PCB initialization succeeded.\n");

		// Read CPU frequency (｡•ᴗ-)_
		time_base = bios_read_fdt(TIMEBASE);

		// Init lock mechanism o(´^｀)o
		init_ipc();
		printk("> [INIT] Lock mechanism initialization succeeded.\n");

		// Init interrupt (^_^)
		init_trap();
		printk("> [INIT] Interrupt processing initialization succeeded.\n");

		// Init system call table (0_0)
		init_syscall();
		printk("> [INIT] System call initialized successfully.\n");

		// Init screen (QAQ)
		init_screen();
		//printk("> [INIT] SCREEN initialization succeeded.\n");

		load_shell();

		wakeup_other_hart();


		/*
		* Just start kernel with VM and print this string
		* in the first part of task 1 of project 4.
		* NOTE: if you use SMP, then every CPU core should call
		*  `kernel_brake()` to stop executing!
		*/
		printl("> [INIT] CPU #%u has entered kernel with VM!\n",
			(unsigned int)get_current_cpu_id());
		// TODO: [p4-task1 cont.] remove the brake and continue to start user processes.
		//kernel_brake();

	}
	else
	{
		smp_init();

		cleanTempPgtab();

		printl("> [INIT] CPU #%u has entered kernel with VM!\n",
			(unsigned int)get_current_cpu_id());

		// kernel_brake();
	}

	init_sstatus();

	// TODO: [p2-task4] Setup timer interrupt and 
	// enable all interrupt globally
	// NOTE: The function of sstatus.sie is different from sie's

	//enable_interrupt();

	bios_set_timer(get_ticks() + TIMER_INTERVAL);


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
