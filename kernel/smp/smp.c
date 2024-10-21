#include <atomic.h>
#include <os/sched.h>
#include <os/smp.h>
#include <os/lock.h>
#include <os/kernel.h>
#include <os/irq.h>

spin_lock_t slock;

void smp_init()
{
	/* TODO: P3-TASK3 multicore*/
	// init tp
	pid0_pcb[get_current_cpu_id()].status = TASK_RUNNING;
	current_running = &pid0_pcb[get_current_cpu_id()];		// current running is kernel
	process_id[get_current_cpu_id()] = pid0_pcb[get_current_cpu_id()].pid;
	// init stvec
	setup_trap();
}

void wakeup_other_hart()
{
	/* TODO: P3-TASK3 multicore*/
	const unsigned long hart_mask = CORE_ONE;
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
