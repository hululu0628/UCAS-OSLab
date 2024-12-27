#include <common.h>
#include <os/kernel.h>
#include <os/fs.h>
#include <os/fs_cache.h>
#include <os/string.h>
#include <os/time.h>

static uint8_t block_buffer[BLOCK_SIZE];

p_cache_t p_cache[P_CACHE_NUM];
int p_lru;
int p_used;

inode_idx_t vm_inode;
int page_cache_policy = WRITE_THROUGH;
uint64_t write_back_freq = 30;

void init_cache()
{
	int i;
	for(i = 0; i < P_CACHE_NUM; i++)
	{
		p_cache[i].cache_idx = i;
		p_cache[i].next = -1;
		p_cache[i].prev = -1;
	}
	p_lru = -1;
	p_used = 0;
}

void get_page(block_idx_t idx, uint8_t *res)
{
	int i = p_lru;
	int k;
	int prev, curr, tail;
	int hit = 0;
	uint8_t * p_array;
	if(p_lru != -1)
	{
		do
		{
			if((p_cache[i].ct & (LINE_VALID)) && (p_cache[i].start_idx == idx))
			{
				p_array = (uint8_t *)(D_CACHE_ADDR + (p_cache[i].cache_idx << BLOCK_SIZE_SHIFT));
				hit = 1;
				break;
			}
			i = p_cache[i].next;
		}while(i != p_lru);
	}

	if(hit == 1)
	{
		// refresh lru queue
		curr = i;
		prev = i - 1;
		p_cache[prev].next = p_cache[curr].next;
		p_cache[p_cache[curr].next].prev = prev;

		if(curr == p_lru)
			p_lru = p_cache[p_lru].next;
		tail = p_cache[p_lru].prev;

		p_cache[curr].next = p_lru;
		p_cache[curr].prev = tail;
		p_cache[p_lru].prev = curr;
		p_cache[tail].next = curr;

		// write dentry
		memcpy(res, p_array, PAGE_SIZE);
	}
	else
	{
		if(p_used < P_CACHE_NUM)
		{
			bios_sd_read(kva2pa((uintptr_t)block_buffer), 8, 
					superblock.start_sector + (idx << 3));
			for(k = 0; k < D_CACHE_NUM; k++)
				if(!(p_cache[k].ct & LINE_VALID))
					break;
			p_array = (uint8_t *)(D_CACHE_ADDR + (p_cache[k].cache_idx << BLOCK_SIZE_SHIFT));
			memcpy(p_array, block_buffer, PAGE_SIZE);
			memcpy(res, p_array, PAGE_SIZE);

			if(p_used == 0)
			{
				p_lru = k;
				p_cache[p_lru].prev = p_lru;
				p_cache[p_lru].next = p_lru;
			}

			tail = p_cache[p_lru].prev;
			p_cache[p_lru].prev = k;
			p_cache[tail].next = k;
			p_cache[k].next = p_lru;
			p_cache[k].prev = tail;

			p_cache[k].ct = LINE_VALID;
			p_cache[k].start_idx = idx;
			p_used++;
		}
		else
		{
			if(page_cache_policy == WRITE_THROUGH)
			{
				bios_sd_read(kva2pa((uintptr_t)block_buffer), 8, 
						superblock.start_sector + (idx << 3));
				p_array = (uint8_t *)(D_CACHE_ADDR + (p_cache[p_lru].cache_idx << BLOCK_SIZE_SHIFT));
				memcpy(p_array, block_buffer, PAGE_SIZE);
				memcpy(res, p_array, PAGE_SIZE);

				p_cache[p_lru].ct = LINE_VALID;
				p_cache[p_lru].start_idx = idx;

				p_lru = p_cache[p_lru].next;
			}
			else if(page_cache_policy == WRITE_BACK)
			{
				if(p_cache[p_lru].ct & LINE_DIRTY)
				{
					p_array = (uint8_t *)(D_CACHE_ADDR + (p_cache[p_lru].cache_idx << BLOCK_SIZE_SHIFT));
					bios_sd_write(kva2pa((uintptr_t)block_buffer), 8, 
						superblock.start_sector + (p_cache[p_lru].start_idx << 3));
				}
				bios_sd_read(kva2pa((uintptr_t)block_buffer), 8, 
						superblock.start_sector + (idx << 3));
				p_array = (uint8_t *)(D_CACHE_ADDR + (p_cache[p_lru].cache_idx << BLOCK_SIZE_SHIFT));
				memcpy(p_array, block_buffer, PAGE_SIZE);
				memcpy(res, p_array, PAGE_SIZE);

				p_cache[p_lru].ct = LINE_VALID;
				p_cache[p_lru].start_idx = idx;

				p_lru = p_cache[p_lru].next;
			}
		}
	}
}

void write_page(block_idx_t idx, uint8_t *data)
{
	int i = p_lru;
	int k;
	int prev, curr, tail;
	int hit = 0;
	uint8_t * p_array;
	if(p_lru != -1)
	{
		do
		{
			if((p_cache[i].ct & (LINE_VALID)) && (p_cache[i].start_idx == idx))
			{
				p_array = (uint8_t *)(D_CACHE_ADDR + (p_cache[i].cache_idx << BLOCK_SIZE_SHIFT));
				hit = 1;
				break;
			}
			i = p_cache[i].next;
		}while(i != p_lru);
	}

	if(hit == 1)
	{
		// refresh lru queue
		curr = i;
		prev = i - 1;
		p_cache[prev].next = p_cache[curr].next;
		p_cache[p_cache[curr].next].prev = prev;

		if(curr == p_lru)
			p_lru = p_cache[p_lru].next;
		tail = p_cache[p_lru].prev;

		p_cache[curr].next = p_lru;
		p_cache[curr].prev = tail;
		p_cache[p_lru].prev = curr;
		p_cache[tail].next = curr;

		// write dentry
		memcpy(p_array, data, BLOCK_SIZE);

		if(page_cache_policy == WRITE_BACK)
			p_cache[curr].ct |= LINE_DIRTY;
		else if(page_cache_policy == WRITE_THROUGH)
		{
			bios_sd_write(kva2pa((uintptr_t)p_array), 8, superblock.start_sector + (idx << 3));
		}
	}
	else
	{
		if(p_used < P_CACHE_NUM)
		{
			bios_sd_read(kva2pa((uintptr_t)block_buffer), 8, 
					superblock.start_sector + (idx << 3));
			for(k = 0; k < D_CACHE_NUM; k++)
				if(!(p_cache[k].ct & LINE_VALID))
					break;
			p_array = (uint8_t *)(D_CACHE_ADDR + (p_cache[k].cache_idx << BLOCK_SIZE_SHIFT));
			memcpy(p_array, data, PAGE_SIZE);

			if(p_used == 0)
			{
				p_lru = k;
				p_cache[p_lru].prev = p_lru;
				p_cache[p_lru].next = p_lru;
			}

			tail = p_cache[p_lru].prev;
			p_cache[p_lru].prev = k;
			p_cache[tail].next = k;
			p_cache[k].next = p_lru;
			p_cache[k].prev = tail;

			p_cache[k].ct = LINE_VALID;
			p_cache[k].start_idx = idx;

			if(page_cache_policy == WRITE_BACK)
				p_cache[k].ct |= LINE_DIRTY;
			else if(page_cache_policy == WRITE_THROUGH)
			{
				bios_sd_write(kva2pa((uintptr_t)p_array), 8, superblock.start_sector + (idx << 3));
			}

			p_used++;
		}
				else
		{
			if(page_cache_policy == WRITE_THROUGH)
			{
				p_array = (uint8_t *)(D_CACHE_ADDR + (p_cache[p_lru].cache_idx << BLOCK_SIZE_SHIFT));
				memcpy(p_array, data, PAGE_SIZE);
				bios_sd_write(kva2pa((uintptr_t)p_array), 8, superblock.start_sector + (idx << 3));

				p_cache[p_lru].ct = LINE_VALID;
				p_cache[p_lru].start_idx = idx;

				p_lru = p_cache[p_lru].next;
			}
			else if(page_cache_policy == WRITE_BACK)
			{
				p_array = (uint8_t *)(D_CACHE_ADDR + (p_cache[p_lru].cache_idx << BLOCK_SIZE_SHIFT));

				if(p_cache[p_lru].ct & LINE_DIRTY)
				{
					bios_sd_write(kva2pa((uintptr_t)p_array), 8, 
						superblock.start_sector + (p_cache[p_lru].start_idx << 3));
				}
				
				memcpy(p_array, data, PAGE_SIZE);

				p_cache[p_lru].ct = LINE_VALID | LINE_DIRTY;
				p_cache[p_lru].start_idx = idx;

				p_lru = p_cache[p_lru].next;
			}
		}
	}
}

void update_cache()
{
	uint8_t * p_array;
	for(int i = 0; i < P_CACHE_NUM; i++)
	{
		if(p_cache[i].ct & (LINE_DIRTY | LINE_VALID))
		{
			p_array = (uint8_t *)(D_CACHE_ADDR + (p_cache[i].cache_idx << BLOCK_SIZE_SHIFT));
			bios_sd_write(kva2pa((uintptr_t)p_array), 8, 
				superblock.start_sector + (p_cache[i].start_idx << 3));
			p_cache[i].ct = LINE_VALID;
		}
	}
}

void refresh_cache()
{
	update_cache();
	init_cache();
}

void change_policy(int policy, int time)
{
	page_cache_policy = policy;
	write_back_freq = time;
	if(policy == WRITE_THROUGH)
		update_cache();
}