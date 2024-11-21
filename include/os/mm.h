/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * * * * * *
 *            Copyright (C) 2018 Institute of Computing Technology, CAS
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * * * * * *
 *                                   Memory Management
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * * * * * *
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * * * * * */
#ifndef INCLUDE_MM_H
#define INCLUDE_MM_H

#include <type.h>
#include <pgtable.h>
#include <os/proc.h>

#define MAP_KERNEL 1
#define MAP_USER 2
#define MEM_SIZE 32

#define PAGE_SIZE 4096 // 4K
#define PT_NUM 512

#define SM_START 0x300000

#define PAGE_NUM ((0x60000000 - 0x50000000) >> NORMAL_PAGE_SHIFT)	// for debugging
#define PGTAB_START ((0x51000000 - 0x50000000) >> NORMAL_PAGE_SHIFT)
#define DYNAMIC_START ((0x52000000 - 0x50000000) >> NORMAL_PAGE_SHIFT)
#define GET_KADDR(num) (((uint64_t)num << NORMAL_PAGE_SHIFT) + 0xffffffc050000000)
#define GET_PAGE_NUM(kaddr) ((kaddr - 0xffffffc050000000) >> NORMAL_PAGE_SHIFT)
#define INIT_KERNEL_STACK 0xffffffc052000000

#define DATA_AND_TEXT_SEG 0
#define USER_STACK_SEG 1
#define KERNEL_STACK_SEG 2

#define UNFREE_FLAG (1 << 0)
#define READ_FLAG (1 << 1)     /* Readable */
#define WRITE_FLAG (1 << 2)    /* Writable */
#define EXEC_FLAG (1 << 3)     /* Executable */
#define USER_FLAG (1 << 4)     /* User */
#define GLOBAL_FLAG (1 << 5)   /* Global */
#define ACCESS_FLAG (1 << 6) /* Set by hardware on any access */
#define DIRTY_FLAG (1 << 7)    /* Set by hardware on any write */
#define SOFT_FLAG (1 << 8)     /* Reserved for software */

/* Rounding; only works for n = power of two */
#define ROUND(a, n)     (((((uint64_t)(a))+(n)-1)) & ~((n)-1))
#define ROUNDDOWN(a, n) (((uint64_t)(a)) & ~((n)-1))


#define PAGE_FREE	0x1

typedef struct pageframe
{
	PTE * pte;
	uint32_t flags;
	uint32_t page_num;
	struct pageframe * next;
	uint32_t ref_cnt;
}pageframe;

extern pageframe pages[PAGE_NUM];

extern pageframe * free_list_proc;
extern pageframe * free_list_pgtab;


#define MAX_SHM_NUM	64
typedef struct shared_mem
{
	uint32_t page_num;
	uint32_t cnt;
	uint32_t key;
}shm;

shm shm_array[MAX_SHM_NUM];

extern void init_page(void);

extern pageframe * allocPgtabPage();
extern pageframe * allocDynPage();


// TODO [P4-task1] */
void freePage(ptr_t baseAddr);

// #define S_CORE
// NOTE: only need for S-core to alloc 2MB large page
#define USER_ENTRYPOINT 0x10000

#ifdef S_CORE

#define LARGE_PAGE_FREEMEM 0xffffffc056000000
#define USER_STACK_ADDR 0x400000
extern ptr_t allocLargePage(int numPage);

#else

// NOTE: A/C-core
#define USER_STACK_ADDR 0xf00010000
#define KERNEL_STACK_ADDR 0xfffffff000000000

#endif

// TODO [P4-task1] */
extern void* kmalloc(size_t size);
extern void share_pgtable(uintptr_t dest_pgdir, uintptr_t src_pgdir);
extern uintptr_t alloc_page_helper(uintptr_t va, PTE * pgdir, uint64_t bits);

extern uint64_t get_kaddr(uint64_t va, PTE * pgdir, int level);	// 给出三级页表，拿到对应的内核地址

extern int uvmcopy(tcb_t * ctcb, tcb_t * ptcb);
extern void uvmfree(PTE * pgdir);
extern int uvmfree_seg(int flag, tcb_t * t, PTE * pgdir);
extern int uvmfree_pgtable(pcb_t * pcb);
extern int uvmumap_seg(int flag, tcb_t * t, PTE * pgdir);


// TODO [P4-task4]: shm_page_get/dt */
uintptr_t shm_page_get(int key);
void shm_page_dt(uintptr_t addr);


extern ptr_t kalloc(int byte_num, int flags);
extern int kfree(ptr_t p);

#endif /* MM_H */
