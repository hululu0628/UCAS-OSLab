/*  Author: Xianan Zhu  */
/*     for p1-task5     */

#ifndef __INCLUDE_PIPE_H__
#define __INCLUDE_PIPE_H__

#include <type.h>

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

#endif