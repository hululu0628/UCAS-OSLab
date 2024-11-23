#include <os/loader.h>
#include <os/swap.h>
#include <os/task.h>
#include <os/mm.h>
#include <os/proc.h>
#include <os/pgfault.h>
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
	
	pte = (PTE *)get_kaddr(stval, pgdir, 2);
	if((pte != NULL) && (*pte & _PAGE_PRESENT) && !(*pte & _PAGE_READ))
	{
		printk("ERROR: can not read from 0x%lx\n",stval);
		do_exit();
	}

	// 可能页表都没有建立
	kaddr = alloc_page_helper(stval, pgdir, bits);
	if(kaddr)
		return;
	
	if(get_kaddr(stval, pgdir, 3) == 0)
	{
		if(*pte == 0)
		{
			// 缺页，目前只可能是数据段
			while(1)
			{
				kaddr = alloc_page_helper(stval, pgdir, bits);
				if(kaddr)
					break;
				else
					swap_out();
			}
			int block_start,load_num;
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
			// 页被换出
			swap_in(stval);
		}
	}
	else if(pte != 0)
	{
		// 需要对页的FLAG更新
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
		assert(0);
}

void handle_store_pgfault(regs_context_t *regs, uint64_t stval, uint64_t scause)
{
	int pid = current_running->pid;
	int task_id = pcb[pid - 1].task_id;
	PTE * pgdir = pcb[pid - 1].pgdir;

	PTE * pte;
	uint64_t bits;
	uint64_t kaddr;
	bits = _PAGE_PRESENT | _PAGE_READ | _PAGE_WRITE | _PAGE_ACCESSED | _PAGE_DIRTY | _PAGE_USER;
	printl("store fault sepc: 0x%lx stval: 0x%lx\n",current_running->trapframe.sepc,stval);


	pte = (PTE *)get_kaddr(stval, pgdir, 2);
	if((pte != NULL) && (*pte & _PAGE_PRESENT) && !(*pte & _PAGE_WRITE))
	{
		printk("ERROR: can not write into 0x%lx\n",stval);
		do_exit();
	}


	kaddr = alloc_page_helper(stval, pgdir, bits);
	if(kaddr)
		return;

	if(get_kaddr(stval, pgdir, 3) == 0)
	{
		if(*pte == 0)
		{
			while(1)
			{
				kaddr = alloc_page_helper(stval, pgdir, bits);
				if(kaddr)
					break;
				else
					swap_out();
			}
			int block_start,load_num;
			block_start = ((stval - USER_ENTRYPOINT) >> NORMAL_PAGE_SHIFT) << 3;
			load_num = tasks[task_id].block_num - block_start;
			block_start = tasks[task_id].block_id + block_start;
			if(load_num >= 8)
				load_task_l(kaddr, block_start, 8);
			else
				load_task_l(kaddr, block_start, load_num);
		}
		else
			swap_in(stval);
	}
	else if(pte != 0)
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
		assert(0);
}

void handle_instr_pgfault(regs_context_t *regs, uint64_t stval, uint64_t scause)
{
	int pid = current_running->pid;
	int task_id = pcb[pid - 1].task_id;
	PTE * pgdir = pcb[pid - 1].pgdir;

	PTE * pte;
	uint64_t bits;
	uint64_t kaddr;
	bits = _PAGE_PRESENT | _PAGE_READ | _PAGE_EXEC | _PAGE_USER;
	printl("instr fault sepc: 0x%lx stval: 0x%lx\n",current_running->trapframe.sepc,stval);


	pte = (PTE *)get_kaddr(stval, pgdir, 2);
	if((pte != NULL) && (*pte & _PAGE_PRESENT) && !(*pte & _PAGE_EXEC))
	{
		printk("ERROR: can not execute the instruction in 0x%lx\n",stval);
		do_exit();
	}



	kaddr = alloc_page_helper(stval, pgdir, bits);
	if(kaddr)
		return;

	if(get_kaddr(stval, pgdir, 3) == 0)
	{
		if(*pte == 0)
		{
			while(1)
			{
				kaddr = alloc_page_helper(stval, pgdir, bits);
				if(kaddr)
					break;
				else
					swap_out();
			}
			int block_start,load_num;
			block_start = ((stval - USER_ENTRYPOINT) >> NORMAL_PAGE_SHIFT) << 3;
			load_num = tasks[task_id].block_num - block_start;
			block_start = tasks[task_id].block_id + block_start;
			if(load_num >= 8)
				load_task_l(kaddr, block_start, 8);
			else
				load_task_l(kaddr, block_start, load_num);
		}
		else
			swap_in(stval);
	}
	else if(pte != 0)
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
		assert(0);
}