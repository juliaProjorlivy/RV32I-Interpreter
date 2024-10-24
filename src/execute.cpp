#include "rv32i.hpp"
#include "cpu.hpp"

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
                cpu.setReg(instr.rd_id, cpu.getReg(instr.rs1_id) + instr.imm);
                break;
            }
        case funct3::ANDI:
            {
                cpu.setReg(instr.rd_id, cpu.getReg(instr.rs1_id) & instr.imm);
                break;
            }
        case funct3::ORI:
            {
                cpu.setReg(instr.rd_id, cpu.getReg(instr.rs1_id) | instr.imm);
                break;
            }
        case funct3::XORI:
            {
                cpu.setReg(instr.rd_id, cpu.getReg(instr.rs1_id) ^ instr.imm);
                break;
            }
        case funct3::SLTI:
            {
                cpu.setReg(instr.rd_id, cpu.getReg(instr.rs1_id) < instr.imm);
                break;
            }
        case funct3::SLTIU:
            {
                cpu.setReg(instr.rd_id, (std::uint32_t)cpu.getReg(instr.rs1_id) < (std::uint32_t)instr.imm);
                break;
            }
        case funct3::SLLI:
            {
                instr.imm = instr.imm & 0b11111;
                cpu.setReg(instr.rd_id, cpu.getReg(instr.rs1_id) << instr.imm);
                break;
            }
        case funct3::SRLI:
            {
                instr.imm = instr.imm & 0b11111;
                //SRAI:
                if (instr.imm >> 5)
                {
                    cpu.setReg(instr.rd_id, (std::uint32_t)cpu.getReg(instr.rs1_id) >> instr.imm);
                }
                //SRLI
                else
                {
                    cpu.setReg(instr.rd_id, (std::uint32_t)cpu.getReg(instr.rs1_id) >> instr.imm);
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
                    cpu.setReg(instr.rd_id, cpu.getReg(instr.rs1_id) - cpu.getReg(instr.rs2_id));
                }
                //ADD
                else
                {
                    cpu.setReg(instr.rd_id, cpu.getReg(instr.rs1_id) + cpu.getReg(instr.rs2_id));
                }
                break;
            }
        case funct3::AND:
            {
                cpu.setReg(instr.rd_id, cpu.getReg(instr.rs1_id) & cpu.getReg(instr.rs2_id));
                break;
            }
        case funct3::OR:
            {
                cpu.setReg(instr.rd_id, cpu.getReg(instr.rs1_id) | cpu.getReg(instr.rs2_id));
                break;
            }
        case funct3::XOR:
            {
                cpu.setReg(instr.rd_id, cpu.getReg(instr.rs1_id) ^ cpu.getReg(instr.rs2_id));
                break;
            }
        case funct3::SLT:
            {
                cpu.setReg(instr.rd_id, cpu.getReg(instr.rs1_id) < cpu.getReg(instr.rs2_id));
                break;
            }
        case funct3::SLTU:
            {
                cpu.setReg(instr.rd_id, (std::uint32_t)cpu.getReg(instr.rs1_id) < (std::uint32_t)cpu.getReg(instr.rs2_id));
                break;
            }
        case funct3::SLL:
            {
                //if SLTIU
                instr.rs2_id = instr.rs2_id & 0b11111;
                cpu.setReg(instr.rd_id, cpu.getReg(instr.rs1_id) << cpu.getReg(instr.rs2_id));
                break;
            }
        case funct3::SRL:
            {
                instr.rs2_id = instr.rs2_id & 0b11111;
                //SRA
                if (instr.funct7)
                {
                    cpu.setReg(instr.rd_id, cpu.getReg(instr.rs1_id) >> cpu.getReg(instr.rs2_id));
                }
                else
                {
                    cpu.setReg(instr.rd_id, (std::uint32_t)cpu.getReg(instr.rs1_id) >> cpu.getReg(instr.rs2_id));
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
    switch ((funct3)instr.funct3)
    {
        case funct3::BEQ:
            {
                if (cpu.getReg(instr.rs1_id) == cpu.getReg(instr.rs2_id)) {cpu.advancePc(instr.imm);}
                else {cpu.advancePc();}
                return;
            }
        case funct3::BNE:
            {
                if (cpu.getReg(instr.rs1_id) != cpu.getReg(instr.rs2_id)) {cpu.advancePc(instr.imm);}
                else {cpu.advancePc();}
                return;
            }
        case funct3::BLT:
            {
                if (cpu.getReg(instr.rs1_id) < cpu.getReg(instr.rs2_id)) {cpu.advancePc(instr.imm);}
                else {cpu.advancePc();}
                return;
            }
        case funct3::BGE:
            {
                if (cpu.getReg(instr.rs1_id) > cpu.getReg(instr.rs2_id)) {cpu.advancePc(instr.imm);}
                else {cpu.advancePc();}
                return;
            }
        case funct3::BLTU:
            {
                if ((addr_t)cpu.getReg(instr.rs1_id) < (addr_t)cpu.getReg(instr.rs2_id)) {cpu.advancePc(instr.imm);}
                else {cpu.advancePc();}
                return;
            }
        case funct3::BGEU:
            {
                if ((addr_t)cpu.getReg(instr.rs1_id) > (addr_t)cpu.getReg(instr.rs2_id)) {cpu.advancePc(instr.imm);}
                else {cpu.advancePc();}
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
                cpu.setReg(instr.rd_id, cpu.load<byte_t>(instr.imm + cpu.getReg(instr.rs1_id)));
                break;
            }
        case funct3::LH:
            {
                cpu.setReg(instr.rd_id, cpu.load<half_t>(instr.imm + cpu.getReg(instr.rs1_id)));
                break;
            }
        case funct3::LW:
            {
                cpu.setReg(instr.rd_id, cpu.load<word_t>(instr.imm + cpu.getReg(instr.rs1_id)));
                break;
            }
        case funct3::LBU:
            {
                cpu.setReg(instr.rd_id, (0x0000ffff & cpu.load<word_t>(instr.imm + cpu.getReg(instr.rs1_id))));
                break;
            }
        case funct3::LHU:
            {
                cpu.setReg(instr.rd_id, (0x0000ffff & cpu.load<word_t>(instr.imm + cpu.getReg(instr.rs1_id))));
                break;
            }
        case funct3::LWU:
            {
                cpu.setReg(instr.rd_id, (0x0000ffff & cpu.load<word_t>(instr.imm + cpu.getReg(instr.rs1_id))));
                break;
            }
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
                cpu.store<byte_t>((addr_t)(instr.imm + cpu.getReg(instr.rs1_id)), cpu.getReg(instr.rs2_id));
                break;
            }
        case funct3::SH:
            {
                cpu.store<half_t>((addr_t)(instr.imm + cpu.getReg(instr.rs1_id)), cpu.getReg(instr.rs2_id));
                break;
            }
        case funct3::SW:
            {
                cpu.store<word_t>((addr_t)(instr.imm + cpu.getReg(instr.rs1_id)), cpu.getReg(instr.rs2_id));
                break;
            }
    }
    cpu.advancePc();
}

void executeLui(Cpu &cpu, Instr &instr)
{
    cpu.setReg(instr.rd_id, instr.imm);
    cpu.advancePc();
}

void executeAuipc(Cpu &cpu, Instr &instr)
{
    cpu.advancePc(instr.imm);
    cpu.setReg(instr.rd_id, cpu.getPc());
}

void executeJalr(Cpu &cpu, Instr &instr)
{
    imm_t target_addr = (cpu.getReg(instr.rs1_id) + instr.imm) & 0xfffffffe; //least-significant bit to zero
    cpu.setReg(instr.rd_id, cpu.getPc() + sizeof(addr_t));
    cpu.setPc(target_addr);
}

void executeJal(Cpu &cpu, Instr &instr)
{
    cpu.setReg(instr.rd_id, cpu.getPc() + sizeof(addr_t));
    cpu.advancePc(instr.imm);
}
void executeSystem(Cpu &cpu, Instr &instr)
{
    (void)instr;
    cpu.setDone();
}

