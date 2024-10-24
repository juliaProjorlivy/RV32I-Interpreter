#include "cpu.hpp"
#include <gmock/gmock.h>
#include <gtest/gtest.h>

struct DecodeTest : public testing::Test
{
    Memory *mem;
    Cpu *cpu;

    enum INSTR_TO_TEST : reg_t
    {
        addi_x3_x4_5 = 0x00520193,
        // addi_x3_x4_5 = ;
        // addi_x3_x4_5 = ;
        // addi_x3_x4_5 = ;
    };

    void SetUp() {mem = new Memory; cpu = new Cpu{mem};};
    void TearDown() {delete mem;};
};

TEST_F(DecodeTest, TEST_ADDI)
{

    Instr instr = decode(INSTR_TO_TEST::addi_x3_x4_5);
    execute( *cpu, instr);
    EXPECT_EQ(cpu->getReg(3), 5);
    EXPECT_EQ(cpu->getReg(4), 0);
}
