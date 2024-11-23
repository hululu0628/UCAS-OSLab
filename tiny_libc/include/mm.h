#ifndef INCLUDE_MM_H
#define INCLUDE_MM_H

/* for mprotect */
#define PROT_NONE (~(0x7 << 1))
#define PROT_READ (1 << 1)
#define PROT_WRITE (1 << 2)
#define PROT_EXEC (1 << 3)

#define NALLOC 4096

typedef long Align;

union header 
{
	struct {
		union header *ptr;
		unsigned size;
	} s;
	Align x;
};

typedef union header Header;

void *malloc(unsigned nbytes);
void free(void *ap);

#endif