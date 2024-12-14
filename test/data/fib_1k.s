	.file	"fib.c"
	.option nopic
	.attribute arch, "rv32i2p1_m2p0_a2p1_c2p0"
	.attribute unaligned_access, 0
	.attribute stack_align, 16
	.text
	.align	1
	.globl	fib
	.type	fib, @function
fib:
	addi	sp,sp,-48
	sw	ra,44(sp)
	sw	s0,40(sp)
	addi	s0,sp,48
	sw	a0,-36(s0)
	sw	a1,-44(s0)
	sw	a2,-40(s0)
	lw	a5,-36(s0)
	bne	a5,zero,.L2
	lw	a5,-44(s0)
	j	.L3
.L2:
	lw	a4,-36(s0)
	li	a5,1
	bne	a4,a5,.L4
	lw	a5,-40(s0)
	j	.L3
.L4:
	lw	a5,-40(s0)
	sw	a5,-24(s0)
	lw	a4,-44(s0)
	lw	a5,-40(s0)
	add	a5,a4,a5
	sw	a5,-20(s0)
	lw	a5,-36(s0)
	addi	a5,a5,-1
	lw	a1,-24(s0)
	lw	a2,-20(s0)
	mv	a0,a5
	call	fib
	mv	a5,a0
.L3:
	mv	a0,a5
	lw	ra,44(sp)
	lw	s0,40(sp)
	addi	sp,sp,48
	jr	ra
	.size	fib, .-fib
	.align	1
	.globl	main
	.type	main, @function
main:
	addi	sp,sp,-32
	sw	ra,28(sp)
	sw	s0,24(sp)
	addi	s0,sp,32
	li	a5,1000
	sw	a5,-20(s0)
	sw	zero,-32(s0)
	li	a5,1
	sw	a5,-28(s0)
	lw	a1,-32(s0)
	lw	a2,-28(s0)
	lw	a0,-20(s0)
	call	fib
	sw	a0,-24(s0)
 #APP
# 22 "fib.c" 1
	ebreak
# 0 "" 2
 #NO_APP
	li	a5,0
	mv	a0,a5
	lw	ra,28(sp)
	lw	s0,24(sp)
	addi	sp,sp,32
	jr	ra
	.size	main, .-main
	.ident	"GCC: ('riscv32-embecosm-ubuntu2004-gcc13.2.0') 13.2.0"
