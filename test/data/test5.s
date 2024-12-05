.global main

.section .rodata
buf:    .asciz  "Hello RISC-V\n\0"
buf_sz: .dword   14

.section .text
main:
    addi    x10, x0, 1
    auipc   x11, buf[31:12]
    addi    x11, x11, buf[11:0]
    addi    x12, x0, 14
    addi    x17, x0, 64
    ecall
    addi    x10, x0, 0xfe
    addi    x17, x0, 93
    ecall
    ebreak
