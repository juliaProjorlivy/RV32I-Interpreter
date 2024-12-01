#include "asmjit/core/codeholder.h"
#include "asmjit/core/compiler.h"
#include "asmjit/core/func.h"
#include "asmjit/core/logger.h"
#include "asmjit/x86/x86compiler.h"
#include "asmjit/x86/x86operand.h"
#include "cpu.hpp"
#include "rv32i.hpp"

bool is_bb_end(Instr &instr)
{
    switch (instr.opcode)
    {
        case Opcode::Branch:
        case Opcode::Jal:
        case Opcode::Jalr:
        case Opcode::System:
        case Opcode::Auipc:
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

// template<typename T>
// asmjit::x86::Mem toDwordPtr(T arg)
// {
//     return asmjit::x86::dword_ptr((uint64_t)(arg));
// }

asmjit::x86::Mem toDwordPtr(Cpu &cpu)
{
    return asmjit::x86::dword_ptr((uint64_t)(cpu.pc_));
}
asmjit::x86::Mem toDwordPtr(Register &reg)
{
    return asmjit::x86::dword_ptr((uint64_t)(&reg.self_->val_));
}


void translateOp(Cpu &cpu, Instr &instr, TranslationAttr &attr)
{
    switch (static_cast<R::Op::funct3>(instr.funct3))
    {
        case R::Op::funct3::ADD:
            {
                attr.cc.mov(attr.dst1, toDwordPtr(cpu.getRegPtr(instr.rs1_id)));
                attr.cc.mov(attr.dst2, toDwordPtr(cpu.getRegPtr(instr.rs2_id)));
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
                attr.cc.mov(toDwordPtr(cpu.getRegPtr(instr.rd_id)), attr.dst1);
                break;
            }
        case R::Op::funct3::AND:
            {
                attr.cc.mov(attr.dst1, toDwordPtr(cpu.getRegPtr(instr.rs1_id)));
                attr.cc.mov(attr.dst2, toDwordPtr(cpu.getRegPtr(instr.rs2_id)));
                attr.cc.and_(attr.dst1, attr.dst2);
                attr.cc.mov(toDwordPtr(cpu.getRegPtr(instr.rd_id)), attr.dst1);
                break;
            }
        case R::Op::funct3::OR:
            {
                attr.cc.mov(attr.dst1, toDwordPtr(cpu.getRegPtr(instr.rs1_id)));
                attr.cc.mov(attr.dst2, toDwordPtr(cpu.getRegPtr(instr.rs2_id)));
                attr.cc.or_(attr.dst1, attr.dst2);
                attr.cc.mov(toDwordPtr(cpu.getRegPtr(instr.rd_id)), attr.dst1);
                break;
            }
        case R::Op::funct3::XOR:
            {
                attr.cc.mov(attr.dst1, toDwordPtr(cpu.getRegPtr(instr.rs1_id)));
                attr.cc.mov(attr.dst2, toDwordPtr(cpu.getRegPtr(instr.rs2_id)));
                attr.cc.xor_(attr.dst1, attr.dst2);
                attr.cc.mov(toDwordPtr(cpu.getRegPtr(instr.rd_id)), attr.dst1);
                break;
            }
        case R::Op::funct3::SLT:
            {
                attr.cc.mov(attr.dst1, toDwordPtr(cpu.getRegPtr(instr.rs1_id)));
                attr.cc.mov(attr.dst2, toDwordPtr(cpu.getRegPtr(instr.rs2_id)));
                attr.cc.cmp(attr.dst1, attr.dst2);
                attr.cc.setl(attr.dst1);
                attr.cc.mov(toDwordPtr(cpu.getRegPtr(instr.rd_id)), attr.dst1);
                break;
            }
        case R::Op::funct3::SLTU:
            {
                attr.cc.mov(attr.dst1, toDwordPtr(cpu.getRegPtr(instr.rs1_id)));
                attr.cc.mov(attr.dst2, toDwordPtr(cpu.getRegPtr(instr.rs2_id)));
                attr.cc.cmp(attr.dst1, attr.dst2);
                attr.cc.setb(attr.dst1);
                attr.cc.mov(toDwordPtr(cpu.getRegPtr(instr.rd_id)), attr.dst1);
                break;
            }
        case R::Op::funct3::SLL:
            {
                attr.cc.mov(attr.dst1, toDwordPtr(cpu.getRegPtr(instr.rs1_id)));
                attr.cc.mov(attr.dst2, toDwordPtr(cpu.getRegPtr(instr.rs2_id)));
                attr.cc.shl(attr.dst1, attr.dst2);
                attr.cc.mov(toDwordPtr(cpu.getRegPtr(instr.rd_id)), attr.dst1);
                break;
            }
        case R::Op::funct3::SRL:
            {
                attr.cc.mov(attr.dst1, toDwordPtr(cpu.getRegPtr(instr.rs1_id)));
                attr.cc.mov(attr.dst2, toDwordPtr(cpu.getRegPtr(instr.rs2_id)));
                //SRA
                if (instr.funct7)
                {
                    attr.cc.sar(attr.dst1, attr.dst2);
                }
                else
                {
                    attr.cc.shr(attr.dst1, attr.dst2);
                }
                attr.cc.mov(toDwordPtr(cpu.getRegPtr(instr.rd_id)), attr.dst1);
                break;
            }
        default: {return;}
    }
}

void translateImm_v2(Instr &instr, Cpu &cpu, TranslationAttr &attr)
{
    switch ((I::Imm::funct3)instr.funct3)
    {
        // x0!!!!!!
        case I::Imm::funct3::ADDI:
            {
                attr.cc.mov(attr.dst1, toDwordPtr((cpu.getRegPtr(instr.rs1_id))));
                attr.cc.add(attr.dst1, instr.imm);
                attr.cc.mov( toDwordPtr((cpu.getRegPtr(instr.rd_id))), attr.dst1);

                break;
                //TODO: ADVANCE PC FOR ALL BLOCK
                //advance pc
                // attr.cc.mov(attr.dst1, toDwordPtr(cpu.getPcPtr()));
                // attr.cc.add(attr.dst1, sizeof(addr_t));
                // attr.cc.mov(toDwordPtr(cpu.getPcPtr()), attr.dst1);
            }
        case I::Imm::funct3::ANDI:
            {
                attr.cc.mov(attr.dst1, toDwordPtr(cpu.getRegPtr(instr.rs1_id)));
                attr.cc.and_(attr.dst1, instr.imm);
                attr.cc.mov( toDwordPtr(cpu.getRegPtr(instr.rd_id)), attr.dst1);
                break;
            }
        case I::Imm::funct3::ORI:
            {
                attr.cc.mov(attr.dst1, toDwordPtr(cpu.getRegPtr(instr.rs1_id)));
                attr.cc.or_(attr.dst1, instr.imm);
                attr.cc.mov( toDwordPtr(cpu.getRegPtr(instr.rd_id)), attr.dst1);
                break;
            }
        case I::Imm::funct3::XORI:
            {
                attr.cc.mov(attr.dst1, toDwordPtr(cpu.getRegPtr(instr.rs1_id)));
                attr.cc.xor_(attr.dst1, instr.imm);
                attr.cc.mov( toDwordPtr(cpu.getRegPtr(instr.rd_id)), attr.dst1);
                break;
            }
        case I::Imm::funct3::SLTI:
            {
                attr.cc.mov(attr.dst1, toDwordPtr(cpu.getRegPtr(instr.rs1_id)));
                attr.cc.cmp(attr.dst1, instr.imm);
                attr.cc.setl(attr.dst1);
                attr.cc.mov( toDwordPtr(cpu.getRegPtr(instr.rd_id)), attr.dst1);
                break;
            }
        case I::Imm::funct3::SLTIU:
            {
                attr.cc.mov(attr.dst1, toDwordPtr(cpu.getRegPtr(instr.rs1_id)));
                attr.cc.cmp(attr.dst1, instr.imm);
                attr.cc.setb(attr.dst1);
                attr.cc.mov( toDwordPtr(cpu.getRegPtr(instr.rd_id)), attr.dst1);
                break;
            }
        case I::Imm::funct3::SLLI:
            {
                attr.cc.mov(attr.dst1, toDwordPtr(cpu.getRegPtr(instr.rs1_id)));
                attr.cc.shl(attr.dst1, instr.imm);
                attr.cc.mov( toDwordPtr(cpu.getRegPtr(instr.rd_id)), attr.dst1);
                break;
            }
        case I::Imm::funct3::SRLI:
            {
                attr.cc.mov(attr.dst1, toDwordPtr(cpu.getRegPtr(instr.rs1_id)));
                //SRAI:
                if (instr.imm >> 5)
                {
                    attr.cc.sar(attr.dst1, instr.imm);
                }
                //SRLI
                else
                {
                    attr.cc.shr(attr.dst1, instr.imm);
                }
                attr.cc.mov( toDwordPtr(cpu.getRegPtr(instr.rd_id)), attr.dst1);
                break;
            }
        default: {return;}
        //TODO: THROW AN ERROR
    }

}

void translateImm(Cpu &cpu, Instr &instr, TranslationAttr &attr)
{
    switch ((I::Imm::funct3)instr.funct3)
    {
        // x0!!!!!!
        case I::Imm::funct3::ADDI:
            {
                attr.cc.mov(attr.dst1, toDwordPtr((cpu.getRegPtr(instr.rs1_id))));
                attr.cc.add(attr.dst1, instr.imm);
                attr.cc.mov( toDwordPtr((cpu.getRegPtr(instr.rd_id))), attr.dst1);

                break;
                //TODO: ADVANCE PC FOR ALL BLOCK
                //advance pc
                // attr.cc.mov(attr.dst1, toDwordPtr(cpu.getPcPtr()));
                // attr.cc.add(attr.dst1, sizeof(addr_t));
                // attr.cc.mov(toDwordPtr(cpu.getPcPtr()), attr.dst1);
            }
        case I::Imm::funct3::ANDI:
            {
                attr.cc.mov(attr.dst1, toDwordPtr(cpu.getRegPtr(instr.rs1_id)));
                attr.cc.and_(attr.dst1, instr.imm);
                attr.cc.mov( toDwordPtr(cpu.getRegPtr(instr.rd_id)), attr.dst1);
                break;
            }
        case I::Imm::funct3::ORI:
            {
                attr.cc.mov(attr.dst1, toDwordPtr(cpu.getRegPtr(instr.rs1_id)));
                attr.cc.or_(attr.dst1, instr.imm);
                attr.cc.mov( toDwordPtr(cpu.getRegPtr(instr.rd_id)), attr.dst1);
                break;
            }
        case I::Imm::funct3::XORI:
            {
                attr.cc.mov(attr.dst1, toDwordPtr(cpu.getRegPtr(instr.rs1_id)));
                attr.cc.xor_(attr.dst1, instr.imm);
                attr.cc.mov( toDwordPtr(cpu.getRegPtr(instr.rd_id)), attr.dst1);
                break;
            }
        case I::Imm::funct3::SLTI:
            {
                attr.cc.mov(attr.dst1, toDwordPtr(cpu.getRegPtr(instr.rs1_id)));
                attr.cc.cmp(attr.dst1, instr.imm);
                attr.cc.setl(attr.dst1);
                attr.cc.mov( toDwordPtr(cpu.getRegPtr(instr.rd_id)), attr.dst1);
                break;
            }
        case I::Imm::funct3::SLTIU:
            {
                attr.cc.mov(attr.dst1, toDwordPtr(cpu.getRegPtr(instr.rs1_id)));
                attr.cc.cmp(attr.dst1, instr.imm);
                attr.cc.setb(attr.dst1);
                attr.cc.mov( toDwordPtr(cpu.getRegPtr(instr.rd_id)), attr.dst1);
                break;
            }
        case I::Imm::funct3::SLLI:
            {
                attr.cc.mov(attr.dst1, toDwordPtr(cpu.getRegPtr(instr.rs1_id)));
                attr.cc.shl(attr.dst1, instr.imm);
                attr.cc.mov( toDwordPtr(cpu.getRegPtr(instr.rd_id)), attr.dst1);
                break;
            }
        case I::Imm::funct3::SRLI:
            {
                attr.cc.mov(attr.dst1, toDwordPtr(cpu.getRegPtr(instr.rs1_id)));
                //SRAI:
                if (instr.imm >> 5)
                {
                    attr.cc.sar(attr.dst1, instr.imm);
                }
                //SRLI
                else
                {
                    attr.cc.shr(attr.dst1, instr.imm);
                }
                attr.cc.mov( toDwordPtr(cpu.getRegPtr(instr.rd_id)), attr.dst1);
                break;
            }
        default: {return;}
        //TODO: THROW AN ERROR
    }
}

void translateBranch(Cpu &cpu, Instr &instr, TranslationAttr &attr)
{
    using namespace B::Branch;
    switch ((B::Branch::funct3)instr.funct3)
    {
        case B::Branch::funct3::BEQ:
            {
                asmjit::Label L_BRANCH = attr.cc.newLabel();
                asmjit::Label L_END = attr.cc.newLabel();

                attr.cc.mov(attr.dst1, toDwordPtr(cpu.getRegPtr(instr.rs1_id)));
                attr.cc.mov(attr.dst2, toDwordPtr(cpu.getRegPtr(instr.rs2_id)));

                attr.cc.cmp(attr.dst1, attr.dst2);
                attr.cc.je(L_BRANCH);
                attr.cc.add(toDwordPtr(cpu.getPcPtr()), sizeof(addr_t));
                attr.cc.jmp(L_END);

                attr.cc.bind(L_BRANCH);
                attr.cc.add(toDwordPtr(cpu.getPcPtr()), instr.imm);

                attr.cc.bind(L_END);
                attr.cc.mov(attr.dst2, toDwordPtr(cpu.getPcPtr()));
                attr.cc.add(attr.dst2, attr.dst1);
                attr.cc.mov(toDwordPtr(cpu.getPcPtr()), attr.dst2);

                return;
            }
        case B::Branch::funct3::BNE:
            {
                asmjit::Label L_BRANCH = attr.cc.newLabel();
                asmjit::Label L_END = attr.cc.newLabel();

                attr.cc.mov(attr.dst1, toDwordPtr(cpu.getRegPtr(instr.rs1_id)));
                attr.cc.mov(attr.dst2, toDwordPtr(cpu.getRegPtr(instr.rs2_id)));

                attr.cc.cmp(attr.dst1, attr.dst2);
                attr.cc.jne(L_BRANCH);
                attr.cc.mov(attr.dst1, sizeof(addr_t));
                attr.cc.jmp(L_END);

                attr.cc.bind(L_BRANCH);
                attr.cc.mov(attr.dst1, instr.imm);

                attr.cc.bind(L_END);
                attr.cc.mov(attr.dst2, toDwordPtr(cpu.getPcPtr()));
                attr.cc.add(attr.dst2, attr.dst1);
                attr.cc.mov(toDwordPtr(cpu.getPcPtr()), attr.dst2);


                return;
            }
        case B::Branch::funct3::BLT:
            {
                asmjit::Label L_BRANCH = attr.cc.newLabel();
                asmjit::Label L_END = attr.cc.newLabel();

                attr.cc.mov(attr.dst1, toDwordPtr(cpu.getRegPtr(instr.rs1_id)));
                attr.cc.mov(attr.dst2, toDwordPtr(cpu.getRegPtr(instr.rs2_id)));

                attr.cc.cmp(attr.dst1, attr.dst2);
                attr.cc.jl(L_BRANCH);
                attr.cc.add(toDwordPtr(cpu.getPcPtr()), sizeof(addr_t));
                attr.cc.jmp(L_END);

                attr.cc.bind(L_BRANCH);
                attr.cc.add(toDwordPtr(cpu.getPcPtr()), instr.imm);

                attr.cc.bind(L_END);

                return;
            }
        case B::Branch::funct3::BGE:
            {
                asmjit::Label L_BRANCH = attr.cc.newLabel();
                asmjit::Label L_END = attr.cc.newLabel();

                attr.cc.mov(attr.dst1, toDwordPtr(cpu.getRegPtr(instr.rs1_id)));
                attr.cc.mov(attr.dst2, toDwordPtr(cpu.getRegPtr(instr.rs2_id)));

                attr.cc.cmp(attr.dst1, attr.dst2);
                attr.cc.jg(L_BRANCH);
                attr.cc.add(toDwordPtr(cpu.getPcPtr()), sizeof(addr_t));
                attr.cc.jmp(L_END);

                attr.cc.bind(L_BRANCH);
                attr.cc.add(toDwordPtr(cpu.getPcPtr()), instr.imm);

                attr.cc.bind(L_END);

                return;
            }
        case B::Branch::funct3::BLTU:
            {
                asmjit::Label L_BRANCH = attr.cc.newLabel();
                asmjit::Label L_END = attr.cc.newLabel();

                attr.cc.mov(attr.dst1, toDwordPtr(cpu.getRegPtr(instr.rs1_id)));
                attr.cc.mov(attr.dst2, toDwordPtr(cpu.getRegPtr(instr.rs2_id)));

                attr.cc.cmp(attr.dst1, attr.dst2);
                attr.cc.jb(L_BRANCH);
                attr.cc.add(toDwordPtr(cpu.getPcPtr()), sizeof(addr_t));
                attr.cc.jmp(L_END);

                attr.cc.bind(L_BRANCH);
                attr.cc.add(toDwordPtr(cpu.getPcPtr()), instr.imm);

                attr.cc.bind(L_END);

                return;
            }
        case B::Branch::funct3::BGEU:
            {
                asmjit::Label L_BRANCH = attr.cc.newLabel();
                asmjit::Label L_END = attr.cc.newLabel();

                attr.cc.mov(attr.dst1, toDwordPtr(cpu.getRegPtr(instr.rs1_id)));
                attr.cc.mov(attr.dst2, toDwordPtr(cpu.getRegPtr(instr.rs2_id)));

                attr.cc.cmp(attr.dst1, attr.dst2);
                attr.cc.ja(L_BRANCH);
                attr.cc.add(toDwordPtr(cpu.getPcPtr()), sizeof(addr_t));
                attr.cc.jmp(L_END);

                attr.cc.bind(L_BRANCH);
                attr.cc.add(toDwordPtr(cpu.getPcPtr()), instr.imm);

                attr.cc.bind(L_END);
                return;
            }
        default: {return;}
    }
}

template<typename ValueT>
reg_t LoadWrapper(Cpu *cpu, addr_t addr)
{
    return cpu->load<ValueT>(addr);
}

void translateLoad (Cpu &cpu, Instr &instr, TranslationAttr &attr)
{
    switch ((I::Load::funct3)instr.funct3)
    {
        case I::Load::funct3::LB:
            {
                asmjit::InvokeNode *invokeNode {};
                attr.cc.mov(attr.dst1, toDwordPtr(cpu.getRegPtr(instr.rs1_id)));
                attr.cc.add(attr.dst1, instr.imm);

                attr.cc.invoke(&invokeNode, (uint64_t)LoadWrapper<byte_t>, asmjit::FuncSignature::build<reg_t, Cpu *, addr_t>());
                invokeNode->setArg(0, &cpu);
                invokeNode->setArg(1, attr.dst1);
                invokeNode->setRet(0, attr.ret);

                attr.cc.mov(toDwordPtr(cpu.getRegPtr(instr.rd_id)), attr.ret);
                break;
            }
        case I::Load::funct3::LH:
            {
                asmjit::InvokeNode *invokeNode {};
                attr.cc.mov(attr.dst1, toDwordPtr(cpu.getRegPtr(instr.rs1_id)));
                attr.cc.add(attr.dst1, instr.imm);

                attr.cc.invoke(&invokeNode, (uint64_t)LoadWrapper<half_t>, asmjit::FuncSignature::build<reg_t, Cpu *, addr_t>());
                invokeNode->setArg(0, &cpu);
                invokeNode->setArg(1, attr.dst1);
                invokeNode->setRet(0, attr.ret);

                attr.cc.mov(toDwordPtr(cpu.getRegPtr(instr.rd_id)), attr.ret);
                break;
            }
        case I::Load::funct3::LW:
            {
                asmjit::InvokeNode *invokeNode {};
                attr.cc.mov(attr.dst1, toDwordPtr(cpu.getRegPtr(instr.rs1_id)));
                attr.cc.add(attr.dst1, instr.imm);

                attr.cc.invoke(&invokeNode, (uint64_t)LoadWrapper<word_t>, asmjit::FuncSignature::build<reg_t, Cpu *, addr_t>());
                invokeNode->setArg(0, &cpu);
                invokeNode->setArg(1, attr.dst1);
                invokeNode->setRet(0, attr.ret);

                attr.cc.mov(toDwordPtr(cpu.getRegPtr(instr.rd_id)), attr.ret);
                break;
            }
        case I::Load::funct3::LBU:
            {
                asmjit::InvokeNode *invokeNode {};
                attr.cc.mov(attr.dst1, toDwordPtr(cpu.getRegPtr(instr.rs1_id)));
                attr.cc.add(attr.dst1, instr.imm);

                attr.cc.invoke(&invokeNode, (uint64_t)LoadWrapper<byte_t>, asmjit::FuncSignature::build<reg_t, Cpu *, addr_t>());
                invokeNode->setArg(0, &cpu);
                invokeNode->setArg(1, attr.dst1);
                invokeNode->setRet(0, attr.ret);

                attr.cc.and_(attr.ret, 0x0000ffff);
                attr.cc.mov(toDwordPtr(cpu.getRegPtr(instr.rd_id)), attr.ret);

                break;
            }
        case I::Load::funct3::LHU:
            {
                asmjit::InvokeNode *invokeNode {};
                attr.cc.mov(attr.dst1, toDwordPtr(cpu.getRegPtr(instr.rs1_id)));
                attr.cc.add(attr.dst1, instr.imm);

                attr.cc.invoke(&invokeNode, (uint64_t)LoadWrapper<half_t>, asmjit::FuncSignature::build<reg_t, Cpu *, addr_t>());
                invokeNode->setArg(0, &cpu);
                invokeNode->setArg(1, attr.dst1);
                invokeNode->setRet(0, attr.ret);

                attr.cc.and_(attr.ret, 0x0000ffff);
                attr.cc.mov(toDwordPtr(cpu.getRegPtr(instr.rd_id)), attr.ret);

                break;
            }
        case I::Load::funct3::LWU:
            {
                asmjit::InvokeNode *invokeNode {};
                attr.cc.mov(attr.dst1, toDwordPtr(cpu.getRegPtr(instr.rs1_id)));
                attr.cc.add(attr.dst1, instr.imm);

                attr.cc.invoke(&invokeNode, (uint64_t)LoadWrapper<word_t>, asmjit::FuncSignature::build<reg_t, Cpu *, addr_t>());
                invokeNode->setArg(0, &cpu);
                invokeNode->setArg(1, attr.dst1);
                invokeNode->setRet(0, attr.ret);

                attr.cc.and_(attr.ret, 0x0000ffff);
                attr.cc.mov(toDwordPtr(cpu.getRegPtr(instr.rd_id)), attr.ret);

                break;
            }
    }
}

template<typename StoreT>
void StoreWrapper(Cpu *cpu, addr_t addr, reg_t val)
{
    cpu->store<StoreT>(addr, val);
}

void translateStore(Cpu &cpu, Instr &instr, TranslationAttr &attr)
{
    switch (static_cast<S::Store::funct3>(instr.funct3))
    {
        case S::Store::funct3::SB:
            {
                asmjit::InvokeNode *invokeNode {};
                attr.cc.mov(attr.dst1, toDwordPtr(cpu.getRegPtr(instr.rs1_id)));
                attr.cc.add(attr.dst1, instr.imm);

                attr.cc.mov(attr.dst2, toDwordPtr(cpu.getRegPtr(instr.rs2_id)));

                attr.cc.invoke(&invokeNode, (uint64_t)StoreWrapper<byte_t>, asmjit::FuncSignature::build<void, Cpu *, addr_t, reg_t>());
                invokeNode->setArg(0, &cpu);
                invokeNode->setArg(1, attr.dst1);
                invokeNode->setArg(2, attr.dst2);

                break;
            }
        case S::Store::funct3::SH:
            {
                asmjit::InvokeNode *invokeNode {};
                attr.cc.mov(attr.dst1, toDwordPtr(cpu.getRegPtr(instr.rs1_id)));
                attr.cc.add(attr.dst1, instr.imm);

                attr.cc.mov(attr.dst2, toDwordPtr(cpu.getRegPtr(instr.rs2_id)));

                attr.cc.invoke(&invokeNode, (uint64_t)StoreWrapper<half_t>, asmjit::FuncSignature::build<void, Cpu *, addr_t, reg_t>());
                invokeNode->setArg(0, &cpu);
                invokeNode->setArg(1, attr.dst1);
                invokeNode->setArg(2, attr.dst2);

                break;
            }
        case S::Store::funct3::SW:
            {
                asmjit::InvokeNode *invokeNode {};
                attr.cc.mov(attr.dst1, toDwordPtr(cpu.getRegPtr(instr.rs1_id)));
                attr.cc.add(attr.dst1, instr.imm);

                attr.cc.mov(attr.dst2, toDwordPtr(cpu.getRegPtr(instr.rs2_id)));

                attr.cc.invoke(&invokeNode, (uint64_t)StoreWrapper<word_t>, asmjit::FuncSignature::build<void, Cpu *, addr_t, reg_t>());
                invokeNode->setArg(0, &cpu);
                invokeNode->setArg(1, attr.dst1);
                invokeNode->setArg(2, attr.dst2);

                break;
            }
    }
}

void translateJalr(Cpu &cpu, Instr &instr, TranslationAttr &attr)
{
    attr.cc.mov(attr.dst1, toDwordPtr(cpu.getRegPtr(instr.rs1_id)));
    attr.cc.add(attr.dst1, instr.imm);
    attr.cc.and_(attr.dst1, 0xfffffffe);

    attr.cc.mov(attr.dst2, cpu.getPc());
    attr.cc.add(attr.dst2, sizeof(addr_t));

    attr.cc.mov(toDwordPtr(cpu.getReg(instr.rd_id)), attr.dst2);

    attr.cc.mov(toDwordPtr(cpu.getPcPtr()), attr.dst1);

    // imm_t target_addr = (cpu.getReg(instr.rs1_id) + instr.imm) & 0xfffffffe; //least-significant bit to zero
    // cpu.setReg(instr.rd_id, cpu.getPc() + sizeof(addr_t));
    // cpu.setPc(target_addr);
}

void translateJal(Cpu &cpu, Instr &instr, TranslationAttr &attr)
{
    attr.cc.mov(attr.dst1, cpu.getPc());
    attr.cc.add(attr.dst1, sizeof(addr_t));

    attr.cc.mov(toDwordPtr(cpu.getPcPtr()), instr.imm);

    // cpu.setReg(instr.rd_id, cpu.getPc() + sizeof(addr_t));
    // cpu.advancePc(instr.imm);
}
void translateAuipc(Cpu &cpu, Instr &instr, TranslationAttr &attr)
{
    attr.cc.mov(attr.dst1, cpu.getPc());
    attr.cc.add(attr.dst1, instr.imm);
    attr.cc.mov(toDwordPtr(cpu.getRegPtr(instr.rd_id)), attr.dst1);

    // cpu.setReg(instr.rd_id, cpu.getPc() + instr.imm);
    // cpu.advancePc()
}

void translateLui(Cpu &cpu, Instr &instr, TranslationAttr &attr)
{
    attr.cc.mov(toDwordPtr(cpu.getRegPtr(instr.rd_id)), instr.imm);
    // cpu.setReg(instr.rd_id, instr.imm);
    // cpu.advancePc();
}

Cpu::func_t translate(Cpu &cpu, std::vector<Instr> &bb)
{
    // addr_t pc_offset = 0;
    asmjit::CodeHolder code;
    code.init(cpu.rt.environment(), cpu.rt.cpuFeatures());

    asmjit::x86::Compiler cc(&code);
    cc.addFunc(asmjit::FuncSignature::build<int>());

    asmjit::FileLogger logger(stdout);
    code.setLogger(&logger);

    asmjit::x86::Gp dst1 = cc.newGpd();
    asmjit::x86::Gp dst2 = cc.newGpd();
    asmjit::x86::Gp ret = cc.newGpd();

    TranslationAttr attr {cc, dst1, dst2, ret};
    int icount = 0;

    for(auto instr : bb)
    {
        switch (instr.opcode)
        {
            case Opcode::Imm:
                {
                    translateImm(cpu, instr, attr);
                    ++icount;
                    break;
                }
            case Opcode::Op:
                {
                    translateOp(cpu, instr, attr);
                    ++icount;
                    break;
                }
            case Opcode::Load:
                {
                    translateLoad(cpu, instr, attr);
                    ++icount;
                    break;
                }
            case Opcode::Store:
                {
                    translateStore(cpu, instr, attr);
                    ++icount;
                    break;
                }
            case Opcode::Branch:
                {
                    cc.mov(attr.dst2, icount * sizeof(addr_t));
                    cc.mov(attr.dst1, toDwordPtr(cpu.getPcPtr()));
                    cc.add(attr.dst1, attr.dst2);
                    cc.mov(toDwordPtr(cpu.getPcPtr()), attr.dst1);
                    icount = 0;
                    translateBranch(cpu, instr, attr);
                    break;
                }
            case Opcode::Jalr:
                {
                    cc.add(toDwordPtr(cpu.getPcPtr()), icount * sizeof(addr_t));
                    icount = 0;
                    translateJalr(cpu, instr, attr);
                    break;
                }
            case Opcode::Jal:
                {
                    cc.add(toDwordPtr(cpu.getPcPtr()), icount * sizeof(addr_t));
                    icount = 0;
                    translateJal(cpu, instr, attr);
                    break;
                }
            case Opcode::Auipc:
                {
                    cc.add(toDwordPtr(cpu.getPcPtr()), icount * sizeof(addr_t));
                    icount = 0;
                    translateAuipc(cpu, instr, attr);
                    break;
                }
            case Opcode::Lui:
                {
                    translateLui(cpu, instr, attr);
                    ++icount;
                    break;
                }
            case Opcode::System:
            default:{}
        }
    }

    ////advance pc
    //cc.add(toDwordPtr(cpu.getPcPtr()), icount * sizeof(addr_t));

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


