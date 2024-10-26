#include <os/lock.h>
#include <os/sched.h>
#include <os/list.h>
#include <os/string.h>
#include <atomic.h>
#include <printk.h>

mutex_lock_t mlocks[LOCK_NUM];
barrier_t barriers[BARRIER_NUM];
condition_t conditions[CONDITION_NUM];
mailbox_t mailboxes[MBOX_NUM];

void init_ipc(void)
{
	init_locks();
	init_barriers();
	init_conditions();
	init_mailboxes();
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
	lock->status = UNLOCKED;
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

void mutex_acquire(mutex_lock_t *lock)
{
	while(lock->lock.status != UNLOCKED)
		do_block(&current_running->list, &lock->block_queue);
	lock->lock.status = LOCKED;
}

void do_mutex_lock_acquire(int mlock_idx)
{
	/* TODO: [p2-task2] acquire mutex lock */

	/* NOTE: This is non-reentrant mutex.
	   Multiple requests for a lock from the same process can lead to deadlocks */
	// The interrupt is disabled gloablly in S-mode, but still using atomic operation
	// The process tries to acquire the lock until it succeed
	// For details, see README.md
	mutex_acquire(&mlocks[mlock_idx]);
	current_running->mlock_table[mlock_idx] = 1;
}

void mutex_release(mutex_lock_t *lock)
{
	lock->lock.status = UNLOCKED;
	if(&lock->block_queue != lock->block_queue.next)	// queue is not empty
		do_unblock(lock->block_queue.next);
}

void do_mutex_lock_release(int mlock_idx)
{
	/* TODO: [p2-task2] release mutex lock */

	mutex_release(&mlocks[mlock_idx]);
	current_running->mlock_table[mlock_idx] = 0;
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
	return key % CONDITION_NUM;
}

void condition_wait(condition_t *cond, mutex_lock_t *lock)
{
	mutex_release(lock);
	do_block(&current_running->list, &cond->block_queue);
	mutex_acquire(lock);
}

void do_condition_wait(int cond_idx,int mutex_idx)
{
	condition_wait(&conditions[cond_idx], &mlocks[mutex_idx]);
}

void condition_signal(condition_t *cond)
{
	if(cond->block_queue.next != &cond->block_queue)
	{
		do_unblock(cond->block_queue.next);
	}
}

void do_condition_signal(int cond_idx)
{
	condition_signal(&conditions[cond_idx]);
}

void condition_broadcast(condition_t *cond)
{
	if(cond->block_queue.next != &cond->block_queue)
	{
		freeQueueToReady(&cond->block_queue);
	}
}

void do_condition_broadcast(int cond_idx)
{
	condition_broadcast(&conditions[cond_idx]);
}

void do_condition_destroy(int cond_idx)
{
	freeQueueToReady(&conditions[cond_idx].block_queue);
}


void init_mailboxes()
{
	for(int i = 0; i < MBOX_NUM; i++)
	{
		mailboxes[i].condition.block_queue.next = &mailboxes[i].condition.block_queue;
		mailboxes[i].condition.block_queue.prev = &mailboxes[i].condition.block_queue;
		mailboxes[i].remain_length = MAX_MBOX_LENGTH;
	}
}

int do_mbox_open(char *name)
{
	for(int i = 0; i < MBOX_NUM; i++)
	{
		if(mailboxes[i].name[0] != '\0' && strcmp(mailboxes[i].name,name) == 0)
		{
			mailboxes[i].ref_cnt++;
			current_running->mbox_table[i] = 1;
			return i;
		}
	}
	for(int i = 0; i < MBOX_NUM; i++)
	{
		if(mailboxes[i].name[0] == '\0')	//ref_cnt == 0
		{
			strcpy(mailboxes[i].name, name);
			mailboxes[i].ref_cnt++;
			current_running->mbox_table[i] = 1;
			return i;
		}
	}
	printl("WARNING: In function do_mbox_open(), there's no mailbox\n");
	return -1;
}

void do_mbox_close(int mbox_idx)
{
	mailboxes[mbox_idx].ref_cnt--;
	if(mailboxes[mbox_idx].ref_cnt == 0)
	{
		mailboxes[mbox_idx].name[0] = '\0';
		mailboxes[mbox_idx].head = 0;
		mailboxes[mbox_idx].tail = 0;
		mailboxes[mbox_idx].remain_length = MAX_MBOX_LENGTH;
		current_running->mbox_table[mbox_idx] = 0;
	}
	else if(mailboxes[mbox_idx].ref_cnt < 0)
	{
		printl("ERROR: In function do_mbox_close(), ref_cnt < 0");
	}
}

static void send_msg(int mbox_idx, void * msg, int msg_length)
{
	for(int i = 0; i < msg_length; i++)
	{
		mailboxes[mbox_idx].msg_array[mailboxes[mbox_idx].tail] = ((uint8_t *)msg)[i];
		if(mailboxes[mbox_idx].tail >= MAX_MBOX_LENGTH - 1)
			mailboxes[mbox_idx].tail = 0;
		else
			mailboxes[mbox_idx].tail++;
	}
	mailboxes[mbox_idx].remain_length -= msg_length;
}

static void recv_msg(int mbox_idx, void * msg, int msg_length)
{
	for(int i = 0; i < msg_length; i++)
	{
		((uint8_t *)msg)[i] = mailboxes[mbox_idx].msg_array[mailboxes[mbox_idx].head];
		if(mailboxes[mbox_idx].head >= MAX_MBOX_LENGTH - 1)
			mailboxes[mbox_idx].head = 0;
		else
			mailboxes[mbox_idx].head++;
	}
	mailboxes[mbox_idx].remain_length += msg_length;
}

int do_mbox_send(int mbox_idx, void * msg, int msg_length)
{
	int blocked = 0;
	while(1)
	{
		mutex_acquire(&mailboxes[mbox_idx].mutex);
		if(msg_length > mailboxes[mbox_idx].remain_length)
		{
			condition_wait(&mailboxes[mbox_idx].condition, &mailboxes[mbox_idx].mutex);
			blocked += 1;
		}
		else
		{
			send_msg(mbox_idx,msg,msg_length);
			condition_broadcast(&mailboxes[mbox_idx].condition);
			mutex_release(&mailboxes[mbox_idx].mutex);
			return blocked;
		}
		mutex_release(&mailboxes[mbox_idx].mutex);
	}
}

int do_mbox_recv(int mbox_idx, void *msg, int msg_length)
{
	int blocked = 0;
	while(1)
	{
		mutex_acquire(&mailboxes[mbox_idx].mutex);
		if(msg_length > MAX_MBOX_LENGTH - mailboxes[mbox_idx].remain_length)
		{
			condition_wait(&mailboxes[mbox_idx].condition, &mailboxes[mbox_idx].mutex);
			blocked += 1;
		}
		else
		{
			recv_msg(mbox_idx,msg,msg_length);
			condition_broadcast(&mailboxes[mbox_idx].condition);
			mutex_release(&mailboxes[mbox_idx].mutex);
			return blocked;
		}
		mutex_release(&mailboxes[mbox_idx].mutex);
	}
}