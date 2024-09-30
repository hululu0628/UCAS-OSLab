#include <syscall.h>
#include <stdint.h>
#include <kernel.h>
#include <unistd.h>

static const long IGNORE = 0L;

static long invoke_syscall(long sysno, long arg0, long arg1, long arg2,
                           long arg3, long arg4)
{
	long result;
	/* TODO: [p2-task3] implement invoke_syscall via inline assembly */
	asm volatile(
		"mv	a0,%[a0]\n\t"
		"mv	a1,%[a1]\n\t"
		"mv 	a2,%[a2]\n\t"
		"mv	a3,%[a3]\n\t"
		"mv	a4,%[a4]\n\t"
		"mv	a7,%[sysno]\n\t"
		"ecall\n\t"
		"mv	%[result],a0"
		:[result]"+r"(result)
		:[a0]"r"(arg0),[a1]"r"(arg1),[a2]"r"(arg2),[a3]"r"(arg3),[a4]"r"(arg4),[sysno]"r"(sysno)
		:"a0","a1","a2","a3","a4","a7"
	);

	return result;
}

void sys_yield(void)
{
	/* TODO: [p2-task1] call call_jmptab to implement sys_yield */
	// call_jmptab(YIELD,0,0,0,0,0);
	/* TODO: [p2-task3] call invoke_syscall to implement sys_yield */
        invoke_syscall(SYSCALL_YIELD, 0, 0, 0, 0, 0);
}

void sys_move_cursor(int x, int y)
{
	/* TODO: [p2-task1] call call_jmptab to implement sys_move_cursor */
	// call_jmptab(MOVE_CURSOR,(long)x,(long)y,0,0,0);
	/* TODO: [p2-task3] call invoke_syscall to implement sys_move_cursor */
        invoke_syscall(SYSCALL_CURSOR, (long)x, (long)y, 0, 0, 0);
}

void sys_write(char *buff)
{
	/* TODO: [p2-task1] call call_jmptab to implement sys_write */
	// the function we need is screen_write
	// call_jmptab(WRITE,(long)buff,0,0,0,0);
	/* TODO: [p2-task3] call invoke_syscall to implement sys_write */
	invoke_syscall(SYSCALL_WRITE, (long)buff, 0, 0, 0, 0);
}

void sys_reflush(void)
{
	/* TODO: [p2-task1] call call_jmptab to implement sys_reflush */
	// the function we need is screen_flush
	// call_jmptab(FLUSH,0,0,0,0,0);
	/* TODO: [p2-task3] call invoke_syscall to implement sys_reflush */
        invoke_syscall(SYSCALL_REFLUSH, 0, 0, 0, 0, 0);
}

int sys_mutex_init(int key)
{
	/* TODO: [p2-task2] call call_jmptab to implement sys_mutex_init */
	// return call_jmptab(MUTEX_INIT, (long)key, 0, 0, 0, 0);
	/* TODO: [p2-task3] call invoke_syscall to implement sys_mutex_init */
        return (int)invoke_syscall(SYSCALL_LOCK_INIT, (long)key, 0, 0, 0, 0);
}

void sys_mutex_acquire(int mutex_idx)
{
	/* TODO: [p2-task2] call call_jmptab to implement sys_mutex_acquire */
	// call_jmptab(MUTEX_ACQ,(long)mutex_idx,0,0,0,0);
	/* TODO: [p2-task3] call invoke_syscall to implement sys_mutex_acquire */
        invoke_syscall(SYSCALL_LOCK_ACQ, (long)mutex_idx, 0, 0, 0, 0);
}

void sys_mutex_release(int mutex_idx)
{
	/* TODO: [p2-task2] call call_jmptab to implement sys_mutex_release */
	// call_jmptab(MUTEX_RELEASE,(long)mutex_idx,0,0,0,0);
	/* TODO: [p2-task3] call invoke_syscall to implement sys_mutex_release */
        invoke_syscall(SYSCALL_LOCK_RELEASE, (long)mutex_idx, 0, 0, 0, 0);
}

long sys_get_timebase(void)
{
	/* TODO: [p2-task3] call invoke_syscall to implement sys_get_timebase */
        return invoke_syscall(SYSCALL_GET_TIMEBASE, 0, 0, 0, 0, 0);
}

long sys_get_tick(void)
{
	/* TODO: [p2-task3] call invoke_syscall to implement sys_get_tick */
	return invoke_syscall(SYSCALL_GET_TICK, 0, 0, 0, 0, 0);
}

void sys_sleep(uint32_t time)
{
	/* TODO: [p2-task3] call invoke_syscall to implement sys_sleep */
	invoke_syscall(SYSCALL_SLEEP, (long)time, 0, 0, 0, 0);
}

// for p2-task5
void sys_set_sche_workload(uint64_t remain)
{
	invoke_syscall(SYSCALL_WORKLOAD, (long)remain, 0, 0, 0, 0);
}

/************************************************************/
/* Do not touch this comment. Reserved for future projects. */
/************************************************************/