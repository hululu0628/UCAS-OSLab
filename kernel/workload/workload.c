#include <os/time.h>
#include <os/sched.h>
#include <os/workload.h>
#include <printk.h>

fly_len_t length[FLY_NUM];

void do_workload_schedule(uint64_t remain)
{
	int i;
	if(!status)
	{
		for(i = 0; i < FLY_NUM; i++)
		{
			if(length[i].pid == 0)
			{
				length[i].pid = process_id;
				length[i].remain_length = remain;
				break;
			}
			else if(length[i].pid == process_id)
			{
				length[i].remain_length = remain;
			}
		}
		if(i == FLY_NUM)
			status = 1;
	}
	else
	{
		length[process_id - 8].remain_length = remain;
		
	}
}