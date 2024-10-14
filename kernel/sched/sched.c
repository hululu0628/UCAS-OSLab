#include "os/kernel.h"
#include <os/list.h>
#include <os/lock.h>
#include <os/sched.h>
#include <os/time.h>
#include <os/mm.h>
#include <os/workload.h>	// for p2-task5
#include <screen.h>
#include <printk.h>
#include <assert.h>

pcb_t pcb[NUM_MAX_TASK];
const ptr_t pid0_stack = INIT_KERNEL_STACK + PAGE_SIZE;
pcb_t pid0_pcb = {
	.pid = 0,
	.kernel_sp = (ptr_t)pid0_stack,
	.user_sp = (ptr_t)pid0_stack,
};

LIST_HEAD(ready_queue);
LIST_HEAD(sleep_queue);

/* global process id */
pid_t process_id = 1;


void do_scheduler(void)
{
	// TODO: [p2-task3] Check sleep queue to wake up PCBs
	check_sleeping();

	/************************************************************/
	/* Do not touch this comment. Reserved for future projects. */
	/************************************************************/

	// TODO: [p2-task1] Modify the current_running pointer.
	pcb_t * prev_process = current_running;
	if(current_running->status == TASK_RUNNING)
	{
		current_running->status = TASK_READY;

		if(current_running != &pid0_pcb)
		{
			if(process_id > 7 && process_id < 13)	// for fly1~fly5：pid8~12
			{
				// 当进程完成了飞行目标，将其放到准备队列末尾
				if(length[process_id - 8].done)
					addToQueue(&current_running->list, &ready_queue);
				else
				{
					// 否则，根据未完成飞行目标的进程数，决定将当前进程放在队列哪个位置
					// 作用是保证所有飞机都完成当前飞行目标再开始下一轮分配
					switch (num)
					{
						case 4: addToQueue(&current_running->list, ready_queue.next);break;
						case 3: addToQueue(&current_running->list, ready_queue.next->next);break;
						case 2: addToQueue(&current_running->list, ready_queue.next->next->next);break;
						case 1: addToQueue(&current_running->list, ready_queue.next->next->next->next);break;
						case 0: addToQueue(&current_running->list, ready_queue.next->next->next->next->next);break;
						default: break;
					}
				}
			}
			else
				addToQueue(&current_running->list, &ready_queue);
		}
	}
	

	current_running = (pcb_t *)getProcess();
	process_id = current_running->pid;
	current_running->status = TASK_RUNNING;
	if(process_id != 0)
		deleteNode(ready_queue.next);

	bios_set_timer(get_ticks() + TIMER_INTERVAL);	// set timer interrupt

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
	if(current_running != &pid0_pcb)
	{
		current_running->status = TASK_BLOCKED;
		current_running->wakeup_time = get_timer() + sleep_time;
		addToQueue(&current_running->list, &sleep_queue);
		do_scheduler();
	}
	else
	{
		latency(sleep_time);		// in kernel process, call latency
	}

}

void do_block(list_node_t *pcb_node, list_head *queue)
{
	// TODO: [p2-task2] block the pcb task into the block queue
	current_running->status = TASK_BLOCKED;
	if(current_running != &pid0_pcb)
		addToQueue(pcb_node, queue);

	do_scheduler();
}

void do_unblock(list_node_t *pcb_node)
{
	// TODO: [p2-task2] unblock the `pcb` from the block queue
	pcb_t * pcb_unblock = FIND_PCB(pcb_node);
	deleteNode(pcb_node);
	pcb_unblock->status = TASK_READY;
	addToQueue(pcb_node, &ready_queue);
}
