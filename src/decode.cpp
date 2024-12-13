#include "rv32i.hpp"
#include "cpu.hpp"
#include <cstdint>


//TODO: DECODE FULLY (FUNC)
Instr decode(reg_t instr_)
{
    Instr instr{};
    Opcode opcode = getOpcode(instr_);
    switch (opcode)
    {
        case Opcode::Imm:
            {
                instr.funct3 = getfunct3(instr_);
                instr.imm    = I::getImm(instr_);
                if(static_cast<I::Imm::funct3>(instr.funct3) == I::Imm::funct3::SRLI && instr.imm >> 5)
                {
                    instr.funct3 = static_cast<uint8_t>(I::Imm::funct3::SRAI);
                }
                instr.rd_id  = getRdId(instr_);
                instr.rs1_id = getRs1Id(instr_);
                instr.exec   = executeImmFuncs[instr.funct3];
                break;
            }
        case Opcode::Op:
            {
                instr.funct3 = getfunct3(instr_);
                instr.funct7 = getfunct7(instr_);
                if(instr.funct7)
                {
                    if(static_cast<R::Op::funct3>(instr.funct3) == R::Op::funct3::ADD)
                    {
                        instr.funct3 = static_cast<uint8_t>(R::Op::funct3::SUB);
                    }
                    else if(static_cast<R::Op::funct3>(instr.funct3) == R::Op::funct3::SRL)
                    {
                        instr.funct3 = static_cast<uint8_t>(R::Op::funct3::SRA);
                    }
                }
                instr.rd_id  = getRdId(instr_);
                instr.rs1_id = getRs1Id(instr_);
                instr.rs2_id = getRs2Id(instr_);
                instr.exec   = executeOpFuncs[instr.funct3];
                break;
            }
        case Opcode::Branch:
            {
                instr.funct3 = getfunct3(instr_);
                instr.imm    = B::getImm(instr_);
                instr.rs1_id = getRs1Id(instr_);
                instr.rs2_id = getRs2Id(instr_);
                instr.exec   = executeBranchFuncs[instr.funct3];
                break;
            }
        case Opcode::Load:
            {
                instr.funct3 = getfunct3(instr_);
                instr.imm    = I::getImm(instr_);
                instr.rd_id  = getRdId(instr_);
                instr.rs1_id = getRs1Id(instr_);
                instr.exec   = executeLoadFuncs[instr.funct3];
                break;
            }
        case Opcode::Store:
            {
                instr.funct3 = getfunct3(instr_);
                instr.imm    = S::getImm(instr_);
                instr.rs1_id = getRs1Id(instr_);
                instr.rs2_id = getRs2Id(instr_);
                instr.exec   = executeStoreFuncs[instr.funct3];
                break;
            }
        case Opcode::System:
            {
                instr.imm  = I::getImm(instr_);
                instr.funct3 = instr.imm > 0;
                instr.exec = executeSystemFuncs[instr.funct3];
                break;
            }
        case Opcode::Lui:
            {
                instr.imm   = U::getImm(instr_);
                instr.rd_id = getRdId(instr_);
                instr.exec  = executeLui;
                break;
            }
        case Opcode::Auipc:
            {
                instr.imm   = U::getImm(instr_);
                instr.rd_id = getRdId(instr_);
                instr.exec  = executeAuipc;
                break;
            }
        case Opcode::Jalr:
            {
                instr.imm    = I::getImm(instr_);
                instr.rd_id  = getRdId(instr_);
                instr.rs1_id = getRs1Id(instr_);
                instr.exec   = executeJalr;
                break;
            }
        case Opcode::Jal:
            {
                instr.imm   = J::getImm(instr_);
                instr.rd_id = getRdId(instr_);
                instr.exec  = executeJal;
                break;
            }
        case Opcode::Fence:
            {
                instr.exec = executeFence;
                break;
            }
        // TODO: DEAL WITH ERROR
        default: {}
    }

    instr.opcode = opcode;
    instr.size = RV32I_INTR_SIZE;
    return instr;
}

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

imm_t I::getImm(reg_t instr)
{
    return (instr >> 20);
}

imm_t B::getImm(reg_t instr)
{
    imm_t imm = ((instr >> 20) & 0xfffff000) + ((instr << 4) & 0x00000800) +
                ((instr >> 20) & 0x000007e0) + ((instr >> 7) & 0x0000001e);
    return imm;
}

imm_t S::getImm(reg_t instr)
{
    imm_t imm = ((instr >> 20) & 0xffffffe0) + ((instr >> 7) & 0x0000001e);
    return imm;
}

imm_t U::getImm(reg_t instr)
{
    imm_t imm = (instr & 0xfffff000) >> 12;
    return imm;
}

imm_t J::getImm(reg_t instr)
{
    imm_t imm = ((instr >> 12) & 0xfff00000) + (instr & 0x000ff000) +
                ((instr >> 9) & 0x00000800) +  ((instr >> 20) & 0x000007fe);
    return imm;
}

