#include <os/mm.h>
#include <os/sched.h>
#include <os/pgfault.h>
#include <pgtable.h>
#include <assert.h>

void handle_load_pgfault(regs_context_t *regs, uint64_t stval, uint64_t scause)
{
	PTE * pte;
	printl("load fault sepc: 0x%lx stval: 0x%lx\n",current_running->trapframe.sepc,stval);
	if(get_kaddr(stval, current_running->pgdir, 3) == 0)
	{
		alloc_page_helper(stval, current_running->pgdir, 
			_PAGE_PRESENT | _PAGE_WRITE | _PAGE_USER);
	}
	else if((pte = (PTE *)get_kaddr(stval, current_running->pgdir, 2)) != 0)
	{
		if((*pte & _PAGE_ACCESSED) == 0)
			*pte |= _PAGE_ACCESSED;
		else
			assert(0);
	}
	else
		assert(0);
}

void handle_store_pgfault(regs_context_t *regs, uint64_t stval, uint64_t scause)
{
	PTE * pte;
	printl("store fault sepc: 0x%lx stval: 0x%lx\n",current_running->trapframe.sepc,stval);
	if(get_kaddr(stval, current_running->pgdir, 3) == 0)
	{
		alloc_page_helper(stval, current_running->pgdir, 
			_PAGE_PRESENT | _PAGE_READ | _PAGE_WRITE | _PAGE_USER);
	}
	else if((pte = (PTE *)get_kaddr(stval, current_running->pgdir, 2)) != 0)
	{
		if((*pte & _PAGE_DIRTY) == 0)
			*pte |= _PAGE_DIRTY;
		else
			assert(0);
	}
	else
		assert(0);
}

void handle_instr_pgfault(regs_context_t *regs, uint64_t stval, uint64_t scause)
{
	PTE * pte;
	printl("instr fault sepc: 0x%lx stval: 0x%lx\n",current_running->trapframe.sepc,stval);
	if(get_kaddr(stval, current_running->pgdir, 3) == 0)
	{
		alloc_page_helper(stval, current_running->pgdir, 
			_PAGE_PRESENT | _PAGE_READ | _PAGE_EXEC | _PAGE_USER);
	}
	else if((pte = (PTE *)get_kaddr(stval, current_running->pgdir, 2)) != 0)
	{
		if((*pte & _PAGE_ACCESSED) == 0)
			*pte |= _PAGE_ACCESSED;
		else
			assert(0);
	}
	else
		assert(0);
}