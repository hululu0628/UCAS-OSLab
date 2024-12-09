#include <pgtable.h>
#include <common.h>
#include <screen.h>
#include <os/net.h>
#include <plic.h>
#include <os/ioremap.h>
#include <e1000.h>
#include <printk.h>
#include <assert.h>
#include <type.h>
#include <csr.h>
#include <asm.h>
#include <asm/unistd.h>
#include <asm/regs.h>
#include <sys/syscall.h>
#include <os/kernel.h>
#include <os/loader.h>
#include <os/task.h>
#include <os/string.h>
#include <os/time.h>
#include <os/irq.h>
#include <os/proc.h>
#include <os/list.h>
#include <os/lock.h>
#include <os/smp.h>
#include <os/mm.h>
#include <os/swap.h>
#include <os/pthread.h>
#include <mode.h>



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

static inline void load_init()
{
	int task_id = find_task("init");
	pcb[0].task_id = task_id;

	ptr_t kernel_stack,kusr_stack;
	PTE * pgdir;
	int block_id;
	int block_num;
	int page_number;
	uint64_t kaddr;
	pageframe * t;

	char*init_argv[1];
	init_argv[0] = "init";

	if(task_id != -1)
	{
		t = allocPgtabPage();
		pgdir = (PTE *)GET_KADDR(t->page_num);

		share_pgtable((uintptr_t)pgdir, pa2kva(PGDIR_PA));

		pcb[0].pgdir = pgdir;
		page_number = 1 + (tasks[task_id].mem_size >> NORMAL_PAGE_SHIFT);

		block_id = tasks[task_id].block_id;
		block_num = tasks[task_id].block_num;
		// 加载init的代码段和数据段
		for(uint64_t va = USER_ENTRYPOINT, i = 0; i < page_number; va += PAGE_SIZE, i++)
		{
			kaddr = alloc_page_helper(va, pgdir, _PAGE_PRESENT 
				| _PAGE_READ | _PAGE_WRITE | _PAGE_EXEC | _PAGE_ACCESSED | _PAGE_DIRTY | _PAGE_USER);
			if(block_num >= 8)
				load_task_l(kaddr,block_id,8);
			else
				load_task_l(kaddr,block_id,block_num);
			if(block_num > 0)
			{
				block_num -= 8;
				block_id += 8;
			}
		}

		// allocate one page for user_stack
		// 此后在init stack的时候已经改变了用户栈
		kusr_stack = alloc_page_helper(USER_STACK_ADDR - PAGE_SIZE, pgdir, 
			_PAGE_PRESENT | _PAGE_READ | _PAGE_WRITE | _PAGE_ACCESSED | _PAGE_DIRTY | _PAGE_USER) + PAGE_SIZE;

		// allocate one page for user's kernel_stack
		kernel_stack = alloc_page_helper(KERNEL_STACK_ADDR - PAGE_SIZE, pgdir, 
			_PAGE_PRESENT | _PAGE_READ | _PAGE_WRITE | _PAGE_ACCESSED | _PAGE_DIRTY) + PAGE_SIZE;
			
		init_pcb_stack(kernel_stack, kusr_stack, USER_ENTRYPOINT, &tcb[0],1,init_argv);

		pcb[0].brk_start = USER_ENTRYPOINT + page_number * PAGE_SIZE;
		pcb[0].brk = pcb[0].brk_start;
		pcb[0].dt_size = page_number * PAGE_SIZE;
		pcb[0].tcb_num = 1;

		tcb[0].pid = 1;
		tcb[0].tid = 1;
		tcb[0].dt_size = page_number * PAGE_SIZE;
		tcb[0].ks_size = PAGE_SIZE;
		tcb[0].us_size = PAGE_SIZE;
		tcb[0].status = TASK_READY;
		tcb[0].core_mask = MASK_ZERO_ONE;

		addToQueue(&tcb[0].list,&ready_queue);
	}

	// there's a temp content in kernel pgdir 
	// (0x5000000~0x51000000 to 0x50000000~0x51000000)
	// the index of pgd is one 
	((PTE *)pcb[0].pgdir)[1] = 0;
}


static void init_pcb(void)
{
	/* TODO: [p2-task1] load needed tasks and init their corresponding PCB */
	int i;
	// initialize pcb array (for user)
	for(i = 0; i < NUM_MAX_PROC; i++)
	{
		pcb[i].pid = i + 1;
		pcb[i].wait_list.next = &pcb[i].wait_list;
		pcb[i].wait_list.prev = &pcb[i].wait_list;
		pcb[i].tcb_num = 0;
	}
}

static void init_tcb(void)
{
	int i;
	for(i = 0; i < NUM_MAX_THREAD; i++)
	{
		tcb[i].list.prev = NULL;
		tcb[i].list.next = NULL;
		tcb[i].list.tcb_ptr = (ptr_t)&tcb[i];
		tcb[i].status = TASK_EXITED;
		tcb[i].current_core_id = NO_CORE;
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
	syscall[SYSCALL_EXEC] 		= (long (*)())do_exec;
	syscall[SYSCALL_EXIT]		= (long (*)())do_exit;
	syscall[SYSCALL_KILL]		= (long (*)())do_kill;
	syscall[SYSCALL_GETPID]		= (long (*)())do_getpid;
	syscall[SYSCALL_WAITPID] 	= (long (*)())do_waitpid;
	syscall[SYSCALL_FORK]		= (long (*)())do_fork;
	syscall[SYSCALL_WAIT]		= (long (*)())do_wait;

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

	// ipc-mutex
	syscall[SYSCALL_LOCK_INIT] 	= (long (*)())do_mutex_lock_init;
	syscall[SYSCALL_LOCK_ACQ] 	= (long (*)())do_mutex_lock_acquire;
	syscall[SYSCALL_LOCK_RELEASE]	= (long (*)())do_mutex_lock_release;
	
	// ipc-barrier
	syscall[SYSCALL_BARR_INIT]	= (long (*)())do_barrier_init;
	syscall[SYSCALL_BARR_WAIT]	= (long (*)())do_barrier_wait;
	syscall[SYSCALL_BARR_DESTROY]	= (long (*)())do_barrier_destroy;

	// ipc-condition
	syscall[SYSCALL_COND_INIT]	= (long (*)())do_condition_init;
	syscall[SYSCALL_COND_WAIT]	= (long (*)())do_condition_wait;
	syscall[SYSCALL_COND_SIGNAL]	= (long (*)())do_condition_signal;
	syscall[SYSCALL_COND_BROADCAST]	= (long (*)())do_condition_broadcast;
	syscall[SYSCALL_COND_DESTROY]	= (long (*)())do_condition_destroy;

	// ipc-mailbox
	syscall[SYSCALL_MBOX_OPEN]	= (long (*)())do_mbox_open;
	syscall[SYSCALL_MBOX_CLOSE]	= (long (*)())do_mbox_close;
	syscall[SYSCALL_MBOX_SEND]	= (long (*)())do_mbox_send;
	syscall[SYSCALL_MBOX_RECV]	= (long (*)())do_mbox_recv;

	syscall[SYSCALL_PTHREAD_CREATE] = (long (*)())pthread_create;
	syscall[SYSCALL_PTHREAD_JOIN] 	= (long (*)())pthread_join;

	// memory management
	syscall[SYSCALL_SHM_GET]	= (long (*)())shm_page_get;
	syscall[SYSCALL_SHM_DT]		= (long (*)())shm_page_dt;
	syscall[SYSCALL_MPROTECT]	= (long (*)())mprotect;
	syscall[SYSCALL_BRK]		= (long (*)())brk;
	syscall[SYSCALL_SBRK]		= (long (*)())sbrk;

	// net
	syscall[SYSCALL_NET_SEND]	= (long (*)())do_net_send;
	syscall[SYSCALL_NET_RECV]	= (long (*)())do_net_recv;
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
		init_tcb();
		init_proc0(0);
		printk("> [INIT] PCB initialization succeeded.\n");

		// Read cpu time base (⊙﹏⊙)
		time_base = bios_read_fdt(TIMEBASE);

		#ifdef NET_DEVICE
		// Read Flatten Device Tree (｡•ᴗ-)_
		e1000 = (volatile uint8_t *)bios_read_fdt(ETHERNET_ADDR);
		uint64_t plic_addr = bios_read_fdt(PLIC_ADDR);
		uint32_t nr_irqs = (uint32_t)bios_read_fdt(NR_IRQS);
		printk("> [INIT] e1000: %lx, plic_addr: %lx, nr_irqs: %lx.\n", e1000, plic_addr, nr_irqs);
		
		// IOremap
		plic_addr = (uintptr_t)ioremap((uint64_t)plic_addr, 0x4000 * NORMAL_PAGE_SIZE);
		e1000 = (uint8_t *)ioremap((uint64_t)e1000, 8 * NORMAL_PAGE_SIZE);
		printk("> [INIT] IOremap initialization succeeded.\n");
		printl("e1000: %lx, plic_addr: %lx\n", e1000, plic_addr);

		// Init network device ( 0_o)
		e1000_init();
		printk("> [INIT] E1000 device initialized successfully.\n");

		plic_init(plic_addr, nr_irqs);
		printk("> [INIT] PLIC device initialized successfully.\n");
		#endif

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

		// Init data for page swaping ( * ^ *)o④
		init_swap();

		// load the first user task
		load_init();

		// wake up the slave core
		wakeup_other_hart();
		#ifndef M_CORE
		cleanTempPgtab();
		#endif

		/*
		* Just start kernel with VM and print this string
		* in the first part of task 1 of project 4.
		* NOTE: if you use SMP, then every CPU core should call
		*  `kernel_brake()` to stop executing!
		*/
		printl("> [INIT] CPU #%u has entered kernel with VM!\n",
			(unsigned int)get_current_cpu_id());
		// TODO: [p4-task1 cont.] remove the brake and continue to start user processes.
		// kernel_brake();
	}
	else
	{
		#ifndef M_CORE
		while(1);
		#endif
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
	//printk("test kernel\na");


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
