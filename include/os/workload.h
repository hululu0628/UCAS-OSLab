/* Author: Xianan Zhu */
/*    for p2-task5    */
#ifndef INCLUDE_WORKLOAD_H
#define INCLUDE_WORKLOAD_H

#include <os/sched.h>

#define LENGTH 60
#define FLY_NUM 5

typedef struct fly_length
{
	uint64_t prev_time;
	uint64_t time_interval;
	int remain_length;
	int pid
} fly_len_t;

extern fly_len_t length[FLY_NUM];

extern void do_workload_schedule(uint64_t remain);


#endif