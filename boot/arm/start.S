

#include "config.h"

#define MODE_USR  0x10
#define MODE_IRQ  0x12
#define MODE_SVC  0x13
#define MODE_BAD  0x1F

#define I_Bit     0x80
#define F_Bit     0x40

#define NOINT     0xc0

.Align 5
.globl _start
_start: b	reset

.globl exception_table
exception_table:
    ldr     pc, [pc, #24]
    ldr     pc, [pc, #24]
    ldr     pc, [pc, #24]
    ldr     pc, [pc, #24]
    ldr     pc, [pc, #24]
    ldr     pc, [pc, #24]
    ldr     pc, [pc, #24]
    ldr     pc, [pc, #24]

	.word success_fix
    b .
    b .
    b .
    b .
    b .
    .word irq
    b .
.globl exception_table_end
exception_table_end:



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

.globl check_remap
check_remap:
	ldr pc,=0x0
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
	sub	r0, r0, #CONFIG_SVC_STACK
	mov sp,r0

	sub	r0, r0, #CONFIG_IRQ_STACK

	msr CPSR_c,#MODE_IRQ|I_Bit|F_Bit
	mov sp,r0

	/* enable SVC irq */
	bic r0,r0,#MODE_BAD|NOINT
	orr r0,r0,#MODE_SVC|F_Bit

	msr CPSR_c,r0

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




#define S_FRAME_SIZE	72

#define S_OLD_R0	72
#define S_PSR		68
#define S_PC		64
#define S_LR		60
#define S_SP		56

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


	.macro	irq_save_svc_regs
	stmdb	sp, {r0-r12}			@ Calling r0-r7

	str lr,[sp, #-S_LR]

	msr CPSR_c,#MODE_IRQ|I_Bit|F_Bit
	sub r0,lr,#4

	msr CPSR_c,#MODE_SVC|I_Bit|F_Bit
	str	r0, [sp, #-S_PC]		@ Save calling PC
	mrs	r6, spsr
	str	r6, [sp, #-S_PSR]		@ Save CPSR
	.endm

	.macro	irq_restore_svc_regs
	ldmdb	sp, {r0-r12}		@ Calling r0 - r7 ... just r7 enough

	ldr lr,[sp,#-S_LR]

	ldr r8,[sp, #-S_PSR]
	msr SPSR_cxsf, r8           @should be SPSP,not CPSP... understand ?

	ldr	pc, [sp, #-S_PC]			@ Get PC
	.endm
irq:
    /* in irq mode */
	msr CPSR_c,#MODE_SVC|I_Bit|F_Bit

	irq_save_svc_regs

	msr CPSR_c,#MODE_IRQ|I_Bit|F_Bit

	bl enter_IRQ
    bl do_IRQ
    bl exit_IRQ

	bic r0,r0,#MODE_BAD|NOINT
	orr r0,r0,#MODE_SVC|F_Bit
	msr CPSR_c,r0

	irq_restore_svc_regs

