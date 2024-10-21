#include <asm.h>

.section ".entry_function","ax"
ENTRY(_start)

	/* TODO: [p1-task3] setup C runtime environment for the user program */
	la 	t0,__bss_start
	la 	t1,__BSS_END__
	bge 	t0,t1,EMPBSS
FLAT:
	sb 	zero,(t0)
	addi 	t0,t0,1
	blt 	t0,t1,FLAT
EMPBSS:

	/* TODO: [p1-task3] enter main function */
	jal 	main
    /************************************************************/
    /* TODO: [P3] call sys_exit */
	jal	sys_exit
    /************************************************************/
// while(1) loop, unreachable here
loop:
    wfi
    j loop

END(_start)
