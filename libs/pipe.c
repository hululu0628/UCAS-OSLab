/*  Author: Xianan Zhu  */
/*     for p1-task5     */

#include <os/kernel.h>
#include <os/pipe.h>

//未实现队列阻塞,不能实时共享指针数据
void bp_putchar(int ch)
{
	task_crtl_info info;
	info = *((task_crtl_info *)PROGRAM_CONTROL_ADDR);
	if(info.io_d & ((uint8_t)PIPE_IN))
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
	if(info.io_d & ((uint8_t)PIPE_IN))
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
	if(info.io_d & ((uint8_t)PIPE_OUT))
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
		return bios_getchar();
}