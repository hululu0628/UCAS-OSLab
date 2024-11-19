#ifndef __INCLUDE_LOADER_H__
#define __INCLUDE_LOADER_H__

#include <type.h>

uint64_t load_task(int taskid, uint64_t kaddr);
uint64_t load_task_l(uint64_t kaddr, uint64_t block_id, uint64_t length);

uint64_t find_task(char * str);

#endif