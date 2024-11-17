#include <os/pgfault.h>

void handle_load_pgfault(regs_context_t *regs, uint64_t stval, uint64_t scause)
{
	stval &= VA_MASK;
	while(1);
}

void handle_store_pgfault(regs_context_t *regs, uint64_t stval, uint64_t scause)
{
	while(1);
}