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
#ifndef MM_H
#define MM_H

#include <type.h>
#include <pgtable.h>

#define MAP_KERNEL 1
#define MAP_USER 2
#define MEM_SIZE 32
#define PAGE_SIZE 4096 // 4K
#define PAGE_NUM ((0x60000000 - 0x50000000) >> NORMAL_PAGE_SHIFT)
#define PGTAB_START ((0x51000000 - 0x50000000) >> NORMAL_PAGE_SHIFT)
#define DYNAMIC_START ((0x52000000 - 0x50000000) >> NORMAL_PAGE_SHIFT)
#define GET_KADDR(num) (((uint64_t)num << NORMAL_PAGE_SHIFT) + 0xffffffc050000000)
#define INIT_KERNEL_STACK 0xffffffc052000000

/* Rounding; only works for n = power of two */
#define ROUND(a, n)     (((((uint64_t)(a))+(n)-1)) & ~((n)-1))
#define ROUNDDOWN(a, n) (((uint64_t)(a)) & ~((n)-1))


#define PAGE_FREE	0x1

typedef struct pageframe
{
	uint32_t flags;
	uint32_t page_num;
	struct pageframe * next;
}pageframe;

extern pageframe pages[PAGE_NUM];

extern pageframe * free_list_proc;
extern pageframe * free_list_pgtab;


extern void init_page(void);

extern ptr_t allocPgtabPage();
extern ptr_t allocDynPage();


// TODO [P4-task1] */
void freePage(ptr_t baseAddr);

// #define S_CORE
// NOTE: only need for S-core to alloc 2MB large page
#define USER_ENTRYPOINT 0x200000

#ifdef S_CORE

#define LARGE_PAGE_FREEMEM 0xffffffc056000000
#define USER_STACK_ADDR 0x400000
extern ptr_t allocLargePage(int numPage);

#else

// NOTE: A/C-core
#define USER_STACK_ADDR 0xf00010000
#define KERNEL_STACK_ADDR 0xffffffc080000000

#endif

// TODO [P4-task1] */
extern void* kmalloc(size_t size);
extern void share_pgtable(uintptr_t dest_pgdir, uintptr_t src_pgdir);
extern uintptr_t alloc_page_helper(uintptr_t va, PTE * pgdir);

// TODO [P4-task4]: shm_page_get/dt */
uintptr_t shm_page_get(int key);
void shm_page_dt(uintptr_t addr);



extern ptr_t allocKernelStack(int numPage);
extern ptr_t allocUserStack(int numPage);



extern ptr_t kalloc(int byte_num, int flags);
extern int kfree(ptr_t p);

#endif /* MM_H */
