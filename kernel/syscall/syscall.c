#include <sys/syscall.h>
#include <asm/regs.h>
#include <asm/unistd.h>

long (*syscall[NUM_SYSCALLS])();

void handle_syscall(regs_context_t *regs, uint64_t interrupt, uint64_t cause)
{
	/* TODO: [p2-task3] handle syscall exception */
	/**
	* HINT: call syscall function like syscall[fn](arg0, arg1, arg2),
	* and pay attention to the return value and sepc
	*/
	// result = syscall[a7](a0,a1,a2,a3,a4)
	int syscall_num = regs->regs[A7];
	regs->sepc += 4;	// for syscall, return pc = next pc of ecall
	long result = syscall[regs->regs[A7]](regs->regs[A0],regs->regs[A1],regs->regs[A2],regs->regs[A3],regs->regs[A4]);
	if(syscall_num != SYSCALL_EXEC)
		regs->regs[A0] = result;
}
