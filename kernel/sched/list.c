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

void deleteNode(list_node_t * listnode)
{
	listnode->prev->next = listnode->next;
	listnode->next->prev = listnode->prev;
	
	// note that the pointer is NULL
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
		return (ptr_t)FIND_PCB(ready_queue.next);
	else
		return (ptr_t)(&pid0_pcb);
}