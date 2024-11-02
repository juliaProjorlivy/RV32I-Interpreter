#ifndef CPU_RV_HPP
#define CPU_RV_HPP

#include <iostream>
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
    virtual void setVal(int val_){val = val_;}
    virtual ~Register() = default;
};

class Register_X0 : public Register
{
public:
    Register_X0 () : Register{0, 0} {}
    virtual void setVal(int val_) { (void)val_;};
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
    mem_t *data;
public:
    Memory(std::size_t MemSize_ = MEMSIZE) : MemSize(MemSize_) {data = new mem_t[MemSize_];}
    ~Memory() {delete [] data;};
    Memory(Memory &) = delete;
    Memory(Memory &&mem) = delete;

    template<typename Value_t>
    reg_t load(addr_t addr) const
    {
        return (reg_t)(*(Value_t *)(data + addr));
    }

    template<typename Store_t, typename Value_t>
    void store(addr_t addr, Value_t val)
    {
        *((Store_t *)(data + addr)) = val;
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
    Cpu(Cpu &) = delete;
    Cpu(Cpu &&cpu) = delete;

    bool isdone() const {return done;}
    void advancePc(std::size_t step = sizeof(reg_t)) {pc += step;}
    reg_t getPc() const {return pc;}
    void setPc(reg_t val) {pc = val;}

    void setReg(int ireg, reg_t value) {regs[ireg]->setVal(value);}
    reg_t getReg(int ireg) const {return regs[ireg]->getVal();}
    void setDone(bool val = 1) {done = val;}

    template<typename Value_t>
    reg_t load(addr_t addr) const
    {
        return mem->load<Value_t>(addr);
    }

    template<typename Store_t, typename Value_t>
    void store(addr_t addr, Value_t val)
    {
        mem->store<Store_t, Value_t>(addr, val);
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

