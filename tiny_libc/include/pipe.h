/*  Author: Xianan Zhu  */
/*     for p1-task5     */

#ifndef __INCLUDE_PIPE_H__
#define __INCLUDE_PIPE_H__

#include "./kernel.h"

#define PROGRAM_CONTROL_ADDR 	0x50601010
#define PIPE_HEAD_ADDR 		0x50601000
#define PIPE_TAIL_ADDR 		0x50601008
#define PIPE_BUFFER_ADDR 	0x50600000

#define TERMINAL_IN	0x00
#define PIPE_IN		0x10
#define TERMINAL_OUT	0x00
#define PIPE_OUT	0x01

typedef struct
{
	unsigned char program_id;
	unsigned char io_d;
} task_crtl_info;

void bp_putstr(char *str);
void bp_putchar(int ch);
int bp_getchar();

void bp_putchar(int ch)
{
	task_crtl_info info;
	info = *((task_crtl_info *)PROGRAM_CONTROL_ADDR);
	if(info.io_d & ((unsigned char)PIPE_IN))
	{
		char* pipe_head_ptr = *((char **)PIPE_HEAD_ADDR);
		char** pipe_head_ptr_loc = (char **)PIPE_HEAD_ADDR;
		*pipe_head_ptr = (char)ch;
		pipe_head_ptr++;
		if(pipe_head_ptr >= (char *)PIPE_HEAD_ADDR)
			pipe_head_ptr = (char *)PIPE_BUFFER_ADDR;
		*pipe_head_ptr = 0;
		*pipe_head_ptr_loc = pipe_head_ptr;
	}
	else
		bios_putchar(ch);
}

void bp_putstr(char *str)
{
	task_crtl_info info;
	info = *((task_crtl_info *)PROGRAM_CONTROL_ADDR);
	if(info.io_d & ((unsigned char)PIPE_IN))
	{
		char* pipe_head_ptr = *((char **)PIPE_HEAD_ADDR);
		char** pipe_head_ptr_loc = (char **)PIPE_HEAD_ADDR;
		while(*str)
		{
			*pipe_head_ptr++ = *str++;
			if(pipe_head_ptr >= (char *)PIPE_HEAD_ADDR)
				pipe_head_ptr = (char *)PIPE_BUFFER_ADDR;
		}
		*pipe_head_ptr = 0;
		*pipe_head_ptr_loc = pipe_head_ptr;
	}
	else
		bios_putstr(str);
}

int bp_getchar()
{
	task_crtl_info info;
	info = *((task_crtl_info *)PROGRAM_CONTROL_ADDR);
	if((info.io_d & ((unsigned char)PIPE_OUT))!=0)
	{
		char* pipe_tail_ptr = *((char **)PIPE_TAIL_ADDR);
		char** pipe_tail_ptr_loc = (char **)PIPE_TAIL_ADDR;
		int result = *pipe_tail_ptr;
		if(result != 0)
		{
			pipe_tail_ptr++;
			if(pipe_tail_ptr >= (char *)PIPE_HEAD_ADDR)
				pipe_tail_ptr = (char *)PIPE_BUFFER_ADDR;
			*pipe_tail_ptr_loc = pipe_tail_ptr;
			return result;
		}
		else
			return -1;
	}
	else
	{
		return bios_getchar();
	}
}

#endif