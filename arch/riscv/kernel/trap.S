#include <asm.h>
#include <csr.h>

ENTRY(setup_trap)

	/* TODO: [p2-task3] save trap_handler_entry into STVEC */
	la 	t0,trap_handler_entry
	csrw 	stvec,t0
	/* TODO: [p2-task4] enable interrupts globally */
	addi 	sp,sp,-8
	sd 	ra,0(sp)
	jal	enable_interrupt
	ld 	ra,0(sp)
	addi 	sp,sp,8
	ret

ENDPROC(setup_trap)
