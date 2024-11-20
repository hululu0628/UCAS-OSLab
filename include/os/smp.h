#ifndef SMP_H
#define SMP_H

#include <type.h>

#define CPU_NUM 2

#define MASK_ZERO 1
#define MASK_ONE 2
#define MASK_ZERO_ONE 3

#define CORE_ZERO 0
#define CORE_ONE 1
#define NO_CORE -1

typedef uint32_t core_mask_t;
typedef int core_id_t;

extern void init_proc0(int cpuid);

extern void smp_init();
extern void wakeup_other_hart();
extern uint64_t get_current_cpu_id();
extern void lock_kernel();
extern void unlock_kernel();

#endif /* SMP_H */
