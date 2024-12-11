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
        memcpy(&ret_val, data.data() + addr, sizeof(ret_val));
        return ret_val;
    }

    template<typename Store_t>
    void store(addr_t addr, reg_t val)
    {
        memcpy(data.data() + addr, &val, sizeof(val));
    }
};

class Cpu
{
private:
    static const int NRegs = 32;
    reg_t pc_;
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

    Cpu (Memory *mem_, addr_t entry = 0, const char *filename = "x86_64") : pc_(entry), mem(mem_)
    {
        output_log = fopen(filename, "w+");
        if(!output_log) {std::cout << "Failed to open a file " << filename << std::endl;}
        for(int i = 0; i < NRegs; i++)
        {
            regs.push_back(Register(i));
        }
    }

    bool isdone() const noexcept {return done;}
    void advancePc(std::size_t step = RV32I_INTR_SIZE) {pc_ += step;}
    reg_t getPc() const noexcept {return pc_;}
    void setPc(reg_t val) noexcept {pc_ = val;}

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

    func_t translate(std::vector<Instr> &bb);
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
    // void (*exec)(Cpu &cpu, Instr &instr);
    std::function<void(Cpu &, Instr &)> exec;
};

    const std::vector<std::function<void(Cpu &, Instr &)>> executeImmFuncs =
    {
        // ADDI  = 0b0000
        [] (Cpu &cpu, Instr &instr) {cpu.setReg(instr.rd_id, cpu.getReg(instr.rs1_id) + instr.imm);cpu.advancePc();},
        // SLLI  = 0b0001
        [] (Cpu &cpu, Instr &instr) {cpu.setReg(instr.rd_id, cpu.getReg(instr.rs1_id) << (instr.imm & 0b11111));cpu.advancePc();},
        // SLTI  = 0b0010
        [] (Cpu &cpu, Instr &instr) {cpu.setReg(instr.rd_id, cpu.getReg(instr.rs1_id) < instr.imm);cpu.advancePc();},
        // SLTIU = 0b0011
        [] (Cpu &cpu, Instr &instr) {cpu.setReg(instr.rd_id, static_cast<std::uint32_t>(cpu.getReg(instr.rs1_id)) < static_cast<std::uint32_t>(instr.imm));cpu.advancePc();},
        // XORI  = 0b0100
        [] (Cpu &cpu, Instr &instr) {cpu.setReg(instr.rd_id, cpu.getReg(instr.rs1_id) ^ instr.imm);cpu.advancePc();},
        // SRLI  = 0b0101
        [] (Cpu &cpu, Instr &instr) {cpu.setReg(instr.rd_id, static_cast<std::uint32_t>(cpu.getReg(instr.rs1_id)) >> instr.imm);cpu.advancePc();},
        // ORI   = 0b0110
        [] (Cpu &cpu, Instr &instr) {cpu.setReg(instr.rd_id, cpu.getReg(instr.rs1_id) | instr.imm);cpu.advancePc();},
        // ANDI  = 0b0111
        [] (Cpu &cpu, Instr &instr) {cpu.setReg(instr.rd_id, cpu.getReg(instr.rs1_id) & instr.imm);cpu.advancePc();},
        // SRAI  = 0b1000
        [] (Cpu &cpu, Instr &instr) {cpu.setReg(instr.rd_id, static_cast<std::uint32_t>(cpu.getReg(instr.rs1_id)) >> instr.imm);cpu.advancePc();},
    };

Instr decode(reg_t instr);
void executeImm (Cpu &cpu, Instr &instr);
void executeOp (Cpu &cpu, Instr &instr);
void executeBranch (Cpu &cpu, Instr &instr);
void executeLoad (Cpu &cpu, Instr &instr);
void executeStore (Cpu &cpu, Instr &instr);
void executeSystem(Cpu &cpu, Instr &instr);
void executeLui(Cpu &cpu, Instr &instr);
void executeAuipc(Cpu &cpu, Instr &instr);
void executeJalr(Cpu &cpu, Instr &instr);
void executeJal(Cpu &cpu, Instr &instr);
void executeFence(Cpu &cpu, Instr &instr);
void execute(Cpu &cpu, Instr &instr);
void interpret_block(Cpu &cpu, std::vector<Instr> instrs);



//translateion
const size_t BB_AVERAGE_SIZE = 10;
const size_t BB_THRESHOLD = 10;
bool is_bb_end(Instr &instr);

// asmjit::x86::Mem toDwordPtr(Register &reg);

void translateOp(Instr &instr, TranslationAttr &attr)    ;
void translateImm(Instr &instr, TranslationAttr &attr)   ;
void translateBranch(Instr &instr, TranslationAttr &attr);
void translateLoad (Instr &instr, TranslationAttr &attr) ;
void translateStore(Instr &instr, TranslationAttr &attr) ;

std::vector<Instr> lookup(Cpu &cpu, addr_t addr);
// Cpu::func_t translate(Cpu &cpu, std::vector<Instr> &bb);

#endif

