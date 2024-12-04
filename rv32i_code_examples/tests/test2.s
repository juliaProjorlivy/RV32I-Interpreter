.global main
.section .text
main:
    li      x26, 5
    li      x27, 0
.L4:
    addi    x27,x27 ,1
    add     x3, x27, x27
	addi	x0 ,x0 ,-32
	addi	x0 ,x0 ,-32
	addi	x0 ,x0 ,-32
	addi	x0 ,x0 ,-32
	addi	x0 ,x0 ,-32
	addi	x0 ,x0 ,-32
	addi	x0 ,x0 ,-32
	addi	x0 ,x0 ,-32
	addi	x0 ,x0 ,-32
	addi	x0 ,x0 ,-32
    jal     x4, .L5
    jal     x4, 56
    jal     x4, main
    addi    x2 , x2, 4
	addi	x6 ,x0 ,-32
	addi	x6 ,x0 ,-32
	addi	x6 ,x0 ,-32
	addi	x6 ,x0 ,-32
	addi	x6 ,x0 ,-32
    bne	    x27,x26,.L4
    lw      x4, -4(x2)
    lw      x5, -8(x2)
    lw      x6, -12(x2)
    lw      x7, -16(x2)
    lw      x8, -20(x2)
    ebreak
.L5:
    addi    x27,x27 ,10
    ebreak

