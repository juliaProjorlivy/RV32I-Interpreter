.global main
.section .text
main:
    li      x11, 5
    li      x10, 0
.L4:
    addi    x10,x10 ,1
	addi	x1 ,x0 ,-32
	addi	x1 ,x0 ,-32
	addi	x1 ,x0 ,-32
	addi	x1 ,x0 ,-32
	addi	x1 ,x0 ,-32
	addi	x1 ,x0 ,-32
	addi	x1 ,x0 ,-32
	addi	x1 ,x0 ,-32
	addi	x1 ,x0 ,-32
	addi	x1 ,x0 ,-32
	addi	x1 ,x0 ,-32
	addi	x2 ,x0 ,-32
	addi	x2 ,x0 ,-32
	addi	x2 ,x0 ,-32
	addi	x2 ,x0 ,-32
	addi	x2 ,x0 ,-32
	addi	x2 ,x0 ,-32
    bne	    x10,x11,.L4
    ebreak

