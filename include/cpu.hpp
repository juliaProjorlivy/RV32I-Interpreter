#ifndef CPU_RV_HPP
#define CPU_RV_HPP

#include <cstddef>
#include <iostream>
#include <memory>
#include <unordered_map>
#include <vector>

#include "asmjit/core/compiler.h"
#include "asmjit/core/jitruntime.h"
#include "asmjit/x86/x86compiler.h"
#include "rv32i.hpp"

enum class RegType {ZERO_REG = 0, STACK_REG = 1, DEFAULT_REG = 2};

struct IRegister
{
    virtual ~IRegister() = default;
    virtual int getId() const noexcept = 0;
    virtual reg_t getVal() const noexcept = 0;
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
        int getId() const noexcept override {return id_;}
        virtual ~RegisterDefault() = default;
        virtual void setVal(int val) override {val_ = val;}
        reg_t getVal() const noexcept override {return val_;}
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

    int getId() const noexcept override {return self_->getId();}
    reg_t getVal() const noexcept override {return self_->getVal();}
    void setVal(int val) override {self_->setVal(val);}


    friend asmjit::x86::Mem toDwordPtr(Register &reg);
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
    std::vector<Register> regs {};
    Memory *mem {};
    bool done {false};

public:
    //for binary translation
    asmjit::JitRuntime rt;
    std::unordered_map<addr_t, std::vector<struct Instr>> bb_cache {};
    typedef  void (*func_t)(void);
    std::unordered_map<addr_t, func_t> bb_translated {};
    FILE *output_log;

    Cpu (Memory *mem_, addr_t entry = 0, const char * filename = "translation.log") : pc_(entry), mem(mem_)
    {
        output_log = fopen(filename, "rw");
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

    //TODO: NOEXCEPT
    bool isdone() const noexcept {return done;}
    //TODO: INSTR SIZE AS ARG
    void advancePc(std::size_t step = sizeof(reg_t)) {pc_ += step;}
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

    friend Cpu::func_t translate(Cpu &cpu, std::vector<Instr> &bb);
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
void executeFence(Cpu &cpu, Instr &instr);
void execute(Cpu &cpu, Instr &instr);
void interpret_block(Cpu &cpu, std::vector<Instr> instrs);

//translateion
const size_t BB_AVERAGE_SIZE = 10;
const size_t BB_THRESHOLD = 10;
bool is_bb_end(Instr &instr);

asmjit::x86::Mem toDwordPtr(Register &reg);

void translateOp(Instr &instr, TranslationAttr &attr)    ;
void translateImm(Instr &instr, TranslationAttr &attr)   ;
void translateBranch(Instr &instr, TranslationAttr &attr);
void translateLoad (Instr &instr, TranslationAttr &attr) ;
void translateStore(Instr &instr, TranslationAttr &attr) ;

std::vector<Instr> lookup(Cpu &cpu, addr_t addr);
Cpu::func_t translate(Cpu &cpu, std::vector<Instr> &bb);

#endif

