#ifndef INCLUDE_SWAP_H
#define INCLUDE_SWAP_H

#include <type.h>
#include <pgtable.h>

#define SECTORS_FOR_A_PAGE 8

// #define SWAP_SPACE	64000000lu
// #define SWAP_SPACE	4096000lu	// 4MB，for debugging
#define SWAP_SPACE 0			
#define MAX_SLOT	(SWAP_SPACE >> 12lu)

#define GET_SECTOR_ID(num) (num << 3)

extern uint64_t swap_id_start;

extern uint64_t clock_hand;

extern uint64_t slot_next;
extern uint64_t free_slot;

extern uint8_t swap_map[MAX_SLOT];

extern void set_swap_entry(PTE * pte, uint64_t slot_index);
extern uint64_t get_swap_entry(PTE * pte);

extern void init_swap(void);

extern int get_swap_page(void);

extern void swap_free(uint64_t slot_index);

extern int swap_in(uint64_t va);

extern int swap_out(void);


#endif