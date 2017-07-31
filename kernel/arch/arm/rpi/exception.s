
	.macro	stub name, exception, pc_offset

	.globl	\name
\name:
	mov	sp, #0x8000
	sub	lr, lr, #\pc_offset		/* lr: correct PC of aborted program */
	stmfd	sp!, {lr}			/* store PC onto stack */
	mrs	lr, spsr			/* lr can be overwritten now */
	stmfd	sp!, {lr}			/* store saved PSR onto stack */
	stmfd	sp, {r0-r14}^			/* store user registers r0-r14 (unbanked) */
	sub	sp, sp, #4*15			/* correct stack (not done by previous instruction */
	mov	r1, sp				/* save sp_abt or sp_und */
	cps	#0x12				/* set IRQ mode to access sp_irq and lr_irq */
	mov	r2, sp
	mov	r3, lr
	cps	#0x1F				/* our abort handler runs in system mode */
	mov	sp, r1				/* set sp_sys to stack top of abort stack */
	stmfd	sp!, {r2, r3}			/* store lr_irq and sp_irq onto stack */
	mov	r1, sp				/* r1: pointer to register frame */
	mov	r0, #\exception			/* r0: exception identifier */
	b	exception_handler		/* jump to ExceptionHandler (never returns) */

	.endm

	.text

/*
 * Abort stubs
 */
	stub	UndefinedInstructionStub, 1,	4
	stub	PrefetchAbortStub,		2,		4
	stub	DataAbortStub,			3,			8

/*
 * IRQ stub
 */
	.globl	IRQStub
IRQStub:
	mov	sp, #0x8000
	sub	lr, lr, #4			/* lr: return address */
	stmfd	sp!, {r0-r12, lr}		/* save r0-r12 and return address */
	bl	irq_handler
	ldmfd	sp!, {r0-r12, pc}^		/* restore registers and return */

/* End */
