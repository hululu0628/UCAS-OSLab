#include <os/list.h>
#include <os/proc.h>
#include <type.h>

uint64_t time_elapsed = 0;
uint64_t time_base = 0;

uint64_t get_ticks()
{
    __asm__ __volatile__(
        "rdtime %0"
        : "=r"(time_elapsed));
    return time_elapsed;
}

uint64_t get_timer()
{
    return get_ticks() / time_base;
}

uint64_t get_time_base()
{
    return time_base;
}

void latency(uint64_t time)
{
    uint64_t begin_time = get_timer();

    while (get_timer() - begin_time < time);
    return;
}

void check_sleeping(void)
{
	// TODO: [p2-task3] Pick out tasks that should wake up from the sleep queue
	list_node_t * sleep_tcb = sleep_queue.next;
	list_node_t * temp;
	uint64_t current_time = get_timer();
	while(sleep_tcb != &sleep_queue)
	{
		if(current_time >= FIND_TCB(sleep_tcb)->wakeup_time)
		{
			FIND_TCB(sleep_tcb)->status = TASK_READY;
			temp = sleep_tcb->next;
			deleteNode(sleep_tcb);
			addToQueue(sleep_tcb, &ready_queue);
			sleep_tcb = temp;
		}
		else
			sleep_tcb = sleep_tcb->next;
	}
}