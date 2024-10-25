#include "test.hpp"

TEST_F(RV32I_Test, TEST_ADDI)
{
    Instr instr = decode(INSTR_TO_TEST::addi_x3_x4_5);
    execute( *cpu, instr);
    EXPECT_EQ(cpu->getReg(3), 5);
    EXPECT_EQ(cpu->getReg(4), 0);
    EXPECT_EQ(cpu->getPc(), 4);
}

TEST_F(RV32I_Test, TEST_SLLI)
{
    cpu->setReg(3, 0);
    cpu->setReg(4, 1);
    Instr instr = decode(INSTR_TO_TEST::slli_x3_x4_5);
    execute( *cpu, instr);
    EXPECT_EQ(cpu->getReg(3), 32);
    EXPECT_EQ(cpu->getReg(4), 1);
    EXPECT_EQ(cpu->getPc(), 4);
}

TEST_F(RV32I_Test, TEST_SLTI)
{
    cpu->setReg(4, 8);
    Instr instr = decode(INSTR_TO_TEST::slti_x3_x4_5);
    execute( *cpu, instr);
    EXPECT_EQ(cpu->getReg(3), 0);
    EXPECT_EQ(cpu->getReg(4), 8);
    EXPECT_EQ(cpu->getPc(), 4);

    //test slti
    cpu->setReg(4, -1);
    instr = decode(INSTR_TO_TEST::slti_x3_x4_5);
    execute( *cpu, instr);
    EXPECT_EQ(cpu->getReg(3), 1);
    EXPECT_EQ(cpu->getReg(4), -1);
    EXPECT_EQ(cpu->getPc(), 8);
}

TEST_F(RV32I_Test, TEST_SLTIU)
{
    cpu->setReg(4, -1);
    Instr instr = decode(INSTR_TO_TEST::sltiu_x3_x4_5);
    execute( *cpu, instr);
    EXPECT_EQ(cpu->getReg(3), 0);
    EXPECT_EQ(cpu->getReg(4), -1);

    cpu->setReg(4, 1);
    instr = decode(INSTR_TO_TEST::sltiu_x3_x4_5);
    execute( *cpu, instr);
    EXPECT_EQ(cpu->getReg(3), 1);
    EXPECT_EQ(cpu->getReg(4), 1);
}

    //TODO: WRITE THE REST INSTRUCTIONS
    //
    //test xori
    //cpu->setReg(4, 1);
    //instr = decode(INSTR_TO_TEST::xori_x3_x4_5);
    //execute( *cpu, instr);
    //EXPECT_EQ(cpu->getReg(3), 32);
    //EXPECT_EQ(cpu->getReg(4), 1);
    //
    ////test slli
    //cpu->setReg(4, 1);
    //instr = decode(INSTR_TO_TEST::slli_x3_x4_5);
    //execute( *cpu, instr);
    //EXPECT_EQ(cpu->getReg(3), 32);
    //EXPECT_EQ(cpu->getReg(4), 1);
    //
    ////test slli
    //cpu->setReg(4, 1);
    //instr = decode(INSTR_TO_TEST::slli_x3_x4_5);
    //execute( *cpu, instr);
    //EXPECT_EQ(cpu->getReg(3), 32);
    //EXPECT_EQ(cpu->getReg(4), 1);
    //
    ////test slli
    //cpu->setReg(4, 1);
    //instr = decode(INSTR_TO_TEST::slli_x3_x4_5);
    //execute( *cpu, instr);
    //EXPECT_EQ(cpu->getReg(3), 32);
    //EXPECT_EQ(cpu->getReg(4), 1);

TEST_F(RV32I_Test, TEST_BEQ)
{
    cpu->setPc(0);
    cpu->setReg(3, 2);
    cpu->setReg(4, -2);
    Instr instr = decode(INSTR_TO_TEST::beq_x3_x4_32);
    execute( *cpu, instr);
    EXPECT_EQ(cpu->getReg(3), 2);
    EXPECT_EQ(cpu->getReg(4), -2);
    EXPECT_EQ(cpu->getPc(), 4);

    cpu->setReg(4, 2);
    instr = decode(INSTR_TO_TEST::beq_x3_x4_32);
    EXPECT_EQ(instr.rs1_id, 3);
    EXPECT_EQ(instr.rs2_id, 4);
    EXPECT_EQ(instr.imm, 32);
    execute( *cpu, instr);
    EXPECT_EQ(cpu->getReg(3), 2);
    EXPECT_EQ(cpu->getReg(4), 2);
    EXPECT_EQ(cpu->getPc(), 36);
    //TODO: WRITE THE REST BRANCH INSTRUCTIONS
}

TEST_F(RV32I_Test, TEST_JAL)
{
    cpu->setPc(0);
    cpu->setReg(3, 0);
    Instr instr = decode(INSTR_TO_TEST::jal_x3_32);
    execute( *cpu, instr);
    EXPECT_EQ(cpu->getReg(3), 4);
    EXPECT_EQ(cpu->getPc(), 32);
}

TEST_F(RV32I_Test, TEST_JALR)
{
    cpu->setPc(0);
    cpu->setReg(3, 0);
    cpu->setReg(4, 8);
    Instr instr = decode(INSTR_TO_TEST::jalr_x3_x4_32);
    execute( *cpu, instr);
    EXPECT_EQ(cpu->getReg(3), 4);
    EXPECT_EQ(cpu->getReg(4), 8);
    EXPECT_EQ(cpu->getPc(), 40);
}
