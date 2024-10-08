#include <assert.h>
#include <elf.h>
#include <errno.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#define IMAGE_FILE "./image"
#define ARGS "[--extended] [--vm] <bootblock> <executable-file> ..."

#define SECTOR_SIZE 512
#define BOOT_LOADER_SIG_OFFSET 0x1fe
#define OS_SIZE_LOC (BOOT_LOADER_SIG_OFFSET - 2)
//a pointer to the information of tasks, which are stored in the end of image
#define TASK_INFO_OFFSET (OS_SIZE_LOC - 4)
#define BOOT_LOADER_SIG_1 0x55
#define BOOT_LOADER_SIG_2 0xaa

//define the maximum length of the file name(add in p1-t4)
#define MAXFILENAME 16

#define NBYTES2SEC(nbytes) (((nbytes) / SECTOR_SIZE) + ((nbytes) % SECTOR_SIZE != 0))

/* TODO: [p1-task4] design your own task_info_t */
// Bytes
typedef struct {
	uint32_t block_id;
	uint32_t block_num;
	uint8_t filename[MAXFILENAME];
} task_info_t;

#define TASK_MAXNUM 16
static task_info_t taskinfo[TASK_MAXNUM];

/* structure to store command line options */
static struct {
	int vm;
	int extended;
} options;

/* prototypes of local functions */
static void create_image(int nfiles, char *files[]);
static void error(char *fmt, ...);
static void read_ehdr(Elf64_Ehdr *ehdr, FILE *fp);
static void read_phdr(Elf64_Phdr *phdr, FILE *fp, int ph, Elf64_Ehdr ehdr);
static uint64_t get_entrypoint(Elf64_Ehdr ehdr);
static uint32_t get_filesz(Elf64_Phdr phdr);
static uint32_t get_memsz(Elf64_Phdr phdr);
static void write_segment(Elf64_Phdr phdr, FILE *fp, FILE *img, int *phyaddr);
static void write_padding(FILE *img, int *phyaddr, int new_phyaddr);
static void write_img_info(int nbytes_kernel, task_info_t *taskinfo,
                           short tasknum, FILE *img, int secnum);

int main(int argc, char **argv)
{
	char *progname = argv[0];

	/* process command line options */
	options.vm = 0;
	options.extended = 0;
	while ((argc > 1) && (argv[1][0] == '-') && (argv[1][1] == '-')) {
		char *option = &argv[1][2];

		if (strcmp(option, "vm") == 0) {
			options.vm = 1;
		} else if (strcmp(option, "extended") == 0) {
			options.extended = 1;
		} else {
		error("%s: invalid option\nusage: %s %s\n", progname,
			progname, ARGS);
		}
		argc--;
		argv++;
	}
	if (options.vm == 1) {
		error("%s: option --vm not implemented\n", progname);
	}
	if (argc < 3) {
		/* at least 3 args (createimage bootblock main) */
		error("usage: %s %s\n", progname, ARGS);
	}
	create_image(argc - 1, argv + 1);
	return 0;
}

/* TODO: [p1-task4] assign your task_info_t somewhere in 'create_image' */
static void create_image(int nfiles, char *files[])
{
	int tasknum = nfiles - 2;
	int nbytes_kernel = 0;
	int phyaddr = 0;
	int before_addr,after_addr;
	FILE *fp = NULL, *img = NULL;
	Elf64_Ehdr ehdr;
	Elf64_Phdr phdr;

	/* open the image file */
	img = fopen(IMAGE_FILE, "w");
	assert(img != NULL);

	/* for each input file */
	// tidx == -2: *files == "bootblock"
	// tidx == -1: *files == "main"
	for (int fidx = 0, tidx = -2; fidx < nfiles; ++fidx, ++tidx) {

		/* open input file */
		fp = fopen(*files, "r");
		assert(fp != NULL);

		/* read ELF header */
		read_ehdr(&ehdr, fp);
		printf("0x%04lx: %s\n", ehdr.e_entry, *files);

		// after_addr - before_addr == 某程序所占空间
		before_addr = phyaddr;

		/* for each program header */
		for (int ph = 0; ph < ehdr.e_phnum; ph++) {

			/* read program header */
			read_phdr(&phdr, fp, ph, ehdr);

			if (phdr.p_type != PT_LOAD) continue;

			/* write segment to the image */
			write_segment(phdr, fp, img, &phyaddr);

			/* update nbytes_kernel */
			if (strcmp(*files, "main") == 0) {
				nbytes_kernel += get_filesz(phdr);
			}
		}

		/* write padding bytes */
		/**
		* TODO:
		* 1. [p1-task3] do padding so that the kernel and every app program
		*  occupies the same number of sectors
		* 2. [p1-task4] only padding bootblock is allowed!
		*/
		if (strcmp(*files, "bootblock") == 0) {
			write_padding(img, &phyaddr, SECTOR_SIZE);
		}
		else
		{
			//in p1-task4, padding kernel and every app program 
			//make sure the entry for each program start from the beginning of a sector
			write_padding(img, &phyaddr, (phyaddr - (phyaddr % SECTOR_SIZE) + SECTOR_SIZE));

			after_addr = phyaddr;

			if(strcmp(*files,"main") != 0)
			{
				taskinfo[tidx].block_id = NBYTES2SEC(before_addr);
				taskinfo[tidx].block_num = NBYTES2SEC(after_addr) - NBYTES2SEC(before_addr);
				strcpy(taskinfo[tidx].filename, *files);
			}
		}

		fclose(fp);
		files++;
	}
	write_img_info(nbytes_kernel, taskinfo, tasknum, img, NBYTES2SEC(phyaddr));
	fclose(img);
}

static void read_ehdr(Elf64_Ehdr * ehdr, FILE * fp)
{
	int ret;

	ret = fread(ehdr, sizeof(*ehdr), 1, fp);
	assert(ret == 1);
	assert(ehdr->e_ident[EI_MAG1] == 'E');
	assert(ehdr->e_ident[EI_MAG2] == 'L');
	assert(ehdr->e_ident[EI_MAG3] == 'F');
}

static void read_phdr(Elf64_Phdr * phdr, FILE * fp, int ph,
                      Elf64_Ehdr ehdr)
{
	int ret;

	fseek(fp, ehdr.e_phoff + ph * ehdr.e_phentsize, SEEK_SET);
	ret = fread(phdr, sizeof(*phdr), 1, fp);
	assert(ret == 1);
	if (options.extended == 1) {
		printf("\tsegment %d\n", ph);
		printf("\t\toffset 0x%04lx", phdr->p_offset);
		printf("\t\tvaddr 0x%04lx\n", phdr->p_vaddr);
		printf("\t\tfilesz 0x%04lx", phdr->p_filesz);
		printf("\t\tmemsz 0x%04lx\n", phdr->p_memsz);
	}
}

static uint64_t get_entrypoint(Elf64_Ehdr ehdr)
{
    	return ehdr.e_entry;
}

static uint32_t get_filesz(Elf64_Phdr phdr)
{
    	return phdr.p_filesz;
}

static uint32_t get_memsz(Elf64_Phdr phdr)
{
    	return phdr.p_memsz;
}

static void write_segment(Elf64_Phdr phdr, FILE *fp, FILE *img, int *phyaddr)
{
	if (phdr.p_memsz != 0 && phdr.p_type == PT_LOAD) {
		/* write the segment itself */
		/* NOTE: expansion of .bss should be done by kernel or runtime env! */
		if (options.extended == 1) {
			printf("\t\twriting 0x%04lx bytes\n", phdr.p_filesz);
		}
		fseek(fp, phdr.p_offset, SEEK_SET);
		while (phdr.p_filesz-- > 0) {
			fputc(fgetc(fp), img);
			(*phyaddr)++;
		}
	}
}

static void write_padding(FILE *img, int *phyaddr, int new_phyaddr)
{
	if (options.extended == 1 && *phyaddr < new_phyaddr) {
		printf("\t\twrite 0x%04x bytes for padding\n", new_phyaddr - *phyaddr);
	}

	while (*phyaddr < new_phyaddr) {
		fputc(0, img);
		(*phyaddr)++;
	}
}

static void write_img_info(int nbytes_kernel, task_info_t *taskinfo,
                           short tasknum, FILE * img, int secnum)
{
	// TODO: [p1-task3] & [p1-task4] write image info to some certain places
	// NOTE: os size, infomation about app-info sector(s) ...
	//os size in 0x1fc, app num in 0x1fe(p1-task3)

	// store the information about programs in ""./test/test_project1" at the end of the image
	fwrite(taskinfo,sizeof(task_info_t),tasknum,img);

	// write data to the last few bytes of the first sector
	fseek(img,TASK_INFO_OFFSET,SEEK_SET);
	// total sectors, occupies 4 bytes
	fwrite(&secnum,sizeof(uint32_t),1,img);
	// number of sectors occupied by the kernel
	// occupies 2 bytes
	// place in little-endian
	fputc(NBYTES2SEC(nbytes_kernel),img);
	fputc(NBYTES2SEC(nbytes_kernel)>>8,img);
	// total number of programs, occupies 2 bytes
	fwrite(&tasknum,sizeof(uint16_t),1,img);

	//debug
	//printf("tasknum:\t\t%d\n",tasknum);
	//printf("secnum:\t\t%d\n",secnum);
}

/* print an error message and exit */
static void error(char *fmt, ...)
{
	va_list args;

	va_start(args, fmt);
	vfprintf(stderr, fmt, args);
	va_end(args);
	if (errno != 0) {
		perror(NULL);
	}
	exit(EXIT_FAILURE);
}