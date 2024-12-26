#include <os/fs_cache.h>
#include <os/proc.h>
#include <os/kernel.h>
#include <assert.h>
#include <os/string.h>
#include <os/fs.h>
#include <os/swap.h>
#include <printk.h>
#include <os/string.h>

static fdesc_t fdesc_array[NUM_FDESCS];

static uint8_t block_buffer[BLOCK_SIZE];
superblock_t superblock;

unsigned long superblock_id;

void init_fs(void)
{
	printk("Initializing File System...\n");
	superblock_id = swap_id_start + (SWAP_SPACE >> 6);
}

static int fill_block(int start, int length, uint8_t * buffer)
{
	int byte_idx, byte_offset;

	if(start + length > (BLOCK_SIZE << 3) || start < 0 || length < 0)
		return -1;
	byte_idx = start >> 3;
	byte_offset = start & 0x7;
	for(int i = 0; i < length; i++)
	{
		buffer[byte_idx] |= (1 << byte_offset);
		byte_offset = (byte_offset + 1) % sizeof(uint8_t);
		byte_idx += (byte_offset == 0);
	}
	return 0;
}

static int clear_block(int start, int length, uint8_t * buffer)
{
	int byte_idx, byte_offset;

	if(start + length > (BLOCK_SIZE << 3) || start < 0 || length < 0)
		return -1;
	byte_idx = start >> 3;
	byte_offset = start & 0x7;
	for(int i = 0; i < length; i++)
	{
		buffer[byte_idx] &= ~((uint8_t)(1 << byte_offset));
		byte_offset = (byte_offset + 1) % sizeof(uint8_t);
		byte_idx += (byte_offset == 0);
	}
	return 0;
}

static int check_block(int start, int length, uint8_t * buffer)
{
	int byte_idx, byte_offset;
	int res = 0;

	if(start + length > (BLOCK_SIZE << 3) || start < 0 || length < 0)
		return -1;
	byte_idx = start >> 3;
	byte_offset = start & 0x7;
	for(int i = 0; i < length; i++)
	{
		if(buffer[byte_idx] & (1 << byte_offset))
			res = i + 1;
		byte_offset = (byte_offset + 1) % sizeof(uint8_t);
		byte_idx += (byte_offset == 0);
	}
	return res;
}

static int allocate_block(int num, uint64_t * bit_idx)
{
	static uint64_t bmap_ptr = 0;
	static uint64_t bmap_ptr_byte = 0;
	uint64_t total = 0;
	int res;
	while(1)
	{
		bios_sd_read(kva2pa((uintptr_t)block_buffer), 8, 
			superblock.start_sector + ((superblock.offset_bmap + (bmap_ptr_byte >> BLOCK_SIZE_SHIFT)) << 3));
		while((res = check_block(bmap_ptr, num, block_buffer)) != 0)
		{
			bmap_ptr += res;
			bmap_ptr_byte = bmap_ptr >> 3;
			total += res;
		}
		if(bmap_ptr_byte > (superblock.bmap_size << BLOCK_SIZE_SHIFT))
			bmap_ptr = 0;
		if(res == 0)
			break;
		if(((total >> 3) >> BLOCK_SIZE_SHIFT) > superblock.bmap_size)
			assert(0);
	}
	if(fill_block(bmap_ptr, num, block_buffer))
		assert(0);
	bios_sd_write(kva2pa((uintptr_t)block_buffer), 8, 
		superblock.start_sector + ((superblock.offset_bmap + (bmap_ptr_byte >> BLOCK_SIZE_SHIFT)) << 3));
	*bit_idx = bmap_ptr;
	return 0;
}

static int allocate_inode(int num, uint64_t * bit_idx)
{
	static uint64_t imap_ptr = 0;
	static uint64_t imap_ptr_byte = 0;
	uint64_t total = 0;
	int res;
	while(1)
	{
		bios_sd_read(kva2pa((uintptr_t)block_buffer), 8, 
			superblock.start_sector + ((superblock.offset_imap + (imap_ptr_byte >> BLOCK_SIZE_SHIFT)) << 3));
		while((res = check_block(imap_ptr, num, block_buffer)) != 0)
		{
			imap_ptr += res;
			imap_ptr_byte = imap_ptr >> 3;
			total += res;
		}
		if(imap_ptr_byte > (superblock.imap_size << BLOCK_SIZE_SHIFT))
			imap_ptr = 0;
		if(res == 0)
			break;
		if(((total >> 3) >> BLOCK_SIZE_SHIFT) > superblock.imap_size)
			assert(0);
	}
	if(fill_block(imap_ptr, num, block_buffer))
		assert(0);
	bios_sd_write(kva2pa((uintptr_t)block_buffer), 8, 
		superblock.start_sector + ((superblock.offset_imap + (imap_ptr_byte >> BLOCK_SIZE_SHIFT)) << 3));
	*bit_idx = imap_ptr;
	return 0;
}

static int free_block(uint32_t bit_idx)
{
	int block_idx;
	block_idx = (bit_idx >> 3) >> BLOCK_SIZE_SHIFT;
	bios_sd_read(kva2pa((uintptr_t)block_buffer), 8, 
		superblock.start_sector + ((superblock.offset_bmap + block_idx) << 3));
	bit_idx -= ((block_idx << BLOCK_SIZE_SHIFT) << 3);
	clear_block(bit_idx, 1, block_buffer);
	bios_sd_write(kva2pa((uintptr_t)block_buffer), 8, 
		superblock.start_sector + ((superblock.offset_bmap + block_idx) << 3));
	return 0;
}

static int free_inode(uint32_t bit_idx)
{
	int block_idx;
	block_idx = (bit_idx >> 3) >> BLOCK_SIZE_SHIFT;
	bios_sd_read(kva2pa((uintptr_t)block_buffer), 8, 
		superblock.start_sector + ((superblock.offset_imap + block_idx) << 3));
	bit_idx -= ((block_idx << BLOCK_SIZE_SHIFT) << 3);
	clear_block(bit_idx, 1, block_buffer);
	bios_sd_write(kva2pa((uintptr_t)block_buffer), 8, 
		superblock.start_sector + ((superblock.offset_imap + block_idx) << 3));
	return 0;
}


block_idx_t find_block(inode_t * inode, int idx, uint8_t * block_buffer)
{
	/* TODO */
	block_idx_t first_level,second_level,third_level;

	if(idx > ((inode->size - 1) >> BLOCK_SIZE_SHIFT))
		return -1;

	if(idx < MAX_DIRECT_BLOCK)
		return inode->direct[idx];
	else if(idx < MAX_INDIRECT_BLOCK + MAX_DIRECT_BLOCK)
	{
		get_page(inode->indirect, block_buffer);
		idx -= MAX_DIRECT_BLOCK;
		return ((block_idx_t *)block_buffer)[idx];
	}
	else if(idx < MAX_DIRECT_BLOCK + MAX_INDIRECT_BLOCK + MAX_DOUBLE_INDIRECT_BLOCK)
	{
		get_page(inode->double_indirect, block_buffer);
		idx -= (MAX_DIRECT_BLOCK + MAX_INDIRECT_BLOCK);
		first_level = idx / MAX_INDIRECT_BLOCK;
		second_level = idx % MAX_INDIRECT_BLOCK;
		get_page(((block_idx_t *)block_buffer)[first_level], block_buffer);
		return ((block_idx_t *)block_buffer)[second_level];
	}
	else
	{
		get_page(inode->triple_indirect, block_buffer);
		idx -= (MAX_DIRECT_BLOCK + MAX_INDIRECT_BLOCK + MAX_DOUBLE_INDIRECT_BLOCK);
		first_level = idx / MAX_DOUBLE_INDIRECT_BLOCK;
		second_level = (idx % MAX_DOUBLE_INDIRECT_BLOCK) / MAX_INDIRECT_BLOCK;
		third_level = idx % MAX_INDIRECT_BLOCK;
		get_page(((block_idx_t *)block_buffer)[first_level], block_buffer);
		get_page(((block_idx_t *)block_buffer)[second_level], block_buffer);
		return ((block_idx_t *)block_buffer)[third_level];
	}

}

block_idx_t extend_file(inode_idx_t i_idx, uint8_t * block_buffer)
{
	int first_level, second_level, third_level;
	block_idx_t first_level_idx,second_level_idx;
	block_idx_t next_block;
	block_idx_t bidx;
	inode_t inode;

	get_page(superblock.offset_iarray + ((i_idx * sizeof(inode_t)) >> BLOCK_SIZE_SHIFT), block_buffer);
	inode = ((inode_t*)block_buffer)[i_idx & 0x1f];

	if(inode.size != 0)
		next_block = ((inode.size - 1) >> BLOCK_SIZE_SHIFT) + 1;
	else
		next_block = 0;

	if(next_block < MAX_DIRECT_BLOCK)
	{
		allocate_block(1, (uint64_t *)&bidx);
		return bidx;
	}
	else if(next_block < MAX_INDIRECT_BLOCK + MAX_DIRECT_BLOCK)
	{
		first_level = next_block - MAX_DIRECT_BLOCK;

		if(first_level == 0)
		{
			allocate_block(1, (uint64_t *)&bidx);
			inode.indirect = bidx;
			get_page(superblock.offset_iarray + ((i_idx * sizeof(inode_t)) >> BLOCK_SIZE_SHIFT), block_buffer);
			((inode_t*)block_buffer)[i_idx & 0x1f] = inode;
			write_page(superblock.offset_iarray + ((i_idx * sizeof(inode_t)) >> BLOCK_SIZE_SHIFT), block_buffer);
		}

		get_page(inode.indirect, block_buffer);

		allocate_block(1, (uint64_t *)&bidx);
		((block_idx_t *)block_buffer)[first_level] = bidx;

		write_page(inode.indirect, block_buffer);

		return bidx;
	}
	else if(next_block < MAX_DIRECT_BLOCK + MAX_INDIRECT_BLOCK + MAX_DOUBLE_INDIRECT_BLOCK)
	{
		next_block -= (MAX_DIRECT_BLOCK + MAX_INDIRECT_BLOCK);
		first_level = next_block / MAX_INDIRECT_BLOCK;
		second_level = next_block % MAX_INDIRECT_BLOCK;

		if(second_level == 0)
		{
			if(first_level == 0)
			{
				allocate_block(1, (uint64_t *)&bidx);
				inode.double_indirect = bidx;
				get_page(superblock.offset_iarray + ((i_idx * sizeof(inode_t)) >> BLOCK_SIZE_SHIFT), block_buffer);
				((inode_t*)block_buffer)[i_idx & 0x1f] = inode;
				write_page(superblock.offset_iarray + ((i_idx * sizeof(inode_t)) >> BLOCK_SIZE_SHIFT), block_buffer);
			}
			get_page(inode.double_indirect, block_buffer);
			allocate_block(1, (uint64_t *)&bidx);
			((block_idx_t *)block_buffer)[second_level] = bidx;
			write_page(inode.double_indirect, block_buffer);
		}


		get_page(inode.double_indirect, block_buffer);
		
		first_level_idx = ((block_idx_t *)block_buffer)[first_level];
		get_page(first_level_idx, block_buffer);

		allocate_block(1, (uint64_t *)&bidx);
		((block_idx_t *)block_buffer)[second_level] = bidx;

		write_page(first_level_idx, block_buffer);

		return bidx;
	}
	else
	{
		next_block -= (MAX_DIRECT_BLOCK + MAX_INDIRECT_BLOCK + MAX_DOUBLE_INDIRECT_BLOCK);
		first_level = next_block / MAX_DOUBLE_INDIRECT_BLOCK;
		second_level = (next_block % MAX_DOUBLE_INDIRECT_BLOCK) / MAX_INDIRECT_BLOCK;
		third_level = next_block % MAX_INDIRECT_BLOCK;

		if(third_level == 0)
		{
			if(second_level == 0)
			{
				if(first_level == 0)
				{
					allocate_block(1, (uint64_t *)&bidx);
					inode.triple_indirect = bidx;
					get_page(superblock.offset_iarray + ((i_idx * sizeof(inode_t)) >> BLOCK_SIZE_SHIFT), block_buffer);
					((inode_t*)block_buffer)[i_idx & 0x1f] = inode;
					write_page(superblock.offset_iarray + ((i_idx * sizeof(inode_t)) >> BLOCK_SIZE_SHIFT), block_buffer);
				}
				get_page(inode.triple_indirect, block_buffer);
				allocate_block(1, (uint64_t *)&bidx);
				((block_idx_t *)block_buffer)[first_level] = bidx;
				write_page(inode.triple_indirect, block_buffer);
			}

			get_page(inode.triple_indirect, block_buffer);
			first_level_idx = ((block_idx_t *)block_buffer)[first_level];

			get_page(first_level_idx, block_buffer);

			allocate_block(1, (uint64_t *)&bidx);
			((block_idx_t *)block_buffer)[second_level] = bidx;

			write_page(first_level_idx, block_buffer);
		}



		get_page(inode.triple_indirect, block_buffer);
		first_level_idx = ((block_idx_t *)block_buffer)[first_level];
		get_page(first_level_idx, block_buffer);
		second_level_idx = ((block_idx_t *)block_buffer)[second_level];
		get_page(second_level_idx, block_buffer);

		allocate_block(1, (uint64_t *)&bidx);
		((block_idx_t *)block_buffer)[third_level] = bidx;

		write_page(second_level_idx, block_buffer);
		
		return bidx;
	}
}

int get_dentry(inode_idx_t inode_idx, char * name, dentry_t * dentry, ftype_t type)
{
	int dentry_num;
	int block_num;
	block_idx_t block_idx;
	dentry_t * dentry_array;
	inode_t inode;
	get_page(superblock.offset_iarray + ((inode_idx * sizeof(inode_t)) >> BLOCK_SIZE_SHIFT), block_buffer);
	inode = ((inode_t *)block_buffer)[inode_idx & 0x1f];

	dentry_num = inode.dentry_num;
	block_num = inode.size / BLOCK_SIZE;

	for(int i = 0; i < block_num; i++)
	{
		block_idx = find_block(&inode, i, block_buffer);
		get_page(block_idx, block_buffer);
		dentry_array = (dentry_t *)block_buffer;

		for(int j = 0; j < DENTRYS_ONE_PAGE; j++)
		{
			if(dentry_array[j].valid)
			{
				if(strcmp(name, dentry_array[j].name) == 0 && dentry_array[j].type == type)
				{
					*dentry = dentry_array[j];
					return 0;
				}
				
				dentry_num--;
			}
			if(dentry_num == 0)
				return -1;
		}
	}
	return -1;
}

int delete_dentry(inode_idx_t inode_idx, char * name, ftype_t type)
{
	int dentry_num;
	int block_num;
	block_idx_t block_idx;
	dentry_t * dentry_array;
	inode_t inode;
	get_page(superblock.offset_iarray + ((inode_idx * sizeof(inode_t)) >> BLOCK_SIZE_SHIFT), block_buffer);
	inode = ((inode_t *)block_buffer)[inode_idx & 0x1f];

	dentry_num = inode.dentry_num;
	block_num = inode.size / BLOCK_SIZE;

	for(int i = 0; i < block_num; i++)
	{
		block_idx = find_block(&inode, i, block_buffer);
		get_page(block_idx, block_buffer);
		dentry_array = (dentry_t *)block_buffer;

		for(int j = 0; j < DENTRYS_ONE_PAGE; j++)
		{
			if(dentry_array[j].valid)
			{
				if(strcmp(name, dentry_array[j].name) == 0 && dentry_array[j].type == type)
				{
					if(dentry_num == 1 && i == block_num - 1)
					{
						free_block(block_idx);
						inode.size -= BLOCK_SIZE;
					}

					inode.dentry_num--;
					get_page(superblock.offset_iarray + ((inode_idx * sizeof(inode_t)) >> BLOCK_SIZE_SHIFT), block_buffer);
					((inode_t *)block_buffer)[inode_idx & 0x1f] = inode;
					write_page(superblock.offset_iarray + ((inode_idx * sizeof(inode_t)) >> BLOCK_SIZE_SHIFT), block_buffer);
					return 0;
				}
				
				dentry_num--;
			}
			if(dentry_num == 0)
				return -1;
		}
	}
	return -1;
}

int create_dentry(inode_idx_t inode_idx, dentry_t * dentry)
{
	int dentry_num;
	int block_num;
	block_idx_t block_idx;
	dentry_t * dentry_array;
	inode_t inode;
	get_page(superblock.offset_iarray + ((inode_idx * sizeof(inode_t)) >> BLOCK_SIZE_SHIFT), block_buffer);
	inode = ((inode_t *)block_buffer)[inode_idx & 0x1f];

	dentry_num = inode.dentry_num;
	block_num = inode.size / BLOCK_SIZE;

	for(int i = 0; i < block_num; i++)
	{
		block_idx = find_block(&inode, i, block_buffer);
		get_page(block_idx, block_buffer);
		dentry_array = (dentry_t *)block_buffer;

		for(int j = 0; j < DENTRYS_ONE_PAGE; j++)
		{
			if(dentry_array[j].valid == 0)
			{
				dentry_array[j] = *dentry;
				inode.dentry_num++;

				write_page(block_idx, block_buffer);

				get_page(superblock.offset_iarray + ((inode_idx * sizeof(inode_t)) >> BLOCK_SIZE_SHIFT), block_buffer);
				((inode_t *)block_buffer)[inode_idx & 0x1f] = inode;
				write_page(superblock.offset_iarray + ((inode_idx * sizeof(inode_t)) >> BLOCK_SIZE_SHIFT), block_buffer);
					
				return 0;
			}
			else
				dentry_num--;

			if(dentry_num == 0)
				break;
		}
		if(dentry_num == 0)
			break;
	}

	dentry_num++;
	if(dentry_num / DENTRYS_ONE_PAGE + 1 > block_num)
	{
		block_num = dentry_num / DENTRYS_ONE_PAGE + 1;
		block_idx = extend_file(inode_idx, block_buffer);

		get_page(block_idx, block_buffer);

		bzero(block_buffer, BLOCK_SIZE);
		
		dentry_array = (dentry_t *)block_buffer;

		dentry_array[0] = *dentry;

		write_page(block_idx, block_buffer);

		inode.size += BLOCK_SIZE;
	}
	else
	{
		block_idx = find_block(&inode, block_num - 1, block_buffer);

		get_page(block_idx, block_buffer);

		dentry_array = (dentry_t *)block_buffer;

		dentry_array[dentry_num & (DENTRYS_ONE_PAGE - 1)] = *dentry;

		write_page(block_idx, block_buffer);
	}

	inode.dentry_num = dentry_num;
	
	get_page(superblock.offset_iarray + ((inode_idx * sizeof(inode_t)) >> BLOCK_SIZE_SHIFT), block_buffer);
	((inode_t *)block_buffer)[inode_idx & 0x1f] = inode;
	write_page(superblock.offset_iarray + ((inode_idx * sizeof(inode_t)) >> BLOCK_SIZE_SHIFT), block_buffer);

	return 0;
}

int create_dir(uint64_t inode_idx, uint64_t pinode_idx)
{
	inode_t inode;
	dentry_t dentry;

	get_page(superblock.offset_iarray + ((inode_idx * sizeof(inode_t)) >> BLOCK_SIZE_SHIFT), block_buffer);

	inode.size = 0;
	inode.hard_link_num = 1;
	inode.dentry_num = 0;
	inode.type = TYPE_DIR;
	inode.mode.user = FILE_READ | FILE_WRITE | FILE_EXEC;
	inode.mode.group = FILE_READ | FILE_WRITE | FILE_EXEC;
	inode.mode.others = FILE_READ | FILE_WRITE | FILE_EXEC;

	((inode_t *)block_buffer)[inode_idx & 0x1f] = inode;

	write_page(superblock.offset_iarray + ((inode_idx * sizeof(inode_t)) >> BLOCK_SIZE_SHIFT), block_buffer);

	/* add dentry: "." and ".." */
	dentry.inode = inode_idx;
	strcpy(dentry.name, ".");
	dentry.valid = 1;
	dentry.type = TYPE_DIR;
	create_dentry(inode_idx, &dentry);

	dentry.inode = pinode_idx;
	strcpy(dentry.name, "..");
	dentry.valid = 1;
	dentry.type = TYPE_DIR;
	create_dentry(inode_idx, &dentry);

	return 0;
}

int do_mkfs(void)
{
	// TODO [P6-task1]: Implement do_mkfs
	int i;
	uint64_t inode_idx;

	printk("[FS] Initializing the file system...\n");

	bios_sd_read((unsigned)block_buffer, 1, superblock_id);
	if(((superblock_t *)block_buffer)->magic_num == SUPERBLOCK_MAGIC)
	{
		printk("[FS] A file system already exists on this device\n");
		return -1;
	}
	// init superblock

	printk("[FS] Setting superblock...\n");

	superblock.magic_num = SUPERBLOCK_MAGIC;
	superblock.total_blocks = FS_SIZE >> BLOCK_SIZE_SHIFT;
	superblock.total_inode = MAX_INODE;
	superblock.block_size = BLOCK_SIZE;
	superblock.bmap_size = (superblock.total_blocks + BLOCK_SIZE) >> BLOCK_SIZE_SHIFT;
	superblock.imap_size = (MAX_INODE + BLOCK_SIZE) >> BLOCK_SIZE_SHIFT;
	superblock.iarray_size = (MAX_INODE * sizeof(inode_t) + BLOCK_SIZE) >> BLOCK_SIZE_SHIFT;
	superblock.data_size = superblock.total_blocks - superblock.bmap_size - superblock.imap_size - superblock.iarray_size;
	superblock.start_sector = superblock_id + 1;	// start of bmap
	superblock.offset_bmap = 0;
	superblock.offset_imap = superblock.offset_bmap + ((superblock.total_blocks + BLOCK_SIZE) >> BLOCK_SIZE_SHIFT);	// div 512
	superblock.offset_iarray = superblock.offset_imap + ((MAX_INODE + BLOCK_SIZE) >> BLOCK_SIZE_SHIFT);
	superblock.offset_data = superblock.offset_iarray + ((MAX_INODE * sizeof(inode_t) + BLOCK_SIZE) >> BLOCK_SIZE_SHIFT);
	*((superblock_t *)block_buffer) = superblock;

	printk("     magic number: 0x%lx\n",superblock.magic_num);
	printk("     block size: %ld, total block number: %ld\n",superblock.block_size,superblock.total_blocks);
	printk("     inode number: %ld\n",superblock.total_inode);
	printk("     start sector for fs: %ld\n",superblock.start_sector);
	printk("     inode map offset(block): %ld\n",superblock.offset_imap);
	printk("     inode array offset(block): %ld\n",superblock.offset_iarray);
	printk("     block map offset(block): %ld\n",superblock.offset_bmap);

	bios_sd_write(kva2pa((uintptr_t)block_buffer), 1, superblock_id);


	printk("[FS] Resetting block map, inode map and inode_array...\n");
	// reset bmap, imap, inode_array
	for(i = 0; i < SECTOR_SIZE; i++)
	{
		block_buffer[i] = 0;
	}
	for(i = 0; i < superblock.bmap_size; i++)
	{
		bios_sd_write(kva2pa((uintptr_t)block_buffer), 8, 
			superblock.start_sector + ((superblock.offset_bmap + i) << 3));
	}
	for(i = 0; i < superblock.imap_size; i++)
	{
		bios_sd_write(kva2pa((uintptr_t)block_buffer), 8, 
			superblock.start_sector + ((superblock.offset_imap + i) << 3));
	}
	for(i = 0; i < superblock.iarray_size; i++)
	{
		bios_sd_write(kva2pa((uintptr_t)block_buffer), 8, 
			superblock.start_sector + ((superblock.offset_iarray + i) << 3));
	}

	
	// init bmap
	printk("[FS] Initializing block map...\n");
	allocate_block(superblock.offset_data, NULL);

	// create root dir
	printk("[FS] Creating root directory...\n");
	allocate_inode(1, &inode_idx);
	create_dir(inode_idx, inode_idx);

	printk("[FS] Complete file system initialization\n");

	return 0;  // do_mkfs succeeds
}

int do_statfs(void)
{
	// TODO [P6-task1]: Implement do_statfs

	return 0;  // do_statfs succeeds
}


// 相对路径
int do_cd(char *path)
{
	// TODO [P6-task1]: Implement do_cd
	int i, j;
	dentry_t dentry;
	char buff[WORKING_PATH];
	inode_idx_t inode_idx;
	inode_idx = pcb[current_running->pid - 1].proc_dir_inode;
	i = 0;

	while((j = strchr(path + i, '/')) != 0)
	{
		strncpy(buff, path + i, j);
		buff[j] = '\0';

		if(get_dentry(inode_idx, buff, &dentry, TYPE_DIR) == -1)
		{
			printk("ERROR: can not find \"%s\"\n",buff);
			return -1;
		}

		inode_idx = dentry.inode;
		
		if((path+i)[j] != '\0')
			i += (j + 1);
		else
			i += j;
	}

	pcb[current_running->pid - 1].proc_dir_inode = inode_idx;
	if(path[0] == '.' && path[1] != '\0')
	{
		strcat(pcb[current_running->pid - 1].path, path + 2);
		if(path[i - 1] != '/')
			strcat(pcb[current_running->pid - 1].path, "/");
	}
	else if(path[0] != '.')
	{
		strcat(pcb[current_running->pid - 1].path, path);
		if(path[i - 1] != '/')
			strcat(pcb[current_running->pid - 1].path, "/");
	}
	else
		assert(0);
	
	return 0;  // do_cd succeeds
}

// 不能支持一次创建多级目录
int do_mkdir(char *path)
{
	// TODO [P6-task1]: Implement do_mkdir
	dentry_t dentry;
	inode_idx_t inode_idx;
	inode_idx_t cinode_idx;
	inode_idx = pcb[current_running->pid - 1].proc_dir_inode;

	if(get_dentry(inode_idx, path, &dentry, TYPE_DIR) != -1)
	{
		printk("ERROR: directory named \"%s\" already existed\n",path);
		return -1;
	}

	allocate_inode(1, (uint64_t *)&cinode_idx);

	strcpy(dentry.name, path);
	dentry.type = TYPE_DIR;
	dentry.valid = 1;
	dentry.inode = cinode_idx;

	create_dir(cinode_idx, inode_idx);

	create_dentry(inode_idx, &dentry);	// TODO

	return 0;  // do_mkdir succeeds
}

int do_rmdir(char *path)
{
	// TODO [P6-task1]: Implement do_rmdir
	dentry_t dentry;
	inode_idx_t inode_idx;
	inode_idx_t cinode_idx;
	inode_t inode;
	inode_idx = pcb[current_running->pid - 1].proc_dir_inode;

	get_dentry(inode_idx, path, &dentry, TYPE_DIR);
	cinode_idx = dentry.inode;

	delete_dentry(inode_idx, path, TYPE_DIR);

	get_page(superblock.offset_iarray + ((cinode_idx * sizeof(inode_t)) >> BLOCK_SIZE_SHIFT), block_buffer);
	inode = ((inode_t *)block_buffer)[cinode_idx & 0x1f];

	inode.hard_link_num--;
	if(inode.hard_link_num == 0)
	{
		int block_num = (inode.size != 0) ? (((inode.size - 1) >> BLOCK_SIZE_SHIFT) + 1) : 0;
		for(int i = 0; i < block_num; i++)
		{
			free_block(find_block(&inode, i, block_buffer));
		}
		inode.size = 0;
		for(int i = 0; i < NUM_FDESCS; i++)
		{
			if(fdesc_array[i].inode == inode_idx)
				fdesc_array[i].proc_ref_cnt = 0;
		}
		free_inode(inode_idx);
	}
	

	return 0;  // do_rmdir succeeds
}

int do_ls(char *path, int option)
{
	// TODO [P6-task1]: Implement do_ls
	// Note: argument 'option' serves for 'ls -l' in A-core
	int i, j;
	int dentry_num;
	int block_num;
	block_idx_t bidx;
	inode_idx_t inode_idx = pcb[current_running->pid - 1].proc_dir_inode;
	inode_idx_t cinode_idx;
	inode_t inode,cinode;
	dentry_t * dentry_array;
	get_page(superblock.offset_iarray + ((inode_idx * sizeof(inode_t)) >> BLOCK_SIZE_SHIFT), block_buffer);
	inode = ((inode_t *)block_buffer)[inode_idx & 0x1f];

	dentry_num = inode.dentry_num;
	block_num = ((inode.size - 1) >> BLOCK_SIZE_SHIFT) + 1;

	for(i = 0; i < block_num; i++)
	{
		bidx = find_block(&inode, i, block_buffer);
		get_page(bidx, block_buffer);
		dentry_array = (dentry_t *)block_buffer;
		for(j = 0; j < DENTRYS_ONE_PAGE; j++)
		{
			if(dentry_array[j].valid)
			{
				// print information
				cinode_idx = dentry_array[j].index;
				get_page(superblock.offset_iarray + ((cinode_idx * sizeof(inode_t)) >> BLOCK_SIZE_SHIFT), block_buffer);
				cinode = ((inode_t *)block_buffer)[cinode_idx & 0x1f];

				printk("%s ",dentry_array[j].name);

				dentry_num--;
			}
			if(dentry_num == 0)
				break;
		}
		if(dentry_num == 0)
			break;
	}
	return 0;  // do_ls succeeds
}

int do_open(char *path, int mode)
{
	// TODO [P6-task2]: Implement do_open
	int i, j;
	dentry_t dentry;
	char buff[WORKING_PATH];
	inode_idx_t inode_idx;
	inode_idx = pcb[current_running->pid - 1].proc_dir_inode;
	i = 0;
	while((j = strchr(path + i, '/')) != 0)
	{
		strncpy(buff, path + i, j);
		buff[j] = '\0';

		if((path+i)[j] != '\0')
		{
			if(get_dentry(inode_idx, buff, &dentry, TYPE_DIR) == -1)
				return -1;
		}
		else
		{
			if(get_dentry(inode_idx, buff, &dentry, TYPE_FILE) == -1)
				return -1;
		}

		inode_idx = dentry.inode;
		
		if((path+i)[j] != '\0')
			i += (j + 1);
		else
			i += j;
	}

	int find_fdesc,find_fd;

	for(i = 0; i < NUM_FDESCS; i++)
	{
		if(fdesc_array[i].inode == inode_idx && fdesc_array[i].proc_ref_cnt != 0)
		{
			find_fdesc = 1;
			break;
		}
	}
	for(i = 0; i < NUM_FDESCS; i++)
	{
		if(fdesc_array[i].proc_ref_cnt == 0)
		{
			find_fdesc = 1;
			break;
		}
	}
	for(j = 0; j < NUM_PROC_FD; j++)
	{
		if(pcb[current_running->pid-1].fd_array[j].fdesc_idx == i)
		{
			printk("ERROR: in open\n");
			return -1;
		}
	}
	for(j = 0; j < NUM_PROC_FD; j++)
	{
		if(pcb[current_running->pid-1].fd_array[j].fdesc_idx == -1)
		{
			find_fd = 1;
			break;
		}
	}

	if(find_fdesc && find_fd)
	{
		fdesc_array[i].inode = inode_idx;
		fdesc_array[i].proc_ref_cnt++;
		pcb[current_running->pid-1].fd_array[j].fdesc_idx = j;
		pcb[current_running->pid-1].fd_array[j].fd_mode = mode;
		pcb[current_running->pid-1].fd_array[j].pos = 0;
	}

	return j;  // return the id of file descriptor
}


int do_read(int fd, char *buff, int length)
{
	// TODO [P6-task2]: Implement do_read
	int fdesc_idx = pcb[current_running->pid-1].fd_array[fd].fdesc_idx;
	uint64_t pos = pcb[current_running->pid-1].fd_array[fd].pos;
	inode_idx_t inode_idx;
	inode_t inode;
	int start_block, end_block;
	uint32_t read_start;
	uint32_t read_len;
	int res;
	block_idx_t bidx;
	if(pcb[current_running->pid-1].fd_array[fd].fd_mode == O_WRONLY)
	{
		printk("ERROR: cannot read this file\n");
		return -1;
	}

	inode_idx = fdesc_array[fdesc_idx].inode;
	get_page(superblock.offset_iarray + ((inode_idx * sizeof(inode_t)) >> BLOCK_SIZE_SHIFT), block_buffer);
	inode = ((inode_t *)block_buffer)[inode_idx & 0x1f];

	if(pos + length >= inode.size)
		length = inode.size - pos;

	start_block = pos >> BLOCK_SIZE_SHIFT;
	end_block = (pos + length) >> BLOCK_SIZE_SHIFT;

	res = pos + length;

	for(int i = start_block; i < end_block; i++)
	{
		bidx = find_block(&inode, i, block_buffer);
		get_page(bidx, block_buffer);
		
		read_start = (i == start_block) ? (pos & 0xfff) : 0;
		read_len = (length < BLOCK_SIZE - read_start) ? length : (BLOCK_SIZE - read_start);

		length -= read_len;

		memcpy((uint8_t *)buff, block_buffer + read_start, read_len);
		buff += read_len;
	}

	pcb[current_running->pid-1].fd_array[fd].pos = res;

	return res;  // return the length of trully read data
}

int do_write(int fd, char *buff, int length)
{
	// TODO [P6-task2]: Implement do_write
	int fdesc_idx = pcb[current_running->pid-1].fd_array[fd].fdesc_idx;
	uint64_t pos = pcb[current_running->pid-1].fd_array[fd].pos;
	inode_idx_t inode_idx;
	inode_t inode;
	int start_block, end_block;
	uint32_t write_start;
	uint32_t write_len;
	uint64_t new_size;
	block_idx_t bidx;
	if(pcb[current_running->pid-1].fd_array[fd].fd_mode == O_RDONLY)
	{
		printk("ERROR: cannot read this file\n");
		return -1;
	}

	inode_idx = fdesc_array[fdesc_idx].inode;
	get_page(superblock.offset_iarray + ((inode_idx * sizeof(inode_t)) >> BLOCK_SIZE_SHIFT), block_buffer);
	inode = ((inode_t *)block_buffer)[inode_idx & 0x1f];

	start_block = pos >> BLOCK_SIZE_SHIFT;
	end_block = (pos + length) >> BLOCK_SIZE_SHIFT;

	new_size = pos + length;

	for(int i = start_block; i < end_block; i++)
	{
		bidx = find_block(&inode, i, block_buffer);
		if(bidx == -1)
			bidx = extend_file(inode_idx, block_buffer);
		
		get_page(bidx, block_buffer);

		write_start = (i == start_block) ? (pos & 0xfff) : 0;
		write_len = (length < BLOCK_SIZE - write_start) ? length : (BLOCK_SIZE - write_start);

		length -= write_len;

		memcpy(block_buffer + write_start, (const uint8_t *)buff, write_len);
		buff += write_len;
	}

	get_page(superblock.offset_iarray + ((inode_idx * sizeof(inode_t)) >> BLOCK_SIZE_SHIFT), block_buffer);
	inode = ((inode_t *)block_buffer)[inode_idx & 0x1f];
	if(new_size > inode.size)
	{
		inode.size = new_size;
		write_page(superblock.offset_iarray + ((inode_idx * sizeof(inode_t)) >> BLOCK_SIZE_SHIFT), block_buffer);
	}
	pcb[current_running->pid-1].fd_array[fd].pos = new_size;

	return new_size - pos;  // return the length of trully written data
}

int do_close(int fd)
{
	// TODO [P6-task2]: Implement do_close
	int i;
	int fdesc_idx;
	for(i = 0; i < NUM_PROC_FD; i++)
	{
		if(pcb[current_running->pid-1].fd_array[i].fdesc_idx == -1)
			return -1;
	}
	fdesc_idx = pcb[current_running->pid-1].fd_array[fd].fdesc_idx;
	fdesc_array[fdesc_idx].proc_ref_cnt--;
	if(fdesc_array[fdesc_idx].proc_ref_cnt == 0)
		fdesc_array[fdesc_idx].inode = 0;
	return 0;  // do_close succeeds
}

int do_ln(char *src_path, char *dst_path)
{
	// TODO [P6-task2]: Implement do_ln
	/*
	int i, j;
	dentry_t dentry;
	char sbuff[WORKING_PATH];
	char dbuff[WORKING_PATH];
	inode_idx_t sinode_idx, dinode_idx;
	dinode_idx = sinode_idx = pcb[current_running->pid - 1].proc_dir_inode;
	i = 0;
	while((j = strchr(src_path + i, '/')) != 0)
	{
		strncpy(sbuff, src_path + i, j);
		sbuff[j] = '\0';

		if((src_path+i)[j] != '\0')
		{
			if(get_dentry(sinode_idx, sbuff, &dentry, TYPE_DIR) == -1)
				return -1;
		}
		else
		{
			if(get_dentry(sinode_idx, sbuff, &dentry, TYPE_FILE) == -1)
				return -1;
		}

		sinode_idx = dentry.inode;
		
		if((src_path+i)[j] != '\0')
			i += (j + 1);
		else
			i += j;
	}

	i = 0;
	while((j = strchr(dst_path + i, '/')) != 0)
	{
		strncpy(dbuff, dst_path + i, j);
		dbuff[j] = '\0';

		if((dst_path+i)[j] != '\0')
		{
			if(get_dentry(dinode_idx, dbuff, &dentry, TYPE_DIR) == -1)
				return -1;
		}
		else
		{
			if(get_dentry(dinode_idx, dbuff, &dentry, TYPE_FILE) == -1)
				return -1;
		}

		dinode_idx = dentry.inode;
		
		if((dst_path+i)[j] != '\0')
			i += (j + 1);
		else
			i += j;
	}


	if(get_dentry(inode_idx, buff, &dentry, TYPE_DIR) != -1)
	{
		printk("ERROR: directory named \"%s\" already existed\n",path);
		return -1;
	}

	allocate_inode(1, (uint64_t *)&cinode_idx);

	strcpy(dentry.name, path);
	dentry.type = TYPE_DIR;
	dentry.valid = 1;
	dentry.inode = cinode_idx;

	create_dir(cinode_idx, inode_idx);

	create_dentry(inode_idx, &dentry);	// TODO
	*/
	return 0;  // do_ln succeeds 
}

int do_rm(char *path)
{
	// TODO [P6-task2]: Implement do_rm
	inode_idx_t p_inode_idx = pcb[current_running->pid - 1].proc_dir_inode;
	inode_idx_t inode_idx;
	inode_t p_inode, inode;
	get_page(superblock.offset_iarray + ((p_inode_idx * sizeof(inode_t)) >> BLOCK_SIZE_SHIFT), block_buffer);
	p_inode = ((inode_t *)block_buffer)[p_inode_idx & 0x1f];

	for(int i = 0; i < MAX_DIRECT_BLOCK; i++)
	{
		get_page(p_inode.direct[i], block_buffer);
		for(int j = 0; j < DENTRYS_ONE_PAGE; j++)
		{
			if(((dentry_t *)block_buffer)[j].valid &&
			   strcmp(((dentry_t *)block_buffer)[j].name, path) == 0)
			{
				((dentry_t *)block_buffer)[j].valid = 0;
				inode_idx = ((dentry_t *)block_buffer)[j].inode;
			}
		}
	}

	// modify inode.dentry_num inode.size
	delete_dentry(p_inode_idx, path, TYPE_FILE);

	get_page(superblock.offset_iarray + ((inode_idx * sizeof(inode_t)) >> BLOCK_SIZE_SHIFT), block_buffer);
	inode = ((inode_t *)block_buffer)[inode_idx & 0x1f];

	inode.hard_link_num--;
	if(inode.hard_link_num == 0)
	{
		int block_num = (inode.size != 0) ? (((inode.size - 1) >> BLOCK_SIZE_SHIFT) + 1) : 0;
		for(int i = 0; i < block_num; i++)
		{
			free_block(find_block(&inode, i, block_buffer));
		}
		inode.size = 0;
		for(int i = 0; i < NUM_FDESCS; i++)
		{
			if(fdesc_array[i].inode == inode_idx)
				fdesc_array[i].proc_ref_cnt = 0;
		}
		free_inode(inode_idx);
	}

	return 0;  // do_rm succeeds 
}

int do_lseek(int fd, int offset, int whence)
{
	// TODO [P6-task2]: Implement do_lseek
	int pid = current_running->pid;
	int fdesc_idx = pcb[pid - 1].fd_array[fd].fdesc_idx;
	inode_idx_t inode_idx = fdesc_array[fdesc_idx].inode;
	inode_t inode;

	switch (whence) 
	{
		case SEEK_SET:
			pcb[pid - 1].fd_array[fd].pos = offset;
			break;
		case SEEK_CUR:
			pcb[pid - 1].fd_array[fd].pos += offset;
			break;
		case SEEK_END:
			get_page(superblock.offset_iarray + ((inode_idx * sizeof(inode_t)) >> BLOCK_SIZE_SHIFT), block_buffer);
			inode = ((inode_t *)block_buffer)[inode_idx & 0x1f];
			pcb[pid - 1].fd_array[fd].pos = inode.size + offset;
			break;
		default:
			break;
	}

	return pcb[pid - 1].fd_array[fd].pos;  // the resulting offset location from the beginning of the file
}

int do_touch(char *path)
{
		// TODO [P6-task1]: Implement do_mkdir
	dentry_t dentry;
	inode_idx_t inode_idx;
	inode_idx_t cinode_idx;
	inode_idx = pcb[current_running->pid - 1].proc_dir_inode;
	inode_t inode;

	if(get_dentry(inode_idx, path, &dentry, TYPE_FILE) != -1)
	{
		printk("ERROR: file named \"%s\" already existed\n",path);
		return -1;
	}

	allocate_inode(1, (uint64_t *)&cinode_idx);

	strcpy(dentry.name, path);
	dentry.type = TYPE_FILE;
	dentry.valid = 1;
	dentry.inode = cinode_idx;

	get_page(superblock.offset_iarray + ((cinode_idx * sizeof(inode_t)) >> BLOCK_SIZE_SHIFT), block_buffer);

	inode.size = 0;
	inode.hard_link_num = 1;
	inode.dentry_num = 0;
	inode.type = TYPE_FILE;
	inode.mode.user = FILE_READ | FILE_WRITE | FILE_EXEC;
	inode.mode.group = FILE_READ | FILE_WRITE | FILE_EXEC;
	inode.mode.others = FILE_READ | FILE_WRITE | FILE_EXEC;

	((inode_t *)block_buffer)[inode_idx & 0x1f] = inode;

	write_page(superblock.offset_iarray + ((cinode_idx * sizeof(inode_t)) >> BLOCK_SIZE_SHIFT), block_buffer);

	create_dentry(inode_idx, &dentry);	// TODO

	return 0;  // do_mkdir succeeds
}

int do_cat(char *path)
{
	inode_idx_t inode_idx = pcb[current_running->pid - 1].proc_dir_inode;
	inode_t inode;
	dentry_t dentry;
	int block_num;
	block_idx_t bidx;
	char print_buffer[BLOCK_SIZE + 1];
	print_buffer[BLOCK_SIZE] = '\0';

	get_page(superblock.offset_iarray + ((inode_idx * sizeof(inode_t)) >> BLOCK_SIZE_SHIFT), block_buffer);
	inode = ((inode_t *)block_buffer)[inode_idx & 0x1f];

	get_dentry(inode_idx, path, &dentry, TYPE_FILE);

	inode_idx = dentry.inode;
	get_page(superblock.offset_iarray + ((inode_idx * sizeof(inode_t)) >> BLOCK_SIZE_SHIFT), block_buffer);
	inode = ((inode_t *)block_buffer)[inode_idx & 0x1f];

	block_num = ((inode.size - 1) >> BLOCK_SIZE_SHIFT) + 1;

	printk("\n");
	for(int i = 0; i < block_num; i++)
	{
		bidx = find_block(&inode, i, (uint8_t *)print_buffer);
		get_page(bidx, (uint8_t *)print_buffer);
		printk("%s",print_buffer);
	}

	return 0;
}