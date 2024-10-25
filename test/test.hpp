#include "cpu.hpp"
#include <gmock/gmock.h>
#include <gtest/gtest.h>

struct RV32I_Test : public testing::Test
{
    Memory *mem;
    Cpu *cpu;

    enum INSTR_TO_TEST : reg_t
    {
        addi_x3_x4_5  = 0x00520193,
        slli_x3_x4_5  = 0x00521193,
        slti_x3_x4_5  = 0x00522193,
        sltiu_x3_x4_5 = 0x00523193,
        // xori_x3_x4_2  =
        // srli_x3_x4_2  =
        // srai_x3_x4_2  =
        // ori_x3_x4_2   =
        // andi_x3_x4_2  = 
        beq_x3_x4_32  = 0x02418063,
        lui_x3_32     = 0x000201b7,
        aupic_x3_32   = 0x00020197,
        jal_x3_32     = 0x020001ef,
        jalr_x3_x4_32 = 0x020201e7,
    };

    void SetUp() {mem = new Memory; cpu = new Cpu{mem};};
    void TearDown() {delete mem; delete cpu;};
};

