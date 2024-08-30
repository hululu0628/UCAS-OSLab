/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Copyright (C) 2012 Regents of the University of California
 */

#include <asm.h>
#include <csr.h>

#define KERNEL_STACK		0x50500000

.section ".entry_function","ax"
ENTRY(_start)
  /* Mask all interrupts */
	csrw CSR_SIE, zero
	csrw CSR_SIP, zero

  /* TODO: [p1-task2] clear BSS for flat non-ELF images */
	la 	a0,__bss_start
	la 	a1,__BSS_END__
	bge 	a0,a1,EMPBSS
FLAT:
	sb 	zero,(a0)
	addi 	a0,a0,1
	blt 	a0,a1,FLAT
EMPBSS:
  /* TODO: [p1-task2] setup C environment */
	li 	sp,KERNEL_STACK
	li 	fp,KERNEL_STACK
	j 	main

loop:
	wfi
	j loop

END(_start)
