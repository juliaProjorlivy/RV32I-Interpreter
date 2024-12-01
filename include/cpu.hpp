#ifndef CPU_RV_HPP
#define CPU_RV_HPP

#include <cstddef>
#include <iostream>
#include <memory>
#include <unordered_map>
#include <vector>

#include "asmjit/core/jitruntime.h"
#include "asmjit/x86/x86compiler.h"
#include "rv32i.hpp"

enum class RegType {ZERO_REG = 0, STACK_REG = 1, DEFAULT_REG = 2};

struct IRegister
{
    virtual ~IRegister() = default;
    virtual int getId() const = 0;
    virtual reg_t getVal() const = 0;
    virtual reg_t *getValPtr() = 0;
    virtual void setVal(int val) = 0;
};

class Register : IRegister
{
    struct RegisterDefault : IRegister
    {
    // private:
        int id_;
        reg_t val_;
    // public:
        RegisterDefault(int id, reg_t val = 0) : id_(id), val_(val) {}
        virtual std::unique_ptr<RegisterDefault> copy_() const
        {
            return std::make_unique<RegisterDefault>(*this);
        }
        int getId() const override {return id_;}
        virtual ~RegisterDefault() = default;
        virtual void setVal(int val) override {val_ = val;}
        reg_t getVal() const override {return val_;}
        reg_t *getValPtr() override {return &val_;}
    };

    struct Register_X0 final : RegisterDefault
    {
        Register_X0() : RegisterDefault {0, 0} {}
        void setVal([[maybe_unused]] int val) override {};
        std::unique_ptr<RegisterDefault> copy_() const override
        {
            return std::make_unique<Register_X0>(*this);
        }
    };

    struct Register_X2 final : RegisterDefault
    {
    private:
        static const reg_t stack_start = 0x000aeffc;
    public:
        Register_X2 () : RegisterDefault{2,stack_start} {}
        std::unique_ptr<RegisterDefault> copy_() const override
        {
            return std::make_unique<Register_X2>(*this);
        }
    };

    std::unique_ptr<RegisterDefault> self_;

public:
    Register(RegType reg_type, int id, int val = 0)
    {
        switch (reg_type)
        {
            case RegType::ZERO_REG:
            {
                self_ = std::make_unique<Register_X0>();
                break;
            }
            case RegType::STACK_REG:
            {
                self_ = std::make_unique<Register_X2>();
                break;
            }
            case RegType::DEFAULT_REG:
            {
                self_ = std::make_unique<RegisterDefault>(id, val);
                break;
            }
        }
    }

    Register(const Register &reg) : self_(reg.self_->copy_()) {}
    Register(Register &&reg) noexcept = default;
    Register &operator=(Register &&reg) noexcept
    {
        self_ = std::move(reg.self_);
        return *this;
    }

    int getId() const override {return self_->getId();}
    reg_t *getValPtr() override {return self_->getValPtr();}
    reg_t getVal() const override {return self_->getVal();}
    void setVal(int val) override {self_->setVal(val);}


    friend asmjit::x86::Mem toDwordPtr(Register &arg);
};

struct TranslationAttr
{
    asmjit::x86::Compiler &cc;
    asmjit::x86::Gp &dst1;
    asmjit::x86::Gp &dst2;
    asmjit::x86::Gp &ret;
};

// class Register
// {
// private:
//     int id;
//     reg_t val;
// public:
//     Register (int id_, reg_t val_ = 0): id{id_}, val{val_} {}
//     int getId() const {return id;}
//
//     reg_t getVal() const {return val;}
//     reg_t &getValPtr() {return val;}
//     virtual void setVal(int val_){val = val_;}
//     virtual ~Register() = default;
// };
//
// class Register_X0 : public Register
// {
// public:
//     Register_X0 () : Register{0, 0} {}
//     virtual void setVal([[maybe_unused]] int val_) {};
//     virtual ~Register_X0() = default;
// };
//
// class Register_X2 : public Register
// {
//     static const reg_t stack_start = 0x000aeffc;
// public:
//     Register_X2 () : Register{2, stack_start} {}
//     virtual ~Register_X2() = default;
// };

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
    reg_t pc_;
    // Register *regs[NRegs] {};
    std::vector<Register> regs {};
    Memory *mem {};
    bool done {false};

public:
    //for binary translation
    asmjit::JitRuntime rt;
    std::unordered_map<addr_t, std::vector<struct Instr>> bb_cache {};
    typedef  void (*func_t)(void);
    std::unordered_map<addr_t, func_t> bb_translated {};

    Cpu (Memory *mem_, addr_t entry = 0) : pc_(entry), mem(mem_)
    {
        regs.push_back(Register(RegType::ZERO_REG, 0));
        regs.push_back(Register(RegType::DEFAULT_REG, 1));
        regs.push_back(Register(RegType::STACK_REG, 2));
        // regs.reserve(NRegs);
        // regs[0] = Register(RegType::ZERO_REG, 0);
        // regs[1] = Register(RegType::DEFAULT_REG, 1);
        // regs[2] = Register(RegType::STACK_REG, 2);
        for (int i = 3; i < NRegs; ++i)
        {
            regs.push_back(Register(RegType::DEFAULT_REG, i));
            // regs[i] = Register(RegType::DEFAULT_REG,i);
        }
    }
    // Cpu (Memory *mem_, addr_t entry = 0) : pc_(entry), mem(mem_)
    // {
    //     regs[0] = new Register_X0();
    //     regs[1] = new Register(1);
    //     regs[2] = new Register_X2();
    //     for (int i = 3; i < NRegs; ++i)
    //     {
    //         regs[i] = new Register(i);
    //     }
    // }
    // ~Cpu ()
    // {
    //     for(auto reg : regs)
    //     {
    //         delete reg;
    //     }
    // }
    // //TODO: ADD ASSIGNMENT
    // Cpu(Cpu &) = delete;
    // Cpu(Cpu &&cpu) = delete;

    //TODO: NOEXCEPT
    bool isdone() const {return done;}
    //TODO: INSTR SIZE AS ARG
    void advancePc(std::size_t step = sizeof(reg_t)) {pc_ += step;}
    reg_t getPc() const {return pc_;}
    reg_t *getPcPtr() {return &pc_;}
    void setPc(reg_t val) {pc_ = val;}

    // void setReg(int ireg, reg_t value) {regs[ireg]->setVal(value);}
    // reg_t getReg(int ireg) const {return regs[ireg]->getVal();}
    // reg_t &getRegPtr(int ireg) {return regs[ireg]->getValPtr();}
    // void setDone(bool val = true) {done = val;}
    void setReg(int ireg, reg_t value) {regs[ireg].setVal(value);}
    reg_t getReg(int ireg) const {return regs[ireg].getVal();}
    reg_t *getRegPtr(int ireg) {return regs[ireg].getValPtr();}
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
        os << "pc_: " << pc_ << std::endl;
        for (int i = 0; i < NRegs; i++)
        {
            os << "x" << i << " = " << regs[i].getVal() << std::endl;
        }
    }

    reg_t fetch() {return mem->load<reg_t>(pc_);}
    reg_t fetch(addr_t addr) {return mem->load<reg_t>(addr);}


    friend asmjit::x86::Mem toDwordPtr(Cpu &cpu);
    friend void translateImm_v2(Instr &instr, Cpu &cpu, TranslationAttr &attr);
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
void interpret_block(Cpu &cpu, std::vector<Instr> instrs);

//translateion
const size_t BB_AVERAGE_SIZE = 12;
const size_t BB_THRESHOLD = 12;
bool is_bb_end(Instr &instr);
// struct TranslationAttr
// {
//     asmjit::x86::Compiler &cc;
//     asmjit::x86::Gp &dst1;
//     asmjit::x86::Gp &dst2;
//     asmjit::x86::Gp &ret;
// };
template<typename T>
asmjit::x86::Mem toDwordPtr(T arg);

template<typename ValueT>
reg_t LoadWrapper(Cpu *cpu, addr_t addr);

template<typename StoreT>
void StoreWrapper(Cpu *cpu, addr_t addr, reg_t val);

void translateOp(Cpu &cpu, Instr &instr, TranslationAttr &attr)    ;
void translateImm(Cpu &cpu, Instr &instr, TranslationAttr &attr)   ;
void translateBranch(Cpu &cpu, Instr &instr, TranslationAttr &attr);
void translateLoad (Cpu &cpu, Instr &instr, TranslationAttr &attr);
void translateStore(Cpu &cpu, Instr &instr, TranslationAttr &attr);
void translateJalr(Cpu &cpu, Instr &instr, TranslationAttr &attr) ;
void translateJal(Cpu &cpu, Instr &instr, TranslationAttr &attr)  ;
void translateAuipc(Cpu &cpu, Instr &instr, TranslationAttr &attr);
void translateLui(Cpu &cpu, Instr &instr, TranslationAttr &attr)  ;

std::vector<Instr> lookup(Cpu &cpu, addr_t addr);
Cpu::func_t translate(Cpu &cpu, std::vector<Instr> &bb);

#endif

