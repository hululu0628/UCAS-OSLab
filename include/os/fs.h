#ifndef __INCLUDE_OS_FS_H__
#define __INCLUDE_OS_FS_H__

#include <type.h>
#include <os/task.h>

#define FS_SIZE 0x40000000	// 1GB
#define SECTOR_SIZE 512
#define BLOCK_SIZE 4096
#define BLOCK_SIZE_SHIFT 12

#define DENTRYS_ONE_PAGE (BLOCK_SIZE / sizeof(dentry_t))
#define INODE_ONE_PAGE (BLOCK_SIZE / sizeof(inode_t))

/* macros of file system */
#define SUPERBLOCK_MAGIC 0xDF4C4459
#define NUM_FDESCS 16
#define NUM_PROC_FD 16
#define MAX_DIRECT_BLOCK 12
#define MAX_INDIRECT_BLOCK (BLOCK_SIZE / sizeof(block_idx_t))
#define MAX_DOUBLE_INDIRECT_BLOCK (MAX_INDIRECT_BLOCK * MAX_INDIRECT_BLOCK)
#define MAX_TRIPLE_INDIRECT_BLOCK (MAX_INDIRECT_BLOCK * MAX_INDIRECT_BLOCK * MAX_INDIRECT_BLOCK)
#define MAX_INODE 1024

/* Modes of File*/
#define FILE_READ 	(1 << 0)
#define FILE_WRITE	(1 << 1)
#define FILE_EXEC 	(1 << 2)

#define TYPE_DIR 0
#define TYPE_FILE 1

typedef uint8_t privilege_t;
typedef uint8_t ftype_t;
typedef uint8_t fdmode_t;
typedef int block_idx_t;
typedef int inode_idx_t;

/* data structures of file system */
typedef struct superblock {
	// TODO [P6-task1]: Implement the data structure of superblock
	uint64_t magic_num;
	uint64_t total_blocks;
	uint64_t total_inode;
	uint64_t block_size;
	uint64_t bmap_size;
	uint64_t imap_size;
	uint64_t iarray_size;
	uint64_t data_size;
	uint64_t start_sector;		// 0: super block
	// uint64_t available_blocks;
	// uint64_t available_inode;
	uint64_t offset_bmap;		// block
	uint64_t offset_imap;
	uint64_t offset_iarray;
	uint64_t offset_data;
} superblock_t;

typedef struct dentry {
	// TODO [P6-task1]: Implement the data structure of directory entry
	char name[MAXFILENAME];
	uint32_t inode;
	uint32_t parent_inode;
	uint32_t index;
	uint8_t valid;
	ftype_t type;
}__attribute__((aligned(64))) dentry_t;

typedef struct inode { 
	// TODO [P6-task1]: Implement the data structure of inode
	struct {
		privilege_t user;
		privilege_t group;
		privilege_t others;
	} mode;
	ftype_t type;
	uint32_t dentry_num;
	uint32_t hard_link_num;
	uint64_t size;
	block_idx_t direct[MAX_DIRECT_BLOCK];
	block_idx_t indirect;
	block_idx_t double_indirect;
	block_idx_t triple_indirect;
}__attribute__((aligned(128))) inode_t;

typedef struct fdesc {
	// TODO [P6-task2]: Implement the data structure of file descriptor
	inode_idx_t inode;
	uint32_t proc_ref_cnt;
} fdesc_t;

typedef struct proc_fd
{
	uint64_t pos;
	int fdesc_idx;
	fdmode_t fd_mode;
} proc_fd_t;

/* modes of do_open */
#define O_RDONLY 1  /* read only open */
#define O_WRONLY 2  /* write only open */
#define O_RDWR   3  /* read/write open */

/* whence of do_lseek */
#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2

extern unsigned long superblock_id;
extern unsigned inode_root;
extern superblock_t superblock;

/* fs function declarations */
extern int create_dir(uint64_t inode_idx, uint64_t pinode_idx);
extern int read_block(block_idx_t idx, inode_idx_t inode_idx, uintptr_t block_buffer);
extern int do_mkfs(void);
extern int do_statfs(void);
extern int do_cd(char *path);
extern int do_mkdir(char *path);
extern int do_rmdir(char *path);
extern int do_ls(char *path, int option);
extern int do_open(char *path, int mode);
extern int do_read(int fd, char *buff, int length);
extern int do_write(int fd, char *buff, int length);
extern int do_close(int fd);
extern int do_ln(char *src_path, char *dst_path);
extern int do_rm(char *path);
extern int do_lseek(int fd, int offset, int whence);
extern int do_touch(char *path);
extern int do_cat(char *path);

#endif