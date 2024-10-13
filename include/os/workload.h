/* Author: Xianan Zhu */
/*    for p2-task5    */
#ifndef INCLUDE_WORKLOAD_H
#define INCLUDE_WORKLOAD_H

#include <os/sched.h>

#define LENGTH 60
#define FLY_NUM 5		// 飞机数为5
#define SPEEDING_PENALTY 10	// 当有飞机飞行次数比其他的都大时，进行惩罚，缩短分配的时间片

// 时间单位均为tick
typedef struct fly_length
{
	int done;
	int remain_length;		// 飞机距离终点的位置
	int pid;			// 对应进程的pid
	int next;
	int num;
} fly_len_t;

// 结构体数组
extern fly_len_t length[FLY_NUM];

extern int status;

extern void do_workload_schedule(uint64_t remain);


#endif