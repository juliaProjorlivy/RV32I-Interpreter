.global main
.section .text
main:
    li      x26, 1
    li      x28, 6
    li      x27, 2
LOOP:
    addi    x28,x28 , -1
    addi    x0,x0 ,1
    addi    x0,x0 ,1
    addi    x0,x0 ,1
    addi    x0,x0 ,1
    addi    x0,x0 ,1
    addi    x0,x0 ,1
    addi    x0,x0 ,1
    addi    x0,x0 ,1
    addi    x0,x0 ,1
    mul     x26, x27, x26
    bne     x28, x0, LOOP
    ebreak

