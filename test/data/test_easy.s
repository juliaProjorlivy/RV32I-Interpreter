.global main
.section .text
main:
    addi    x28, x0, 6
LOOP:
    addi    x28,x28 , -1
    addi    x29, x0, 29
    addi    x0,x0 ,1
    addi    x0,x0 ,1
    addi    x0,x0 ,1
    addi    x0,x0 ,1
    addi    x0,x0 ,1
    addi    x0,x0 ,1
    addi    x0,x0 ,1
    addi    x0,x0 ,1
    addi    x0,x0 ,1
    bne     x28, x0, LOOP
    ebreak

