#include <os/smp.h>
#include <os/string.h>
#include <os/loader.h>
#include <os/irq.h>
#include <os/string.h>
#include <os/task.h>
#include <csr.h>
#include <asm/regs.h>
#include <pgtable.h>
#include <os/kernel.h>
#include <os/list.h>
#include <os/lock.h>
#include <os/sched.h>
#include <os/time.h>
#include <os/mm.h>
#include <screen.h>
#include <printk.h>
#include <assert.h>

// sched.c, or proc.c

pcb_t pcb[NUM_MAX_TASK];
pcb_t pid0_pcb[CPU_NUM];

LIST_HEAD(ready_queue);
LIST_HEAD(sleep_queue);
LIST_HEAD(wait_queue);

/* global process id */
pid_t process_id[CPU_NUM];

PTE * get_pgdir(pcb_t * pcb)
{
	return pcb->pgdir;
}

void do_scheduler(void)
{
	// TODO: [p2-task3] Check sleep queue to wake up PCBs
	check_sleeping();

	/************************************************************/
	/* Do not touch this comment. Reserved for future projects. */
	/************************************************************/

	// TODO: [p2-task1] Modify the current_running pointer.
	int current_cpuid = get_current_cpu_id();
	pcb_t * prev_process = current_running;
	list_head * queue;
	if(current_running->status == TASK_RUNNING)
	{
		current_running->status = TASK_READY;

		// Round Robin
		if(current_running != &pid0_pcb[current_cpuid])
		{
			addToQueue(&current_running->list, &ready_queue);
		}
	}

	current_running->current_core_id = NO_CORE;	

	// select next process by core-mask
	queue = &ready_queue;
	while(1)
	{
		current_running = (pcb_t *)getReadyProcess(queue);
		// printl("0x%x core: %d pid: %d\n",current_running,current_cpuid,process_id[current_cpuid]);
		if(current_running->core_mask & (1 << current_cpuid))
			break;
		queue = queue->next;
		if(queue->next == &ready_queue)
		{
			current_running = &pid0_pcb[current_cpuid];
			break;
		}
	}

	current_running->current_core_id = current_cpuid;

	process_id[current_cpuid] = current_running->pid;
	current_running->status = TASK_RUNNING;
	if(process_id[current_cpuid] != 0)
		deleteNode(&current_running->list);

	bios_set_timer(get_ticks() + TIMER_INTERVAL);	// set timer interrupt

	// TODO: [p2-task1] switch_to current_running
	switch_to(prev_process,current_running);
}

void do_sleep(uint32_t sleep_time)
{
	// TODO: [p2-task3] sleep(seconds)
	// NOTE: you can assume: 1 second = 1 `timebase` ticks
	// 1. block the current_running
	// 2. set the wake up time for the blocked task
	// 3. reschedule because the current_running is blocked.
	if(current_running != &pid0_pcb[get_current_cpu_id()])
	{
		current_running->status = TASK_BLOCKED;
		current_running->wakeup_time = get_timer() + sleep_time;
		addToQueue(&current_running->list, &sleep_queue);
		do_scheduler();
	}
	else
	{
		latency(sleep_time);		// in kernel process, call latency
	}

}

void do_block(list_node_t *pcb_node, list_head *queue)
{
	// TODO: [p2-task2] block the pcb task into the block queue
	current_running->status = TASK_BLOCKED;
	if(current_running != &pid0_pcb[get_current_cpu_id()])
		addToQueue(pcb_node, queue);

	do_scheduler();
}

void do_unblock(list_node_t *pcb_node)
{
	// TODO: [p2-task2] unblock the `pcb` from the block queue
	pcb_t * pcb_unblock = FIND_PCB(pcb_node);
	deleteNode(pcb_node);
	pcb_unblock->status = TASK_READY;
	addToQueue(pcb_node, &ready_queue);
}

void do_process_show()
{
	int i;
	int j = 0;
	int has_process = 0;
	for(i = 0; i < NUM_MAX_TASK; i++)
	{
		if(pcb[i].status != TASK_EXITED)
		{
			if(has_process == 0)
			{
				printk("[Process Table]:\n");
				has_process = 1;
			}
			printk("[%d] PID: %d ",j,pcb[i].pid);
			j++;
			if(pcb[i].status == TASK_RUNNING)
				printk("STATUS: %s ","TASK_RUNNING");
			else if(pcb[i].status == TASK_BLOCKED)
				printk("STATUS: %s ","TASK_BLOCKED");
			else if(pcb[i].status == TASK_READY)
				printk("STATUS: %s ","TASK_READY");

			printk("MASK: 0x%x",pcb[i].core_mask);

			if(pcb[i].current_core_id == NO_CORE)
				printk("\n");
			else
				printk(" Core: %d\n",pcb[i].current_core_id);
		}
	}
	if(has_process == 0)
	{
		printk("Huh? There is no process?");
	}
}

pid_t do_getpid()
{
	return current_running->pid;
}

pid_t do_fork(void)
{
	int i;
	int pid;
	if((pid = alloc_proc()) == -1)
		_panic("sched.c", 179, "do_fork");
	
	i = pid - 1;
	pcb[i].pgdir = (PTE *)allocPgtabPage();
	uvmcopy(&pcb[i], current_running);

	pcb[i].kernel_sp = KERNEL_STACK_ADDR - sizeof(switchto_context_t);

	init_switch_to(PAGE_SIZE + get_kaddr(KERNEL_STACK_ADDR - PAGE_SIZE, pcb[i].pgdir, 3), &pcb[i]);

	pcb[i].dt_size = current_running->dt_size;
	pcb[i].us_size = current_running->us_size;
	pcb[i].ks_size = current_running->ks_size;

	pcb[i].trapframe = current_running->trapframe;
	pcb[i].trapframe.regs[A0] = 0;
	pcb[i].trapframe.regs[TP] = (reg_t)&pcb[i];

	pcb[i].parent = current_running;

	pcb[i].status = TASK_READY;
	addToQueue(&pcb[i].list,&ready_queue);

	return pid;
}

pid_t do_exec(char *name, int argc, char **argv)
{
	char args_buf[10][20];		// shell的最大允许参数
	char * kname = args_buf[0];
	for(int i = 0; i < argc; i++)
	{
		strcpy(args_buf[i], argv[i]);
	}

	PTE * pgdir = current_running->pgdir;
	uvmfree_seg(DATA_AND_TEXT_SEG, current_running->dt_size, pgdir);
	uvmumap_seg(DATA_AND_TEXT_SEG, current_running->dt_size, pgdir);
	uvmfree_seg(USER_STACK_SEG, current_running->us_size, pgdir);
	uvmumap_seg(USER_STACK_SEG, current_running->us_size, pgdir);


	ptr_t kusr_stack;
	int task_id;
	int block_id;
	int block_num;
	int page_number;
	uint64_t kaddr;
	if((task_id = find_task(kname)) != -1)
	{
		page_number = 1 + (tasks[task_id].mem_size >> NORMAL_PAGE_SHIFT);

		block_id = tasks[task_id].block_id;
		block_num = tasks[task_id].block_num;
		for(uint64_t va = USER_ENTRYPOINT, i = 0; i < page_number; va += PAGE_SIZE, i++)
		{
			kaddr = alloc_page_helper(va, pgdir, _PAGE_PRESENT 
				| _PAGE_READ | _PAGE_WRITE | _PAGE_EXEC | _PAGE_USER);
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
		kusr_stack = alloc_page_helper(USER_STACK_ADDR - PAGE_SIZE, pgdir, 
			_PAGE_PRESENT | _PAGE_READ | _PAGE_WRITE | _PAGE_USER) + PAGE_SIZE;


		ptr_t argv_base,kustack_start;
		ptr_t usp;
		kustack_start = kusr_stack;
		argv_base = kusr_stack - sizeof(char *) * argc;
		kusr_stack = argv_base;
		usp = argv_base;
		for(int i = 0; i < argc; i++)
		{
			kusr_stack = kusr_stack - (strlen(args_buf[i]) + 1);
			strcpy((char *)kusr_stack,args_buf[i]);
			memcpy((uint8_t *)usp, (const uint8_t *)&kusr_stack, sizeof(char *));
			usp += sizeof(char *);
		}
		kusr_stack = kusr_stack & 0xffffffffffffff80;

		
		current_running->trapframe.sstatus = SR_SPIE | SR_SUM;
		current_running->trapframe.sepc = USER_ENTRYPOINT;
		current_running->trapframe.regs[SP] = USER_STACK_ADDR - (kustack_start - kusr_stack);
		current_running->trapframe.regs[TP] = (reg_t)current_running;
		current_running->trapframe.regs[A0] = (reg_t)argc;
		current_running->trapframe.regs[A1] = (reg_t)(USER_STACK_ADDR - (kustack_start - argv_base));

		current_running->dt_size = page_number * PAGE_SIZE;
		current_running->ks_size = PAGE_SIZE;
		current_running->us_size = PAGE_SIZE;

		// modified third level page table,
		// therefore the tlb must be reflushed
		local_flush_tlb_all();

		return 0;
	}
}

/*
pid_t do_exec(char *name, int argc, char **argv)
{
	int i;
	pid_t pid = -1;
	for(i = 0; i < NUM_MAX_TASK; i++)
	{
		if(pcb[i].status == TASK_EXITED)
		{		
			pid = i + 1;	
			if(add_new_task(name,argc,argv,pid) == -1)
			{
				printl("Error: In function do_exec, cannot load task called %s\n",name);
				return -1;
			}
			addToQueue(&pcb[i].list,&ready_queue);
			pcb[i].core_mask = current_running->core_mask;
			break;
		}
	}
	return pid;	// 修改：错误返回-1
}*/

// 回收内存
void do_exit(void)
{
	current_running->status = TASK_ZOMBIE;
	freeQueueToReady(&current_running->wait_list);
	reparent(&pcb[0], current_running);
	wakeup(current_running->parent);
	do_scheduler();
}

int do_kill(pid_t pid)
{
	if(pcb[pid - 1].status != TASK_EXITED && pcb[pid - 1].status != TASK_ZOMBIE)
	{
		deleteNode(&pcb[pid - 1].list);
		pcb[pid - 1].status = TASK_ZOMBIE;
		freeQueueToReady(&pcb[pid - 1].wait_list);
		// 多把锁
		for(int i = 0; i < LOCK_NUM; i++)
		{
			if(pcb[pid - 1].mlock_table[i] == 1)
				do_mutex_lock_release(i);
		}
		for(int i = 0; i < MBOX_NUM; i++)
		{
			if(pcb[pid - 1].mbox_table[i] == 1)
				do_mbox_close(i);
		}

		reparent(&pcb[0], &pcb[pid - 1]);
		wakeup(current_running->parent);
	}
	return 0;
}

int do_wait(int * status)
{
	int i;
	while(1)
	{
		for(i = 0; i < NUM_MAX_TASK; i++)
		{
			if(pcb[i].parent == current_running && pcb[i].status == TASK_ZOMBIE)
			{
				free_proc(&pcb[i]);
				return i + 1;
			}
		}
		do_block(&current_running->list, &wait_queue);
	}
	return 0;
}

int do_waitpid(pid_t pid)
{
	if(pid > 0 && pid <= NUM_MAX_TASK)
	{
		if(pcb[pid - 1].status != TASK_EXITED)
		{
			do_block(&current_running->list, &pcb[pid - 1].wait_list);
			return pid;
		}
	}
	return 0;
}

void wakeup(pcb_t *pcb)
{
	do_unblock(&pcb->list);
}

int alloc_proc()
{
	int i;
	for(i = 0; i < NUM_MAX_TASK; i++)
	{
		if(pcb[i].status == TASK_EXITED)
		{
			pcb[i].pid = i + 1;
			pcb[i].core_mask = current_running->core_mask;
			pcb[i].current_core_id = NO_CORE;
			pcb[i].pgdir = (PTE *)allocPgtabPage();
			return i + 1;
		}
	}
	return -1;
}

void free_proc(pcb_t *pcb)
{
	uvmfree_seg(DATA_AND_TEXT_SEG, pcb->dt_size, pcb->pgdir);
	uvmfree_seg(USER_STACK_SEG, pcb->us_size, pcb->pgdir);
	uvmfree_seg(KERNEL_STACK_SEG, pcb->ks_size, pcb->pgdir);
	
	uvmfree_pgtable(pcb);

	pcb->status = TASK_EXITED;
	pcb->parent = 0;
	pcb->pgdir = NULL;
}

void reparent(pcb_t *parent, pcb_t *child)
{
	child->parent = parent;
}

void init_switch_to(ptr_t kernel_stack, pcb_t * pcb)
{
	switchto_context_t *pt_switchto = (switchto_context_t *)((ptr_t)kernel_stack - sizeof(switchto_context_t));

	// for user process, jump to entrypoint by using sret
	pt_switchto->regs[0] = (reg_t)ret_from_trap;
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

void init_pcb_stack(
    ptr_t kernel_stack, ptr_t kuser_stack, ptr_t entry_point,
    pcb_t *pcb, int argc, char **argv)
{
	// P3, pass parameter to the user stack
	ptr_t argv_base,kustack_start;
	ptr_t usp;
	kustack_start = kuser_stack;
	argv_base = kuser_stack - sizeof(char *) * argc;
	kuser_stack = argv_base;
	usp = argv_base;
	for(int i = 0; i < argc; i++)
	{
		kuser_stack = kuser_stack - (strlen(argv[i]) + 1);
		strcpy((char *)kuser_stack,argv[i]);
		memcpy((uint8_t *)usp, (const uint8_t *)&kuser_stack, sizeof(char *));
		usp += sizeof(char *);
	}
	kuser_stack = kuser_stack & 0xffffffffffffff80;

	/* TODO: [p2-task3] initialization of registers on kernel stack
	* HINT: sp, ra, sepc, sstatus
	* NOTE: To run the task in user mode, you should set corresponding bits
	*     of sstatus(SPP, SPIE, etc.).
	*/
	
	pcb->trapframe.sstatus = SR_SPIE | SR_SUM;	// return U-mode(SPP == 0) and enable interrupt gloablly(SPIE == 1)
	pcb->trapframe.sepc = entry_point;		// jump to entrypoint using sret
	pcb->trapframe.regs[SP] = USER_STACK_ADDR - (kustack_start - kuser_stack);
	pcb->trapframe.regs[TP] = (reg_t)pcb;
	pcb->trapframe.regs[A0] = (reg_t)argc;
	pcb->trapframe.regs[A1] = (reg_t)(USER_STACK_ADDR - (kustack_start - argv_base));


	/* TODO: [p2-task1] set sp to simulate just returning from switch_to
	* NOTE: you should prepare a stack, and push some values to
	* simulate a callee-saved context.
	*/

	pcb->kernel_sp = (reg_t)KERNEL_STACK_ADDR - sizeof(switchto_context_t);

	
	init_switch_to(kernel_stack, pcb);

}


/*
uint64_t add_new_task(char * str, int argc, char *argv[], int pid)
{
	ptr_t kernel_stack,kusr_stack;
	PTE * pgdir;
	int task_id;
	int block_id;
	int block_num;
	int page_number;
	uint64_t kaddr;
	if((task_id = find_task(str)) != -1)
	{
		pgdir = (PTE *)allocPgtabPage();

		share_pgtable((uintptr_t)pgdir, pa2kva(PGDIR_PA));

		pcb[pid - 1].pgdir = pgdir;
		page_number = 1 + (tasks[task_id].mem_size >> NORMAL_PAGE_SHIFT);

		block_id = tasks[task_id].block_id;
		block_num = tasks[task_id].block_num;
		for(uint64_t va = USER_ENTRYPOINT, i = 0; i < page_number; va += PAGE_SIZE, i++)
		{
			kaddr = alloc_page_helper(va, pgdir, _PAGE_PRESENT 
				| _PAGE_READ | _PAGE_WRITE | _PAGE_EXEC | _PAGE_USER);
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
		kusr_stack = alloc_page_helper(USER_STACK_ADDR - PAGE_SIZE, pgdir, 
			_PAGE_PRESENT | _PAGE_READ | _PAGE_WRITE | _PAGE_USER) + PAGE_SIZE;

		// allocate one page for user's kernel_stack
		kernel_stack = alloc_page_helper(KERNEL_STACK_ADDR - PAGE_SIZE, pgdir, 
			_PAGE_PRESENT | _PAGE_READ | _PAGE_WRITE) + PAGE_SIZE;
			
		init_pcb_stack(kernel_stack, kusr_stack, USER_ENTRYPOINT, &pcb[pid-1],argc,argv);

		pcb[pid-1].dt_size = page_number * PAGE_SIZE;
		pcb[pid-1].ks_size = PAGE_SIZE;
		pcb[pid-1].us_size = PAGE_SIZE;
		pcb[pid-1].status = TASK_READY;

		return 0;
	}
	else  
		return -1;
}*/