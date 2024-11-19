#include <os/task.h>
#include <os/string.h>
#include <os/kernel.h>
#include <os/loader.h>
#include <type.h>


uint64_t load_task(int taskid, uint64_t kaddr)
{
	/**
	* TODO:
	* 1. [p1-task3] load task from image via task id, and return its entrypoint
	* 2. [p1-task4] load task via task name, thus the arg should be 'char *taskname'
	*/
	// APP X starts from (0x52000000 + X * 0x10000)
	int num = tasks[taskid].block_num;
	int id = tasks[taskid].block_id;
	while(num > 64)
	{
		bios_sd_read(kaddr,64,id);
		id += 64;
		num -= 64;
		kaddr += (SECTOR_SIZE << 6);
	}
	if(num > 0)
		bios_sd_read(kaddr,num,id);
	
	return 1;
}

uint64_t load_task_l(uint64_t kaddr, uint64_t block_id,uint64_t length)
{
	if(length > 0 && length <= 64)
		bios_sd_read(kaddr,length,block_id);
	else
		return -1;
	return 1;
}

uint64_t find_task(char * str)
{
	for(int i = 0; i < TASK_MAXNUM; i++)
	{
		if(strcmp(tasks[i].filename,str)==0)
		{
			return i;
		}
	}
	return -1;
}