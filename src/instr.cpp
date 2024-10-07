#include "rv32i.hpp"
#include <cstdint>

//TODO: GET RID OF MAGIC NUMBERS
Opcode getOpcode(reg_t instr)
{
    return (Opcode)(instr & 0b01111111);
}

uint8_t getfunct3(reg_t instr)
{
    return ((instr >> 12) & 0b111);
}

uint8_t getfunct7(reg_t instr)
{
    return ((instr >> 30) & 1);
}

int getRdId(reg_t instr)
{
    return (instr >> 7) & regsize;
}

int getRs1Id(reg_t instr)
{
    return (instr >> 15) & regsize;
}

int getRs2Id(reg_t instr)
{
    return (instr >> 20) & regsize;
}

Instr decode(reg_t &instr_)
{
    Instr instr{};
    Opcode opcode = getOpcode(instr_);
    switch (opcode)
    {
        //TODO: WRITE THE REST
        case Opcode::Imm:
            {
                instr.funct3 = getfunct3(instr_);
                instr.imm    = I::getImm(instr_);
                instr.rd_id  = getRdId(instr_);
                instr.rs1_id = getRs1Id(instr_);
                instr.exec = executeImm;
                break;
            }
        case Opcode::Op:
            {
                instr.funct3 = getfunct3(instr_);
                instr.rd_id  = getRdId(instr_);
                instr.rs1_id = getRs1Id(instr_);
                instr.rs2_id = getRs2Id(instr_);
                instr.exec = executeOp;
                break;
            }
        case Opcode::Branch:
            {
                instr.funct3 = getfunct3(instr_);
                instr.imm    = B::getImm(instr_);
                instr.rs1_id = getRs1Id(instr_);
                instr.rs2_id = getRs2Id(instr_);
                instr.exec = executeBranch;
                break;
            }
        case Opcode::System:
            {
                instr.imm = I::getImm(instr_);
                instr.exec = executeSystem;
                break;
            }
    }

    instr.opcode = opcode;
    return instr;
}

imm_t I::getImm(reg_t instr)
{
    return (instr >> 20);
}

imm_t B::getImm(reg_t instr)
{
    imm_t imm = ((instr >> 8) & 0b1111) + ((instr >> 21) & 0b1111110000) +
                ((instr << 10) & 0b10000000000) +
                ((instr >> 31) & 0b100000000000);
    return imm;
}


