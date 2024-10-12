#ifndef RV32I_HPP
#define RV32I_HPP

#include <cstddef>
#include <cstdint>
#include <vector>
#include <iostream>

const std::uint8_t regsize = 0b11111;

typedef std::int32_t reg_t;
typedef std::int32_t imm_t;
typedef std::uint32_t addr_t;
typedef std::int32_t mem_t;
typedef std::int8_t byte_t;
typedef std::int16_t half_t;
typedef std::int32_t word_t;
constexpr std::size_t NRegs = 32;

enum class Opcode : std::uint8_t
{
    // Unknown = 0b0000000,
    Load    = 0b0000011,
    Imm     = 0b0010011,
    Auipc   = 0b0010111,
    Store   = 0b0100011,
    Op      = 0b0110011,
    Lui     = 0b0110111,
    Branch  = 0b1100011,
    Jalr    = 0b1100111,
    Jal     = 0b1101111,
    System  = 0b1110011,
};

namespace I
{
    namespace Imm {
    enum class funct3 : std::uint8_t
    {
        ADDI  = 0b0000,
        SLLI  = 0b0001,
        SLTI  = 0b0010,
        SLTIU = 0b0011,
        XORI  = 0b0100,
        SRLI  = 0b0101,
        SRAI  = 0b1101,
        ORI   = 0b0110,
        ANDI  = 0b0111,
    };}
    namespace Load {
    enum class funct3 : std::uint8_t
    {
        LB  = 0b000,
        LH  = 0b001,
        LW  = 0b010,
        LBU = 0b100,
        LHU = 0b101,
        LWU = 0b110,
    };}
    namespace System {
    enum class funct3 : std::uint8_t
    {
        ECALL  = 0b000,
        EBREAK  = 0b001,
    };}

    //TODO:FINISH FOR SHIFTS
    imm_t getImm(reg_t instr);
};

namespace R
{
    namespace Op {
    enum class funct3 : std::uint8_t
    {
        ADD  = 0b0000,
        SUB  = 0b1000,
        SLL  = 0b0001,
        SLT  = 0b0010,
        SLTU = 0b0011,
        XOR  = 0b0100,
        SRL  = 0b0101,
        SRA  = 0b1101,
        OR   = 0b0110,
        AND  = 0b0111,
    };}
};

namespace B
{
    namespace Branch {
    enum class funct3 : std::uint8_t
    {
        BEQ  = 0b000,
        BNE  = 0b001,
        BLT  = 0b100,
        BGE  = 0b101,
        BLTU = 0b110,
        BGEU = 0b111,
    };}

    imm_t getImm(reg_t instr);
}

namespace S
{
    namespace Store {
    enum class funct3 : std::uint8_t
    {
        SB = 0b000,
        SH = 0b001,
        SW = 0b010,
    };}

    imm_t getImm(reg_t instr);
}

namespace U
{
    imm_t getImm(reg_t instr);
}

namespace J
{
    imm_t getImm(reg_t instr);
}


static const std::size_t MEMSIZE = 0xbadface;
class Memory
{
private:
    std::size_t MemSize;
    mem_t *data;
public:
    Memory(std::size_t MemSize_ = MEMSIZE) : MemSize(MemSize_) {data = new mem_t[MemSize_];}
    ~Memory() {delete [] data;};

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
    reg_t pc;
    reg_t regs[NRegs] {};
    Memory *mem {};
    bool done {false};

public:
    Cpu (Memory *mem_, addr_t entry = 0) : mem(mem_), pc(entry) {}
    ~Cpu () = default;

    bool isdone() const {return done;}
    void advancePc(std::size_t step = sizeof(reg_t)) {pc += step;}
    reg_t getPc() const {return pc;}
    void setPc(reg_t val) {pc = val;}

    void setReg(int ireg, reg_t value) {regs[ireg] = value;}
    reg_t getReg(int ireg) const {return regs[ireg];}
    void setDone(bool val = 1) {done = val;}

    template<typename Value_t>
    reg_t load(addr_t addr) const
    {
        mem->load<Value_t>(addr);
    }

    template<typename Store_t, typename Value_t>
    void store(addr_t addr, Value_t val)
    {
        mem->store<Store_t, Value_t>(addr, val);
    }

    void dump(std::ostream &os)
    {
        for (int i = 0; i < NRegs; i++)
        {
            os << "pc: " << pc << std::endl;
            os << "regs:" << std::endl;
            os << "x" << i << " = " << regs[i] << std::endl;
        }
    }

    reg_t fetch() {return mem->load<reg_t>(pc);}
};

//TODO: mb union or
//for every template it's own struct
//mb class
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

Instr decode(reg_t &instr);

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

Opcode  getOpcode(reg_t instr);
uint8_t getfunct3(reg_t instr);
int     getRdId(reg_t instr);
int     getRs1Id(reg_t instr);
int     getRs2Id(reg_t instr);

#endif


