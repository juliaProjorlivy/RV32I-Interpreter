.global main
.section .text
main:
    li      x26, 0
    li      x28, 6
    li      x27, 10
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
    div     x25, x27, x26
    bne     x28, x0, LOOP
    ebreak

