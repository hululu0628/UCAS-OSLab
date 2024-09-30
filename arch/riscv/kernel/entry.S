#include <asm.h>
#include <csr.h>
#include <asm/regs.h>

.macro SAVE_CONTEXT
	/* TODO: [p2-task3] save all general purpose registers here! */
	/* HINT: Pay attention to the function of tp and sp, and save them carefully! */
	csrw 	sscratch,sp
	ld 	sp,PCB_KERNEL_SP(tp)

	addi 	sp,sp,-(OFFSET_SIZE)

	sd 	zero,OFFSET_REG_ZERO(sp)
	sd 	ra,OFFSET_REG_RA(sp)

	sd 	gp,OFFSET_REG_GP(sp)
	sd 	tp,OFFSET_REG_TP(sp)
	sd 	t0,OFFSET_REG_T0(sp)
	sd 	t1,OFFSET_REG_T1(sp)
	sd 	t2,OFFSET_REG_T2(sp)
	sd 	s0,OFFSET_REG_S0(sp)
	sd 	s1,OFFSET_REG_S1(sp)
	sd 	a0,OFFSET_REG_A0(sp)
	sd 	a1,OFFSET_REG_A1(sp)
	sd 	a2,OFFSET_REG_A2(sp)
	sd 	a3,OFFSET_REG_A3(sp)
	sd 	a4,OFFSET_REG_A4(sp)
	sd 	a5,OFFSET_REG_A5(sp)
	sd 	a6,OFFSET_REG_A6(sp)
	sd 	a7,OFFSET_REG_A7(sp)
	sd 	s2,OFFSET_REG_S2(sp)
	sd 	s3,OFFSET_REG_S3(sp)
	sd 	s4,OFFSET_REG_S4(sp)
	sd 	s5,OFFSET_REG_S5(sp)
	sd 	s6,OFFSET_REG_S6(sp)
	sd 	s7,OFFSET_REG_S7(sp)
	sd 	s8,OFFSET_REG_S8(sp)
	sd 	s9,OFFSET_REG_S9(sp)
	sd 	s10,OFFSET_REG_S10(sp)
	sd 	s11,OFFSET_REG_S11(sp)
	sd 	t3,OFFSET_REG_T3(sp)
	sd 	t4,OFFSET_REG_T4(sp)
	sd 	t5,OFFSET_REG_T5(sp)
	sd 	t6,OFFSET_REG_T6(sp)

	csrr	t0,sscratch
	sd 	t0,OFFSET_REG_SP(sp)			# sp points to user-stack
	/*
	* Disable user-mode memory access as it should only be set in the
	* actual user copy routines.
	*
	* Disable the FPU to detect illegal usage of floating point in kernel
	* space.
	*/
	li t0, SR_SUM | SR_FS	# ???

	/* TODO: [p2-task3] save sstatus, sepc, stval and scause on kernel stack */

	csrr 	t0,sstatus
	csrr 	t1,sepc
	csrr	t2,stval
	csrr 	t3,scause

	blt 	t3,zero,INTER			# scause >= 0, handling exception, sepc = nextpc
	addi 	t1,t1,4
INTER:
	sd 	t0,OFFSET_REG_SSTATUS(sp)
	sd 	t1,OFFSET_REG_SEPC(sp)
	sd 	t2,OFFSET_REG_STVAL(sp)
	sd 	t3,OFFSET_REG_SCAUSE(sp)


.endm

.macro RESTORE_CONTEXT
	/* TODO: Restore all general purpose registers and sepc, sstatus */
	/* HINT: Pay attention to sp again! */

	
	ld  	t3, OFFSET_REG_SCAUSE(sp)
	ld  	t2, OFFSET_REG_STVAL(sp)
	ld  	t1, OFFSET_REG_SEPC(sp)
	ld  	t0, OFFSET_REG_SSTATUS(sp)
	csrw 	scause, t3
	csrw 	stval, t2
	csrw 	sepc, t1
	csrw 	sstatus, t0

	ld  	t0, OFFSET_REG_SP(sp)
	csrw 	sscratch, t0

	ld  	t6, OFFSET_REG_T6(sp)
	ld  	t5, OFFSET_REG_T5(sp)
	ld  	t4, OFFSET_REG_T4(sp)
	ld  	t3, OFFSET_REG_T3(sp)
	ld  	s11, OFFSET_REG_S11(sp)
	ld  	s10, OFFSET_REG_S10(sp)
	ld  	s9, OFFSET_REG_S9(sp)
	ld  	s8, OFFSET_REG_S8(sp)
	ld  	s7, OFFSET_REG_S7(sp)
	ld  	s6, OFFSET_REG_S6(sp)
	ld  	s5, OFFSET_REG_S5(sp)
	ld  	s4, OFFSET_REG_S4(sp)
	ld  	s3, OFFSET_REG_S3(sp)
	ld  	s2, OFFSET_REG_S2(sp)
	ld  	a7, OFFSET_REG_A7(sp)
	ld  	a6, OFFSET_REG_A6(sp)
	ld  	a5, OFFSET_REG_A5(sp)
	ld  	a4, OFFSET_REG_A4(sp)
	ld  	a3, OFFSET_REG_A3(sp)
	ld  	a2, OFFSET_REG_A2(sp)
	ld  	a1, OFFSET_REG_A1(sp)
	ld  	a0, OFFSET_REG_A0(sp)
	ld  	s1, OFFSET_REG_S1(sp)
	ld  	s0, OFFSET_REG_S0(sp)
	ld  	t2, OFFSET_REG_T2(sp)
	ld  	t1, OFFSET_REG_T1(sp)
	ld  	t0, OFFSET_REG_T0(sp)
	ld  	tp, OFFSET_REG_TP(sp)
	ld  	gp, OFFSET_REG_GP(sp)
	
	ld  	ra, OFFSET_REG_RA(sp)
	ld  	zero, OFFSET_REG_ZERO(sp)


	addi 	sp, sp, OFFSET_SIZE

	sd 	sp,PCB_KERNEL_SP(tp)
	csrr 	sp,sscratch

.endm

ENTRY(enable_preempt)
	not t0, x0
	csrs CSR_SIE, t0
	jr ra
ENDPROC(enable_preempt)

ENTRY(disable_preempt)
	csrw CSR_SIE, zero
	jr ra
ENDPROC(disable_preempt)

ENTRY(enable_interrupt)
	li t0, SR_SIE
	csrs CSR_SSTATUS, t0
	jr ra
ENDPROC(enable_interrupt)

ENTRY(disable_interrupt)
	li t0, SR_SIE
	csrc CSR_SSTATUS, t0
	jr ra
ENDPROC(disable_interrupt)

	// NOTE: the address of previous pcb in a0
	// NOTE: the address of next pcb in a1
	// process switch from a0 to a1
ENTRY(switch_to)
	// (╯▔皿▔)╯
	addi 	sp, sp, -(SWITCH_TO_SIZE)

	/* TODO: [p2-task1] save all callee save registers on kernel stack,
	* see the definition of `struct switchto_context` in sched.h*/
	sd 	ra,SWITCH_TO_RA(sp)	# for user process, ra points to do_scheduler
					# (do_scheduler->handler->inter_helper->ret_from_exception)

	sd 	sp,SWITCH_TO_SP(sp)	# sp points to kernel-stack
	sd 	s0,SWITCH_TO_S0(sp)
	sd 	s1,SWITCH_TO_S1(sp)
	sd 	s2,SWITCH_TO_S2(sp)
	sd 	s3,SWITCH_TO_S3(sp)
	sd	s4,SWITCH_TO_S4(sp)
	sd 	s5,SWITCH_TO_S5(sp)
	sd 	s6,SWITCH_TO_S6(sp)
	sd 	s7,SWITCH_TO_S7(sp)
	sd 	s8,SWITCH_TO_S8(sp)
	sd 	s9,SWITCH_TO_S9(sp)
	sd 	s10,SWITCH_TO_S10(sp)
	sd	s11,SWITCH_TO_S11(sp)

	sd 	sp,PCB_KERNEL_SP(a0)	# update kernel sp in prev pcb
	ld 	sp,PCB_KERNEL_SP(a1)	# switch to next pcb's kernel sp

	/* TODO: [p2-task1] restore all callee save registers from kernel stack,
	* see the definition of `struct switchto_context` in sched.h*/
	ld	s11,SWITCH_TO_S11(sp)
	ld 	s10,SWITCH_TO_S10(sp)
	ld 	s9,SWITCH_TO_S9(sp)
	ld 	s8,SWITCH_TO_S8(sp)
	ld 	s7,SWITCH_TO_S7(sp)
	ld 	s6,SWITCH_TO_S6(sp)
	ld 	s5,SWITCH_TO_S5(sp)
	ld	s4,SWITCH_TO_S4(sp)
	ld 	s3,SWITCH_TO_S3(sp)
	ld 	s2,SWITCH_TO_S2(sp)
	ld 	s1,SWITCH_TO_S1(sp)
	ld 	s0,SWITCH_TO_S0(sp)
	ld 	sp,SWITCH_TO_SP(sp)
	ld 	ra,SWITCH_TO_RA(sp)

	addi 	sp, sp, SWITCH_TO_SIZE

	jr ra
ENDPROC(switch_to)

ENTRY(ret_from_exception)
	/* TODO: [p2-task3] restore context via provided macro and return to sepc */
	/* HINT: remember to check your sp, does it point to the right address? */
	RESTORE_CONTEXT

	sret

ENDPROC(ret_from_exception)

ENTRY(exception_handler_entry)

	/* TODO: [p2-task3] save context via the provided macro */
	SAVE_CONTEXT

	/* TODO: [p2-task3] load ret_from_exception into $ra so that we can return to
	* ret_from_exception when interrupt_help complete.
	*/
	la	ra,ret_from_exception

	/* TODO: [p2-task3] call interrupt_helper
	* NOTE: don't forget to pass parameters for it.
	*/
	mv 	a0,sp
	ld 	a1,OFFSET_REG_STVAL(a0)
	ld 	a2,OFFSET_REG_SCAUSE(a0)
	j 	interrupt_helper



ENDPROC(exception_handler_entry)
