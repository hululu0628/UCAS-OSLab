#include <os/proc.h>
#include <pgtable.h>
#include <os/mm.h>
#include <os/string.h>
#include <os/smp.h>
#include <os/swap.h>
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
pageframe * allocPgtabPage()
{
	pageframe * t;
	t = free_list_pgtab;
	if(t)
	{
		free_list_pgtab = free_list_pgtab->next;
		t->next = NULL;
		t->ref_cnt++;
		printl("page address %lx\n",GET_KADDR(t->page_num));
	}
	return t;
}

// 0x52000000~0x60000000 for dynamic allocation
// some pages were allocated for kernel satck
pageframe * allocDynPage()
{
	pageframe * t;
	t = free_list_proc;
	if(t)
	{
		free_list_proc = free_list_proc->next;
		t->next = NULL;
		t->ref_cnt++;
		printl("page address %lx\n",GET_KADDR(t->page_num));
	}
	return t;
}

// free a page (for dynamic or page directory)
void freePage(ptr_t baseAddr)
{
	// TODO [P4-task1] (design you 'freePage' here if you need):
	if(!baseAddr)
		assert(0);
	uint64_t i = (kva2pa(baseAddr) - 0x50000000) >> NORMAL_PAGE_SHIFT;
	pages[i].ref_cnt--;
	if(pages[i].ref_cnt == 0)
	{
		if(i >= PGTAB_START && i <= DYNAMIC_START)
		{
			pages[i].next = free_list_pgtab;
			pages[i].flags = 0;
			pages[i].pte = NULL;
			free_list_pgtab = &pages[i];
			printl("Free %lx\n",baseAddr);
			// for a page directory, 
			// clear the content when tries to free
			clear_pgdir(baseAddr);
		}
		else if(i >= DYNAMIC_START && i <= PAGE_NUM)
		{
			pages[i].next = free_list_proc;
			pages[i].flags = 0;
			pages[i].pte = NULL;
			free_list_proc = &pages[i];

			printl("Free %lx\n",baseAddr);
		}
	}
}

void *kmalloc(size_t size)
{
	// TODO [P4-task1] (design you 'kmalloc' here if you need):
	return NULL;
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
	uintptr_t kaddr;
	pageframe * t;
	va &= VA_MASK;
	uint64_t vpn2 = va >> (NORMAL_PAGE_SHIFT + PPN_BITS + PPN_BITS);
	uint64_t vpn1 = (vpn2 << PPN_BITS) ^ (va >> (NORMAL_PAGE_SHIFT + PPN_BITS));
	uint64_t vpn0 = ((va >> NORMAL_PAGE_SHIFT) ^ 
			(vpn2 << (PPN_BITS + PPN_BITS))) ^ 
			(vpn1 << PPN_BITS);
	if((pgdir[vpn2] & _PAGE_PRESENT) == 0)
	{
		t = allocPgtabPage();
		if(!t)
			return 0;

		kaddr = GET_KADDR(t->page_num);
		set_pfn(&pgdir[vpn2], kva2pa(kaddr) >> NORMAL_PAGE_SHIFT);
		change_attribute(&pgdir[vpn2], _PAGE_PRESENT);

		t->pte = &pgdir[vpn2];
		t->flags |= UNFREE_FLAG;
	}

	PTE * pmd = (PTE *)pa2kva(get_pa(pgdir[vpn2]));
	if((pmd[vpn1] & _PAGE_PRESENT) == 0)
	{
		t = allocPgtabPage();
		if(!t)
			return 0;

		kaddr = GET_KADDR(t->page_num);
		set_pfn(&pmd[vpn1], kva2pa(kaddr) >> NORMAL_PAGE_SHIFT);
		change_attribute(&pmd[vpn1], _PAGE_PRESENT);

		t->pte = &pmd[vpn1];
		t->flags |= UNFREE_FLAG;
	}

	PTE * pt = (PTE *)pa2kva(get_pa(pmd[vpn1]));

	// 当前页不存在于物理内存中，分配
	// 页框的pte和flag都在此设置完成
	// 页表项的pfn和flag都在此设置完成
	if((pt[vpn0] & _PAGE_PRESENT) == 0)
	{
		t = allocDynPage();
		if(!t)
			return 0;

		kaddr = GET_KADDR(t->page_num);
		set_pfn(&pt[vpn0], kva2pa(kaddr) >> NORMAL_PAGE_SHIFT);
		change_attribute(&pt[vpn0], bits);

		t->pte = &pt[vpn0];
		t->flags |= bits;

		return kaddr;
	}
	else
		_panic("mm.c", 149, "alloc_page_helper");
	return 0;
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
	
	if(!(pgdir[vpn2] & _PAGE_PRESENT))
		return 0;
	pmd = (PTE *)pa2kva(get_pa(pgdir[vpn2]));

	if(level == 1)
		return (uint64_t)(pmd + vpn1);

	if(!(pmd[vpn1] & _PAGE_PRESENT))
		return 0;
	pt = (PTE *)pa2kva(get_pa(pmd[vpn1]));

	if(level == 2)
		return (uint64_t)(pt + vpn0);

	if(!(pt[vpn0] & _PAGE_PRESENT))
		return 0;
	return pa2kva(get_pa(pt[vpn0])) + (va & (NORMAL_PAGE_SIZE - 1));

}

// copy data/text, user stack; init kernel stack
// 如果copy的对象页不全在内存中怎么办
int uvmcopy(tcb_t * dest_tcb, tcb_t * src_tcb)
{
	int i;
	uint64_t kaddr;
	uint64_t bits;
	PTE *dest_pgdir, *src_pgdir;

	dest_pgdir = pcb[dest_tcb->pid - 1].pgdir;
	src_pgdir = pcb[src_tcb->pid - 1].pgdir;

	share_pgtable((uintptr_t)dest_pgdir, pa2kva(PGDIR_PA));
	// 复制数据和代码段
	for(i = 0; i < src_tcb->dt_size; i += PAGE_SIZE)
	{
		bits = get_attribute(*((PTE *)get_kaddr(USER_ENTRYPOINT + i, src_pgdir, 2)), TOTAL_FLAG_MASK);
		kaddr = alloc_page_helper(USER_ENTRYPOINT + i, dest_pgdir, bits);
		memcpy((uint8_t *)kaddr, 
			(const uint8_t *)get_kaddr(USER_ENTRYPOINT + i, src_pgdir, 3), PAGE_SIZE);
	}
	// 复制用户栈
	for(i = 0; i < src_tcb->us_size; i += PAGE_SIZE)
	{
		kaddr = alloc_page_helper(USER_STACK_ADDR - i - PAGE_SIZE, dest_pgdir, 
				_PAGE_PRESENT | _PAGE_READ | _PAGE_WRITE | _PAGE_DIRTY | _PAGE_USER);
		memcpy((uint8_t *)kaddr, 
			(const uint8_t *)get_kaddr(USER_STACK_ADDR - i - PAGE_SIZE, src_pgdir, 3), PAGE_SIZE);

	}
	// 分配内核栈并清零
	for(i = 0; i < src_tcb->ks_size; i += PAGE_SIZE)
	{
		kaddr = alloc_page_helper(KERNEL_STACK_ADDR - i - PAGE_SIZE, dest_pgdir, 
				_PAGE_PRESENT | _PAGE_READ | _PAGE_WRITE | _PAGE_ACCESSED | _PAGE_DIRTY);
		bzero((uint8_t *)kaddr, PAGE_SIZE);

	}
	return 1;
}

void uvmfree(PTE * pgdir)
{
	int i,j,k;
	PTE *pmd, *pt;
	for(i = 0; i < (PT_NUM >> 1); i++)
	{
		if(pgdir[i] & _PAGE_PRESENT)
		{
			pmd = (PTE *)pa2kva(get_pa(pgdir[i]));
			for(j = 0; j < PT_NUM; j++)
			{
				if(pmd[j] & _PAGE_PRESENT)
				{
					pt = (PTE *)pa2kva(get_pa(pmd[j]));
					for(k = 0; k < PT_NUM; k++)
					{
						if(pt[k] != 0)
						{
							if(pt[k] & _PAGE_PRESENT)
							{
								freePage(pa2kva(get_pa(pt[k])));
							}
							else
							{
								int slot_index;
								slot_index = get_swap_entry(&pt[k]);
								swap_free(slot_index);
							}
						}
					}
					freePage((ptr_t)pt);
				}
			}
			freePage((ptr_t)pmd);
		}
	}
	freePage((ptr_t)pgdir);
}

int uvmfree_seg(int flag, tcb_t * t, PTE * pgdir)
{
	uint64_t va_start,i;
	uint64_t size;
	switch(flag)
	{
		case DATA_AND_TEXT_SEG: 
			va_start = USER_ENTRYPOINT;
			size = t->dt_size;
			break;
		case USER_STACK_SEG: 
			va_start = t->user_stack_base - t->us_size; 
			size = t->us_size;
			break;
		case KERNEL_STACK_SEG: 
			va_start = t->kernel_stack_base - t->ks_size; 
			size = t->ks_size;
			break;
		default: size = 0; break;
	}
	for(i = 0; i < size; i += PAGE_SIZE)
	{
		freePage(get_kaddr(va_start + i, pgdir, 3));
	}
	return 0;
}

int uvmumap_seg(int flag, tcb_t * t, PTE * pgdir)
{
	uint64_t va_start,i;
	uint64_t size;
	PTE * pte;
	switch(flag)
	{
		case DATA_AND_TEXT_SEG: 
			va_start = USER_ENTRYPOINT;
			size = t->dt_size;
			break;
		case USER_STACK_SEG: 
			va_start = t->user_stack_base - t->us_size; 
			size = t->us_size;
			break;
		case KERNEL_STACK_SEG: 
			va_start = t->kernel_stack_base - t->ks_size; 
			size = t->ks_size;
			break;
		default: size = 0; break;
	}
	for(i = 0; i < size; i += PAGE_SIZE)
	{
		pte = (PTE *)get_kaddr(va_start + i, pgdir, 2);
		*pte = 0;
	}
	return 0;
}

int uvmfree_pgtable(pcb_t *pcb)
{
	uint64_t va_start,offset,i;
	int pid = pcb->pid;
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
		
		for(i = 0; i < NUM_MAX_THREAD; i++)
		{
			if(tcb[i].pid == pid)
			{
				va_start = tcb[i].user_stack_base - tcb[i].us_size;
				for(offset = 0; offset < tcb[i].us_size; offset += PAGE_SIZE)
				{
					if((pte = (PTE *)get_kaddr(va_start + offset,pcb->pgdir,level)) != 0)
					{
						freePage(pa2kva(get_pa(*pte)));
					}
				}

				va_start = tcb[i].kernel_stack_base - tcb[i].ks_size;
				for(offset = 0; offset < tcb[i].ks_size; offset += PAGE_SIZE)
				{
					if((pte = (PTE *)get_kaddr(va_start + offset,pcb->pgdir,level)) != 0)
					{
						freePage(pa2kva(get_pa(*pte)));
					}
				}	
			}
		}
	}
	freePage((ptr_t)pcb->pgdir);
	return 0;
}


uintptr_t shm_page_get(int key)
{
	// TODO [P4-task5] shm_page_get:
	int i = key % MAX_SHM_NUM;
	pageframe * t;
	uint64_t va,kaddr,kaddr1;
	uint64_t bits = _PAGE_PRESENT | _PAGE_READ | _PAGE_WRITE | _PAGE_USER;
	PTE * pte;
	PTE * pgdir = pcb[current_running->pid - 1].pgdir;

	if(shm_array[i].page_num == 0)
	{
		t = allocDynPage();
		t->flags = bits;

		shm_array[i].page_num = t->page_num;
		shm_array[i].key = key;
		shm_array[i].cnt++;

		kaddr = GET_KADDR(t->page_num);
	}
	else if(shm_array[i].key == key)
	{
		shm_array[i].cnt++;
		kaddr = GET_KADDR(shm_array[i].page_num);
		pages[shm_array[i].page_num].ref_cnt++;
	}

	for(va = SM_START; va < USER_STACK_ADDR; va += PAGE_SIZE)
	{
		if(get_kaddr(va, pgdir, 3) == 0)
		{
			kaddr1 = alloc_page_helper(va, pgdir, bits);
			freePage(kaddr1);
			pte = (PTE *)get_kaddr(va, pgdir, 2);
			set_pfn(pte, kva2pa(kaddr) >> NORMAL_PAGE_SHIFT);
			return va;
		}
	}
	return 0;
}

void shm_page_dt(uintptr_t addr)
{
	// TODO [P4-task5] shm_page_dt:
	PTE * pgdir = pcb[current_running->pid - 1].pgdir;
	uint64_t kaddr = get_kaddr(addr, pgdir, 3);
	PTE * pte = (PTE *)get_kaddr(addr, pgdir, 2);
	freePage(kaddr);
	*pte = 0;
	for(int i = 0; i < MAX_SHM_NUM; i++)
	{
		if(shm_array[i].page_num == GET_PAGE_NUM(kaddr))
		{
			shm_array[i].cnt--;
			if(shm_array[i].cnt == 0)
			{
				shm_array[i].page_num = 0;
				shm_array[i].key = 0;
			}
			break;
		}
	}
	local_flush_tlb_all();
}