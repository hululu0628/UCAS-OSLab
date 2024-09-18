#include <os/list.h>
#include <os/lock.h>
#include <os/sched.h>
#include <os/time.h>
#include <os/mm.h>
#include <screen.h>
#include <printk.h>
#include <assert.h>

pcb_t pcb[NUM_MAX_TASK];
const ptr_t pid0_stack = INIT_KERNEL_STACK + PAGE_SIZE;
pcb_t pid0_pcb = {
	.pid = 0,
	.kernel_sp = (ptr_t)pid0_stack,
	.user_sp = (ptr_t)pid0_stack
};

LIST_HEAD(ready_queue);
LIST_HEAD(sleep_queue);

/* global process id */
pid_t process_id = 1;

void do_scheduler(void)
{
	// TODO: [p2-task3] Check sleep queue to wake up PCBs

	/************************************************************/
	/* Do not touch this comment. Reserved for future projects. */
	/************************************************************/

	// TODO: [p2-task1] Modify the current_running pointer.
	pcb_t * prev_process = current_running;
	if(current_running->status == TASK_RUNNING)
	{
		current_running->status = TASK_READY;

		if(current_running != &pid0_pcb)
			addToQueue(&current_running->list,&ready_queue);

		current_running = (pcb_t *)getProcess();
		current_running->status = TASK_RUNNING;
		if(current_running->pid != 0)
			deleteHead(ready_queue.next);
	}

	// TODO: [p2-task1] switch_to current_running
	switch_to(prev_process,current_running);
}

void do_sleep(uint32_t sleep_time)
{
    // TODO: [p2-task3] sleep(seconds)
    // NOTE: you can assume: 1 second = 1 `timebase` ticks
    // 1. block the current_running
    // 2. set the wake up time for the blocked task
    // 3. reschedule because the current_running is blocked.
}

void do_block(list_node_t *pcb_node, list_head *queue)
{
	// TODO: [p2-task2] block the pcb task into the block queue
	pcb_t * prev_process = current_running;
	current_running->status = TASK_BLOCKED;
	if(current_running != &pid0_pcb)
		addToQueue(pcb_node, queue);

	current_running = (pcb_t *)getProcess();
	current_running->status = TASK_RUNNING;
	if(current_running->pid != 0)
		deleteHead(ready_queue.next);

	switch_to(prev_process,current_running);
}

void do_unblock(list_node_t *pcb_node)
{
	// TODO: [p2-task2] unblock the `pcb` from the block queue
	pcb_t * pcb_unblock = (pcb_t *)pcb_node->pcb_ptr;
	deleteHead(pcb_node);
	pcb_unblock->status = TASK_READY;
	addToQueue(pcb_node, &ready_queue);
}
