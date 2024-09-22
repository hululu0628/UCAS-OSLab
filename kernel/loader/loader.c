#include <os/task.h>
#include <os/string.h>
#include <os/kernel.h>
#include <os/loader.h>
#include <type.h>


uint64_t load_task_img(int taskid)
{
	/**
	* TODO:
	* 1. [p1-task3] load task from image via task id, and return its entrypoint
	* 2. [p1-task4] load task via task name, thus the arg should be 'char *taskname'
	*/
	// APP X starts from (0x52000000 + X * 0x10000)
	bios_sd_read((TASK_MEM_BASE + taskid * TASK_SIZE),tasks[taskid].block_num,tasks[taskid].block_id);
	return (TASK_MEM_BASE + taskid * TASK_SIZE);
}

uint64_t load_task_img_by_name(char * str)
{
	for(int i = 0; i < TASK_MAXNUM; i++)
	{
		if(strcmp(tasks[i].filename,str)==0)
		{
			bios_sd_read((TASK_MEM_BASE + i * TASK_SIZE),tasks[i].block_num,tasks[i].block_id);
			return (TASK_MEM_BASE + i * TASK_SIZE);
		}
	}
	return 0;
}