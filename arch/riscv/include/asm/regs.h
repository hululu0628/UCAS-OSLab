/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * * * * *
 *            Copyright (C) 2018 Institute of Computing Technology, CAS
 *               Author : Han Shukai (email : hanshukai@ict.ac.cn)
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * * * * *
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * * * * * */

#ifndef INCLUDE_REGS_H_
#define INCLUDE_REGS_H_

/* This is for struct TrapFrame in scheduler.h
 * Stack layout for all exceptions:
 *
 * ptrace needs to have all regs on the stack. If the order here is changed,
 * it needs to be updated in include/asm-mips/ptrace.h
 *
 * The first PTRSIZE*5 bytes are argument save space for C subroutines.
 */

#define OFFSET_REG_ZERO         0

/* return address */
#define OFFSET_REG_RA           8

/* pointers */
#define OFFSET_REG_SP           16 // stack
#define OFFSET_REG_GP           24 // global
#define OFFSET_REG_TP           32 // thread

/* temporary */
#define OFFSET_REG_T0           40
#define OFFSET_REG_T1           48
#define OFFSET_REG_T2           56

/* saved register */
#define OFFSET_REG_S0           64
#define OFFSET_REG_S1           72

/* args */
#define OFFSET_REG_A0           80
#define OFFSET_REG_A1           88
#define OFFSET_REG_A2           96
#define OFFSET_REG_A3           104
#define OFFSET_REG_A4           112
#define OFFSET_REG_A5           120
#define OFFSET_REG_A6           128
#define OFFSET_REG_A7           136

/* saved register */
#define OFFSET_REG_S2           144
#define OFFSET_REG_S3           152
#define OFFSET_REG_S4           160
#define OFFSET_REG_S5           168
#define OFFSET_REG_S6           176
#define OFFSET_REG_S7           184
#define OFFSET_REG_S8           192
#define OFFSET_REG_S9           200
#define OFFSET_REG_S10          208
#define OFFSET_REG_S11          216

/* temporary register */
#define OFFSET_REG_T3           224
#define OFFSET_REG_T4           232
#define OFFSET_REG_T5           240
#define OFFSET_REG_T6           248

/* privileged register */
#define OFFSET_REG_SSTATUS      256
#define OFFSET_REG_SEPC         264
#define OFFSET_REG_STVAL     272
#define OFFSET_REG_SCAUSE       280

/* Size of stack frame, word/double word alignment */
#define OFFSET_SIZE             288

#define PCB_KERNEL_SP          0
#define PCB_USER_SP            8

/* offset in switch_to */
#define SWITCH_TO_RA     0
#define SWITCH_TO_SP     8
#define SWITCH_TO_S0     16
#define SWITCH_TO_S1     24
#define SWITCH_TO_S2     32
#define SWITCH_TO_S3     40
#define SWITCH_TO_S4     48
#define SWITCH_TO_S5     56
#define SWITCH_TO_S6     64
#define SWITCH_TO_S7     72
#define SWITCH_TO_S8     80
#define SWITCH_TO_S9     88
#define SWITCH_TO_S10    96
#define SWITCH_TO_S11    104

#define SWITCH_TO_SIZE   112


#define	ZERO	0	// Zero (x0)
#define RA    	1   	// Return Address (x1)
#define SP   	2   	// Stack Pointer (x2)
#define GP    	3   	// Global Pointer (x3)
#define TP    	4   	// Thread Pointer (x4)
#define T0    	5   	// Temporary Register (x5)
#define T1    	6   	// Temporary Register (x6)
#define T2    	7   	// Temporary Register (x7)
#define S0    	8   	// Saved Register / Frame Pointer (x8)
#define S1    	9   	// Saved Register (x9)

#define A0   	10   	// Argument Register (x10)
#define A1   	11   	// Argument Register (x11)
#define A2   	12   	// Argument Register (x12)
#define A3   	13   	// Argument Register (x13)
#define A4   	14   	// Argument Register (x14)
#define A5   	15   	// Argument Register (x15)
#define A6   	16   	// Argument Register (x16)
#define A7   	17   	// Argument Register (x17)

#define S2   	18   	// Saved Register (x18)
#define S3   	19   	// Saved Register (x19)
#define S4   	20   	// Saved Register (x20)
#define S5   	21   	// Saved Register (x21)
#define S6   	22   	// Saved Register (x22)
#define S7   	23   	// Saved Register (x23)
#define S8   	24   	// Saved Register (x24)
#define S9   	25   	// Saved Register (x25)
#define S10  	26   	// Saved Register (x26)
#define S11  	27   	// Saved Register (x27)

#define T3   	28   	// Temporary Register (x28)
#define T4   	29   	// Temporary Register (x29)
#define T5   	30   	// Temporary Register (x30)
#define T6   	31   	// Temporary Register (x31)


#endif
