.global main
.section .text
main:
.L4:
    addi    x4,x0 ,10
	addi	x0, x0 ,-32
	addi	x0, x0 ,-32
	addi	x0, x0 ,-32
	addi	x0, x0 ,-32
	addi	x0, x0 ,-32
	addi	x0, x0 ,-32
	addi	x0, x0 ,-32
	addi	x0, x0 ,-32
	addi	x0, x0 ,-32
    add     x3, x0 , x0
    jal     x1, .L5
    addi    x3, x3 ,  1
    bne     x3, x4, .L5
    addi    x5,x0 ,-100
    ebreak
.L5:
    sw      x1, 44(x2)
    addi    x0, x27, 10
    addi    x0, x27, 10
    addi    x0, x27, 10
    addi    x0, x27, 10
    addi    x0, x27, 10
    addi    x0, x27, 10
    addi    x0, x27, 10
    addi    x0, x27, 10
    jalr    x0, 0(x1)
    ebreak

