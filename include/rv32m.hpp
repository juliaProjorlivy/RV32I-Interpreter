#ifndef RV32M_HPP
#define RV32M_HPP

#include <cstdint>

namespace R
{
    namespace MulDiv {
    enum class funct3 : std::uint8_t
    {
        MUL    = 0b000,
        MULH   = 0b001,
        MULHSU = 0b010,
        MULHU  = 0b011,
        DIV    = 0b100,
        DIVU   = 0b101,
        REM    = 0b110,
        REMU   = 0b111,
    };}
};

#endif

