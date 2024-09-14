#include <asm/biosdef.h>

// os size location (os_size could be sectors [p1-task3] or bytes [p1-task4])
.equ os_size_loc, 0x502001fc
// task num location(add in p1-t4)
.equ taskinfo_size_loc, 0x502001fe
// taskinfo offset(add in p1-t4)
.equ taskinfo_sec_num, 0x502001f8

// kernel address (move kernel to here, and jmp here to start kernel)
.equ kernel, 0x50201000
//taskinfo address (add in p1-t4)
.equ taskinfo, 0x50200200

// BIOS function entry (jump here to use BIOS APIs)
.equ bios_func_entry, 0x50150000

.text
.global main

main:
	// fence on all memory and I/O
	fence

	// TODO: [p1-task1] call BIOS to print string "It's bootblock!"
	li 	a7,9			# call bios_pustr
	la 	a0,msg
	jal 	bios_func_entry

	// TODO: [p1-task2] call BIOS to read kernel in SD card
	li 	a0,os_size_loc		# os size(sectors) in 0x502001fc
	lh 	a1,0(a0)
	li 	a0,kernel		# move data to 0x50201000
	li 	a2,1			# kernel data starts at sector 1
	li 	a7,11			# call bios_sd_read
	jal 	bios_func_entry

	// TODO: [p1-task4] load task-related arguments and pass them to kernel
	// 将image最后的数据搬运到0x50200200
	li 	a0,taskinfo_sec_num	# where the task information start, in 0x502001f8
	lw 	a2,0(a0)
	li 	a0,taskinfo		# move data to 0x50200200
	li 	a1,1			# size of task information(less than one sector), occupies 1 sector
	li 	a7,11			# call bios_sd_read
	jal 	bios_func_entry

	// TODO: [p1-task2] jump to kernel to start UCAS-OS
	jal 	kernel
	/************************************************************/
	/* Do not touch this comment. Reserved for future projects. */
	/************************************************************/
// while(1) --> stop here
stop:
	j stop
	nop

.data

msg: .string "It's hululu's bootloader...\n\r"

