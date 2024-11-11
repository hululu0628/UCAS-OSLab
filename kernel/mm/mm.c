#include <pgtable.h>
#include <os/mm.h>
#include <os/string.h>

// NOTE: A/C-core
static ptr_t kernMemCurr = FREEMEM_KERNEL;

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
	for(i = PGTAB_START + 4; i < DYNAMIC_START; i++)
	{
		pages[i-1].next = &pages[i];
	}
	free_list_pgtab = &pages[PGTAB_START + 3];

	// the first four pages were used for pcb0
	for(i = DYNAMIC_START + 5; i < PAGE_NUM; i++)
	{
		pages[i-1].next = &pages[i];
	}
	free_list_proc = &pages[DYNAMIC_START + 4];
}


ptr_t allocPgtabPage()
{
	pageframe * t;
	t = free_list_pgtab;
	if(t)
		free_list_pgtab = free_list_pgtab->next;
	t->next = NULL;
	return GET_KADDR(t->page_num);
}

ptr_t allocDynPage()
{
	pageframe * t;
	t = free_list_proc;
	if(t)
		free_list_proc = free_list_proc->next;
	t->next = NULL;
	return GET_KADDR(t->page_num);
}


ptr_t allocKernelStack(int numPage)
{
	return 0;
}

ptr_t allocUserStack(int numPage)
{
	return 0;
}

void freePage(ptr_t baseAddr)
{
	// TODO [P4-task1] (design you 'freePage' here if you need):
	int i = kva2pa(baseAddr) - 0x50000000;
	if(i >= PGTAB_START && i <= DYNAMIC_START)
	{
		pages[i].next = free_list_pgtab;
		free_list_pgtab = &pages[i];
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
uintptr_t alloc_page_helper(uintptr_t va, PTE * pgdir)
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
	uintptr_t kaddr = allocDynPage();
	set_pfn(&pt[vpn0], kva2pa(kaddr) >> NORMAL_PAGE_SHIFT);
	set_attribute(&pt[vpn0], _PAGE_PRESENT | _PAGE_READ | _PAGE_WRITE |
				_PAGE_EXEC | _PAGE_USER);

	return kaddr;
}

uintptr_t shm_page_get(int key)
{
    // TODO [P4-task4] shm_page_get:
}

void shm_page_dt(uintptr_t addr)
{
    // TODO [P4-task4] shm_page_dt:
}