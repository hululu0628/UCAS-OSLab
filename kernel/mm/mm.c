#include "os/sched.h"
#include <pgtable.h>
#include <os/mm.h>
#include <os/string.h>
#include <os/smp.h>
#include <assert.h>
#include <os/string.h>

// NOTE: A/C-core

pageframe pages[PAGE_NUM];

pageframe * free_list_pgtab;
pageframe * free_list_proc;

// NOTE: Only need for S-core to alloc 2MB large page
#ifdef S_CORE
static ptr_t largePageMemCurr = LARGE_PAGE_FREEMEM;
ptr_t allocLargePage(int numPage)
{
    // align LARGE_PAGE_SIZE
    ptr_t ret = ROUND(largePageMemCurr, LARGE_PAGE_SIZE);
    largePageMemCurr = ret + numPage * LARGE_PAGE_SIZE;
    return ret;    
}
#endif

void init_page()
{
	int i;
	for(i = 0; i < PAGE_NUM; i++)
	{
		pages[i].page_num = i;
	}

	// the first three pages were used in boot.c
	for(i = PGTAB_START + KERNEL_PGDIR_NUM + 1; i < DYNAMIC_START; i++)
	{
		pages[i-1].next = &pages[i];
	}
	free_list_pgtab = &pages[PGTAB_START + KERNEL_PGDIR_NUM];

	// the first four pages were used for pcb0
	for(i = DYNAMIC_START + 2 * CPU_NUM + 1; i < PAGE_NUM; i++)
	{
		pages[i-1].next = &pages[i];
	}
	free_list_proc = &pages[DYNAMIC_START + 2 * CPU_NUM];
}

// 0x51000000~0x52000000 for page directory
// some pages were allocated for kernel page directory
ptr_t allocPgtabPage()
{
	pageframe * t;
	t = free_list_pgtab;
	if(t)
		free_list_pgtab = free_list_pgtab->next;
	t->next = NULL;
	return GET_KADDR(t->page_num);
}

// 0x52000000~0x60000000 for dynamic allocation
// some pages were allocated for kernel satck
ptr_t allocDynPage()
{
	pageframe * t;
	t = free_list_proc;
	if(t)
		free_list_proc = free_list_proc->next;
	t->next = NULL;
	return GET_KADDR(t->page_num);
}

// free a page (for dynamic or page directory)
void freePage(ptr_t baseAddr)
{
	// TODO [P4-task1] (design you 'freePage' here if you need):
	uint64_t i = kva2pa(baseAddr) - 0x50000000;
	if(i >= PGTAB_START && i <= DYNAMIC_START)
	{
		pages[i].next = free_list_pgtab;
		free_list_pgtab = &pages[i];

		// for a page directory, 
		// clear the content when tries to free
		clear_pgdir(baseAddr);
	}
	else if(i >= DYNAMIC_START && i <= PAGE_NUM)
	{
		pages[i].next = free_list_proc;
		free_list_proc = &pages[i];
	}
}

void *kmalloc(size_t size)
{
	// TODO [P4-task1] (design you 'kmalloc' here if you need):
	//
}


/* this is used for mapping kernel virtual address into user page table */
void share_pgtable(uintptr_t dest_pgdir, uintptr_t src_pgdir)
{
	// TODO [P4-task1] share_pgtable:
	memcpy((uint8_t *)dest_pgdir,(const uint8_t *)src_pgdir,PAGE_SIZE);
}

/* allocate physical page for `va`, mapping it into `pgdir`,
   return the kernel virtual address for the page
   */

// Same as map_page
// return value: kernel virtual memory;
// va: user virtual memory; pgdir: user pgdir;
// bits: the attribute of this page
uintptr_t alloc_page_helper(uintptr_t va, PTE * pgdir, uint64_t bits)
{
	// TODO [P4-task1] alloc_page_helper:
	va &= VA_MASK;
	uint64_t vpn2 = va >> (NORMAL_PAGE_SHIFT + PPN_BITS + PPN_BITS);
	uint64_t vpn1 = (vpn2 << PPN_BITS) ^ (va >> (NORMAL_PAGE_SHIFT + PPN_BITS));
	uint64_t vpn0 = ((va >> NORMAL_PAGE_SHIFT) ^ 
			(vpn2 << (PPN_BITS + PPN_BITS))) ^ 
			(vpn1 << PPN_BITS);
	if(pgdir[vpn2] == 0)
	{
		set_pfn(&pgdir[vpn2], kva2pa(allocPgtabPage()) >> NORMAL_PAGE_SHIFT);
		set_attribute(&pgdir[vpn2], _PAGE_PRESENT);
	}

	PTE * pmd = (PTE *)pa2kva(get_pa(pgdir[vpn2]));
	if(pmd[vpn1] == 0)
	{
		set_pfn(&pmd[vpn1], kva2pa(allocPgtabPage()) >> NORMAL_PAGE_SHIFT);
		set_attribute(&pmd[vpn1], _PAGE_PRESENT);
	}

	PTE * pt = (PTE *)pa2kva(get_pa(pmd[vpn1]));

	if(pt[vpn0] == 0)
	{
		uintptr_t kaddr = allocDynPage();
		set_pfn(&pt[vpn0], kva2pa(kaddr) >> NORMAL_PAGE_SHIFT);
		set_attribute(&pt[vpn0], bits);

		return kaddr;
	}
	else
		_panic("mm.c", 149, "alloc_page_helper");

}

// level 3: page addr for va; 2: pt for va; 1: pmd for va; 0: pgd for va
uint64_t get_kaddr(uint64_t va, PTE *pgdir, int level)
{
	PTE * pmd;
	PTE * pt;

	assert((level < 4) && (level >= 0));

	va &= VA_MASK;
	uint64_t vpn2 = va >> (NORMAL_PAGE_SHIFT + PPN_BITS + PPN_BITS);
	uint64_t vpn1 = (vpn2 << PPN_BITS) ^ (va >> (NORMAL_PAGE_SHIFT + PPN_BITS));
	uint64_t vpn0 = ((va >> NORMAL_PAGE_SHIFT) ^ 
			(vpn2 << (PPN_BITS + PPN_BITS))) ^ 
			(vpn1 << PPN_BITS);

	if(level == 0)
		return (uint64_t)(pgdir + vpn2);
	
	if(pgdir[vpn2] == 0)
		return 0;
	pmd = (PTE *)pa2kva(get_pa(pgdir[vpn2]));

	if(level == 1)
		return (uint64_t)(pmd + vpn1);

	if(pmd[vpn1] == 0)
		return 0;
	pt = (PTE *)pa2kva(get_pa(pmd[vpn1]));

	if(level == 2)
		return (uint64_t)(pt + vpn0);

	if(pt[vpn0] == 0)
		return 0;
	return pa2kva(get_pa(pt[vpn0])) + (va & (NORMAL_PAGE_SIZE - 1));

}

// copy data/text, user stack; init kernel stack
int uvmcopy(pcb_t * dest_pcb, pcb_t * src_pcb)
{
	int i;
	uint64_t kaddr;
	share_pgtable((uintptr_t)dest_pcb->pgdir, pa2kva(PGDIR_PA));
	for(i = 0; i < src_pcb->dt_size; i += PAGE_SIZE)
	{
		kaddr = alloc_page_helper(USER_ENTRYPOINT + i, dest_pcb->pgdir, _PAGE_PRESENT 
				| _PAGE_READ | _PAGE_WRITE | _PAGE_EXEC | _PAGE_USER);
		memcpy((uint8_t *)kaddr, 
			(const uint8_t *)get_kaddr(USER_ENTRYPOINT + i, src_pcb->pgdir, 3), PAGE_SIZE);
	}
	for(i = 0; i < src_pcb->us_size; i += PAGE_SIZE)
	{
		kaddr = alloc_page_helper(USER_STACK_ADDR - i - PAGE_SIZE, dest_pcb->pgdir, 
				_PAGE_PRESENT | _PAGE_READ | _PAGE_WRITE | _PAGE_USER);
		memcpy((uint8_t *)kaddr, 
			(const uint8_t *)get_kaddr(USER_STACK_ADDR - i - PAGE_SIZE, src_pcb->pgdir, 3), PAGE_SIZE);

	}
	for(i = 0; i < src_pcb->ks_size; i += PAGE_SIZE)
	{
		kaddr = alloc_page_helper(KERNEL_STACK_ADDR - i - PAGE_SIZE, dest_pcb->pgdir, 
				_PAGE_PRESENT | _PAGE_READ | _PAGE_WRITE);
		bzero((uint8_t *)kaddr, PAGE_SIZE);

	}
	return 1;
}

int uvmfree_seg(int flag, int size, PTE * pgdir)
{
	uint64_t va_start,i;
	switch(flag)
	{
		case DATA_AND_TEXT_SEG: va_start = USER_ENTRYPOINT; break;
		case USER_STACK_SEG: va_start = USER_STACK_ADDR - size; break;
		case KERNEL_STACK_SEG: va_start = KERNEL_STACK_ADDR - size; break;
		default: break;
	}
	for(i = 0; i < size; i += PAGE_SIZE)
	{
		freePage(get_kaddr(va_start + i, pgdir, 3));
	}
}

int uvmumap_seg(int flag, int size, PTE * pgdir)
{
	uint64_t va_start,i;
	PTE * pte;
	switch(flag)
	{
		case DATA_AND_TEXT_SEG: va_start = USER_ENTRYPOINT; break;
		case USER_STACK_SEG: va_start = USER_STACK_ADDR - size; break;
		case KERNEL_STACK_SEG: va_start = KERNEL_STACK_ADDR - size; break;
		default: break;
	}
	for(i = 0; i < size; i += PAGE_SIZE)
	{
		pte = (PTE *)get_kaddr(va_start + i, pgdir, 2);
		*pte = 0;
	}
}

int uvmfree_pgtable(pcb_t *pcb)
{
	uint64_t va_start,offset;
	PTE * pte;
	int level;
	for(level = 1; level >= 0; level--)
	{
		va_start = USER_ENTRYPOINT;
		for(offset = 0; offset < pcb->dt_size; offset += PAGE_SIZE)
		{
			if((pte = (PTE *)get_kaddr(va_start + offset,pcb->pgdir,level)) != 0)
			{
				freePage(pa2kva(get_pa(*pte)));
			}
		}
		
		va_start = USER_STACK_ADDR - pcb->us_size;
		for(offset = 0; offset < pcb->us_size; offset += PAGE_SIZE)
		{
			if((pte = (PTE *)get_kaddr(va_start + offset,pcb->pgdir,level)) != 0)
			{
				freePage(pa2kva(get_pa(*pte)));
			}
		}

		va_start = KERNEL_STACK_ADDR - pcb->ks_size;
		for(offset = 0; offset < pcb->ks_size; offset += PAGE_SIZE)
		{
			if((pte = (PTE *)get_kaddr(va_start + offset,pcb->pgdir,level)) != 0)
			{
				freePage(pa2kva(get_pa(*pte)));
			}
		}
	}
	freePage((ptr_t)pcb->pgdir);
}


uintptr_t shm_page_get(int key)
{
    // TODO [P4-task4] shm_page_get:
}

void shm_page_dt(uintptr_t addr)
{
    // TODO [P4-task4] shm_page_dt:
}