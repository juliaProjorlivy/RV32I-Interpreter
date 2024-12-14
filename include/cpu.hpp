#ifndef CPU_RV_HPP
#define CPU_RV_HPP

#include <cstddef>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <type_traits>
#include <unordered_map>
#include <vector>
#include <functional>

#include "asmjit/core/compiler.h"
#include "asmjit/core/jitruntime.h"
#include "asmjit/x86/x86compiler.h"
#include "rv32i.hpp"
#include "trace.hpp"

struct Register
{
private:
    int id_;
    reg_t val_;
    static const reg_t stack_start = 0x000aeffc;

public:
    Register(int id, reg_t val = 0) : id_(id), val_(val)
    {
        if(id_ == 0) {val_ = 0;}
        else if(id_ == 2) {val_ = stack_start;}
    }
    void setVal(reg_t val) noexcept {if(id_ != 0) {val_ = val;}}
    reg_t getVal() const noexcept {return val_;}

    asmjit::x86::Mem toDwordPtr()
    {
        return asmjit::x86::dword_ptr(reinterpret_cast<uint64_t>(&(val_)));
    }
};

struct TranslationAttr
{
    asmjit::x86::Compiler &cc;
    asmjit::x86::Gp &dst1;
    asmjit::x86::Gp &dst2;
    asmjit::x86::Gp &ret;

    asmjit::InvokeNode **invokeNode;
    asmjit::Label *L_BRANCH;
};

class Memory
{
private:
    static const std::size_t MEMSIZE = 0x00ffffff;
    std::size_t MemSize;
    std::vector<mem_t> data{};
public:
    Memory(std::size_t MemSize_ = MEMSIZE) : MemSize(MemSize_) {data.reserve(MEMSIZE);};

    template<typename Value_t>
    reg_t load(addr_t addr) const
    {
        reg_t ret_val = 0;
        memcpy(&ret_val, data.data() + addr, sizeof(Value_t));
        return ret_val;
    }

    template<typename Store_t>
    void store(addr_t addr, reg_t val)
    {
        memcpy(data.data() + addr, &val, sizeof(Store_t));
    }
};

class Cpu
{
private:
    static const int NRegs = 32;
    reg_t pc_;
    reg_t next_pc_;
    std::vector<Register> regs {};
    Memory *mem;
    bool done {false};

    //for tracing
    // Trace tracer;

public:
    //for binary translation
    asmjit::JitRuntime rt;
    std::unordered_map<addr_t, std::vector<struct Instr>> bb_cache {};
    typedef  void (*func_t)(void);
    std::unordered_map<addr_t, func_t> bb_translated {};
    FILE *output_log;

    Cpu (Memory *mem_, addr_t entry = 0, const char *filename = "x86_64") : pc_(entry), next_pc_(entry), mem(mem_)
    {
        output_log = fopen(filename, "w+");
        if(!output_log) {std::cout << "Failed to open a file " << filename << std::endl;}
        for(int i = 0; i < NRegs; i++)
        {
            regs.push_back(Register(i));
        }
    }

    bool isdone() const noexcept {return done;}
    void advancePc(std::size_t step = RV32I_INTR_SIZE) {pc_ += step; next_pc_ = pc_;}
    void advanceNextPc(std::size_t step = RV32I_INTR_SIZE) {next_pc_+= step;}
    reg_t getPc() const noexcept {return pc_;}
    reg_t getNextPc() const noexcept {return next_pc_;}
    void setPc(reg_t val) noexcept {pc_ = val; next_pc_ = val;}
    void setNextPc(reg_t val) noexcept {next_pc_ = val;}

    void setReg(int ireg, reg_t value) {regs[ireg].setVal(value);}
    reg_t getReg(int ireg) const {return regs[ireg].getVal();}
    void setDone(bool val = true) noexcept {done = val;}

    template<typename Value_t>
    reg_t load(addr_t addr) const
    {
        return mem->load<Value_t>(addr);
    }

    template<typename Store_t>
    void store(addr_t addr, addr_t val)
    {
        mem->store<Store_t>(addr, val);
    }

    void dump(std::ostream &os)
    {
        os << "regs:" << std::endl;
        os << "pc: " << pc_ << std::endl;
        for (int i = 0; i < NRegs; i++)
        {
            os << "x" << i << " = " << regs[i].getVal() << std::endl;
        }
    }

    reg_t fetch() {return mem->load<reg_t>(pc_);}
    reg_t fetch(addr_t addr) {return mem->load<reg_t>(addr);}

    friend func_t translate(    Cpu &cpu, std::vector<Instr> &bb);
    friend void translateImm(   Cpu &cpu, Instr &instr, TranslationAttr &attr);
    friend void translateOp(    Cpu &cpu, Instr &instr, TranslationAttr &attr);
    friend void translateBranch(Cpu &cpu, Instr &instr, TranslationAttr &attr);
    friend void translateLoad(  Cpu &cpu, Instr &instr, TranslationAttr &attr);
    friend void translateStore( Cpu &cpu, Instr &instr, TranslationAttr &attr);
    friend void translateAuipc( Cpu &cpu, Instr &instr, TranslationAttr &attr);
    friend void translateJalr(  Cpu &cpu, Instr &instr, TranslationAttr &attr);
    friend void translateJal(   Cpu &cpu, Instr &instr, TranslationAttr &attr);
    friend void translateLui(   Cpu &cpu, Instr &instr, TranslationAttr &attr);
    friend void translateFence( Cpu &cpu, Instr &instr, TranslationAttr &attr);
    friend void translateEbreak(Cpu &cpu ,Instr &instr, TranslationAttr &attr);
    friend void translateEcall( Cpu &cpu ,Instr &instr, TranslationAttr &attr);
};

struct Instr
{
    Opcode opcode;
    uint8_t funct3;
    uint8_t funct7;
    imm_t imm;
    int rd_id;
    int rs1_id;
    int rs2_id;

    size_t size;
    std::function<void(Cpu &, Instr &)> exec;
    std::function<void(Cpu &cpu, Instr &instr, TranslationAttr &attr)> translate;
};

Instr decode(reg_t instr);
void executeEbreak(Cpu &cpu,[[maybe_unused]] Instr &instr);
void executeEcall(Cpu &cpu,[[maybe_unused]] Instr &instr);
void executeLui(Cpu &cpu, Instr &instr);
void executeAuipc(Cpu &cpu, Instr &instr);
void executeJalr(Cpu &cpu, Instr &instr);
void executeJal(Cpu &cpu, Instr &instr);
void executeFence([[maybe_unused]] Cpu &cpu,[[maybe_unused]] Instr &instr);
void interpret_block(Cpu &cpu, std::vector<Instr> &instrs);

const std::vector<std::function<void(Cpu &, Instr &)>> executeImmFuncs =
    {
        // ADDI  = 0b0000
        [] (Cpu &cpu, Instr &instr) {cpu.setReg(instr.rd_id, cpu.getReg(instr.rs1_id) + instr.imm);},
        // SLLI  = 0b0001
        [] (Cpu &cpu, Instr &instr) {cpu.setReg(instr.rd_id, cpu.getReg(instr.rs1_id) << (instr.imm & 0b11111));},
        // SLTI  = 0b0010
        [] (Cpu &cpu, Instr &instr) {cpu.setReg(instr.rd_id, cpu.getReg(instr.rs1_id) < instr.imm);},
        // SLTIU = 0b0011
        [] (Cpu &cpu, Instr &instr) {cpu.setReg(instr.rd_id, static_cast<std::uint32_t>(cpu.getReg(instr.rs1_id)) < static_cast<std::uint32_t>(instr.imm));},
        // XORI  = 0b0100
        [] (Cpu &cpu, Instr &instr) {cpu.setReg(instr.rd_id, cpu.getReg(instr.rs1_id) ^ instr.imm);},
        // SRLI  = 0b0101
        [] (Cpu &cpu, Instr &instr) {cpu.setReg(instr.rd_id, static_cast<std::uint32_t>(cpu.getReg(instr.rs1_id)) >> instr.imm);},
        // ORI   = 0b0110
        [] (Cpu &cpu, Instr &instr) {cpu.setReg(instr.rd_id, cpu.getReg(instr.rs1_id) | instr.imm);},
        // ANDI  = 0b0111
        [] (Cpu &cpu, Instr &instr) {cpu.setReg(instr.rd_id, cpu.getReg(instr.rs1_id) & instr.imm);},
        // SRAI  = 0b1000
        [] (Cpu &cpu, Instr &instr) {cpu.setReg(instr.rd_id, static_cast<std::uint32_t>(cpu.getReg(instr.rs1_id)) >> instr.imm);},
    };

const std::vector<std::function<void(Cpu &, Instr &)>> executeOpFuncs =
    {
        // ADD  = 0b0000,
        [] (Cpu &cpu, Instr &instr) {cpu.setReg(instr.rd_id, cpu.getReg(instr.rs1_id) + cpu.getReg(instr.rs2_id));},
        // SLL  = 0b0001,
        [] (Cpu &cpu, Instr &instr) {cpu.setReg(instr.rd_id, cpu.getReg(instr.rs1_id) << (cpu.getReg(instr.rs2_id) & 0b11111));},
        // SLT  = 0b0010,
        [] (Cpu &cpu, Instr &instr) {cpu.setReg(instr.rd_id, cpu.getReg(instr.rs1_id) < cpu.getReg(instr.rs2_id));},
        // SLTU = 0b0011,
        [] (Cpu &cpu, Instr &instr) {cpu.setReg(instr.rd_id, static_cast<uint32_t>(cpu.getReg(instr.rs1_id)) < static_cast<uint32_t>(cpu.getReg(instr.rs2_id)));},
        // XOR  = 0b0100,
        [] (Cpu &cpu, Instr &instr) {cpu.setReg(instr.rd_id, cpu.getReg(instr.rs1_id) ^ cpu.getReg(instr.rs2_id));},
        // SRL  = 0b0101,
        [] (Cpu &cpu, Instr &instr) {cpu.setReg(instr.rd_id, static_cast<uint32_t>(cpu.getReg(instr.rs1_id)) >> cpu.getReg(instr.rs2_id));},
        // OR   = 0b0110,
        [] (Cpu &cpu, Instr &instr) {cpu.setReg(instr.rd_id, cpu.getReg(instr.rs1_id) | cpu.getReg(instr.rs2_id));},
        // AND  = 0b0111,
        [] (Cpu &cpu, Instr &instr) {cpu.setReg(instr.rd_id, cpu.getReg(instr.rs1_id) & cpu.getReg(instr.rs2_id));},
        // SUB  = 0b1000,
        [] (Cpu &cpu, Instr &instr) {cpu.setReg(instr.rd_id, cpu.getReg(instr.rs1_id) - cpu.getReg(instr.rs2_id));},
        // SRA  = 0b1001,
        [] (Cpu &cpu, Instr &instr) {cpu.setReg(instr.rd_id, cpu.getReg(instr.rs1_id) >> cpu.getReg(instr.rs2_id));},
    };

const std::vector<std::function<void(Cpu &, Instr &)>> executeBranchFuncs =
    {
        // BEQ  = 0b000
        [] (Cpu &cpu, Instr &instr) {if (cpu.getReg(instr.rs1_id) == cpu.getReg(instr.rs2_id)) {cpu.advanceNextPc(instr.imm);}},
        // BNE  = 0b001
        [] (Cpu &cpu, Instr &instr) {if (cpu.getReg(instr.rs1_id) != cpu.getReg(instr.rs2_id)) {cpu.advanceNextPc(instr.imm);}},
        // FILLER = 0b010
        [] (Cpu &cpu, Instr &instr) {},
        // FILLER = 0b011
        [] (Cpu &cpu, Instr &instr) {},
        // BLT  = 0b100
        [] (Cpu &cpu, Instr &instr) {if (cpu.getReg(instr.rs1_id) < cpu.getReg(instr.rs2_id)) {cpu.advanceNextPc(instr.imm);}},
        // BGE  = 0b101
        [] (Cpu &cpu, Instr &instr) {if (cpu.getReg(instr.rs1_id) > cpu.getReg(instr.rs2_id)) {cpu.advanceNextPc(instr.imm);}},
        // BLTU = 0b110
        [] (Cpu &cpu, Instr &instr) {if (static_cast<uint32_t>(cpu.getReg(instr.rs1_id)) < static_cast<uint32_t>(cpu.getReg(instr.rs2_id))) {cpu.advanceNextPc(instr.imm);}},
        // BGEU = 0b111
        [] (Cpu &cpu, Instr &instr) {if (static_cast<uint32_t>(cpu.getReg(instr.rs1_id)) > static_cast<uint32_t>(cpu.getReg(instr.rs2_id))) {cpu.advanceNextPc(instr.imm);}},
    };

const std::vector<std::function<void(Cpu &, Instr &)>> executeLoadFuncs =
    {
        // LB  = 0b000
        [] (Cpu &cpu, Instr &instr) {cpu.setReg(instr.rd_id, cpu.load<byte_t>(instr.imm + cpu.getReg(instr.rs1_id)));},
        // LH  = 0b001
        [] (Cpu &cpu, Instr &instr) {cpu.setReg(instr.rd_id, cpu.load<half_t>(instr.imm + cpu.getReg(instr.rs1_id)));},
        // LW  = 0b010
        [] (Cpu &cpu, Instr &instr) {cpu.setReg(instr.rd_id, cpu.load<word_t>(instr.imm + cpu.getReg(instr.rs1_id)));},
        // FILLER = 0b011
        [] (Cpu &cpu, Instr &instr) {},
        // LBU = 0b100
        [] (Cpu &cpu, Instr &instr) {cpu.setReg(instr.rd_id, (0x000000ff & cpu.load<byte_t>(instr.imm + cpu.getReg(instr.rs1_id))));},
        // LHU = 0b101
        [] (Cpu &cpu, Instr &instr) {cpu.setReg(instr.rd_id, (0x0000ffff & cpu.load<half_t>(instr.imm + cpu.getReg(instr.rs1_id))));},
    };

const std::vector<std::function<void(Cpu &, Instr &)>> executeStoreFuncs =
    {
        // SB = 0b000
        [] (Cpu &cpu, Instr &instr) {cpu.store<byte_t>((addr_t)(instr.imm + cpu.getReg(instr.rs1_id)), cpu.getReg(instr.rs2_id));},
        // SH = 0b001
        [] (Cpu &cpu, Instr &instr) {cpu.store<half_t>((addr_t)(instr.imm + cpu.getReg(instr.rs1_id)), cpu.getReg(instr.rs2_id));},
        // SW = 0b010
        [] (Cpu &cpu, Instr &instr) {cpu.store<word_t>((addr_t)(instr.imm + cpu.getReg(instr.rs1_id)), cpu.getReg(instr.rs2_id));},
    };

const std::vector<std::function<void(Cpu &, Instr &)>> executeSystemFuncs =
    {
        executeEcall,
        executeEbreak,
    };


//translation
const size_t BB_AVERAGE_SIZE = 10;
const size_t BB_THRESHOLD = 10;
bool is_bb_end(Instr &instr);

Cpu::func_t translate(    Cpu &cpu, std::vector<Instr> &bb);
void translateImm(   Cpu &cpu, Instr &instr, TranslationAttr &attr);
void translateOp(    Cpu &cpu, Instr &instr, TranslationAttr &attr);
void translateBranch(Cpu &cpu, Instr &instr, TranslationAttr &attr);
void translateLoad(  Cpu &cpu, Instr &instr, TranslationAttr &attr);
void translateStore( Cpu &cpu, Instr &instr, TranslationAttr &attr);
void translateAuipc( Cpu &cpu, Instr &instr, TranslationAttr &attr);
void translateJalr(  Cpu &cpu, Instr &instr, TranslationAttr &attr);
void translateJal(   Cpu &cpu, Instr &instr, TranslationAttr &attr);
void translateLui(   Cpu &cpu, Instr &instr, TranslationAttr &attr);
void translateFence( Cpu &cpu, Instr &instr, TranslationAttr &attr);
// ssize_t ReadWrapper(Cpu *cpu, int fd, addr_t start_buf, size_t buf_size);
// ssize_t WriteWrapper(Cpu *cpu, int fd, addr_t start_buf, size_t buf_size);
void translateEbreak(Cpu &cpu ,Instr &instr, TranslationAttr &attr);
void translateEcall( Cpu &cpu ,Instr &instr, TranslationAttr &attr);

const std::vector<std::function<void(Instr &instr, TranslationAttr &attr)>> translateImmFuncs =
    {
        // ADDI  = 0b0000
        [] (Instr &instr, TranslationAttr &attr) {attr.cc.add(attr.dst1, attr.dst2);},
        // SLLI  = 0b0001
        [] (Instr &instr, TranslationAttr &attr) {attr.cc.shl(attr.dst1, attr.dst2);},
        // SLTI  = 0b0010
        [] (Instr &instr, TranslationAttr &attr) {attr.cc.cmp(attr.dst1, attr.dst2);
                attr.cc.setl(attr.dst1);},
        // SLTIU = 0b0011
        [] (Instr &instr, TranslationAttr &attr) {attr.cc.cmp(attr.dst1, attr.dst2);
                attr.cc.setb(attr.dst1);},
        // XORI  = 0b0100
        [] (Instr &instr, TranslationAttr &attr) {attr.cc.xor_(attr.dst1, attr.dst2);},
        // SRLI  = 0b0101
        [] (Instr &instr, TranslationAttr &attr) {attr.cc.shr(attr.dst1, attr.dst2);},
        // ORI   = 0b0110
        [] (Instr &instr, TranslationAttr &attr) {attr.cc.or_(attr.dst1, attr.dst2);},
        // ANDI  = 0b0111
        [] (Instr &instr, TranslationAttr &attr) {attr.cc.and_(attr.dst1, attr.dst2);},
        // SRAI  = 0b1000
        [] (Instr &instr, TranslationAttr &attr) {attr.cc.sar(attr.dst1, attr.dst2);},
    };

const std::vector<std::function<void(Instr &instr, TranslationAttr &attr)>> translateOpFuncs =
    {
        // ADD  = 0b0000
        [] (Instr &instr, TranslationAttr &attr) {attr.cc.add(attr.dst1, attr.dst2);},
        // SLL  = 0b0001
        [] (Instr &instr, TranslationAttr &attr) {attr.cc.shl(attr.dst1, attr.dst2);},
        // SLT  = 0b0010
        [] (Instr &instr, TranslationAttr &attr) {attr.cc.cmp(attr.dst1, attr.dst2);
                attr.cc.setl(attr.dst1);},
        // SLTU = 0b0011
        [] (Instr &instr, TranslationAttr &attr) {attr.cc.cmp(attr.dst1, attr.dst2);
                attr.cc.setb(attr.dst1);},
        // XOR  = 0b0100
        [] (Instr &instr, TranslationAttr &attr) {attr.cc.xor_(attr.dst1, attr.dst2);},
        // SRL  = 0b0101
        [] (Instr &instr, TranslationAttr &attr) {attr.cc.shr(attr.dst1, attr.dst2);},
        // OR   = 0b0110
        [] (Instr &instr, TranslationAttr &attr) {attr.cc.or_(attr.dst1, attr.dst2);},
        // AND  = 0b0111
        [] (Instr &instr, TranslationAttr &attr) {attr.cc.and_(attr.dst1, attr.dst2);},
        // SUB  = 0b1000
        [] (Instr &instr, TranslationAttr &attr) {attr.cc.sub(attr.dst1, attr.dst2);},
        // SRA  = 0b1001
        [] (Instr &instr, TranslationAttr &attr) {attr.cc.sar(attr.dst1, attr.dst2);},

    };

const std::vector<std::function<void(Instr &instr, TranslationAttr &attr)>> translateBranchFuncs =
    {
        // BEQ  = 0b000
        [] (Instr &instr, TranslationAttr &attr) {attr.cc.je(*attr.L_BRANCH);},
        // BNE  = 0b001
        [] (Instr &instr, TranslationAttr &attr) {attr.cc.jne(*attr.L_BRANCH);},
        // FILLER = 0b010
        [] (Instr &instr, TranslationAttr &attr) {},
        // FILLER = 0b011
        [] (Instr &instr, TranslationAttr &attr) {},
        // BLT  = 0b100
        [] (Instr &instr, TranslationAttr &attr) {attr.cc.jl(*attr.L_BRANCH);},
        // BGE  = 0b101
        [] (Instr &instr, TranslationAttr &attr) {attr.cc.jg(*attr.L_BRANCH);},
        // BLTU = 0b110
        [] (Instr &instr, TranslationAttr &attr) {attr.cc.jb(*attr.L_BRANCH);},
        // BGEU = 0b111
        [] (Instr &instr, TranslationAttr &attr) {attr.cc.ja(*attr.L_BRANCH);},
    };

template<typename ValueT>
reg_t LoadWrapper(Cpu *cpu, addr_t addr)
{
    return cpu->load<ValueT>(addr);
}

const std::vector<std::function<void(Instr &instr, TranslationAttr &attr)>> translateLoadFuncs =
    {
        // LB  = 0b000
        [] (Instr &instr, TranslationAttr &attr) 
        {
            attr.cc.invoke(attr.invokeNode, (uint64_t)LoadWrapper<byte_t>, asmjit::FuncSignature::build<reg_t, Cpu *, addr_t>());
        },
        // LH  = 0b001
        [] (Instr &instr, TranslationAttr &attr) 
        {
            attr.cc.invoke(attr.invokeNode, (uint64_t)LoadWrapper<half_t>, asmjit::FuncSignature::build<reg_t, Cpu *, addr_t>());
        },
        // LW  = 0b010
        [] (Instr &instr, TranslationAttr &attr) 
        {
            attr.cc.invoke(attr.invokeNode, (uint64_t)LoadWrapper<word_t>, asmjit::FuncSignature::build<reg_t, Cpu *, addr_t>());
        },
        // FILLER  = 0b011
        [] (Instr &instr, TranslationAttr &attr) {},
        // LBU = 0b100
        [] (Instr &instr, TranslationAttr &attr) 
        {
            attr.cc.invoke(attr.invokeNode, (uint64_t)LoadWrapper<byte_t>, asmjit::FuncSignature::build<reg_t, Cpu *, addr_t>());
            attr.cc.mov(attr.dst2, 0x000000ff);
        },
        // LHU = 0b101
        [] (Instr &instr, TranslationAttr &attr) 
        {
            attr.cc.invoke(attr.invokeNode, (uint64_t)LoadWrapper<half_t>, asmjit::FuncSignature::build<reg_t, Cpu *, addr_t>());
            attr.cc.mov(attr.dst2, 0x0000ffff);
        },
    };

template<typename StoreT>
static void StoreWrapper(Cpu *cpu, addr_t addr, reg_t val)
{
    cpu->store<StoreT>(addr, val);
}

const std::vector<std::function<void(Instr &instr, TranslationAttr &attr)>> translateStoreFuncs =
    {
        // SB = 0b000
        [] (Instr &instr, TranslationAttr &attr) 
        {
            attr.cc.invoke(attr.invokeNode, (uint64_t)StoreWrapper<byte_t>, asmjit::FuncSignature::build<void, Cpu *, addr_t, reg_t>());
        },
        // SH = 0b001
        [] (Instr &instr, TranslationAttr &attr) 
        {
            attr.cc.invoke(attr.invokeNode, (uint64_t)StoreWrapper<half_t>, asmjit::FuncSignature::build<void, Cpu *, addr_t, reg_t>());
        },
        // SW = 0b010
        [] (Instr &instr, TranslationAttr &attr) 
        {
            attr.cc.invoke(attr.invokeNode, (uint64_t)StoreWrapper<word_t>, asmjit::FuncSignature::build<void, Cpu *, addr_t, reg_t>());

        },
    };

std::vector<Instr> lookup(Cpu &cpu, addr_t addr);
// Cpu::func_t translate(Cpu &cpu, std::vector<Instr> &bb);

#endif

