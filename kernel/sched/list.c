#include <os/list.h>
#include <os/sched.h>
void addToReadyQueue(list_node_t * listnode)
{
	(ready_queue.prev)->next = listnode;
	listnode->prev = ready_queue.prev;
	listnode->next = &ready_queue;
	ready_queue.prev = listnode;
}

void deleteReadyHead()
{
	// need comments //
	if(ready_queue.next != &ready_queue)
	{
		ready_queue.next = ready_queue.next->next;
		ready_queue.next->prev->next = NULL;
		ready_queue.next->prev->prev = NULL;
		ready_queue.next->prev = &ready_queue;
	}
}

void allocReadyProcess()
{
	int i;
	for(i=0; i < NUM_MAX_TASK; i++)
	{
		if(pcb[i].status == TASK_READY)
			addToReadyQueue(&pcb[i].list);
	}
}

ptr_t getProcess()
{
	if(ready_queue.next != &ready_queue)
		return ready_queue.next->pcb_ptr;
	else
		return (ptr_t)(&pid0_pcb);
}