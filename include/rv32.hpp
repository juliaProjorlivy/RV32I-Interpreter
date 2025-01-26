#ifndef RV32_HPP
#define RV32_HPP

#include <cstdint>

const std::uint8_t regsize = 0b11111;

using addr_t = uint32_t ;
using mem_t  = uint8_t  ;
using reg_t  = int32_t  ;
using imm_t  = int32_t  ;
using byte_t = int8_t   ;
using half_t = int16_t  ;
using word_t = int32_t  ;
using instr_t = uint32_t;
const std::size_t RV32I_INTR_SIZE = 4;

#endif

