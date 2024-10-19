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
	// call_jmptab(REFLUSH,0,0,0,0,0);
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

/* for p2-task5
void sys_set_sche_workload(uint64_t remain)
{
	invoke_syscall(SYSCALL_WORKLOAD, (long)remain, 0, 0, 0, 0);
}*/

/************************************************************/
#ifdef S_CORE
pid_t  sys_exec(int id, int argc, uint64_t arg0, uint64_t arg1, uint64_t arg2)
{
    /* TODO: [p3-task1] call invoke_syscall to implement sys_exec for S_CORE */
}    
#else
pid_t  sys_exec(char *name, int argc, char **argv)
{
	/* TODO: [p3-task1] call invoke_syscall to implement sys_exec */
	invoke_syscall(SYSCALL_EXEC, (long)name, (long)argc, (long)argv, 0, 0);
}
#endif

void sys_exit(void)
{
	/* TODO: [p3-task1] call invoke_syscall to implement sys_exit */
	invoke_syscall(SYSCALL_EXIT, 0, 0, 0, 0, 0);
}

int  sys_kill(pid_t pid)
{
	/* TODO: [p3-task1] call invoke_syscall to implement sys_kill */
	invoke_syscall(SYSCALL_KILL, (long)pid, 0, 0, 0, 0);
}

int  sys_waitpid(pid_t pid)
{
	/* TODO: [p3-task1] call invoke_syscall to implement sys_waitpid */
	invoke_syscall(SYSCALL_WAITPID, (long)pid, 0, 0, 0, 0);
}


void sys_ps(void)
{
	/* TODO: [p3-task1] call invoke_syscall to implement sys_ps */
	invoke_syscall(SYSCALL_PS, 0, 0, 0, 0, 0);
}

pid_t sys_getpid()
{
	/* TODO: [p3-task1] call invoke_syscall to implement sys_getpid */
	return invoke_syscall(SYSCALL_GETPID, 0, 0, 0, 0, 0);
}

int  sys_getchar(void)
{
	/* TODO: [p3-task1] call invoke_syscall to implement sys_getchar */
	return invoke_syscall(SYSCALL_GETCHAR, 0, 0, 0, 0, 0);
}

// Considering the inconsistency of line buffering and screen display array
// I added this system call to make it easier to implement input reading
void sys_putchar(int ch)
{
	invoke_syscall(SYSCALL_PUTCHAR, (long)ch, 0, 0, 0, 0);
}

void sys_clear(void)
{
	invoke_syscall(SYSCALL_CLEAR, 0, 0, 0, 0, 0);
}

int  sys_barrier_init(int key, int goal)
{
	/* TODO: [p3-task2] call invoke_syscall to implement sys_barrier_init */
	invoke_syscall(SYSCALL_BARR_INIT, (long)key, (long)goal, 0, 0, 0);
}

void sys_barrier_wait(int bar_idx)
{
	/* TODO: [p3-task2] call invoke_syscall to implement sys_barrie_wait */
	invoke_syscall(SYSCALL_BARR_WAIT, (long)bar_idx, 0, 0, 0, 0);
}

void sys_barrier_destroy(int bar_idx)
{
	/* TODO: [p3-task2] call invoke_syscall to implement sys_barrie_destory */
	invoke_syscall(SYSCALL_BARR_DESTROY, (long)bar_idx, 0, 0, 0, 0);
}

int sys_condition_init(int key)
{
	/* TODO: [p3-task2] call invoke_syscall to implement sys_condition_init */
	invoke_syscall(SYSCALL_COND_INIT, (long)key, 0, 0, 0, 0);
}

void sys_condition_wait(int cond_idx, int mutex_idx)
{
	/* TODO: [p3-task2] call invoke_syscall to implement sys_condition_wait */
	invoke_syscall(SYSCALL_COND_WAIT, (long)cond_idx, (long)mutex_idx, 0, 0, 0);
}

void sys_condition_signal(int cond_idx)
{
	/* TODO: [p3-task2] call invoke_syscall to implement sys_condition_signal */
	invoke_syscall(SYSCALL_COND_SIGNAL, (long)cond_idx, 0, 0, 0, 0);
}

void sys_condition_broadcast(int cond_idx)
{
	/* TODO: [p3-task2] call invoke_syscall to implement sys_condition_broadcast */
	invoke_syscall(SYSCALL_COND_BROADCAST, (long)cond_idx, 0, 0, 0, 0);
}

void sys_condition_destroy(int cond_idx)
{
	/* TODO: [p3-task2] call invoke_syscall to implement sys_condition_destroy */
	invoke_syscall(SYSCALL_COND_DESTROY, (long)cond_idx, 0, 0, 0, 0);
}

int sys_semaphore_init(int key, int init)
{
    	/* TODO: [p3-task2] call invoke_syscall to implement sys_semaphore_init */
}

void sys_semaphore_up(int sema_idx)
{
	/* TODO: [p3-task2] call invoke_syscall to implement sys_semaphore_up */
}

void sys_semaphore_down(int sema_idx)
{
	/* TODO: [p3-task2] call invoke_syscall to implement sys_semaphore_down */
}

void sys_semaphore_destroy(int sema_idx)
{
	/* TODO: [p3-task2] call invoke_syscall to implement sys_semaphore_destroy */
}

int sys_mbox_open(char * name)
{
	/* TODO: [p3-task2] call invoke_syscall to implement sys_mbox_open */
	return invoke_syscall(SYSCALL_MBOX_OPEN, (long)name, 0, 0, 0, 0);
}

void sys_mbox_close(int mbox_id)
{
	/* TODO: [p3-task2] call invoke_syscall to implement sys_mbox_close */
	invoke_syscall(SYSCALL_MBOX_CLOSE, (long)mbox_id, 0, 0, 0, 0);
}

int sys_mbox_send(int mbox_idx, void *msg, int msg_length)
{
	/* TODO: [p3-task2] call invoke_syscall to implement sys_mbox_send */
	return invoke_syscall(SYSCALL_MBOX_SEND, (long)mbox_idx, (long)msg, (long)msg_length, 0, 0);
}

int sys_mbox_recv(int mbox_idx, void *msg, int msg_length)
{
    	/* TODO: [p3-task2] call invoke_syscall to implement sys_mbox_recv */
	return invoke_syscall(SYSCALL_MBOX_RECV, (long)mbox_idx, (long)msg, (long)msg_length, 0, 0);
}
/************************************************************/