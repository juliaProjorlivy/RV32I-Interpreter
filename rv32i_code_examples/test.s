.global main
.section .text
main:
    li      x26, 5
    li      x27, 0
.L4:
    addi    x27,x27 ,1
	addi	x28 ,x29 ,-32
	addi	x28 ,x29 ,-32
	addi	x28 ,x29 ,-32
	addi	x28 ,x29 ,-32
	addi	x28 ,x29 ,-32
	addi	x28 ,x29 ,-32
	addi	x28 ,x29 ,-32
	addi	x28 ,x29 ,-32
	addi	x28 ,x29 ,-32
	addi	x28 ,x29 ,-32
	addi	x28 ,x29 ,-32
	addi	x28 ,x29 ,-32
	addi	x28 ,x29 ,-32
	addi	x28 ,x29 ,-32
	addi	x28 ,x29 ,-32
	addi	x28 ,x29 ,-32
	addi	x28 ,x29 ,-32
    bne	    x27,x26,.L4
    ebreak

