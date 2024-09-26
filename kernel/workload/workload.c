#include <os/time.h>
#include <os/sched.h>
#include <os/workload.h>
#include <printk.h>

fly_len_t length[FLY_NUM];

void do_workload_schedule(uint64_t remain)
{
	int i = 0;
	int status = 1;
	int min = INT32_MAX;
	int pid = current_running->pid;
	uint64_t interval = 0, s = 0;
	uint64_t total;
	if(length[i].pid == 0)
	{
		for(i=0;i<FLY_NUM;i++)
		{
			length[i].remain_length = LENGTH;
			length[i].time_interval = TIMER_INTERVAL / 3;
			length[i].pid = i + 8;
			length[i].prev_time = 0;
			length[i].flights = 0;
		}
	}

	printl("pid: %d, remain: %d,total_running: %ld,prev: %ld\n",
		current_running->pid,remain,current_running->total_running, length[pid-8].prev_time);

	if(remain > length[pid-8].remain_length)
	{
		status = 0;
		length[pid-8].flights += 1;
	}
	length[pid-8].remain_length = remain;

	interval = get_ticks() - current_running->start_tick;
	s = interval;
	if(current_running->total_running != length[pid-8].prev_time)
	{
		interval += current_running->total_running - length[pid-8].prev_time;
		length[pid-8].prev_time = current_running->total_running;
	}
	length[pid-8].time_interval = interval;


	for(i=0;i<FLY_NUM;i++)
	{
		if(length[i].flights < min)
			min = length[i].flights;
	}
	for(i=0,total=0;i<FLY_NUM;i++)
	{
		if(length[i].flights == min)
			total += (length[i].remain_length + 1) * length[i].time_interval;
		else
			total += (length[i].remain_length + 1) * length[i].time_interval / SPEEDING_PENALTY;
	}
	if(length[pid-8].flights == min)
	{
		current_running->time_chunk = ((remain + 1) * interval) * (FLY_NUM * TIMER_INTERVAL) / total;
		if(current_running->time_chunk < s)
			do_scheduler();
	}
	else
	{
		current_running->time_chunk = (((remain + 1) * interval / SPEEDING_PENALTY) * (FLY_NUM * TIMER_INTERVAL) / total);
		do_scheduler();
	}
}