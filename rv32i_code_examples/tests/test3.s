.global main
.section .text
main:
.L4:
    addi    x10,x0 ,10
    add     x3 ,x27, x27
	addi	x0, x0 ,-32
	addi	x0, x0 ,-32
	addi	x0, x0 ,-32
	addi	x0, x0 ,-32
	addi	x0, x0 ,-32
	addi	x0, x0 ,-32
	addi	x0, x0 ,-32
	addi	x0, x0 ,-32
    add     x9, x0 , x0
    jal     x1, .L5
    addi    x9, x9 ,  1
    bne     x9, x10, .L5
    addi    x15,x0 ,-100
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

