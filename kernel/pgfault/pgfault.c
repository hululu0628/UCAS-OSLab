#include <os/loader.h>
#include <os/swap.h>
#include <os/task.h>
#include <os/mm.h>
#include <os/proc.h>
#include <os/pthread.h>
#include <os/pgfault.h>
#include <asm/regs.h>
#include <pgtable.h>
#include <assert.h>

void handle_load_pgfault(regs_context_t *regs, uint64_t stval, uint64_t scause)
{
	int pid = current_running->pid;
	int task_id = pcb[pid - 1].task_id;
	PTE * pgdir = pcb[pid - 1].pgdir;

	PTE * pte;
	uint64_t bits;
	uint64_t kaddr;
	bits = _PAGE_PRESENT | _PAGE_READ | _PAGE_WRITE | _PAGE_USER;
	printl("load fault sepc: 0x%lx stval: 0x%lx\n",current_running->trapframe.sepc,stval);

	// If the address valid?
	if(stval >= pcb[pid - 1].brk && stval < (USER_STACK_ADDR - MAX_USTACK_SIZE))
	{
		if(stval > (USER_STACK_ADDR - MAX_USTACK_SIZE -PAGE_SIZE))
		{
			printk("[PID %d] ERROR: stack overflow (In 0x%lx)\n",current_running->pid,stval);
			do_exit();
		}
		else
		{
			printk("[PID %d] ERROR: segment fault (In 0x%lx)\n",current_running->pid,stval);
			do_exit();
		}
	}
	else
	{
		uint64_t sp = current_running->trapframe.regs[SP];
		if(sp < pcb[pid - 1].brk && sp < current_running->user_stack_base - THREAD_USTACK_SIZE)
		{
			printk("[PID %d] ERROR: stack overflow (In 0x%lx)\n",current_running->pid,stval);
			do_exit();
		}
	}
	
	pte = (PTE *)get_kaddr(stval, pgdir, 2);
	// If page missing?
	if(pte == NULL || *pte == 0)
	{
		int block_start,load_num;
		// page missing, happened in data segment
		while(1)
		{
			kaddr = alloc_page_helper(stval, pgdir, bits);
			// any free pageframe?
			if(kaddr)
				break;
			else
				swap_out();
		}
			
		block_start = ((stval - USER_ENTRYPOINT) >> NORMAL_PAGE_SHIFT) << 3;
		load_num = tasks[task_id].block_num - block_start;
		block_start = tasks[task_id].block_id + block_start;
		if(load_num >= 8)
			load_task_l(kaddr, block_start, 8);
		else
			load_task_l(kaddr, block_start, load_num);
		
		if(stval < USER_STACK_ADDR && stval >= USER_STACK_ADDR - MAX_USTACK_SIZE)
			current_running->us_size++;
	}
	else
	{
		// If permission allowed
		if((pte != NULL) && (*pte & _PAGE_PRESENT) && !(*pte & _PAGE_READ))
		{
			printk("ERROR: can not read from 0x%lx\n",stval);
			do_exit();
		}

		// If the page in the memory
		if(*pte & _PAGE_PRESENT)
		{
			if((*pte & _PAGE_ACCESSED) == 0)
			{
				*pte |= _PAGE_ACCESSED;
				kaddr = pa2kva(get_pa(*pte));
				pages[GET_PAGE_NUM(kaddr)].flags |= ACCESS_FLAG;
			}
			else
				assert(0);
		}
		else
		{
			swap_in(stval);
		}
	}
}

void handle_store_pgfault(regs_context_t *regs, uint64_t stval, uint64_t scause)
{
	int pid = current_running->pid;
	int task_id = pcb[pid - 1].task_id;
	PTE * pgdir = pcb[pid - 1].pgdir;

	PTE * pte;
	uint64_t bits;
	uint64_t kaddr;
	bits = _PAGE_PRESENT | _PAGE_READ | _PAGE_WRITE | _PAGE_USER;
	printl("store fault sepc: 0x%lx stval: 0x%lx\n",current_running->trapframe.sepc,stval);

	// If the address valid?
	if(stval >= pcb[pid - 1].brk && stval < (USER_STACK_ADDR - MAX_USTACK_SIZE))
	{
		if(stval > (USER_STACK_ADDR - MAX_USTACK_SIZE -PAGE_SIZE))
		{
			printk("[PID %d] ERROR: stack overflow (In 0x%lx)\n",current_running->pid,stval);
			do_exit();
		}
		else
		{
			printk("[PID %d] ERROR: segment fault (In 0x%lx)\n",current_running->pid,stval);
			do_exit();
		}
	}
	else
	{
		uint64_t sp = current_running->trapframe.regs[SP];
		if(sp < pcb[pid - 1].brk && sp < current_running->user_stack_base - THREAD_USTACK_SIZE)
		{
			printk("[PID %d] ERROR: stack overflow (In 0x%lx)\n",current_running->pid,stval);
			do_exit();
		}
	}
	
	pte = (PTE *)get_kaddr(stval, pgdir, 2);
	// If page missing?
	if(pte == NULL || *pte == 0)
	{
		int block_start,load_num;
		// page missing, happened in data segment
		while(1)
		{
			kaddr = alloc_page_helper(stval, pgdir, bits);
			// any free pageframe?
			if(kaddr)
				break;
			else
				swap_out();
		}
			
		block_start = ((stval - USER_ENTRYPOINT) >> NORMAL_PAGE_SHIFT) << 3;
		load_num = tasks[task_id].block_num - block_start;
		block_start = tasks[task_id].block_id + block_start;
		if(load_num >= 8)
			load_task_l(kaddr, block_start, 8);
		else
			load_task_l(kaddr, block_start, load_num);
		
		if(stval < USER_STACK_ADDR && stval >= USER_STACK_ADDR - MAX_USTACK_SIZE)
			current_running->us_size++;
	}
	else
	{
		// If permission allowed
		if((pte != NULL) && (*pte & _PAGE_PRESENT) && !(*pte & _PAGE_WRITE))
		{
			printk("ERROR: can not write into 0x%lx\n",stval);
			do_exit();
		}

		// If the page in the memory
		if(*pte & _PAGE_PRESENT)
		{
			if((*pte & _PAGE_DIRTY) == 0)
			{
				*pte |= (_PAGE_ACCESSED | _PAGE_DIRTY);
				kaddr = pa2kva(get_pa(*pte));
				pages[GET_PAGE_NUM(kaddr)].flags |= (DIRTY_FLAG | ACCESS_FLAG);
			}
			else
				assert(0);
		}
		else
		{
			swap_in(stval);
		}
	}
}

void handle_instr_pgfault(regs_context_t *regs, uint64_t stval, uint64_t scause)
{
	int pid = current_running->pid;
	int task_id = pcb[pid - 1].task_id;
	PTE * pgdir = pcb[pid - 1].pgdir;

	PTE * pte;
	uint64_t bits;
	uint64_t kaddr;
	bits = _PAGE_PRESENT | _PAGE_READ | _PAGE_WRITE | _PAGE_USER;
	printl("instruction page fault sepc: 0x%lx stval: 0x%lx\n",current_running->trapframe.sepc,stval);

	// If the address valid?
	if(stval >= pcb[pid - 1].brk && stval < (USER_STACK_ADDR - MAX_USTACK_SIZE))
	{
		if(stval > (USER_STACK_ADDR - MAX_USTACK_SIZE -PAGE_SIZE))
		{
			printk("[PID %d] ERROR: stack overflow (In 0x%lx)\n",current_running->pid,stval);
			do_exit();
		}
		else
		{
			printk("[PID %d] ERROR: segment fault (In 0x%lx)\n",current_running->pid,stval);
			do_exit();
		}
	}
	else
	{
		uint64_t sp = current_running->trapframe.regs[SP];
		if(sp < pcb[pid - 1].brk && sp < current_running->user_stack_base - THREAD_USTACK_SIZE)
		{
			printk("[PID %d] ERROR: stack overflow (In 0x%lx)\n",current_running->pid,stval);
			do_exit();
		}
	}
	
	pte = (PTE *)get_kaddr(stval, pgdir, 2);
	// If page missing?
	if(pte == NULL || *pte == 0)
	{
		int block_start,load_num;
		// page missing, happened in data segment
		while(1)
		{
			kaddr = alloc_page_helper(stval, pgdir, bits);
			// any free pageframe?
			if(kaddr)
				break;
			else
				swap_out();
		}
			
		block_start = ((stval - USER_ENTRYPOINT) >> NORMAL_PAGE_SHIFT) << 3;
		load_num = tasks[task_id].block_num - block_start;
		block_start = tasks[task_id].block_id + block_start;
		if(load_num >= 8)
			load_task_l(kaddr, block_start, 8);
		else
			load_task_l(kaddr, block_start, load_num);
		
	}
	else
	{
		// If permission allowed
		if((pte != NULL) && (*pte & _PAGE_PRESENT) && !(*pte & _PAGE_EXEC))
		{
			printk("ERROR: can not execute the instruction in 0x%lx\n",stval);
			do_exit();
		}

		// If the page in the memory
		if(*pte & _PAGE_PRESENT)
		{
			if((*pte & _PAGE_ACCESSED) == 0)
			{
				*pte |= _PAGE_ACCESSED;
				kaddr = pa2kva(get_pa(*pte));
				pages[GET_PAGE_NUM(kaddr)].flags |= ACCESS_FLAG;
			}
			else
				assert(0);
		}
		else
		{
			swap_in(stval);
		}
	}
}