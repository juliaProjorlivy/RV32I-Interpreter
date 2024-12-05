.global main
.section .text
main:
.L4:
    addi    x1,x0 ,0
	addi	x0 ,x0 ,-32
	addi	x0 ,x0 ,-32
	addi	x0 ,x0 ,-32
	addi	x0 ,x0 ,-32
	addi	x0 ,x0 ,-32
	addi	x0 ,x0 ,-32
	addi	x0 ,x0 ,-32
	addi	x0 ,x0 ,-32
	addi	x0 ,x0 ,-32
    jal     x0, .L5
    addi    x1,x1 ,1
    ebreak
.L5:
    addi    x1,x1 ,10
    ebreak

