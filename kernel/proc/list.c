#include <os/list.h>
#include <os/proc.h>
#include <printk.h>
void addToQueue(list_node_t * listnode, list_head * queue)
{
	(queue->prev)->next = listnode;
	listnode->prev = queue->prev;
	listnode->next = queue;
	queue->prev = listnode;
}

void deleteNode(list_node_t * listnode)
{
	if(listnode->next || listnode->prev)
	{
		listnode->prev->next = listnode->next;
		listnode->next->prev = listnode->prev;
	
		// note that the pointer is NULL
		listnode->next = NULL;
		listnode->prev = NULL;
	}
	else
		printl("WARNING: In function deleteNode, the list is not in a queue\n");
}

ptr_t getReadyProcess(list_head * queue)
{
	if(queue->next != queue)
		return (ptr_t)FIND_TCB(queue->next);
	else
		return (ptr_t)(&pid0_tcb[get_current_cpu_id()]);
}

void freeQueueToReady(list_head * head)
{
	while(head->next != head)
	{
		do_unblock(head->next);
	}
}