#include <sys/syscall.h>
#include <asm/regs.h>

long (*syscall[NUM_SYSCALLS])();

void handle_syscall(regs_context_t *regs, uint64_t interrupt, uint64_t cause)
{
	/* TODO: [p2-task3] handle syscall exception */
	/**
	* HINT: call syscall function like syscall[fn](arg0, arg1, arg2),
	* and pay attention to the return value and sepc
	*/
	// result = syscall[a7](a0,a1,a2,a3,a4)
	long result = syscall[regs->regs[A7]](regs->regs[A0],regs->regs[A1],regs->regs[A2],regs->regs[A3],regs->regs[A4]);
	regs->regs[10] = result;
}
