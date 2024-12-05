.global main

.section .rodata
buf_sz: .word   14
.section .data
buf:    .asciz

.section .text
main:
    li a0, 0            #first arg - stdin
    la a1, buf          #second arg - address to the buffer
    lw a2, buf_sz       #third arg - buffer size
    li a7, 63           #read syscall
    ecall
    li a0, 1            #first arg - stdout
    la a1, buf          #second arg - address to the buffer
    lw a2, buf_sz       #third arg - buffer size
    li a7, 64           #write syscall
    ecall
    li a0, 0xfe         #ret value
    li a7, 93           #exit syscall
    ecall
