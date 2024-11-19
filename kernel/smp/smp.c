#include <os/mm.h>
#include <pgtable.h>
#include <atomic.h>
#include <os/sched.h>
#include <os/smp.h>
#include <os/lock.h>
#include <os/kernel.h>
#include <os/irq.h>
#include <os/string.h>
#include <os/stdlib.h>
#include <printk.h>

spin_lock_t slock;


void init_pcb0(int cpuid)
{
	pid0_pcb[cpuid].pid = 0;
	pid0_pcb[cpuid].task_id = -1;
	pid0_pcb[cpuid].kernel_sp = INIT_KERNEL_STACK + 2 * (1 + cpuid) * PAGE_SIZE;
	pid0_pcb[cpuid].core_mask = 1 << cpuid;
	pid0_pcb[cpuid].pgdir = (PTE *)pa2kva(PGDIR_PA);
	pid0_pcb[cpuid].status = TASK_RUNNING;
	pid0_pcb[cpuid].current_core_id = cpuid;
	current_running = &pid0_pcb[cpuid];		// current running is kernel
	process_id[cpuid] = pid0_pcb[cpuid].pid;
}

void smp_init()
{
	/* TODO: P3-TASK3 multicore*/
	// init tp
	int current_cpu_id = get_current_cpu_id();

	// init pcb0[cpuid]
	init_pcb0(current_cpu_id);

	// init stvec
	setup_trap();
}

void wakeup_other_hart()
{
	/* TODO: P3-TASK3 multicore*/
	const unsigned long hart_mask = MASK_ONE;
	send_ipi(&hart_mask);
}

void lock_kernel()
{
	/* TODO: P3-TASK3 multicore*/
	while(atomic_swap(LOCKED, (ptr_t)&slock.status) == LOCKED)
		;
}

void unlock_kernel()
{
	/* TODO: P3-TASK3 multicore*/
	atomic_swap(UNLOCKED, (ptr_t)&slock.status);
}

void lock_pcb()
{
	while(atomic_swap(LOCKED, (ptr_t)&slock.status) == LOCKED)
		;
}

void unlock_pcb()
{
	atomic_swap(UNLOCKED, (ptr_t)&slock.status);
}


// 输入不规范没有过多检查
int do_taskset(int argc, char **argv)
{
	int mask;
	int pid;
	if(strcmp(argv[0], "-p") == 0)
	{
		if(argc != 3)
			printl("ERROR: In function do_taskset, argc != 3\n");
		else
		{
			pid = atoi(argv[2]);
			mask = atoi(argv[1]);
			pcb[pid - 1].core_mask = mask;
			return pid;
		}
	}
	else
	{
		printl("In function do_taskset, argv[0] = %s\n",argv[0]);
		mask = atoi(argv[0]);
		do_exec(argv[1], argc - 1, argv + 1);
		current_running->core_mask = mask;
		return current_running->pid;
	}
	return -1;
}