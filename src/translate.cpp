#include "asmjit/core/codeholder.h"
#include "asmjit/core/compiler.h"
#include "asmjit/core/func.h"
#include "asmjit/core/logger.h"
#include "asmjit/x86/x86compiler.h"
#include "asmjit/x86/x86operand.h"
#include "cpu.hpp"
#include "rv32i.hpp"
#include <cstdint>
#include <unistd.h>

static void TraceMWWrapper(Cpu *cpu, addr_t addr, reg_t val)
{
    cpu->tracer.MemoryWrite(addr, val);
}

static void TraceMRWrapper(Cpu *cpu, addr_t addr, reg_t val)
{
    cpu->tracer.MemoryRead(addr, val);
}

static void TraceRCWrapper(Cpu *cpu, int ireg, reg_t val)
{
    cpu->tracer.RegisterChange(ireg, val);
}

static void TracePcCWrapper(Cpu *cpu, reg_t val)
{
    cpu->tracer.PcChange(val);
}

static void TraceInterruptWrapper(Cpu *cpu, int inumber)
{
    cpu->tracer.Interrupt(inumber);
}

static void TraceRC(Cpu &cpu, Instr &instr, TranslationAttr &attr)
{
    attr.cc.mov(attr.dst2, instr.rd_id);
    asmjit::InvokeNode *invokeNode{};
    attr.cc.invoke(&invokeNode, (uint64_t)TraceRCWrapper, asmjit::FuncSignature::build<void, Cpu *, int, reg_t>());

    invokeNode->setArg(0, &cpu);
    invokeNode->setArg(1, attr.dst2);
    invokeNode->setArg(2, attr.ret);
}

static void TracePcC(Cpu &cpu, TranslationAttr &attr)
{
    asmjit::InvokeNode *invokeNode{};
    attr.cc.invoke(&invokeNode, (uint64_t)TracePcCWrapper, asmjit::FuncSignature::build<void, Cpu *, reg_t>());

    invokeNode->setArg(0, &cpu);
    invokeNode->setArg(1, attr.dst1);
}

void translateImm(Cpu &cpu, Instr &instr, TranslationAttr &attr)
{
    if(instr.rd_id != 0)
    {
        attr.cc.mov(attr.ret, cpu.regs[instr.rs1_id].toDwordPtr());
        attr.cc.mov(attr.dst2, instr.imm);
        translateImmFuncs[instr.funct3](instr, attr);
        attr.cc.mov((cpu.regs[instr.rd_id].toDwordPtr()), attr.ret);

        #ifdef TRACING
        TraceRC(cpu, instr, attr);
        #endif
    }
}
void translateOp(Cpu &cpu, Instr &instr, TranslationAttr &attr)
{
    if(instr.rd_id != 0)
    {
        attr.cc.mov(attr.ret, cpu.regs[instr.rs1_id].toDwordPtr());
        attr.cc.mov(attr.dst2, cpu.regs[instr.rs2_id].toDwordPtr());
        translateOpFuncs[instr.funct3](instr, attr);
        attr.cc.mov(cpu.regs[instr.rd_id].toDwordPtr(), attr.ret);

        #ifdef TRACING
        TraceRC(cpu, instr, attr);
        #endif
    }
}

void translateLoad(Cpu &cpu, Instr &instr, TranslationAttr &attr)
{
    if(instr.rd_id != 0)
    {
        attr.cc.mov(attr.dst1, cpu.regs[instr.rs1_id].toDwordPtr());
        attr.cc.mov(attr.dst2, instr.imm);
        attr.cc.add(attr.dst1, attr.dst2);

        asmjit::InvokeNode *invokeNode {};
        attr.invokeNode = &invokeNode;
        translateLoadFuncs[instr.funct3](instr, attr);

        invokeNode->setArg(0, &cpu);
        invokeNode->setArg(1, attr.dst1);
        invokeNode->setRet(0, attr.ret);
        if((I::Load::funct3)instr.funct3 == I::Load::funct3::LBU || (I::Load::funct3)instr.funct3 == I::Load::funct3::LHU)
        {
            attr.cc.and_(attr.ret, attr.dst2);
        }
        attr.cc.mov(cpu.regs[instr.rd_id].toDwordPtr(), attr.ret);

        #ifdef TRACING
        asmjit::InvokeNode *invokeNodeTrace {};
        attr.cc.invoke(&invokeNodeTrace, (uint64_t)TraceMRWrapper, asmjit::FuncSignature::build<void, Cpu *, addr_t, reg_t>());
        invokeNodeTrace->setArg(0, &cpu);
        invokeNodeTrace->setArg(1, attr.dst1);
        invokeNodeTrace->setArg(2, attr.ret);

        TraceRC(cpu, instr, attr);
        #endif
    }
}

void translateStore(Cpu &cpu, Instr &instr, TranslationAttr &attr)
{
    attr.cc.mov(attr.dst1, cpu.regs[instr.rs1_id].toDwordPtr());
    attr.cc.mov(attr.dst2, instr.imm);
    attr.cc.add(attr.dst1, attr.dst2);
    attr.cc.mov(attr.dst2, cpu.regs[instr.rs2_id].toDwordPtr());

    asmjit::InvokeNode *invokeNode {};
    attr.invokeNode = &invokeNode;
    translateStoreFuncs[instr.funct3](instr, attr);
    invokeNode->setArg(0, &cpu);
    invokeNode->setArg(1,attr.dst1);
    invokeNode->setArg(2,attr.dst2);

    #ifdef TRACING
    asmjit::InvokeNode *invokeNodeTrace {};
    attr.cc.invoke(&invokeNodeTrace, (uint64_t)TraceMWWrapper, asmjit::FuncSignature::build<void, Cpu *, addr_t, reg_t>());
    invokeNodeTrace->setArg(0, &cpu);
    invokeNodeTrace->setArg(1, attr.dst1);
    invokeNodeTrace->setArg(2, attr.dst2);

    TraceRC(cpu, instr, attr);
    #endif
}

void translateBranch(Cpu &cpu, Instr &instr, TranslationAttr &attr)
{
    asmjit::Label L_BRANCH = attr.cc.newLabel();
    asmjit::Label L_END = attr.cc.newLabel();
    attr.L_BRANCH = &L_BRANCH;

    attr.cc.mov(attr.dst1, cpu.regs[instr.rs1_id].toDwordPtr());
    attr.cc.mov(attr.dst2, cpu.regs[instr.rs2_id].toDwordPtr());

    attr.cc.cmp(attr.dst1, attr.dst2);
    translateBranchFuncs[instr.funct3](instr, attr);
    attr.cc.mov(attr.dst1, instr.size);
    attr.cc.jmp(L_END);

    attr.cc.bind(L_BRANCH);
    attr.cc.mov(attr.dst1, instr.imm);

    attr.cc.bind(L_END);
    attr.cc.mov(attr.dst2, cpu.getPc());
    attr.cc.add(attr.dst1, attr.dst2);
    attr.cc.mov(asmjit::x86::dword_ptr((uint64_t)(&(cpu.pc_))), attr.dst1);

    #ifdef TRACING
    TracePcC(cpu, attr);
    #endif
}

void translateJalr(Cpu &cpu, Instr &instr, TranslationAttr &attr)
{
    if(instr.rd_id != 0)
    {
        attr.cc.mov(attr.ret, cpu.getPc());
        attr.cc.mov(attr.dst1, instr.size);
        attr.cc.add(attr.ret, attr.dst1);
        attr.cc.mov(cpu.regs[instr.rd_id].toDwordPtr(), attr.ret);

        #ifdef TRACING
        TraceRC(cpu, instr, attr);
        #endif
    }

    attr.cc.mov(attr.dst1, cpu.regs[instr.rs1_id].toDwordPtr());
    attr.cc.mov(attr.dst2, instr.imm);
    attr.cc.add(attr.dst1, attr.dst2);
    attr.cc.mov(attr.dst2, 0xfffffffe);
    attr.cc.and_(attr.dst1, attr.dst2);

    attr.cc.mov(asmjit::x86::dword_ptr((uint64_t)(&(cpu.pc_))),attr.dst1);
    #ifdef TRACING
    TracePcC(cpu, attr);
    #endif
}

void translateJal(Cpu &cpu, Instr &instr, TranslationAttr &attr)
{
    if(instr.rd_id != 0)
    {
        attr.cc.mov(attr.ret, cpu.getPc() + instr.size);
        attr.cc.mov(cpu.regs[instr.rd_id].toDwordPtr(), attr.ret);
        #ifdef TRACING
        TraceRC(cpu, instr, attr);
        #endif
    }

    attr.cc.mov(attr.dst1, cpu.getPc() + instr.imm);
    attr.cc.mov(asmjit::x86::dword_ptr((uint64_t)(&(cpu.pc_))), attr.dst1);
    #ifdef TRACING
    TracePcC(cpu, attr);
    #endif
}

void translateLui(Cpu &cpu, Instr &instr, TranslationAttr &attr)
{
    if(instr.rd_id != 0)
    {
        attr.cc.mov(attr.ret, (instr.imm << 12));
        attr.cc.mov(cpu.regs[instr.rd_id].toDwordPtr(), attr.ret);
        #ifdef TRACING
        TraceRC(cpu, instr, attr);
        #endif
    }
}

void translateAuipc(Cpu &cpu, Instr &instr, TranslationAttr &attr)
{
    if(instr.rd_id != 0)
    {
        addr_t new_pc = cpu.getPc() + (instr.imm << 12);

        attr.cc.mov(attr.ret, new_pc);
        attr.cc.mov(cpu.regs[instr.rd_id].toDwordPtr(), attr.ret);

        #ifdef TRACING
        TraceRC(cpu, instr, attr);
        #endif
    }
}

static ssize_t ReadWrapper(Cpu *cpu, int fd, addr_t start_buf, size_t buf_size)
{
    std::vector<byte_t> buf = {};
    buf.reserve(buf_size);
    ssize_t ret_val = read(fd, static_cast<void *>(buf.data()), buf_size);

    for(int i = 0; i < buf_size; i++)
    {
        cpu->store<byte_t>(start_buf + i * sizeof(byte_t), buf[i]);
    }
    return ret_val;
}

static ssize_t WriteWrapper(Cpu *cpu, int fd, addr_t start_buf, size_t buf_size)
{
    std::vector<byte_t> buf = {};

    for(int i = 0; i < buf_size; i++)
    {
        buf.push_back(cpu->load<byte_t>(start_buf + i * sizeof(byte_t)));
    }
    return write(fd, static_cast<void *>(buf.data()), buf_size);
}

void translateEcall(Cpu &cpu ,Instr &instr, TranslationAttr &attr)
{
    asmjit::Label L_END = attr.cc.newLabel();
    asmjit::Label L_READ = attr.cc.newLabel();
    asmjit::Label L_WRITE = attr.cc.newLabel();
    asmjit::Label L_EXIT = attr.cc.newLabel();
    asmjit::x86::Gp dst3 = attr.cc.newGpd();
    attr.cc.mov(attr.dst1, cpu.regs[17].toDwordPtr());

    #ifdef TRACING
    asmjit::InvokeNode *invokeNodeTrace {};
    attr.cc.invoke(&invokeNodeTrace, (uint64_t)TraceInterruptWrapper, asmjit::FuncSignature::build<ssize_t, Cpu *, int>());
    invokeNodeTrace->setArg(0, &cpu);
    invokeNodeTrace->setArg(1, attr.dst1);
    #endif

    attr.cc.mov(attr.dst2, Syscall::rv::READ);
    attr.cc.cmp(attr.dst1, attr.dst2);
    attr.cc.je(L_READ);

    attr.cc.mov(attr.dst2, Syscall::rv::WRITE);
    attr.cc.cmp(attr.dst1, attr.dst2);
    attr.cc.je(L_WRITE);

    attr.cc.mov(attr.dst2, Syscall::rv::EXIT);
    attr.cc.cmp(attr.dst1, attr.dst2);
    attr.cc.je(L_EXIT);

    attr.cc.jmp(L_END);

    //READ SYSCALL
    attr.cc.bind(L_READ);
    attr.cc.mov(attr.dst1, cpu.regs[10].toDwordPtr());
    attr.cc.mov(attr.dst2, cpu.regs[11].toDwordPtr());
    attr.cc.mov(dst3, cpu.regs[12].toDwordPtr());
    asmjit::InvokeNode *invokeNodeRead {};
    attr.cc.invoke(&invokeNodeRead, (uint64_t)ReadWrapper, asmjit::FuncSignature::build<ssize_t, Cpu *, int, addr_t, size_t>());

    invokeNodeRead->setArg(0, &cpu);
    invokeNodeRead->setArg(1, attr.dst1);
    invokeNodeRead->setArg(2, attr.dst2);
    invokeNodeRead->setArg(3, dst3);
    invokeNodeRead->setRet(0, attr.ret);
    attr.cc.jmp(L_END);

    //WRITE SYSCALL
    attr.cc.bind(L_WRITE);
    attr.cc.mov(attr.dst1, cpu.regs[10].toDwordPtr());
    attr.cc.mov(attr.dst2, cpu.regs[11].toDwordPtr());
    attr.cc.mov(dst3, cpu.regs[12].toDwordPtr());
    asmjit::InvokeNode *invokeNodeWrite {};
    attr.cc.invoke(&invokeNodeWrite, (uint64_t)WriteWrapper, asmjit::FuncSignature::build<ssize_t, Cpu *, int, addr_t, size_t>());

    invokeNodeWrite->setArg(0, &cpu);
    invokeNodeWrite->setArg(1, attr.dst1);
    invokeNodeWrite->setArg(2, attr.dst2);
    invokeNodeWrite->setArg(3, dst3);
    invokeNodeWrite->setRet(0, attr.ret);
    attr.cc.jmp(L_END);

    //EXIT SYSCALL
    attr.cc.bind(L_EXIT);
    attr.cc.mov(attr.ret, cpu.regs[10].toDwordPtr());
    attr.cc.mov(attr.dst1, 1);
    attr.cc.mov(asmjit::x86::dword_ptr((uint64_t)(&(cpu.done))), attr.dst1);

    attr.cc.bind(L_END);
    attr.cc.mov(cpu.regs[1].toDwordPtr(), attr.ret);

    #ifdef TRACING
    TraceRC(cpu, instr, attr);
    #endif
}

void translateEbreak(Cpu &cpu ,Instr &instr, TranslationAttr &attr)
{
    attr.cc.mov(attr.dst1, 1);
    attr.cc.mov(asmjit::x86::dword_ptr((uint64_t)(&(cpu.done))), attr.dst1);
    attr.cc.mov(attr.dst1, cpu.getPc() + instr.size);
    attr.cc.mov(asmjit::x86::dword_ptr((uint64_t)(&(cpu.pc_))), attr.dst1);

    #ifdef TRACING
    TracePcC(cpu, attr);
    #endif
}

void translateFence([[maybe_unused]] Cpu &cpu, [[maybe_unused]] Instr &instr, TranslationAttr &attr)
{
    attr.cc.nop();
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

    auto instr = bb.begin();
    for(; instr != (bb.end() - 1); ++instr)
    {
        if(instr->opcode == Opcode::Auipc)
        {
            addr_t pc_offset = (instr - bb.begin()) * RV32I_INTR_SIZE;
            cpu.advanceNextPc(pc_offset);
            cpu.advancePc(pc_offset);
            instr->translate(cpu, *instr, attr);
            cpu.setPc(cpu.getPc() - pc_offset);
        }
        else
        {
            instr->translate(cpu, *instr, attr);
        }
    }

    //Last branch/jal/jalr instruction
    addr_t cur_pc = cpu.getPc();
    cpu.advancePc((bb.size() - 1) * RV32I_INTR_SIZE);
    instr->translate(cpu, *instr, attr);
    cpu.setPc(cur_pc);

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

