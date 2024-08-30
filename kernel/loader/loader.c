#include <os/task.h>
#include <os/string.h>
#include <os/kernel.h>
#include <type.h>



uint64_t load_task_img(int taskid)
{
	/**
	* TODO:
	* 1. [p1-task3] load task from image via task id, and return its entrypoint
	* 2. [p1-task4] load task via task name, thus the arg should be 'char *taskname'
	*/
	bios_sd_read(TASK_MEM_BASE,tasks[taskid].block_num,tasks[taskid].block_id);
	return TASK_MEM_BASE;
}