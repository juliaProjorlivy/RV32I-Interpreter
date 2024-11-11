#ifndef RV32I_HPP
#define RV32I_HPP

#include <cstdint>

const std::uint8_t regsize = 0b11111;

typedef std::int32_t reg_t;
typedef std::int32_t imm_t;
typedef std::uint32_t addr_t;
typedef std::int32_t mem_t;
typedef std::int8_t byte_t;
typedef std::int16_t half_t;
typedef std::int32_t word_t;

enum class Opcode : std::uint8_t
{
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
        EBREAK = 0b001,
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

Opcode  getOpcode(reg_t instr);
uint8_t getfunct3(reg_t instr);
int     getRdId(reg_t instr);
int     getRs1Id(reg_t instr);
int     getRs2Id(reg_t instr);

#endif


