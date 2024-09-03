#ifndef __INCLUDE_BATCH_H__
#define __INCLUDE_BATCH_H__

typedef struct task_queue
{
	int taskid;
	char object_file;
	int (*p)();
} task_queue;

void batch_sh(task_queue array[],int * taskhead);

#endif