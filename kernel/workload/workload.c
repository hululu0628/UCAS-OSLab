#include <os/time.h>
#include <os/sched.h>
#include <os/workload.h>
#include <printk.h>

fly_len_t length[FLY_NUM];

void do_workload_schedule(uint64_t remain)
{
	int i = 0;
	int status = 1;
	int pid = current_running->pid;
	uint64_t interval = 0;
	uint64_t total;
	if(length[i].pid == 0)
	{
		for(i=0;i<FLY_NUM;i++)
		{
			length[i].remain_length = LENGTH;
			length[i].time_interval = TIMER_INTERVAL / 3;
			length[i].pid = i + 8;
			length[i].prev_time = 0;
		}
	}

	printl("pid: %d, remain: %d,total_running: %ld,prev: %ld\n",
		current_running->pid,remain,current_running->total_running, length[pid-8].prev_time);

	for(i = 0; i < FLY_NUM; i++)
	{
		if(length[i].pid == current_running->pid)
		{
			if(remain > length[i].remain_length)
				status = 0;
			length[i].remain_length = remain;

			interval = get_ticks() - current_running->start_tick;
			if(current_running->total_running != length[i].prev_time)
			{
				interval += current_running->total_running - length[i].prev_time;
				length[i].prev_time = current_running->total_running;
			}

			length[i].time_interval = interval;
			break;
		}
	}
	for(i=0,total=0;i<FLY_NUM;i++)
	{
		total += (length[i].remain_length + 1) * length[i].time_interval;
	}
	if(1)
		current_running->time_chunk = ((remain + 1) * interval/2) * (FLY_NUM * TIMER_INTERVAL) / total;
	else  
		current_running->time_chunk = TIMER_INTERVAL;
	

	printl("interval: %ld,total: %u, chunk: %ld\n",
		interval,total,current_running->time_chunk);

	if(current_running->time_chunk < (get_ticks() - current_running->start_tick))
		do_scheduler();
	
}