#ifndef INCLUDE_PGFAULT_H
#define INCLUDE_PGFAULT_H

#include <type.h>
#include <os/proc.h>

extern void handle_load_pgfault(regs_context_t *regs, uint64_t stval, uint64_t scause);

extern void handle_store_pgfault(regs_context_t *regs, uint64_t stval, uint64_t scause);

extern void handle_instr_pgfault(regs_context_t *regs, uint64_t stval, uint64_t scause);

#endif