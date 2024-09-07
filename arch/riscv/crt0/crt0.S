#include <asm.h>

.section ".entry_function","ax"
ENTRY(_start)

	/* TODO: [p1-task3] setup C runtime environment for the user program */
	la 	a0,__bss_start
	la 	a1,__BSS_END__
	bge 	a0,a1,EMPBSS
FLAT:
	sb 	zero,(a0)
	addi 	a0,a0,1
	blt 	a0,a1,FLAT
EMPBSS:
	addi 	sp,sp,-64
	sd 	t0,0(sp)
	sd 	t1,8(sp)
	sd 	t2,16(sp)
	sd 	t3,24(sp)
	sd 	t4,32(sp)
	sd	t5,40(sp)
	sd 	t6,48(sp)
	/* TODO: [p1-task3] enter main function */
	jal 	main
	/* TODO: [p1-task3] finish task and return to the kernel, replace this in p3-task2! */
	ld 	t6,48(sp)
	ld 	t5,40(sp)
	ld	t4,32(sp)
	ld 	t3,24(sp)
	ld 	t2,16(sp)
	ld 	t1,8(sp)
	ld 	t0,0(sp)
	addi 	sp,sp,64
	ret
	/************************************************************/
		/* Do not touch this comment. Reserved for future projects. */
		/************************************************************/
	// while(1) loop, unreachable here
	loop:
	wfi
	j loop

END(_start)
