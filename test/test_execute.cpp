#include "test.hpp"

TEST_F(RV32I_Test, TEST_EXECUTE_ADDI)
{
    Instr instr = decode(INSTR_TO_TEST::addi_x3_x4_5);
    execute( *cpu, instr);
    EXPECT_EQ(cpu->getReg(3), 5);
    EXPECT_EQ(cpu->getReg(4), 0);
    EXPECT_EQ(cpu->getPc(), 4);
}
TEST_F(RV32I_Test, TEST_EXECUTE_SLLI)
{
    cpu->setReg(3, 0);
    cpu->setReg(4, 1);
    Instr instr = decode(INSTR_TO_TEST::slli_x3_x4_5);
    execute( *cpu, instr);
    EXPECT_EQ(cpu->getReg(3), 32);
    EXPECT_EQ(cpu->getReg(4), 1);
    EXPECT_EQ(cpu->getPc(), 4);
}
TEST_F(RV32I_Test, TEST_EXECUTE_SLTI)
{
    cpu->setReg(4, 8);
    Instr instr = decode(INSTR_TO_TEST::slti_x3_x4_5);
    execute( *cpu, instr);
    EXPECT_EQ(cpu->getReg(3), 0);
    EXPECT_EQ(cpu->getReg(4), 8);
    EXPECT_EQ(cpu->getPc(), 4);

    //test slti
    cpu->setReg(4, -1);
    Instr new_instr = decode(INSTR_TO_TEST::slti_x3_x4_5);
    execute( *cpu, new_instr);
    EXPECT_EQ(cpu->getReg(3), 1);
    EXPECT_EQ(cpu->getReg(4), -1);
    EXPECT_EQ(cpu->getPc(), 8);
}
TEST_F(RV32I_Test, TEST_EXECUTE_SLTIU)
{
    cpu->setReg(4, -1);
    Instr instr = decode(INSTR_TO_TEST::sltiu_x3_x4_5);
    execute( *cpu, instr);
    EXPECT_EQ(cpu->getReg(3), 0);
    EXPECT_EQ(cpu->getReg(4), -1);

    cpu->setReg(4, 1);
    Instr new_instr = decode(INSTR_TO_TEST::sltiu_x3_x4_5);
    execute( *cpu, new_instr);
    EXPECT_EQ(cpu->getReg(3), 1);
    EXPECT_EQ(cpu->getReg(4), 1);
}
TEST_F(RV32I_Test, TEST_EXECUTE_BEQ)
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
    Instr new_instr = decode(INSTR_TO_TEST::beq_x3_x4_32);
    EXPECT_EQ(new_instr.rs1_id, 3);
    EXPECT_EQ(new_instr.rs2_id, 4);
    EXPECT_EQ(new_instr.imm, 32);
    execute( *cpu,new_instr);
    EXPECT_EQ(cpu->getReg(3), 2);
    EXPECT_EQ(cpu->getReg(4), 2);
    EXPECT_EQ(cpu->getPc(), 36);
}
TEST_F(RV32I_Test, TEST_EXECUTE_JAL)
{
    cpu->setPc(0);
    cpu->setReg(3, 0);
    Instr instr = decode(INSTR_TO_TEST::jal_x3_32);
    execute( *cpu, instr);
    EXPECT_EQ(cpu->getReg(3), 4);
    EXPECT_EQ(cpu->getPc(), 32);
}
TEST_F(RV32I_Test, TEST_EXECUTE_JALR)
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

TEST_F(RV32I_Test, TEST_EXECUTE_AUIPC)
{
    cpu->setPc(0);
    Instr instr = decode(INSTR_TO_TEST::auipc_x3_32);
    execute( *cpu, instr);
    EXPECT_EQ(cpu->getReg(3), (32 << 12));
    EXPECT_EQ(cpu->getPc(), 4);
}

TEST_F(RV32I_Test, TEST_EXECUTE_LUI)
{
    cpu->setPc(0);
    cpu->setReg(3, 0);
    Instr instr = decode(INSTR_TO_TEST::lui_x3_32);
    execute( *cpu, instr);
    EXPECT_EQ(cpu->getReg(3), (32 << 12));
    EXPECT_EQ(cpu->getPc(), 4);
}

TEST_F(RV32I_Test, TEST_EXECUTE_LOAD_STORE)
{
    cpu->setPc(0);
    cpu->setReg(3, 77);
    cpu->setReg(4,0);
    Instr instr_store = decode(INSTR_TO_TEST::sw_x3_x4_32);
    execute( *cpu, instr_store);
    EXPECT_EQ(cpu->getReg(3), 77);
    EXPECT_EQ(cpu->getReg(4), 0);
    EXPECT_EQ(cpu->getPc(), 4);

    cpu->setReg(3, 0);
    Instr instr_load = decode(INSTR_TO_TEST::lw_x3_x4_32);
    execute( *cpu, instr_load);
    EXPECT_EQ(cpu->getReg(3), 77);
    EXPECT_EQ(cpu->getReg(4), 0);
    EXPECT_EQ(cpu->getPc(), 8);


    cpu->setPc(0);
    cpu->setReg(3, -77);
    cpu->setReg(4,0);
    execute( *cpu, instr_store);
    EXPECT_EQ(cpu->getReg(3), -77);
    EXPECT_EQ(cpu->getReg(4), 0);
    EXPECT_EQ(cpu->getPc(), 4);

    cpu->setReg(3, 0);
    execute( *cpu, instr_load);
    EXPECT_EQ(cpu->getReg(3), -77);
    EXPECT_EQ(cpu->getReg(4), 0);
    EXPECT_EQ(cpu->getPc(), 8);
}

