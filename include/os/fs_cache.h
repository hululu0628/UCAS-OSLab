#ifndef INCLUDE_FS_CACHE_H
#define INCLUDE_FS_CACHE_H

#include <os/fs.h>
#include <os/mm.h>
#include <type.h>

#define CACHE_START_ADDR 0xffffffc051800000	// also in mm.h

#define PAGE_LINE_SIZE	1
#define DENTRY_LINE_SIZE DENTRYS_ONE_PAGE
#define IARRAY_LINE_SIZE INODE_ONE_PAGE

#define DENTRY_CACHE_SIZE	0x10000
#define IARRAY_CACHE_SIZE	0x10000
#define PAGE_CACHE_SIZE		0x100000

#define D_CACHE_ADDR	CACHE_START_ADDR
#define I_CACHE_ADDR	(D_CACHE_ADDR + DENTRY_CACHE_SIZE)
#define PAGE_CACHE_ADDR	(I_CACHE_SIZE + IARRAY_CACHE_SIZE)

#define D_CACHE_NUM	(DENTRY_CACHE_SIZE / sizeof(dentry_t))
#define I_CACHE_NUM	(IARRAY_CACHE_SIZE / PAGE_SIZE)
#define P_CACHE_NUM	(PAGE_CACHE_SIZE / (PAGE_SIZE * PAGE_LINE_SIZE))

#define LINE_DIRTY	0x1
#define LINE_VALID	0x2

#define WRITE_BACK	1
#define WRITE_THROUGH	2

typedef uint8_t cache_state_t;

extern void init_cache(void);

typedef struct d_cache_line
{
	int cache_idx;
	int prev;
	int next;
	inode_idx_t parent_inode;
	cache_state_t ct;
} d_cache_t;

extern d_cache_t d_cache[D_CACHE_NUM];
extern int d_lru;

extern int get_dentry(inode_idx_t pinode, char * name, dentry_t * res, ftype_t type);
extern void write_dentry(inode_idx_t pinode, char * name, dentry_t * data, ftype_t type);

/*
typedef struct i_cache_line
{
	int cache_idx;
	int prev;
	int next;
	inode_idx_t start_idx;
	cache_state_t ct;
}i_cache_t;

extern i_cache_t i_cache[I_CACHE_NUM];
extern int i_lru;

extern void get_inode(inode_idx_t idx, inode_t * res);
extern void write_inode(inode_idx_t idx, inode_t * data);
*/

typedef struct p_cache_line
{
	int cache_idx;
	int prev;
	int next;
	block_idx_t start_idx;
	cache_state_t ct;
}p_cache_t;

extern p_cache_t p_cache[P_CACHE_NUM];
extern int p_lru;

extern int page_cache_policy;
extern uint64_t write_back_freq;

extern void get_page(block_idx_t idx, uint8_t * res);
extern void write_page(block_idx_t idx, uint8_t * data);

extern void do_fsync(void);

#endif