#include "rv32i.hpp"
#include "cpu.hpp"
#include <cstddef>
#include <cstdint>
#include <functional>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <vector>

void interpret_block(Cpu &cpu, std::vector<Instr> &instrs)
{
    auto instr = instrs.begin();
    for(; instr != (instrs.end() - 1); ++instr)
    {
        if(instr->opcode == Opcode::Auipc)
        {
            addr_t pc_offset = (instr - instrs.begin()) * RV32I_INTR_SIZE;
            cpu.advanceNextPc(pc_offset);
            instr->exec(cpu, *instr);
            cpu.setNextPc(cpu.getPc());
        }
        else
        {
            instr->exec(cpu, *instr);
        }
    }

    //Last branch/jal/jalr instruction
    cpu.advanceNextPc((instrs.size() - 1) * RV32I_INTR_SIZE);
    instr->exec(cpu, *instr);

    //If branch/jump happened
    if(cpu.getNextPc() != cpu.getPc())
    {
        cpu.setPc(cpu.getNextPc());
    }
    //If no branch
    else
    {
        cpu.advancePc(RV32I_INTR_SIZE);
    }
}

//TODO: IS USED ONLY IN TESTS -> GET RID OF
void execute (Cpu &cpu, Instr &instr)
{
    instr.exec(cpu, instr);
}

        // ADDI  = 0b0000,
        // SLLI  = 0b0001,
        // SLTI  = 0b0010,
        // SLTIU = 0b0011,
        // XORI  = 0b0100,
        // SRLI  = 0b0101,
        // ORI   = 0b0110,
        // ANDI  = 0b0111,
        // SRAI  = 0b1000,
void executeImm (Cpu &cpu, Instr &instr)
{
    using namespace I::Imm;
    switch (static_cast<funct3>(instr.funct3))
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
                cpu.setReg(instr.rd_id, static_cast<std::uint32_t>(cpu.getReg(instr.rs1_id)) < static_cast<std::uint32_t>(instr.imm));
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
                    cpu.setReg(instr.rd_id, static_cast<std::uint32_t>(cpu.getReg(instr.rs1_id)) >> instr.imm);
                }
                //SRLI
                else
                {
                    cpu.setReg(instr.rd_id, static_cast<std::uint32_t>(cpu.getReg(instr.rs1_id)) >> instr.imm);
                }
                break;
            }
        //TODO: THROW AN ERROR
        default: {}
    }
    // cpu.advancePc();
}

        // ADD  = 0b0000,
        // SLL  = 0b0001,
        // SLT  = 0b0010,
        // SLTU = 0b0011,
        // XOR  = 0b0100,
        // SRL  = 0b0101,
        // OR   = 0b0110,
        // AND  = 0b0111,
        // SUB  = 0b1000,
        // SRA  = 0b1001,

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
                cpu.setReg(instr.rd_id, static_cast<uint32_t>(cpu.getReg(instr.rs1_id)) < static_cast<uint32_t>(cpu.getReg(instr.rs2_id)));
                break;
            }
        case funct3::SLL:
            {
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
                    cpu.setReg(instr.rd_id, static_cast<uint32_t>(cpu.getReg(instr.rs1_id)) >> cpu.getReg(instr.rs2_id));
                }
                break;
            }
        //TODO: THROW AND ERROR
        default: {}
    }
    // cpu.advancePc();
}

        // BEQ  = 0b000,
        // BNE  = 0b001,
        // BLT  = 0b100,
        // BGE  = 0b101,
        // BLTU = 0b110,
        // BGEU = 0b111,
void executeBranch (Cpu &cpu, Instr &instr)
{
    //TODO: NEXT_PC
    using namespace B::Branch;
    switch ((funct3)instr.funct3)
    {
        case funct3::BEQ:
            {
                // if (cpu.getReg(instr.rs1_id) == cpu.getReg(instr.rs2_id)) {cpu.advancePc(instr.imm);}
                // else {cpu.advancePc();}
                if (cpu.getReg(instr.rs1_id) == cpu.getReg(instr.rs2_id)) {cpu.advanceNextPc(instr.imm);}
                // else {cpu.advancePc();}
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
                if (static_cast<uint32_t>(cpu.getReg(instr.rs1_id)) < static_cast<uint32_t>(cpu.getReg(instr.rs2_id))) {cpu.advancePc(instr.imm);}
                else {cpu.advancePc();}
                return;
            }
        case funct3::BGEU:
            {
                if (static_cast<uint32_t>(cpu.getReg(instr.rs1_id)) > static_cast<uint32_t>(cpu.getReg(instr.rs2_id))) {cpu.advancePc(instr.imm);}
                else {cpu.advancePc();}
                return;
            }
    }
}

        // LB  = 0b000
        // LH  = 0b001
        // LW  = 0b010
        // LBU = 0b100
        // LHU = 0b101
     
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
                cpu.setReg(instr.rd_id, (0x000000ff & cpu.load<byte_t>(instr.imm + cpu.getReg(instr.rs1_id))));
                break;
            }
        case funct3::LHU:
            {
                cpu.setReg(instr.rd_id, (0x0000ffff & cpu.load<half_t>(instr.imm + cpu.getReg(instr.rs1_id))));
                break;
            }
    }
    // cpu.advancePc();
}

        // SB = 0b000
        // SH = 0b001
        // SW = 0b010

//TODO: CONST
//TODO: CLANG FORMAT
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
}

void executeLui(Cpu &cpu, Instr &instr)
{
    cpu.setReg(instr.rd_id, (instr.imm << 12));
}

void executeAuipc(Cpu &cpu, Instr &instr)
{
    cpu.setReg(instr.rd_id, cpu.getNextPc() + (instr.imm << 12));
}

void executeJalr(Cpu &cpu, Instr &instr)
{
    cpu.setReg(instr.rd_id, cpu.getNextPc() + instr.size);
    imm_t target_addr = (cpu.getReg(instr.rs1_id) + instr.imm) & 0xfffffffe; //least-significant bit to zero
    cpu.setNextPc(target_addr);
}

void executeJal(Cpu &cpu, Instr &instr)
{
    cpu.setReg(instr.rd_id, cpu.getNextPc() + instr.size);
    cpu.advanceNextPc(instr.imm);
}

void executeEbreak(Cpu &cpu,[[maybe_unused]] Instr &instr) {cpu.setDone();}
void executeEcall(Cpu &cpu,[[maybe_unused]] Instr &instr)
{
    switch (static_cast<Syscall::rv>(cpu.getReg(17)))
    {
        case Syscall::rv::READ:
            {
                int fd = cpu.getReg(10);
                size_t buf_size = cpu.getReg(12);
                addr_t start_buf = cpu.getReg(11);

                std::vector<byte_t> buf = {};
                buf.reserve(buf_size);
                ssize_t ret_val = read(fd, static_cast<void *>(buf.data()), buf_size);

                for(int i = 0; i < buf_size; i++)
                {
                    cpu.store<byte_t>(start_buf + i * sizeof(byte_t), buf[i]);
                }

                cpu.setReg(1, ret_val);
                break;
            }
        case Syscall::rv::WRITE:
            {
                size_t buf_size = cpu.getReg(12);
                addr_t start_buf = cpu.getReg(11);

                std::vector<byte_t> buf = {};

                for(int i = 0; i < buf_size; i++)
                {
                    buf.push_back(cpu.load<byte_t>(start_buf + i * sizeof(byte_t)));
                }
                ssize_t ret_val = write(cpu.getReg(10), static_cast<void *>(buf.data()), buf_size);

                cpu.setReg(1, ret_val);
                break;
            }
        case Syscall::rv::EXIT:
            {
                //TODO: RETURN VALUE
                reg_t exit_code = cpu.getReg(10);
                cpu.setDone();
                break;
            }
        case Syscall::rv::MMAP:
            {

            }
        case Syscall::rv::CLOSE:
            {
                int fd = cpu.getReg(10);
                int ret_val = close(fd);
                cpu.setReg(1, ret_val);
                break;
            }
        case Syscall::rv::LSEEK:
            {
                int fd = cpu.getReg(10);
                __off_t offset = cpu.getReg(11);
                int whence = cpu.getReg(12);
                int ret_val = lseek(fd, offset, whence);
                cpu.setReg(1, ret_val);
                break;
            }
        default: {}
    }
}

void executeFence([[maybe_unused]] Cpu &cpu,[[maybe_unused]] Instr &instr) {}

