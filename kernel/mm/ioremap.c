#include <assert.h>
#include <os/ioremap.h>
#include <os/mm.h>
#include <pgtable.h>
#include <type.h>

// maybe you can map it to IO_ADDR_START ?
static uintptr_t io_base = IO_ADDR_START;

static uintptr_t io_offset;

// only used when initializing kernel
void *ioremap(unsigned long phys_addr, unsigned long size)
{
	// TODO: [p5-task1] map one specific physical region to virtual address
	int isfirst = 1;
	uint64_t ret_addr;

	uint64_t offset;
	uint64_t kaddr;
	PTE * pte;
	for(offset = 0; offset < size; offset += PAGE_SIZE, io_offset += PAGE_SIZE)
	{
		kaddr = alloc_page_helper(io_base+io_offset, (PTE *)pa2kva(PGDIR_PA), 
			_PAGE_PRESENT | _PAGE_READ | _PAGE_WRITE | _PAGE_ACCESSED | _PAGE_DIRTY);
		pte = (PTE *)get_kaddr(io_base + io_offset, (PTE *)pa2kva(PGDIR_PA), 2);
		if(kaddr != 0)
		{
			freePage(kaddr);
			set_pfn(pte, (phys_addr + offset) >> NORMAL_PAGE_SHIFT);
		}
		else
			assert(0);

		if(isfirst)
		{
			isfirst = 0;
			ret_addr = io_base + io_offset;
		}
	}
	local_flush_tlb_all();
	return (void *)ret_addr;
}

void iounmap(void *io_addr)
{
	// TODO: [p5-task1] a very naive iounmap() is OK
	// maybe no one would call this function?
	PTE * pte;
	pte = (PTE *)get_kaddr((uint64_t)io_addr, (PTE *)pa2kva(PGDIR_PA), 2);
	*pte = 0;
}
