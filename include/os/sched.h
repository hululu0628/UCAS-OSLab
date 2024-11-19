/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * * * * * *
 *            Copyright (C) 2018 Institute of Computing Technology, CAS
 *               Author : Han Shukai (email : hanshukai@ict.ac.cn)
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * * * * * *
 *        Process scheduling related content, such as: scheduler, process blocking,
 *                 process wakeup, process creation, process kill, etc.
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

#ifndef INCLUDE_SCHEDULER_H_
#define INCLUDE_SCHEDULER_H_

#include <type.h>
#include <os/list.h>
#include <os/smp.h>
#include <os/lock.h>
#include <pgtable.h>

#define NUM_MAX_TASK 16

#define FIND_PCB(name) ((pcb_t *)(name->pcb_ptr))	// find pcb

/* used to save register infomation */
typedef struct regs_context
{
	/* Saved main processor registers.*/
	reg_t regs[32];

	/* Saved special registers. */
	reg_t sstatus;
	reg_t sepc;
	// reg_t sbadaddr;
	/* The latest version of the RISC-V specification replaces sbadaddr with stval */
	reg_t stval;
	reg_t scause;
} regs_context_t;

/* used to save register infomation in switch_to */
typedef struct switchto_context
{
	/* Callee saved registers.*/
	// 0~13: ra, sp, s0~s11
	reg_t regs[14];
} switchto_context_t;

typedef enum {
	TASK_BLOCKED,
	TASK_RUNNING,
	TASK_READY,
	TASK_ZOMBIE,
	TASK_EXITED,
} task_status_t;

/* Process Control Block */
typedef struct pcb
{
	/* register context */
	// in entry.S, when storing trapframe, using offset
	// modify the offset when modifying the place of trapframe
	regs_context_t trapframe;

	// NOTE: this order must be preserved, which is defined in regs.h!!
	reg_t kernel_sp;
	reg_t user_sp;
	ptr_t kernel_stack_base;
	ptr_t user_stack_base;

	/* previous, next pointer, pcb pointer */
	list_node_t list;
	list_head wait_list;

	// pointer to parent process
	struct pcb * parent;

	/* process id */
	pid_t pid;

	int task_id;

	/* BLOCK | READY | RUNNING */
	task_status_t status;

	PTE * pgdir;

	uint32_t dt_size;
	uint32_t us_size;
	uint32_t ks_size;

	/* for taskset */
	core_mask_t core_mask;
	core_id_t current_core_id;

	/* cursor position */
	int cursor_x;
	int cursor_y;

	/* time(seconds) to wake up sleeping PCB */
	uint64_t wakeup_time;

	/* mutex and mailbox index */
	int mlock_table[LOCK_NUM];
	int mbox_table[MBOX_NUM];

} pcb_t;

/* ready queue to run */
extern list_head ready_queue;

/* sleep queue to be blocked in */
extern list_head sleep_queue;

/* waitlist */
extern list_head wait_queue;

/* current running task PCB */
register pcb_t * current_running asm("tp");
extern pid_t process_id[CPU_NUM];

extern pcb_t pcb[NUM_MAX_TASK]; 	// pid from 1 to 16
extern pcb_t pid0_pcb[CPU_NUM];

extern void switch_to(pcb_t *prev, pcb_t *next);
void do_scheduler(void);
void do_sleep(uint32_t);

void do_block(list_node_t *, list_head *queue);
void do_unblock(list_node_t *);

/************************************************************/
/* TODO [P3-TASK1] exec exit kill waitpid ps*/
#ifdef S_CORE
extern pid_t do_exec(int id, int argc, uint64_t arg0, uint64_t arg1, uint64_t arg2);
#else
extern void do_exec(char *name, int argc, char *argv[]);
#endif
extern pid_t do_fork(void);
extern int do_wait(int * status);
extern void do_exit(void);
extern int do_kill(pid_t pid);
extern int do_waitpid(pid_t pid);
extern void do_process_show();
extern pid_t do_getpid();
extern int do_taskset(int argc, char **argv); 

extern int alloc_proc(void);
extern void free_proc(pcb_t * pcb);

extern void wakeup(pcb_t * pcb);

extern void reparent(pcb_t * parent, pcb_t * child);

// extern uint64_t add_new_task(char *str, int argc, char **argv, int pid);
extern void init_switch_to(ptr_t kernel_stack, pcb_t * pcb);
extern void init_pcb_stack(
    ptr_t kernel_stack, ptr_t kuser_stack, ptr_t entry_point,
    pcb_t *pcb, int argc, char **argv);
/************************************************************/

extern pcb_t * switch_pgtable(pcb_t * pcb);

extern PTE * get_pgdir(pcb_t * pcb);

#endif
