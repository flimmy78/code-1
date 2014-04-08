

#include "config.h"



.globl _start
_start: b	reset
    ldr     pc, [pc, #20]
    ldr     pc, [pc, #20]
    ldr     pc, [pc, #20]
    ldr     pc, [pc, #20]
    ldr     pc, [pc, #20]
    ldr     pc, [pc, #20]
    ldr     pc, [pc, #20]

exception_table:
    b .
    b .
    b .
    b .
    b .
    b irq
    b .



/*
 *************************************************************************
 *
 * Startup Code (reset vector)
 *
 * do important init only if we don't start from memory!
 * setup Memory and board specific bits prior to relocation.
 * relocate armboot to ram
 * setup stack
 *
 *************************************************************************
 */

_TEXT_BASE:
	.word	TEXT_BASE


.globl _armboot_start
_armboot_start:
	.word _start

/*
 * These are defined in the board-specific linker script.
 */
.globl _bss_start
_bss_start:
	.word __bss_start

.globl _bss_end
_bss_end:
	.word _end

.globl print
print:
	ldr r3,=0x20080000
loop:
    ldr r4,[r3,#0x18]
	and r4,r4,#0x20
	cmp r4,#0x20
	beq loop
	str r0,[r3]
	mov pc,lr

/*
 * the actual reset code
 */

reset:
	/*
	 * set the cpu to SVC32 mode
	 */
	mrs	r0,cpsr
	bic	r0,r0,#0x1f
	orr	r0,r0,#0xd3
	msr	cpsr,r0

	/*
	 * we do sys-critical inits only at reboot,
	 * not when booting from ram!
	 */

	/*
	 * flush v4 I/D caches
	 */
	mov	r0, #0
	mcr	p15, 0, r0, c7, c7, 0	/* flush v3/v4 cache */
	mcr	p15, 0, r0, c8, c7, 0	/* flush v4 TLB */

	/*
	 * disable MMU stuff and caches
	 */
	mrc	p15, 0, r0, c1, c0, 0
	bic	r0, r0, #0x00002300	/* clear bits 13, 9:8 (--V- --RS) */
	bic	r0, r0, #0x00000087	/* clear bits 7, 2:0 (B--- -CAM) */
	orr	r0, r0, #0x00000002	/* set bit 2 (A) Align */
	mcr	p15, 0, r0, c1, c0, 0


#ifndef CONFIG_SKIP_RELOCATE_UBOOT

	@relocate U-Boot to RAM
	adrl	r0, _start		/* r0 <- current position of code   */
	ldr	r1, _TEXT_BASE		/* test if we run from flash or RAM */
	cmp     r0, r1                  /* don't reloc during debug         */
	beq     stack_setup
	ldr	r2, _armboot_start
	ldr	r3, _bss_start
	sub	r2, r3, r2		/* r2 <- size of armboot            */
	add	r2, r0, r2		/* r2 <- source end address         */

copy_loop:
	ldmia	r0!, {r3-r10}		/* copy from source address [r0]    */
	stmia	r1!, {r3-r10}		/* copy to   target address [r1]    */
	cmp	r0, r2			/* until source end addreee [r2]    */
	ble	copy_loop
#endif	/* CONFIG_SKIP_RELOCATE_UBOOT */
stack_setup:
	ldr	r0, _TEXT_BASE		/* upper 128 KiB: relocated uboot   */
	sub	r0, r0, #CONFIG_SYS_MALLOC_LEN	/* malloc area		    */
	sub	r0, r0, #CONFIG_SYS_GBL_DATA_SIZE /* bdinfo		    */
	sub	r0, r0, #(CONFIG_STACKSIZE_IRQ)
	sub	sp, r0, #12		/* leave 3 words for abort-stack    */
	bic	sp, sp, #7		/*8-byte alignment for ABI compliance*/

clear_bss:
	ldr	r0, _bss_start		/* find start of bss segment        */
	ldr	r1, _bss_end		/* stop here                        */
	mov	r2, #0x00000000		/* clear                            */

clbss_l:str	r2, [r0]		/* clear loop...                    */
	add	r0, r0, #4
	cmp	r0, r1
	ble	clbss_l


	ldr	pc, _start_armboot

_start_armboot:
	.word start_armboot   /* jump to 80800000 + x ! Tips : relocated  */


.globl enable_irq
enable_irq:
	mrs	r0,CPSR
	bic r0,r0,#0x80
	msr cpsr_c,r0

.globl disable_irq
disable_irq:
	mrs	r0,CPSR
	orr r0,r0,#0x80
	msr cpsr_c,r0



#define S_FRAME_SIZE	72

#define S_OLD_R0	68
#define S_PSR		64
#define S_PC		60
#define S_LR		56
#define S_SP		52

#define S_IP		48
#define S_FP		44
#define S_R10		40
#define S_R9		36
#define S_R8		32
#define S_R7		28
#define S_R6		24
#define S_R5		20
#define S_R4		16
#define S_R3		12
#define S_R2		8
#define S_R1		4
#define S_R0		0

	.macro get_irq_stack			@ setup IRQ stack
	ldr	r0, _TEXT_BASE		/* upper 128 KiB: relocated uboot   */
	sub	r0, r0, #CONFIG_SYS_MALLOC_LEN	/* malloc area		    */
	sub	r0, r0, #CONFIG_SYS_GBL_DATA_SIZE /* bdinfo		    */
	sub	r0, r0, #CONFIG_STACKSIZE_IRQ
	mov sp, r0
	.endm

	.macro	irq_save_user_regs
	sub	sp, sp, #S_FRAME_SIZE
	stmia	sp, {r0 - r12}			@ Calling r0-r12
	@ !!!! R8 NEEDS to be saved !!!! a reserved stack spot would be good.
	add	r8, sp, #S_PC
	stmdb	r8, {sp, lr}		@ Calling SP, LR
	str	lr, [r8, #0]		@ Save calling PC
	mrs	r6, spsr
	str	r6, [r8, #4]		@ Save CPSR
	str	r0, [r8, #8]		@ Save OLD_R0
	mov	r0, sp
	.endm

	.macro	irq_restore_user_regs
	ldmia	sp, {r0 - lr}		@ Calling r0 - lr
	mov	r0, r0
	ldr	lr, [sp, #S_PC]			@ Get PC
	add	sp, sp, #S_FRAME_SIZE
	subs	pc, lr, #4		@ return & move spsr_svc into cpsr
	.endm
irq:
/* print pc */
   mov r1,pc
   mov r2,#31
bitcheck:
   mov r0,r1,lsr r2
   and r0,r0,#0x1
   add r0,r0,#0x30
   bl print
   sub r2,r2,#1

   cmp r2,#0
   bne bitcheck
   mov r0,#13
   bl print
/* print pc  end */

