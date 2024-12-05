.global main

.section .rodata
buf:    .asciz  "Hello RISC-V\n\0"
buf_sz: .word   14

.section .text
main:
    li a0, 1            #first arg - stdout
    la a1, buf          #second arg - address to the buffer
    lw a2, buf_sz       #third arg - buffer size
    li a7, 64           #write syscall
    ecall
    li a0, 0xfe         #ret value
    li a7, 93           #exit syscall
    ecall
