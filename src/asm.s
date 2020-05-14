	.syntax unified
	.cpu cortex-m4
	.thumb
.data
	arr: .byte 0x7E, 0x30, 0x6D, 0x79, 0x33, 0x5B, 0x5F, 0x70, 0x7F, 0x7B, 0x77, 0x1F, 0x4E, 0x3D, 0x4F, 0x47 //TODO: put 0 to F 7-Seg LED pattern here
	student_id: .byte 0, 4, 1, 6, 3, 0, 6 //TODO: put your student id here
	X: .word 1000
	Y: .word 1000

.text
	.global GPIO_init
	.global MAX7219Send
	.global max7219_init

	.equ DECODE_MODE, 	0x09
	.equ DISPLAY_TEST,	0x0F
	.equ SCAN_LIMIT, 	0x0B
	.equ INTENSITY,		0x0A
	.equ SHUTDOWN,		0x0C
	.equ D0,	0x01

	.equ RCC_AHB2ENR, 	0x4002104C

	.equ GPIOC_BASE, 		0x48000800
	.equ GPIO_BSRR_OFFSET, 	0x18
	.equ GPIO_BRR_OFFSET, 	0x28

	.equ GPIOC_MODER, 	0x48000800
	.equ GPIOC_OTYPER, 	0x48000804
	.equ GPIOC_OSPEEDR, 0x48000808
	.equ GPIOC_PUPDR, 	0x4800080C
	.equ GPIOC_IDR, 	0x48000810
	.equ GPIOC_ODR, 	0x48000814

	.equ DATA, 	0b001 //PC0
	.equ LOAD, 	0b010 //PC1
	.equ CLOCK, 0b100 //PC2



GPIO_init:
	//TODO: Initialize three GPIO pins as output for max7219 DIN, CS and CLK
	push {r0, r1, r2, lr }
	movs r0,	#0b111
	ldr r1,	=RCC_AHB2ENR
	str r0,	[r1]

	MOVS	R0, #0b010101
	LDR		R1, =GPIOC_MODER
	LDR		R2,[R1]
	AND		R2, #0XFFFFFFC0 //Mask MODER5
	ORRS	R2, R2, R0
	AND		R2, #0XF3FFFFFF
	STR		R2, [R1]

	MOVS	R0, #0b101010
	LDR		R1, =GPIOC_OSPEEDR
	STRH	R0, [R1]

	//PC_0, PC_1, PC_2 for DIN, CS and CLK
	LDR		R1, =GPIOC_ODR
	movs	r0, #0b000
	STRH	R0, [R1]

	pop {r0, r1, r2, pc}
	BX LR


MAX7219Send:
   //input parameter: r0 is ADDRESS , r1 is DATA
	//TODO: Use this function to send a message to max7219
	push {r0, r1, r2, r3, r4, r5, r6, r7, r8, r9, lr}
	lsl r0, r0, #8
	add r0, r0, r1 //address+data->DIN

	ldr r1, =#GPIOC_BASE
	ldr r2, =#LOAD
	ldr r3, =#DATA
	ldr r4, =#CLOCK
	ldr r5, =#GPIO_BSRR_OFFSET
	ldr r6, =#GPIO_BRR_OFFSET
	mov r7, #16//r7 = i =16--

.max7219send_loop:
	mov r8, #1
	sub r9, r7, #1
	lsl r8, r8, r9 // r8 = mask
	str r4, [r1,r6]//HAL_GPIO_WritePin(GPIOA, CLOCK, 0);
	tst r0, r8
	beq .bit_not_set//bit not set
	str r3, [r1,r5]
	b .if_done
.bit_not_set:
	str r3, [r1,r6]
.if_done:
	str r4, [r1,r5]
	subs r7, r7, #1
	bgt .max7219send_loop

	str r2, [r1,r6]
	str r2, [r1,r5]
	pop {r0, r1, r2, r3, r4, r5, r6, r7, r8, r9, pc}
	BX LR

max7219_init:
	//TODO: Initialize max7219 registers
	push {r0, r1, r2, r3, r4, r5, r6, r7, r8, r9, lr}

	ldr r0, =#DECODE_MODE
	ldr r1, =#0xFF
	BL MAX7219Send

	ldr r0, =#SCAN_LIMIT
	ldr r1, =0x7
	BL MAX7219Send

	ldr r0, =#INTENSITY
	ldr r1, =#0xA
	BL MAX7219Send

	ldr r0, =#SHUTDOWN
	ldr r1, =#0x1
	BL MAX7219Send

	ldr r0, =#DISPLAY_TEST
	ldr r1, =#0x0
	BL MAX7219Send
	pop {r0, r1, r2, r3, r4, r5, r6, r7, r8, r9, pc}
	BX LR


