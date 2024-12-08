.global main

.section .rodata
buf_sz: .word   1
.section .data
buf:    .asciz " "

.section .text
main:
    addi    x28, x0, 10
    addi    x29, x0, 65
    la      x30, buf
print:
    sb      x29, 0(x30)
    addi    x29, x29, 1
    addi    x28, x28, -1
    li a0, 1            #first arg - stdout
    la a1, buf          #second arg - address to the buffer
    lw a2, buf_sz       #third arg - buffer size
    li a7, 64           #write syscall
    ecall
    addi    x0, x0, 1
    addi    x0, x0, 1
    addi    x0, x0, 1
    addi    x0, x0, 1
    addi    x0, x0, 1
    bne     x0, x28, print
    li a0, 0xfe         #ret value
    li a7, 93           #exit syscall
    ecall

