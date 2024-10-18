#include <os/lock.h>
#include <os/sched.h>
#include <os/list.h>
#include <atomic.h>

mutex_lock_t mlocks[LOCK_NUM];
barrier_t barriers[BARRIER_NUM];
condition_t conditions[CONDITION_NUM];

void init_ipc(void)
{
	init_locks();
	init_barriers();
	init_conditions();
}


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
	// For details, see README.md
	while(mlocks[mlock_idx].lock.status != UNLOCKED)
		do_block(&current_running->list, &mlocks[mlock_idx].block_queue);
	mlocks[mlock_idx].lock.status = LOCKED;
}

void do_mutex_lock_release(int mlock_idx)
{
	/* TODO: [p2-task2] release mutex lock */

	mlocks[mlock_idx].lock.status = UNLOCKED;
	if(&mlocks[mlock_idx].block_queue != mlocks[mlock_idx].block_queue.next)	// queue is not empty
		do_unblock(mlocks[mlock_idx].block_queue.next);
}



void init_barriers(void)
{
	for(int i = 0; i < BARRIER_NUM; i++)
	{
		barriers[i].block_queue.next = &barriers[i].block_queue;
		barriers[i].block_queue.prev = &barriers[i].block_queue;
		barriers[i].block_queue.pcb_ptr = (ptr_t)NULL;
	}
}

int do_barrier_init(int key,int goal)
{
	barriers[key % BARRIER_NUM].key = key;
	barriers[key % BARRIER_NUM].goal = goal;
	barriers[key % BARRIER_NUM].arrived = 0;
	return key % BARRIER_NUM;
}

void do_barrier_wait(int bar_idx)
{
	barriers[bar_idx].arrived++;
	if(barriers[bar_idx].arrived < barriers[bar_idx].goal)
		do_block(&current_running->list, &barriers[bar_idx].block_queue);
	else
	{
		freeQueueToReady(&barriers[bar_idx].block_queue);
		barriers[bar_idx].arrived = 0;
	}
}

void do_barrier_destroy(int bar_idx)
{
	barriers[bar_idx].goal = 0;
}


void init_conditions(void)
{
	for(int i = 0; i < CONDITION_NUM; i++)
	{
		conditions[i].block_queue.next = &conditions[i].block_queue;
		conditions[i].block_queue.prev = &conditions[i].block_queue;
		conditions[i].block_queue.pcb_ptr = (ptr_t)NULL;
	}
}

int do_condition_init(int key)
{
	conditions[key % CONDITION_NUM].key = key;
	conditions[key % CONDITION_NUM].status = false;
	return key % CONDITION_NUM;
}

void do_condition_wait(int cond_idx,int mutex_idx)
{
	if(conditions[cond_idx].status == false)
	{
		do_mutex_lock_release(mutex_idx);
		do_block(&current_running->list,&conditions[cond_idx].block_queue);
		do_mutex_lock_acquire(mutex_idx);
	}
	else
		conditions[cond_idx].status = false;
}

void do_condition_signal(int cond_idx)
{
	if(conditions[cond_idx].block_queue.next != &conditions[cond_idx].block_queue)
	{
		do_unblock(conditions[cond_idx].block_queue.next);
	}
	else
		conditions[cond_idx].status = true;
}

void do_condition_broadcast(int cond_idx)
{
	if(conditions[cond_idx].block_queue.next != &conditions[cond_idx].block_queue)
	{
		freeQueueToReady(&conditions[cond_idx].block_queue);
	}
	else
		conditions[cond_idx].status = true;
}

void do_condition_destroy(int cond_idx)
{
	freeQueueToReady(&conditions[cond_idx].block_queue);
}