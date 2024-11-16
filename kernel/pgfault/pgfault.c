#include <os/pgfault.h>

void handle_load_pgfault(regs_context_t *regs, uint64_t stval, uint64_t scause)
{
	stval &= VA_MASK;
	
}

void handle_store_pgfault(regs_context_t *regs, uint64_t stval, uint64_t scause)
{
	;
}