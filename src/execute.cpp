#include "asmjit/core/compiler.h"
#include "rv32i.hpp"
#include "cpu.hpp"
#include <cstddef>
#include <cstdint>
#include <cstdio>
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
        //auipc uses current pc
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
    cpu.advancePc((instrs.size() - 1) * RV32I_INTR_SIZE);
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
                addr_t start_buf = cpu.getReg(11);
                size_t buf_size = cpu.getReg(12);

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
                int fd = cpu.getReg(10);
                addr_t start_buf = cpu.getReg(11);
                size_t buf_size = cpu.getReg(12);

                std::vector<byte_t> buf = {};

                for(int i = 0; i < buf_size; i++)
                {
                    buf.push_back(cpu.load<byte_t>(start_buf + i * sizeof(byte_t)));
                }
                ssize_t ret_val = write(fd, static_cast<void *>(buf.data()), buf_size);

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
        case Syscall::rv::CLOSE:
            {
                int fd = cpu.getReg(10);
                int ret_val = close(fd);
                cpu.setReg(1, ret_val);
                break;
            }
        default: {}
    }
}

void executeFence([[maybe_unused]] Cpu &cpu,[[maybe_unused]] Instr &instr) {}

