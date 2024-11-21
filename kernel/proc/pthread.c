#include "csr.h"
#include <os/pthread.h>
#include <os/proc.h>
#include <os/mm.h>
#include <asm/regs.h>
#include <assert.h>

void pthread_create(pthread_t *thread, void (*start_routine)(void*), void *arg)
{
	int tid = 0;
	int pid = current_running->pid;
	int i;
	for(i = 0; i < NUM_MAX_THREAD; i++)
	{
		if(tcb[i].status == TASK_EXITED)
		{
			tid = i + 1;
			break;
		}
	}
	if(tid == 0)
		assert(0);

	pcb[pid - 1].tcb_num += 1;

	tcb[i].tid = i + 1;
	tcb[i].core_mask = current_running->core_mask;
	tcb[i].current_core_id = NO_CORE;
	tcb[i].pid = pid;
	
	i = tid - 1;

	tcb[i].kernel_sp = KERNEL_STACK_ADDR - pcb[pid - 1].tcb_num * THREAD_STACK - sizeof(switchto_context_t);
	tcb[i].kernel_stack_base = KERNEL_STACK_ADDR - pcb[pid - 1].tcb_num * THREAD_STACK;
	tcb[i].user_stack_base = USER_STACK_ADDR - pcb[pid - 1].tcb_num * THREAD_STACK;

	alloc_page_helper(tcb[i].kernel_stack_base - PAGE_SIZE, pcb[pid - 1].pgdir, 
		_PAGE_PRESENT | _PAGE_READ | _PAGE_WRITE | _PAGE_ACCESSED | _PAGE_DIRTY);
	alloc_page_helper(tcb[i].user_stack_base - PAGE_SIZE, pcb[pid - 1].pgdir, 
		_PAGE_PRESENT | _PAGE_READ | _PAGE_WRITE | _PAGE_ACCESSED | _PAGE_DIRTY);

	init_switch_to(PAGE_SIZE + get_kaddr(tcb[i].kernel_stack_base - PAGE_SIZE, pcb[pid - 1].pgdir, 3), &tcb[i]);

	tcb[i].dt_size = current_running->dt_size;
	tcb[i].us_size = PAGE_SIZE;
	tcb[i].ks_size = PAGE_SIZE;


	tcb[i].trapframe.sepc = (reg_t)start_routine;
	tcb[i].trapframe.sstatus = SR_SUM | SR_SPIE;
	tcb[i].trapframe.regs[A0] = (reg_t)arg;
	tcb[i].trapframe.regs[TP] = (reg_t)&tcb[i];

	tcb[i].status = TASK_READY;
	addToQueue(&tcb[i].list,&ready_queue);

	pcb[pid - 1].tcb_num += 1;

	*thread = tid;
}

int pthread_join(pthread_t thread)
{
	while(tcb[thread].pid == current_running->pid && tcb[thread].status != TASK_ZOMBIE)
		do_block(&current_running->list, &wait_queue);
	return 0;
}