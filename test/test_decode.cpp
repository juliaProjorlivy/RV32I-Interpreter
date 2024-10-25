#include "rv32i.hpp"
#include "test.hpp"
#include <cstdint>

TEST_F(RV32I_Test, TEST_DECODE_IMM)
{
    Instr instr = decode(INSTR_TO_TEST::addi_x3_x4_5);
    EXPECT_EQ(instr.opcode, Opcode::Imm);
    EXPECT_EQ(instr.funct3, static_cast<uint8_t>(I::Imm::funct3::ADDI));
    EXPECT_EQ(instr.rd_id, 3);
    EXPECT_EQ(instr.rs1_id, 4);
    EXPECT_EQ(instr.imm, 5);
}

// TEST_F(RV32I_Test, TEST_DECODE_OP)
// {
//     Instr instr = decode(INSTR_TO_TEST::add_x3_x4_x5);
//     EXPECT_EQ(instr.opcode, Opcode::Op);
//     EXPECT_EQ(instr.funct3, R::Op::funct3::ADD);
//     EXPECT_EQ(instr.rd_id, 3);
//     EXPECT_EQ(instr.rs1_id, 4);
//     EXPECT_EQ(instr.rs2_id, 5);
// }

TEST_F(RV32I_Test, TEST_DECODE_BRANCH)
{
    Instr instr = decode(INSTR_TO_TEST::beq_x3_x4_32);
    EXPECT_EQ(instr.opcode, Opcode::Branch);
    EXPECT_EQ(instr.funct3, static_cast<uint8_t>(B::Branch::funct3::BEQ));
    EXPECT_EQ(instr.rs1_id, 3);
    EXPECT_EQ(instr.rs2_id, 4);
    EXPECT_EQ(instr.imm, 32);
}

TEST_F(RV32I_Test, TEST_DECODE_JALR)
{
    Instr instr = decode(INSTR_TO_TEST::jalr_x3_x4_32);
    EXPECT_EQ(instr.opcode, Opcode::Jalr);
    EXPECT_EQ(instr.rd_id, 3);
    EXPECT_EQ(instr.rs1_id, 4);
    EXPECT_EQ(instr.imm, 32);
}
TEST_F(RV32I_Test, TEST_DECODE_JAL)
{
    Instr instr = decode(INSTR_TO_TEST::jal_x3_32);
    EXPECT_EQ(instr.opcode, Opcode::Jal);
    EXPECT_EQ(instr.rd_id, 3);
    EXPECT_EQ(instr.imm, 32);
}
