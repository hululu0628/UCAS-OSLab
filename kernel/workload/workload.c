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

/* 对于这五个进程，为它们设置一个目标的移动距离，设置后记作undone，
当所有飞机都done之后，根据现在的飞机位置再次分配下一次的目标移动距离，将这个过程记作一轮。
在每一轮内，对于所有undone的飞机，采用最简单的时间片轮转调度，
这样实现起来，粗看效果可以保证，但是实际上轮转导致了每一轮中飞机的飞行速度并不那么美观。 */

void do_workload_schedule(uint64_t remain)
{
	int i;
	int flag = 1;

	/*
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
	*/

	length[process_id - 8].remain_length = remain;
	if(!init)	// 初始化
		length[process_id - 8].done = 1;
	for(i=0;i<FLY_NUM;i++)
	{
		if(length[i].done == 0)
		{
			flag = 0; 	// 当一轮结束，flag置1
			break;
		}
	}
	if(flag)
	{
		// 每一轮结束后，分配下一轮目标距离
		int min = 60;
		init = 1;
		// 具体算法为将当前距离终点的最小值min作为单位1，计算出next = remain_length / min
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
		// 进程达成目标，设置done，计算出已经达成目标的飞机数
		length[process_id - 8].done = 1;
		num = 0;
		for(i=0;i<FLY_NUM;i++)
			num += length[i].done;
		do_scheduler();
	}
}