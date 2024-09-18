#include "type.h"
#include <os/list.h>
#include <os/sched.h>
void addToQueue(list_node_t * listnode, list_head * queue)
{
	(queue->prev)->next = listnode;
	listnode->prev = queue->prev;
	listnode->next = queue;
	queue->prev = listnode;
}

void deleteHead(list_node_t * listnode)
{
	// need comments //
	listnode->prev->next = listnode->next;
	listnode->next->prev = listnode->prev;
	listnode->next = NULL;
	listnode->prev = NULL;
}

void allocReadyProcess()
{
	int i;
	for(i=0; i < NUM_MAX_TASK; i++)
	{
		if(pcb[i].status == TASK_READY)
			addToQueue(&pcb[i].list,&ready_queue);
	}
}

ptr_t getProcess()
{
	if(ready_queue.next != &ready_queue)
		return ready_queue.next->pcb_ptr;
	else
		return (ptr_t)(&pid0_pcb);
}