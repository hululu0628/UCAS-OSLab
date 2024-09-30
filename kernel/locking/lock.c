#include <os/lock.h>
#include <os/sched.h>
#include <os/list.h>
#include <atomic.h>

mutex_lock_t mlocks[LOCK_NUM];

void init_locks(void)
{
	/* TODO: [p2-task2] initialize mlocks */
	for(int i = 0; i < LOCK_NUM; i++)
	{
		mlocks[i].block_queue.next = &mlocks[i].block_queue;
		mlocks[i].block_queue.prev = &mlocks[i].block_queue;
		mlocks[i].block_queue.pcb_ptr = (ptr_t)NULL;
		mlocks[i].lock.status = UNLOCKED;
	}
}

void spin_lock_init(spin_lock_t *lock)
{
	/* TODO: [p2-task2] initialize spin lock */
}

int spin_lock_try_acquire(spin_lock_t *lock)
{
	/* TODO: [p2-task2] try to acquire spin lock */
	return 0;
}

void spin_lock_acquire(spin_lock_t *lock)
{
	/* TODO: [p2-task2] acquire spin lock */
}

void spin_lock_release(spin_lock_t *lock)
{
	/* TODO: [p2-task2] release spin lock */
}

int do_mutex_lock_init(int key)
{
	/* TODO: [p2-task2] initialize mutex lock */

	// a simple hash function
	mlocks[key % LOCK_NUM].key = key;
	return key % LOCK_NUM;
}

void do_mutex_lock_acquire(int mlock_idx)
{
	/* TODO: [p2-task2] acquire mutex lock */

	/* NOTE: This is non-reentrant mutex.
	   Multiple requests for a lock from the same process can lead to deadlocks */
	// The interrupt is disabled gloablly in S-mode, but still using atomic operation
	// The process tries to acquire the lock until it succeed
	while(atomic_cmpxchg(UNLOCKED, LOCKED, (ptr_t)(&mlocks[mlock_idx].lock.status)) == LOCKED)
		do_block(&current_running->list, &mlocks[mlock_idx].block_queue);
}

void do_mutex_lock_release(int mlock_idx)
{
	/* TODO: [p2-task2] release mutex lock */

	atomic_cmpxchg(LOCKED, UNLOCKED,(ptr_t)(&mlocks[mlock_idx].lock.status));
	if(&mlocks[mlock_idx].block_queue != mlocks[mlock_idx].block_queue.next)	// queue is not empty
		do_unblock(mlocks[mlock_idx].block_queue.next);
}
