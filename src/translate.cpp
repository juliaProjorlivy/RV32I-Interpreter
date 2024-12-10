#include "asmjit/core/codeholder.h"
#include "asmjit/core/compiler.h"
#include "asmjit/core/func.h"
#include "asmjit/core/logger.h"
#include "asmjit/x86/x86compiler.h"
#include "asmjit/x86/x86operand.h"
#include "cpu.hpp"
#include "rv32i.hpp"
#include <cstdint>

bool is_bb_end(Instr &instr)
{
    switch (instr.opcode)
    {
        case Opcode::Branch:
        case Opcode::Jal:
        case Opcode::Jalr:
        case Opcode::System:
            return true;
        default:
            return false;
    }
    return false;
}

std::vector<Instr> lookup(Cpu &cpu, addr_t addr)
{
    auto basic_block_res = cpu.bb_cache.find(addr);

    //if bb was not found -> update bb_cache
    if(basic_block_res == cpu.bb_cache.end())
    {
        Instr cur_instr {};
        addr_t cur_addr = addr;
        std::vector<Instr> bb;
        bb.reserve(BB_AVERAGE_SIZE);

        do
        {
            reg_t command = cpu.fetch(cur_addr);
            cur_instr = decode(command);
            bb.push_back(cur_instr);
            cur_addr += sizeof(addr_t);
        } while (!is_bb_end(cur_instr));

        basic_block_res = cpu.bb_cache.emplace(addr, bb).first;
    }

    return basic_block_res->second;
}

asmjit::x86::Mem toDwordPtr(Register &reg)
{
    return asmjit::x86::dword_ptr((uint64_t)(&(reg.self_->val_)));
}

void translateOp(Instr &instr, TranslationAttr &attr)
{
    switch (static_cast<R::Op::funct3>(instr.funct3))
    {
        case R::Op::funct3::ADD:
            {
                //SUB
                if (instr.funct7)
                {
                    attr.cc.sub(attr.dst1, attr.dst2);
                }
                //ADD
                else
                {
                    attr.cc.add(attr.dst1, attr.dst2);
                }
                break;
            }
        case R::Op::funct3::AND:
            {
                attr.cc.and_(attr.dst1, attr.dst2);
                break;
            }
        case R::Op::funct3::OR:
            {
                attr.cc.or_(attr.dst1, attr.dst2);
                break;
            }
        case R::Op::funct3::XOR:
            {
                attr.cc.xor_(attr.dst1, attr.dst2);
                break;
            }
        case R::Op::funct3::SLT:
            {
                attr.cc.cmp(attr.dst1, attr.dst2);
                attr.cc.setl(attr.dst1);
                break;
            }
        case R::Op::funct3::SLTU:
            {
                attr.cc.cmp(attr.dst1, attr.dst2);
                attr.cc.setb(attr.dst1);
                break;
            }
        case R::Op::funct3::SLL:
            {
                attr.cc.shl(attr.dst1, attr.dst2);
                break;
            }
        case R::Op::funct3::SRL:
            {
                //SRA
                if (instr.funct7)
                {
                    attr.cc.sar(attr.dst1, attr.dst2);
                }
                else
                {
                    attr.cc.shr(attr.dst1, attr.dst2);
                }
                break;
            }
        default: {return;}
    }
}

void translateImm(Instr &instr, TranslationAttr &attr)
{
    switch ((I::Imm::funct3)instr.funct3)
    {
        case I::Imm::funct3::ADDI:
            {
                attr.cc.add(attr.dst1, attr.dst2);
                break;
            }
        case I::Imm::funct3::ANDI:
            {
                attr.cc.and_(attr.dst1, attr.dst2);
                break;
            }
        case I::Imm::funct3::ORI:
            {
                attr.cc.or_(attr.dst1, attr.dst2);
                break;
            }
        case I::Imm::funct3::XORI:
            {
                attr.cc.xor_(attr.dst1, attr.dst2);
                break;
            }
        case I::Imm::funct3::SLTI:
            {
                attr.cc.cmp(attr.dst1, attr.dst2);
                attr.cc.setl(attr.dst1);
                break;
            }
        case I::Imm::funct3::SLTIU:
            {
                attr.cc.cmp(attr.dst1, attr.dst2);
                attr.cc.setb(attr.dst1);
                break;
            }
        case I::Imm::funct3::SLLI:
            {
                attr.cc.shl(attr.dst1, attr.dst2);
                break;
            }
        case I::Imm::funct3::SRLI:
            {
                //SRAI:
                if (instr.imm >> 5)
                {
                    attr.cc.sar(attr.dst1, attr.dst2);
                }
                //SRLI
                else
                {
                    attr.cc.shr(attr.dst1, attr.dst2);
                }
                break;
            }
        default: {return;}
        //TODO: THROW AN ERROR
    }
}

void translateBranch(Instr &instr, TranslationAttr &attr)
{
    using namespace B::Branch;
    switch ((B::Branch::funct3)instr.funct3)
    {
        case B::Branch::funct3::BEQ:
            {
                attr.cc.je(*attr.L_BRANCH);
                return;
            }
        case B::Branch::funct3::BNE:
            {
                attr.cc.jne(*attr.L_BRANCH);
                return;
            }
        case B::Branch::funct3::BLT:
            {
                attr.cc.jl(*attr.L_BRANCH);
                return;
            }
        case B::Branch::funct3::BGE:
            {
                attr.cc.jg(*attr.L_BRANCH);
                return;
            }
        case B::Branch::funct3::BLTU:
            {
                attr.cc.jb(*attr.L_BRANCH);
                return;
            }
        case B::Branch::funct3::BGEU:
            {
                attr.cc.ja(*attr.L_BRANCH);
                return;
            }
        default: {return;}
    }
}

template<typename ValueT>
static reg_t LoadWrapper(Cpu *cpu, addr_t addr)
{
    return cpu->load<ValueT>(addr);
}

void translateLoad(Instr &instr, TranslationAttr &attr)
{
    switch ((I::Load::funct3)instr.funct3)
    {
        case I::Load::funct3::LB:
            {
                attr.cc.invoke(attr.invokeNode, (uint64_t)LoadWrapper<byte_t>, asmjit::FuncSignature::build<reg_t, Cpu *, addr_t>());
                break;
            }
        case I::Load::funct3::LH:
            {
                attr.cc.invoke(attr.invokeNode, (uint64_t)LoadWrapper<half_t>, asmjit::FuncSignature::build<reg_t, Cpu *, addr_t>());
                break;
            }
        case I::Load::funct3::LW:
            {
                attr.cc.invoke(attr.invokeNode, (uint64_t)LoadWrapper<word_t>, asmjit::FuncSignature::build<reg_t, Cpu *, addr_t>());
                break;
            }
        case I::Load::funct3::LBU:
            {
                attr.cc.invoke(attr.invokeNode, (uint64_t)LoadWrapper<byte_t>, asmjit::FuncSignature::build<reg_t, Cpu *, addr_t>());
                attr.cc.mov(attr.dst2, 0x000000ff);
                break;
            }
        case I::Load::funct3::LHU:
            {
                attr.cc.invoke(attr.invokeNode, (uint64_t)LoadWrapper<half_t>, asmjit::FuncSignature::build<reg_t, Cpu *, addr_t>());
                attr.cc.mov(attr.dst2, 0x0000ffff);
                break;
            }
    }
}

template<typename StoreT>
static void StoreWrapper(Cpu *cpu, addr_t addr, reg_t val)
{
    cpu->store<StoreT>(addr, val);
}

void translateStore(Instr &instr, TranslationAttr &attr)
{
    switch (static_cast<S::Store::funct3>(instr.funct3))
    {
        case S::Store::funct3::SB:
            {
                attr.cc.invoke(attr.invokeNode, (uint64_t)StoreWrapper<byte_t>, asmjit::FuncSignature::build<void, Cpu *, addr_t, reg_t>());
                break;
            }
        case S::Store::funct3::SH:
            {
                attr.cc.invoke(attr.invokeNode, (uint64_t)StoreWrapper<half_t>, asmjit::FuncSignature::build<void, Cpu *, addr_t, reg_t>());
                break;
            }
        case S::Store::funct3::SW:
            {
                attr.cc.invoke(attr.invokeNode, (uint64_t)StoreWrapper<word_t>, asmjit::FuncSignature::build<void, Cpu *, addr_t, reg_t>());
                break;
            }
    }
}

Cpu::func_t translate(Cpu &cpu, std::vector<Instr> &bb)
{
    asmjit::CodeHolder code;
    code.init(cpu.rt.environment(), cpu.rt.cpuFeatures());

    asmjit::x86::Compiler cc(&code);
    cc.addFunc(asmjit::FuncSignature::build<void>());

    asmjit::FileLogger logger(cpu.output_log);
    code.setLogger(&logger);

    asmjit::x86::Gp dst1 = cc.newGpd();
    asmjit::x86::Gp dst2 = cc.newGpd();
    asmjit::x86::Gp ret = cc.newGpd();

    TranslationAttr attr {cc, dst1, dst2, ret, nullptr, nullptr};
    int pc_offset = 0;

    for(auto instr : bb)
    {
        switch (instr.opcode)
        {
            case Opcode::Imm:
                {
                    if(instr.rd_id == 0)
                    {
                        cc.nop();
                    }
                    else
                    {
                        cc.mov(dst1, toDwordPtr(cpu.regs[instr.rs1_id]));
                        cc.mov(dst2, instr.imm);
                        translateImm(instr, attr);
                        cc.mov( toDwordPtr((cpu.regs[instr.rd_id])), dst1);
                    }
                    pc_offset += instr.size;
                    break;
                }
            case Opcode::Op:
                {
                    if(instr.rd_id == 0)
                    {
                        cc.nop();
                    }
                    else
                    {
                        cc.mov(dst1, toDwordPtr(cpu.regs[instr.rs1_id]));
                        cc.mov(dst2, toDwordPtr(cpu.regs[instr.rs2_id]));
                        translateOp(instr, attr);
                        cc.mov(toDwordPtr(cpu.regs[instr.rd_id]), dst1);
                    }

                    pc_offset += instr.size;
                    break;
                }
            case Opcode::Load:
                {
                    if(instr.rd_id == 0)
                    {
                        cc.nop();
                    }
                    else
                    {
                        cc.mov(dst1, toDwordPtr(cpu.regs[instr.rs1_id]));
                        cc.mov(dst2, instr.imm);
                        cc.add(dst1, dst2);

                        asmjit::InvokeNode *invokeNode {};
                        attr.invokeNode = &invokeNode;
                        translateLoad(instr, attr);

                        invokeNode->setArg(0, &cpu);
                        invokeNode->setArg(1, dst1);
                        invokeNode->setRet(0, ret);
                        if((I::Load::funct3)instr.funct3 == I::Load::funct3::LBU || (I::Load::funct3)instr.funct3 == I::Load::funct3::LHU)
                        {
                            cc.and_(ret, dst2);
                        }
                        cc.mov(toDwordPtr(cpu.regs[instr.rd_id]), ret);
                    }

                    pc_offset += instr.size;
                    break;
                }
            case Opcode::Store:
                {
                    cc.mov(dst1, toDwordPtr(cpu.regs[instr.rs1_id]));
                    cc.mov(dst2, instr.imm);
                    cc.add(dst1, dst2);
                    cc.mov(dst2, toDwordPtr(cpu.regs[instr.rs2_id]));

                    asmjit::InvokeNode *invokeNode {};
                    attr.invokeNode = &invokeNode;
                    translateStore(instr, attr);
                    invokeNode->setArg(0, &cpu);
                    invokeNode->setArg(1,dst1);
                    invokeNode->setArg(2,dst2);

                    pc_offset += instr.size;
                    break;
                }
            case Opcode::Branch:
                {
                    pc_offset += cpu.getPc();

                    asmjit::Label L_BRANCH = cc.newLabel();
                    asmjit::Label L_END = cc.newLabel();
                    attr.L_BRANCH = &L_BRANCH;

                    cc.mov(dst1, toDwordPtr(cpu.regs[instr.rs1_id]));
                    cc.mov(dst2, toDwordPtr(cpu.regs[instr.rs2_id]));

                    cc.cmp(dst1, dst2);
                    translateBranch(instr, attr);
                    cc.mov(dst1, instr.size);
                    cc.jmp(L_END);

                    cc.bind(L_BRANCH);
                    cc.mov(dst1, instr.imm);

                    cc.bind(L_END);
                    cc.mov(dst2, pc_offset);
                    cc.add(dst2, dst1);
                    cc.mov(asmjit::x86::dword_ptr((uint64_t)(&(cpu.pc_))),dst2);

                    pc_offset = 0;
                    break;
                }
            case Opcode::Jalr:
                {
                    //advance previous pc
                    pc_offset += cpu.getPc();

                    if(instr.rd_id != 0)
                    {
                        cc.mov(dst2, pc_offset);
                        cc.mov(dst1, instr.size);
                        cc.add(dst2, dst1);
                        cc.mov(toDwordPtr(cpu.regs[instr.rd_id]), dst2);
                    }

                    cc.mov(dst1, toDwordPtr(cpu.regs[instr.rs1_id]));
                    cc.mov(dst2, instr.imm);
                    cc.add(dst1, dst2);
                    cc.mov(dst2, 0xfffffffe);
                    cc.and_(dst1, dst2);

                    cc.mov(asmjit::x86::dword_ptr((uint64_t)(&(cpu.pc_))),dst1);

                    pc_offset = 0;
                    break;
                }
            case Opcode::Jal:
                {
                    pc_offset = pc_offset + cpu.getPc();
                    if(instr.rd_id != 0)
                    {
                        cc.mov(dst1, pc_offset + instr.size);
                        cc.mov(toDwordPtr(cpu.regs[instr.rd_id]), dst1);
                    }

                    cc.mov(dst1,pc_offset + instr.imm);
                    cc.mov(asmjit::x86::dword_ptr((uint64_t)(&(cpu.pc_))),dst1);
                    pc_offset = 0;
                    break;
                }
            case Opcode::Auipc:
                {
                    //advance previous pc
                    addr_t new_pc = pc_offset + cpu.getPc() + (instr.imm << 12);

                    cc.mov(dst1, new_pc);
                    cc.mov(toDwordPtr(cpu.regs[instr.rd_id]), dst1);

                    pc_offset += instr.size;
                    break;
                }
            case Opcode::Lui:
                {
                    if(instr.rd_id != 0)
                    {
                        cc.nop();
                    }
                    else
                    {
                        cc.mov(dst1, (instr.imm << 12));
                        cc.mov(toDwordPtr(cpu.regs[instr.rd_id]), dst1);
                    }
                    pc_offset += instr.size;
                    break;
                }
            case Opcode::Fence:
                {
                    cc.nop();
                    pc_offset += instr.size;
                }
            case Opcode::System:
                {
                    //TODO:: INVOKENODE
                    pc_offset += cpu.getPc();
                    cc.mov(dst1, pc_offset);
                    cc.mov(asmjit::x86::dword_ptr((uint64_t)(&(cpu.pc_))),dst1);
                    pc_offset = 0;
                }
            default:{}
        }
    }

    cc.endFunc();
    cc.finalize();

    Cpu::func_t exec;
    asmjit::Error err = cpu.rt.add(&exec, &code);
    if (err)
    {
        std::cout << "Failed to translate\n"
            << asmjit::DebugUtils::errorAsString(err)
            << std::endl;
        return nullptr;
    }

    return exec;
}


