#include <asm.h>
#include <csr.h>
#include <asm/regs.h>

.macro SAVE_CONTEXT
	/* TODO: [p2-task3] save all general purpose registers here! */
	/* HINT: Pay attention to the function of tp and sp, and save them carefully! */

	/*
	* Disable user-mode memory access as it should only be set in the
	* actual user copy routines.
	*
	* Disable the FPU to detect illegal usage of floating point in kernel
	* space.
	*/
	li t0, SR_SUM | SR_FS

	/* TODO: [p2-task3] save sstatus, sepc, stval and scause on kernel stack */

.endm

.macro RESTORE_CONTEXT
	/* TODO: Restore all general purpose registers and sepc, sstatus */
	/* HINT: Pay attention to sp again! */
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
	addi sp, sp, -(SWITCH_TO_SIZE)

	/* TODO: [p2-task1] save all callee save registers on kernel stack,
	* see the definition of `struct switchto_context` in sched.h*/
	sd 	ra,0(sp)
	sd 	t0,8(sp)
	sd 	t1,16(sp)
	sd 	t2,24(sp)
	sd 	t3,32(sp)
	sd 	t4,40(sp)
	sd	t5,48(sp)
	sd 	t6,56(sp)
	sd 	a0,64(sp)
	sd 	a1,72(sp)
	sd 	a2,80(sp)
	sd 	a3,88(sp)
	sd 	a4,96(sp)
	sd	a5,104(sp)
	sd 	a6,112(sp)
	sd 	a7,120(sp)

	sd 	sp,0(a0)	# update kernel sp in prev pcb
	ld 	sp,0(a1)	# switch to next pcb's kernel sp

	/* TODO: [p2-task1] restore all callee save registers from kernel stack,
	* see the definition of `struct switchto_context` in sched.h*/
	ld 	a7,120(sp)
	ld 	a6,112(sp)
	ld 	a5,104(sp)
	ld 	a4,96(sp)
	ld 	a3,88(sp)
	ld 	a2,80(sp)
	ld 	a1,72(sp)
	ld 	a0,64(sp)
	ld 	t6,56(sp)
	ld 	t5,48(sp)
	ld	t4,40(sp)
	ld 	t3,32(sp)
	ld 	t2,24(sp)
	ld 	t1,16(sp)
	ld 	t0,8(sp)
	ld 	ra,0(sp)

	addi sp, sp, SWITCH_TO_SIZE
	jr ra
ENDPROC(switch_to)

ENTRY(ret_from_exception)
	/* TODO: [p2-task3] restore context via provided macro and return to sepc */
	/* HINT: remember to check your sp, does it point to the right address? */
ENDPROC(ret_from_exception)

ENTRY(exception_handler_entry)

	/* TODO: [p2-task3] save context via the provided macro */


	/* TODO: [p2-task3] load ret_from_exception into $ra so that we can return to
	* ret_from_exception when interrupt_help complete.
	*/


	/* TODO: [p2-task3] call interrupt_helper
	* NOTE: don't forget to pass parameters for it.
	*/


ENDPROC(exception_handler_entry)
