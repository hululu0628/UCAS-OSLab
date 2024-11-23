#include <os/smp.h>
#include <os/proc.h>
#include <pgtable.h>
#include <os/kernel.h>
#include <os/swap.h>
#include <os/mm.h>
#include <assert.h>
#include <printk.h>

uint64_t swap_id_start;
uint64_t clock_hand = DYNAMIC_START + (CPU_NUM << 1);
uint64_t free_slot = MAX_SLOT;
uint64_t slot_next;
uint8_t swap_map[MAX_SLOT];

void set_swap_entry(PTE * pte, uint64_t slot_index)
{
	// PPN->slot_index
	*pte = (*pte & 0xfflu) ^ (slot_index << _PAGE_PFN_SHIFT);
}
uint64_t get_swap_entry(PTE *pte)
{
	return *pte >> _PAGE_PFN_SHIFT;
}

void init_swap(void)
{
	uint32_t * taskinfo_sec_offset_loc = (uint32_t *)0xffffffc0502001f8;
	uint16_t * taskinfo_sec_num_loc = (uint16_t *)0xffffffc0502001f6;
	swap_id_start = *taskinfo_sec_offset_loc + *taskinfo_sec_num_loc;
}

int get_swap_page(void)
{
	if(free_slot == 0)
		return -1;
	for(int i = 0; i < MAX_SLOT; i++)
	{
		if(swap_map[slot_next] == 0)
		{
			(swap_map[slot_next])++;
			return slot_next++;
		}
		slot_next++;
	}
	return -1;
}

void swap_free(uint64_t slot_index)
{
	if(swap_map[slot_index] > 0)
		swap_map[slot_index]--;
	else
		assert(0);
}

int swap_out(void)
{
	uint64_t start;
	uint64_t slot_index;
	uint32_t flags;
	PTE * pte;

	// 需要重复两轮，且理论上至多两轮
	while(1)
	{
		// 查找A = 0，D = 0的USER页框，发现直接将该页释放，然后退出
		for(start = 0; start < PAGE_NUM - DYNAMIC_START - (CPU_NUM << 1); start ++)
		{
			// check if the page attribute is invalid or reserved
			if(!(pages[clock_hand].flags & UNFREE_FLAG) || 
				(pages[clock_hand].flags & RESERVED_FLAG))
			{
				clock_hand++;
				continue;
			}

			pte = pages[clock_hand].pte;
			// for qemu
			flags = get_attribute(*pte, TOTAL_FLAG_MASK);

			if(!(flags & ACCESS_FLAG) && 
			   !(flags & DIRTY_FLAG) && 
			    (flags & USER_FLAG) )
			{
				*pte = 0;
				freePage(GET_KADDR(pages[clock_hand].page_num));
				clock_hand++;
				if(clock_hand >= PAGE_NUM)
					clock_hand = DYNAMIC_START + (CPU_NUM << 1);
				return 0;
			}
			else
			{
				clock_hand++;
				if(clock_hand >= PAGE_NUM)
					clock_hand = DYNAMIC_START + (CPU_NUM << 1);
			}
		}
		// 查找A = 0， D = 1的USER页框，发现后将该页换出，然后释放页框，退出
		// 对于扫描到的A = 1的页框，将A设为0。
		for(start = 0; start < PAGE_NUM - DYNAMIC_START - (CPU_NUM << 1); start++)
		{
			if(!(pages[clock_hand].flags & UNFREE_FLAG) || 
				(pages[clock_hand].flags & RESERVED_FLAG))
			{
				clock_hand++;
				continue;
			}


			pte = pages[clock_hand].pte;
			// for qemu
			flags = get_attribute(*pte, TOTAL_FLAG_MASK);
			
			if(!(flags & ACCESS_FLAG) && 
			    (flags & DIRTY_FLAG) && 
			    (flags & USER_FLAG) )
			{
				slot_index = get_swap_page();
				set_swap_entry(pte, slot_index);
				clear_attribute(pte, _PAGE_PRESENT);
				bios_sd_write(GET_KADDR(pages[clock_hand].page_num),
					SECTORS_FOR_A_PAGE, swap_id_start + GET_SECTOR_ID(slot_index));
				
				freePage(GET_KADDR(pages[clock_hand].page_num));
				clock_hand++;
				if(clock_hand >= PAGE_NUM)
					clock_hand = DYNAMIC_START + (CPU_NUM << 1);
				return 0;
			}
			else if(flags & ACCESS_FLAG)
			{
				// pages[clock_hand].flags &= ~ACCESS_FLAG;
				clear_attribute(pte, _PAGE_ACCESSED);
			}
			clock_hand++;
			if(clock_hand >= PAGE_NUM)
				clock_hand = DYNAMIC_START + (CPU_NUM << 1);
		}
	}
}

int swap_in(uint64_t va)
{
	PTE * pte;
	PTE * pgdir = pcb[current_running->pid - 1].pgdir;
	uint64_t slot_index;
	uint64_t kaddr;
	uint64_t bits;

	pte = (PTE *)get_kaddr(va, pgdir, 2);
	slot_index = get_swap_entry(pte);
	
	// 尝试分配一个页，经过设计该页一定是DIRTY的USER页，分配失败则进行换出操作
	// 这个页有可能是普通的数据页，也可能是数据和代码混合的一页，因此bits要做区分
	bits = _PAGE_PRESENT | _PAGE_READ | _PAGE_WRITE | _PAGE_DIRTY | _PAGE_USER;
	if(va - USER_ENTRYPOINT < current_running->dt_size)
		bits |= _PAGE_EXEC;
	while((kaddr = alloc_page_helper(va, pgdir, 
			_PAGE_PRESENT | _PAGE_READ | _PAGE_WRITE | _PAGE_EXEC | _PAGE_DIRTY | _PAGE_USER)) == 0)
	{
		swap_out();
	}

	bios_sd_read(kaddr, SECTORS_FOR_A_PAGE, swap_id_start + GET_SECTOR_ID(slot_index));
	swap_free(slot_index);

	return 0;
}