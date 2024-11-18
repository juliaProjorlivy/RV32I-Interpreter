#ifndef CPU_RV_HPP
#define CPU_RV_HPP

#include <cstddef>
#include <iostream>
#include <unordered_map>
#include <vector>

#include "asmjit/core/jitruntime.h"
#include "rv32i.hpp"

class Register
{
private:
    int id;
    reg_t val;
public:
    Register (int id_ = 1, reg_t val_ = 0): id{id_}, val{val_} {}
    int getId() const {return id;}

    reg_t getVal() const {return val;}
    reg_t *getValPtr() {return &val;}
    virtual void setVal(int val_){val = val_;}
    virtual ~Register() = default;
};

class Register_X0 : public Register
{
public:
    Register_X0 () : Register{0, 0} {}
    virtual void setVal([[maybe_unused]] int val_) {};
    virtual ~Register_X0() = default;
};

class Register_X2 : public Register
{
    static const reg_t stack_start = 0x000aeffc;
public:
    Register_X2 () : Register{2, stack_start} {}
    virtual ~Register_X2() = default;
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
        return static_cast<reg_t>(*(reinterpret_cast<const Value_t *>(data.data() + addr)));
    }

    template<typename Store_t>
    void store(addr_t addr, reg_t val)
    {
        *(reinterpret_cast<Store_t *>((data.data() + addr))) = val;
    }
};

class Cpu
{
private:
    static const int NRegs = 32;
    reg_t pc;
    Register *regs[NRegs] {};
    Memory *mem {};
    bool done {false};

public:
    //for binary translation
    asmjit::JitRuntime rt;
    std::unordered_map<addr_t, std::vector<struct Instr>> bb_cache {};
    typedef  void (*func_t)(void);
    std::unordered_map<addr_t, func_t> bb_translated {}; //translated block function and pc offset after that bb

    Cpu (Memory *mem_, addr_t entry = 0) : pc(entry), mem(mem_)
    {
        regs[0] = new Register_X0();
        regs[1] = new Register();
        regs[2] = new Register_X2();
        for (int i = 3; i < NRegs; ++i)
        {
            regs[i] = new Register(i);
        }
    }
    ~Cpu ()
    {
        for(auto reg : regs)
        {
            delete reg;
        }
    }
    //TODO: ADD ASSIGNMENT
    Cpu(Cpu &) = delete;
    Cpu(Cpu &&cpu) = delete;

    //TODO: NOEXEPT
    bool isdone() const {return done;}
    //TODO: INSTR SIZE AS ARG
    void advancePc(std::size_t step = sizeof(reg_t)) {pc += step;}
    reg_t getPc() const {return pc;}
    reg_t *getPcPtr() {return &pc;}
    void setPc(reg_t val) {pc = val;}

    void setReg(int ireg, reg_t value) {regs[ireg]->setVal(value);}
    reg_t getReg(int ireg) const {return regs[ireg]->getVal();}
    reg_t *getRegPtr(int ireg) {return regs[ireg]->getValPtr();}
    void setDone(bool val = true) {done = val;}

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
        os << "pc: " << pc << std::endl;
        for (int i = 0; i < NRegs; i++)
        {
            os << "x" << i << " = " << regs[i]->getVal() << std::endl;
        }
    }

    reg_t fetch() {return mem->load<reg_t>(pc);}
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

    void (*exec)(Cpu &cpu, Instr &instr);
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
void execute(Cpu &cpu, Instr &instr);

#endif

