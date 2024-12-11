#include <assert.h>
#include <os/ioremap.h>
#include <os/mm.h>
#include <pgtable.h>
#include <type.h>

// maybe you can map it to IO_ADDR_START ?
static uintptr_t io_base = IO_ADDR_START;

// only used when initializing kernel
// 1GB size page
void *ioremap(unsigned long phys_addr, unsigned long size)
{
	// TODO: [p5-task1] map one specific physical region to virtual address
	uint64_t va = io_base + phys_addr;
	uint64_t vpn2 = (va & VA_MASK) >> (NORMAL_PAGE_SHIFT + PPN_BITS + PPN_BITS);
	if(get_attribute(((PTE *)pa2kva(PGDIR_PA))[vpn2], _PAGE_PRESENT | _PAGE_READ | _PAGE_WRITE))
		return (void *)va;

	// when size > 1GB, assert
	if(size >> 30)
		assert(0);

	set_pfn((PTE *)pa2kva(PGDIR_PA) + vpn2, ((phys_addr & 0xc0000000) >> NORMAL_PAGE_SHIFT));
	set_attribute((PTE *)pa2kva(PGDIR_PA) + vpn2, 
		_PAGE_PRESENT | _PAGE_READ | _PAGE_WRITE | _PAGE_ACCESSED | _PAGE_DIRTY);
	local_flush_tlb_all();
	return (void *)va;
}

void iounmap(void *io_addr)
{
	// TODO: [p5-task1] a very naive iounmap() is OK
	// maybe no one would call this function?
	PTE * pte;
	pte = (PTE *)get_kaddr((uint64_t)io_addr, (PTE *)pa2kva(PGDIR_PA), 2);
	*pte = 0;
}
