#include "pgtable.h"
#include <csr.h>
#include <os/pthread.h>
#include <os/proc.h>
#include <os/mm.h>
#include <asm/regs.h>
#include <assert.h>

void pthread_create(pthread_t *thread, void (*start_routine)(void*), void *arg)
{
	uint64_t old_brk,new_brk;
	int tid = 0;
	int pid = current_running->pid;
	int tidx;
	for(tidx = 0; tidx < NUM_MAX_THREAD; tidx++)
	{
		if(tcb[tidx].status == TASK_EXITED)
		{
			tid = tidx + 1;
			break;
		}
	}
	if(tid == 0)
		assert(0);

	pcb[pid - 1].tcb_num += 1;

	tcb[tidx].tid = tidx + 1;
	tcb[tidx].core_mask = current_running->core_mask;
	tcb[tidx].current_core_id = NO_CORE;
	tcb[tidx].pid = pid;
	
	tidx = tid - 1;

	old_brk = pcb[pid - 1].brk;
	if(brk((void *)(PAGE_ALIGNED(old_brk) + THREAD_USTACK_SIZE + PAGE_SIZE)) == -1)
		assert(0);
	new_brk = pcb[pid - 1].brk;

	tcb[tidx].kernel_sp = KERNEL_STACK_ADDR - pcb[pid - 1].tcb_num * THREAD_STACK - sizeof(switchto_context_t);
	tcb[tidx].kernel_stack_base = KERNEL_STACK_ADDR - pcb[pid - 1].tcb_num * THREAD_STACK;
	tcb[tidx].user_stack_base = new_brk;

	tcb[tidx].dt_size = current_running->dt_size;
	tcb[tidx].us_size = THREAD_USTACK_SIZE;
	tcb[tidx].ks_size = PAGE_SIZE;

	alloc_page_helper(tcb[tidx].kernel_stack_base - PAGE_SIZE, pcb[pid - 1].pgdir, 
		_PAGE_PRESENT | _PAGE_READ | _PAGE_WRITE | _PAGE_ACCESSED | _PAGE_DIRTY);

	// allocate for user stack
	for(int j = 0; j < tcb[tidx].us_size + PAGE_SIZE; j += PAGE_SIZE)
	{
		alloc_page_helper(tcb[tidx].user_stack_base - PAGE_SIZE - j, pcb[pid - 1].pgdir, 
			_PAGE_PRESENT | _PAGE_READ | _PAGE_WRITE | _PAGE_ACCESSED | _PAGE_DIRTY | _PAGE_USER);
	}
	mprotect((void *)(tcb[tidx].user_stack_base - PAGE_SIZE - THREAD_USTACK_SIZE), PAGE_SIZE, PROT_NONE);
	
	init_switch_to(PAGE_SIZE + get_kaddr(tcb[tidx].kernel_stack_base - PAGE_SIZE, pcb[pid - 1].pgdir, 3), &tcb[tidx]);


	tcb[tidx].trapframe.sepc = (reg_t)start_routine;
	tcb[tidx].trapframe.sstatus = SR_SUM | SR_SPIE;
	tcb[tidx].trapframe.regs[A0] = (reg_t)arg;
	tcb[tidx].trapframe.regs[SP] = (reg_t)tcb[tidx].user_stack_base;
	tcb[tidx].trapframe.regs[TP] = (reg_t)&tcb[tidx];

	tcb[tidx].status = TASK_READY;
	addToQueue(&tcb[tidx].list,&ready_queue);

	pcb[pid - 1].tcb_num += 1;

	*thread = tid;
}

int pthread_join(pthread_t thread)
{
	while(tcb[thread].pid == current_running->pid && tcb[thread].status != TASK_ZOMBIE)
		do_block(&current_running->list, &wait_queue);
	return 0;
}