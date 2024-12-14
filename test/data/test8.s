.global main

.section .rodata
buf:    .asciz  "Hello RISC-V\n\0"
buf_sz: .word   14

.section .text
main:
    addi x20, x0, 10
    addi x19, x0, 1
print:
    sub x20, x20, x19
    li a0, 1            #first arg - stdout
    la a1, buf          #second arg - address to the buffer
    lw a2, buf_sz       #third arg - buffer size
    li a7, 64           #write syscall
    ecall
    addi x0, x0, 0
    addi x0, x0, 0
    addi x0, x0, 0
    addi x0, x0, 0
    addi x0, x0, 0
    addi x0, x0, 0
    bne x20, x0, print
    li a0, 0xfe         #ret value
    li a7, 93           #exit syscall
    ecall
    ebreak
