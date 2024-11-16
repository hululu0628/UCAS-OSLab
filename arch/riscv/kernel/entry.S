#include <asm.h>
#include <csr.h>
#include <asm/regs.h>


.macro SAVE_CONTEXT
	/* TODO: [p2-task3] save all general purpose registers here! */
	/* HINT: Pay attention to the function of tp and sp, and save them carefully! */
	sd 	zero, OFFSET_REG_ZERO(tp)
	sd 	ra, OFFSET_REG_RA(tp)
	sd  	sp, OFFSET_REG_SP(tp)
	sd 	gp, OFFSET_REG_GP(tp)
	sd 	tp, OFFSET_REG_TP(tp)
	sd 	t0, OFFSET_REG_T0(tp)
	sd 	t1, OFFSET_REG_T1(tp)
	sd 	t2, OFFSET_REG_T2(tp)
	sd 	s0, OFFSET_REG_S0(tp)
	sd 	s1, OFFSET_REG_S1(tp)
	sd 	a0, OFFSET_REG_A0(tp)
	sd 	a1, OFFSET_REG_A1(tp)
	sd 	a2, OFFSET_REG_A2(tp)
	sd 	a3, OFFSET_REG_A3(tp)
	sd 	a4, OFFSET_REG_A4(tp)
	sd 	a5, OFFSET_REG_A5(tp)
	sd 	a6, OFFSET_REG_A6(tp)
	sd 	a7, OFFSET_REG_A7(tp)
	sd 	s2, OFFSET_REG_S2(tp)
	sd 	s3, OFFSET_REG_S3(tp)
	sd 	s4, OFFSET_REG_S4(tp)
	sd 	s5, OFFSET_REG_S5(tp)
	sd 	s6, OFFSET_REG_S6(tp)
	sd 	s7, OFFSET_REG_S7(tp)
	sd 	s8, OFFSET_REG_S8(tp)
	sd 	s9, OFFSET_REG_S9(tp)
	sd 	s10, OFFSET_REG_S10(tp)
	sd 	s11, OFFSET_REG_S11(tp)
	sd 	t3, OFFSET_REG_T3(tp)
	sd 	t4, OFFSET_REG_T4(tp)
	sd 	t5, OFFSET_REG_T5(tp)
	sd 	t6, OFFSET_REG_T6(tp)
	/*
	* Disable user-mode memory access as it should only be set in the
	* actual user copy routines.
	*
	* Disable the FPU to detect illegal usage of floating point in kernel
	* space.
	*/
	# li t0, SR_SUM | SR_FS	# ???

	/* TODO: [p2-task3] save sstatus, sepc, stval and scause on kernel stack */

	csrr 	t0, sstatus
	csrr 	t1, sepc
	csrr	t2, stval
	csrr 	t3, scause

	sd 	t0, OFFSET_CSR_SSTATUS(tp)
	sd 	t1, OFFSET_CSR_SEPC(tp)
	sd 	t2, OFFSET_CSR_STVAL(tp)
	sd 	t3, OFFSET_CSR_SCAUSE(tp)


	ld 	sp, PCB_KERNEL_SP(tp)

.endm

.macro RESTORE_CONTEXT
	/* TODO: Restore all general purpose registers and sepc, sstatus */
	/* HINT: Pay attention to sp again! */

	sd 	sp, PCB_KERNEL_SP(tp)

	
	ld  	t3, OFFSET_CSR_SCAUSE(tp)
	ld  	t2, OFFSET_CSR_STVAL(tp)
	ld  	t1, OFFSET_CSR_SEPC(tp)
	ld  	t0, OFFSET_CSR_SSTATUS(tp)
	csrw 	scause, t3
	csrw 	stval, t2
	csrw 	sepc, t1
	csrw 	sstatus, t0

	ld  	t6, OFFSET_REG_T6(tp)
	ld  	t5, OFFSET_REG_T5(tp)
	ld  	t4, OFFSET_REG_T4(tp)
	ld  	t3, OFFSET_REG_T3(tp)
	ld  	s11, OFFSET_REG_S11(tp)
	ld  	s10, OFFSET_REG_S10(tp)
	ld  	s9, OFFSET_REG_S9(tp)
	ld  	s8, OFFSET_REG_S8(tp)
	ld  	s7, OFFSET_REG_S7(tp)
	ld  	s6, OFFSET_REG_S6(tp)
	ld  	s5, OFFSET_REG_S5(tp)
	ld  	s4, OFFSET_REG_S4(tp)
	ld  	s3, OFFSET_REG_S3(tp)
	ld  	s2, OFFSET_REG_S2(tp)
	ld  	a7, OFFSET_REG_A7(tp)
	ld  	a6, OFFSET_REG_A6(tp)
	ld  	a5, OFFSET_REG_A5(tp)
	ld  	a4, OFFSET_REG_A4(tp)
	ld  	a3, OFFSET_REG_A3(tp)
	ld  	a2, OFFSET_REG_A2(tp)
	ld  	a1, OFFSET_REG_A1(tp)
	ld  	a0, OFFSET_REG_A0(tp)
	ld  	s1, OFFSET_REG_S1(tp)
	ld  	s0, OFFSET_REG_S0(tp)
	ld  	t2, OFFSET_REG_T2(tp)
	ld  	t1, OFFSET_REG_T1(tp)
	ld  	t0, OFFSET_REG_T0(tp)
	ld  	tp, OFFSET_REG_TP(tp)
	ld  	gp, OFFSET_REG_GP(tp)
	ld 	sp, OFFSET_REG_SP(tp)
	ld  	ra, OFFSET_REG_RA(tp)
	ld  	zero, OFFSET_REG_ZERO(tp)

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


ENTRY(switch_pgtable)
	li	a1,0xffffffc000000000
	xor	a0,a0,a1	# kva2pa

	srli	a0,a0,12	# NORMAL_PAGE_SHIFT 12
	li 	a1,8		# SATP_MODE_SV39 8
	slli	a1,a1,60	# SATP_MODE_SHIFT 60
	or	a0,a0,a1
	sfence.vma
	csrw 	satp,a0
	sfence.vma
	jr 	ra
ENDPROC(switch_pgtable)


	// NOTE: the address of previous pcb in a0
	// NOTE: the address of next pcb in a1
	// process switch from a0 to a1
ENTRY(switch_to)
	// (╯▔皿▔)╯
	addi 	sp, sp, -(SWITCH_TO_SIZE)

	/* TODO: [p2-task1] save all callee save registers on kernel stack,
	* see the definition of `struct switchto_context` in sched.h*/
	sd 	ra,SWITCH_TO_RA(sp)	# for user process, ra points to do_scheduler
					# (do_scheduler->handler->inter_helper->ret_from_trap)

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

	mv 	a0,a1
	mv 	s0,a1
	jal 	get_pgdir

	jal	switch_pgtable

	mv 	a0,s0
	ld 	sp,PCB_KERNEL_SP(a0)	# switch to next pcb's kernel sp

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

ENTRY(ret_from_trap)
	/* TODO: [p2-task3] restore context via provided macro and return to sepc */
	/* HINT: remember to check your sp, does it point to the right address? */

	jal	unlock_kernel

	RESTORE_CONTEXT

	sret

ENDPROC(ret_from_trap)

ENTRY(trap_handler_entry)

	/* TODO: [p2-task3] save context via the provided macro */
	SAVE_CONTEXT

	jal	lock_kernel

	/* TODO: [p2-task3] load ret_from_trap into $ra so that we can return to
	* ret_from_trap when interrupt_help complete.
	*/
	la	ra,ret_from_trap

	/* TODO: [p2-task3] call trap_helper
	* NOTE: don't forget to pass parameters for it.
	*/
	mv 	a0,tp
	ld 	a1,OFFSET_CSR_STVAL(tp)
	ld 	a2,OFFSET_CSR_SCAUSE(tp)
	j 	trap_helper



ENDPROC(trap_handler_entry)
