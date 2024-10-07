#include "rv32i.hpp"

void execute (Cpu &cpu, Instr &instr)
{
    instr.exec(cpu, instr);
}

void executeImm (Cpu &cpu, Instr &instr)
{
    using namespace I::Imm;
    switch ((funct3)instr.funct3)
    {
        case funct3::ADDI:
            {
                cpu.regs[instr.rd_id] = cpu.regs[instr.rs1_id] + instr.imm;
                break;
            }
        case funct3::ANDI:
            {
                cpu.regs[instr.rd_id] = cpu.regs[instr.rs1_id] & instr.imm;
                break;
            }
        case funct3::ORI:
            {
                cpu.regs[instr.rd_id] = cpu.regs[instr.rs1_id] | instr.imm;
                break;
            }
        case funct3::XORI:
            {
                cpu.regs[instr.rd_id] = cpu.regs[instr.rs1_id] ^ instr.imm;
                break;
            }
        case funct3::SLTI:
            {
                cpu.regs[instr.rd_id] = cpu.regs[instr.rs1_id] < instr.imm;
                break;
            }
        case funct3::SLTIU:
            {
                cpu.regs[instr.rd_id] = cpu.regs[instr.rs1_id] < (std::uint32_t)instr.imm;
                break;
            }
        case funct3::SLLI:
            {
                instr.imm = instr.imm & 0b11111;
                cpu.regs[instr.rd_id] = cpu.regs[instr.rs1_id] << instr.imm;
                break;
            }
        case funct3::SRLI:
            {
                instr.imm = instr.imm & 0b11111;
                //SRAI:
                if (instr.imm >> 5)
                {
                    cpu.regs[instr.rd_id] = (std::uint32_t)cpu.regs[instr.rs1_id] >> instr.imm;
                }
                //SRLI
                else
                {
                    cpu.regs[instr.rd_id] = (std::uint32_t)cpu.regs[instr.rs1_id] >> instr.imm;
                }
                break;
            }
        //TODO: THROW AND ERROR
        default: {}
    }
    cpu.advancePc();
}

void executeOp (Cpu &cpu, Instr &instr)
{
    using namespace R::Op;
    switch ((funct3)instr.funct3)
    {
        case funct3::ADD:
            {
                //SUB
                if  (instr.funct7)
                {
                    cpu.regs[instr.rd_id] = cpu.regs[instr.rs1_id] - cpu.regs[instr.rs2_id];
                }
                //ADD
                else
            {
                    cpu.regs[instr.rd_id] = cpu.regs[instr.rs1_id] + cpu.regs[instr.rs2_id];
                }
                break;
            }
        case funct3::AND:
            {
                cpu.regs[instr.rd_id] = cpu.regs[instr.rs1_id] & cpu.regs[instr.rs2_id];
                break;
            }
        case funct3::OR:
            {
                cpu.regs[instr.rd_id] = cpu.regs[instr.rs1_id] | cpu.regs[instr.rs2_id];
                break;
            }
        case funct3::XOR:
            {
                cpu.regs[instr.rd_id] = cpu.regs[instr.rs1_id] ^ cpu.regs[instr.rs2_id];
                break;
            }
        case funct3::SLT:
            {
                cpu.regs[instr.rd_id] = cpu.regs[instr.rs1_id] < cpu.regs[instr.rs2_id];
                break;
            }
        case funct3::SLTU:
            {
                cpu.regs[instr.rd_id] = cpu.regs[instr.rs1_id] < (std::uint32_t)instr.rs2_id;
                break;
            }
        case funct3::SLL:
            {
                //if SLTIU
                instr.rs2_id = instr.rs2_id & 0b11111;
                cpu.regs[instr.rd_id] = cpu.regs[instr.rs1_id] << instr.rs2_id;
                break;
            }
        case funct3::SRL:
            {
                instr.rs2_id = instr.rs2_id & 0b11111;
                //SRA
                if (instr.funct7) 
                {
                    cpu.regs[instr.rd_id] = cpu.regs[instr.rs1_id] >> instr.rs2_id;
                }
                else
                {
                    cpu.regs[instr.rd_id] = (std::uint32_t)cpu.regs[instr.rs1_id] >> instr.rs2_id;
                }
                break;
            }
        //TODO: THROW AND ERROR
        default: {}
    }
    cpu.advancePc();
}

void executeBranch (Cpu &cpu, Instr &instr)
{
    using namespace B::Branch;
    //TODO: WRITE THE REST
    switch ((funct3)instr.funct3)
    {
        case funct3::BEQ:
            {
                if (cpu.regs[instr.rs1_id] == cpu.regs[instr.rs2_id]) {cpu.pc += instr.imm;}
                return;
            }
        case funct3::BNE:
            {
                if (cpu.regs[instr.rs1_id] != cpu.regs[instr.rs2_id]) {cpu.pc += instr.imm;}
                return;
            }
    }
}

void executeLoad (Cpu &cpu, Instr &instr)
{
    using namespace I::Load;
    switch ((funct3)instr.funct3)
    {
        case funct3::LB:
            {
                cpu.regs[instr.rd_id] = cpu.mem->load(instr.imm + cpu.regs[instr.rs1_id]);
                break;
            }
        // case funct3::LH:
        //     {
        //         cpu.regs[instr.rd_id] = cpu.mem->load(instr.imm + cpu.regs[instr.rs1_id], sizeof(half_t));
        //         break;
        //     }
        // case funct3::LW:
        //     {
        //         cpu.regs[instr.rd_id] = cpu.mem->load(instr.imm + cpu.regs[instr.rs1_id], sizeof(word_t));
        //         break;
        //     }
        //TODO: WRITE THE REST
    }
    cpu.advancePc();
}

void executeStore (Cpu &cpu, Instr &instr)
{
    using namespace S::Store;
    switch ((funct3)instr.funct3)
    {
        case funct3::SB:
            {
                cpu.mem->store(instr.imm + cpu.regs[instr.rs1_id], cpu.regs[instr.rs2_id]);
                break;
            }
        // case funct3::SH:
        //     {
        //         cpu.mem->store(instr.imm + cpu.regs[instr.rs1_id], cpu.regs[instr.rs2_id], sizeof(half_t));
        //         break;
        //     }
        // case funct3::SW:
        //     {
        //         cpu.mem->store(instr.imm + cpu.regs[instr.rs1_id], cpu.regs[instr.rs2_id], sizeof(word_t));
        //         break;
        //     }
        //TODO: WRITE THE REST
    }
    cpu.advancePc();
}

void executeSystem(Cpu &cpu, Instr &instr)
{
    cpu.done = 1;
}

