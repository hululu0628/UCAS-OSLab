/* for p2-task5 */
#include <os/time.h>
#include <os/sched.h>
#include <os/workload.h>
#include <printk.h>

/* a piece of **** */
fly_len_t length[FLY_NUM];
static int init;
int num;
int flight;
void do_workload_schedule(uint64_t remain)
{
	int i;
	int flag = 1;
	if(remain > length[process_id - 8].remain_length)
	{
		length[process_id - 8].num += 1;
		flight = length[process_id - 8].num;
		for(i=0;i<FLY_NUM;i++)
		{
			if(length[i].num < flight)
			{
				while(length[i].num < flight)
					do_scheduler();
			}
		}
	}
	length[process_id - 8].remain_length = remain;
	if(!init)
		length[process_id - 8].done = 1;
	for(i=0;i<FLY_NUM;i++)
	{
		if(length[i].done == 0)
		{
			flag = 0;
			break;
		}
	}
	if(flag)
	{
		int min = 60;
		init = 1;
		for(i=0;i<FLY_NUM;i++)
		{
			if(length[i].remain_length < min && length[i].remain_length > 0)
				min = length[i].remain_length;
		}
		for(i=0;i<FLY_NUM;i++)
		{
			if(min != 1)
				length[i].next = length[i].remain_length - length[i].remain_length / min;
			else
				length[i].next = 0;
			if(length[i].remain_length != 0)
				length[i].done = 0;
		}
		num = 0;
	}
	if(length[process_id - 8].next == remain)
	{
		length[process_id - 8].done = 1;
		num = 0;
		for(i=0;i<FLY_NUM;i++)
			num += length[i].done;
		do_scheduler();
	}
}