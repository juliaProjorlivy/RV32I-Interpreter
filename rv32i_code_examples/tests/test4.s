.global main
.section .text
main:
    addi    x15,x0, 10
    addi    x16,x0, 1
    beq     x15,x0, .L0
    beq     x15,x16,.L1
    addi    x18,x0, 1
    add     x16,x0, x0
    addi    x17,x0, 1
.L_ALG:
    add     x14, x17, x16
    add     x16,x17, x0
    add     x17,x14, x0
    addi    x18,x18, 1
    bne     x15, x18, .L_ALG
    jal     x0, .L_END

.L0:
    addi    x14,x0, 0
    jal     x0, .L_END
.L1:
    addi    x14,x0, 1
    jal     x0, .L_END
.L_END:
    ebreak

