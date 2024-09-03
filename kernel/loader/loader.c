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
	bios_sd_read(task_addr,tasks[taskid].block_num,tasks[taskid].block_id);
	task_addr += TASK_SIZE;
	return (task_addr - TASK_SIZE);
}