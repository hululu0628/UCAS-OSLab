#ifndef INCLUDE_STDIO_H_
#define INCLUDE_STDIO_H_

#include <stdarg.h>

/* modes of sys_open */
#define O_RDONLY 1  /* read only open */
#define O_WRONLY 2  /* write only open */
#define O_RDWR   3  /* read/write open */

/* whence of sys_lseek */
#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2

/* options for ls */
#define NORMAL_LIST	0
#define LONG_LIST	1

/* cache policy */
#define WRITE_BACK	1
#define WRITE_THROUGH	2

int printf(const char *fmt, ...);
int vprintf(const char *fmt, va_list va);

int getchar(void);
int gets(char *str);

#endif
