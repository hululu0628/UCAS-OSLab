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
	init_mbox();
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
	current_running->mlock_idx = mlock_idx;
}

void do_mutex_lock_release(int mlock_idx)
{
	/* TODO: [p2-task2] release mutex lock */

	mlocks[mlock_idx].lock.status = UNLOCKED;
	if(&mlocks[mlock_idx].block_queue != mlocks[mlock_idx].block_queue.next)	// queue is not empty
		do_unblock(mlocks[mlock_idx].block_queue.next);
	current_running->mlock_idx = -1;
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


void init_mbox()
{
	for(int i = 0; i < MBOX_NUM; i++)
	{
		mailboxes[i].rev_block_queue.next = &mailboxes[i].rev_block_queue;
		mailboxes[i].rev_block_queue.prev = &mailboxes[i].rev_block_queue;
		mailboxes[i].send_block_queue.next = &mailboxes[i].send_block_queue;
		mailboxes[i].send_block_queue.prev = &mailboxes[i].send_block_queue;
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
			current_running->mbox_idx = i;
			return i;
		}
	}
	for(int i = 0; i < MBOX_NUM; i++)
	{
		if(mailboxes[i].name[0] == '\0')	//ref_cnt == 0
		{
			strcpy(mailboxes[i].name, name);
			mailboxes[i].ref_cnt++;
			current_running->mbox_idx = i;
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
		current_running->mbox_idx = -1;
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
		if(msg_length > mailboxes[mbox_idx].remain_length)
		{
			do_block(&current_running->list, &mailboxes[mbox_idx].send_block_queue);
			blocked += 1;
		}
		else
		{
			send_msg(mbox_idx,msg,msg_length);
			if(mailboxes[mbox_idx].rev_block_queue.next != &mailboxes[mbox_idx].rev_block_queue)
				freeQueueToReady(&mailboxes[mbox_idx].rev_block_queue);
			return blocked;
		}
	}
}

int do_mbox_recv(int mbox_idx, void *msg, int msg_length)
{
	int blocked = 0;
	while(1)
	{
		if(msg_length > MAX_MBOX_LENGTH - mailboxes[mbox_idx].remain_length)
		{
			do_block(&current_running->list, &mailboxes[mbox_idx].rev_block_queue);
			blocked += 1;
		}
		else
		{
			recv_msg(mbox_idx,msg,msg_length);
			if(mailboxes[mbox_idx].send_block_queue.next != &mailboxes[mbox_idx].send_block_queue)
				freeQueueToReady(&mailboxes[mbox_idx].send_block_queue);
			return blocked;
		}
	}
}