#include "test.hpp"
#include "io.hpp"

//TESTS STORE AND LOAD
TEST_F(RV32I_Test_Translate, Test_1)
{
    const char *filename = "./test/data/test1";

    if(elfio_manager(filename, *cpu)){return;};
    if(run_simulation(*cpu)) {return;};

    EXPECT_EQ(cpu->getReg(8), 2);
    EXPECT_EQ(cpu->getReg(7), 4);
    EXPECT_EQ(cpu->getReg(6), 6);
    EXPECT_EQ(cpu->getReg(5), 8);
    EXPECT_EQ(cpu->getReg(4), 10);
}

//TESTS JAL
TEST_F(RV32I_Test_Translate, Test_2)
{
    const char *filename = "./test/data/test2";

    if(elfio_manager(filename, *cpu)) {return;};
    if(run_simulation(*cpu)) {return;};

    EXPECT_EQ(cpu->getReg(0), 0);
    EXPECT_EQ(cpu->getReg(1), 10);
    for(int i = 3; i < 32; ++i)
    {
        EXPECT_EQ(cpu->getReg(i), 0);
    }
}

TEST_F(RV32I_Test_Translate, Test_3)
{
    const char *filename = "./test/data/test3";

    if(elfio_manager(filename, *cpu)) {return;};
    if(run_simulation(*cpu)) {return;};

    EXPECT_EQ(cpu->getReg(0), 0);
    EXPECT_EQ(cpu->getReg(3), 10);
    EXPECT_EQ(cpu->getReg(4), 10);
    EXPECT_EQ(cpu->getReg(5), -100);
    for(int i = 6; i < 32; ++i)
    {
        EXPECT_EQ(cpu->getReg(i), 0);
    }
}

TEST_F(RV32I_Test_Translate, Test_fibonacci_10)
{
    const char *filename = "./test/data/fib_out_10";

    if(elfio_manager(filename, *cpu)) {return;};

    if(run_simulation(*cpu)) {return;};

    EXPECT_EQ(cpu->getReg(0), 0);
    EXPECT_EQ(cpu->getReg(15), 55);
}

TEST_F(RV32I_Test_Translate, Test_fibonacci_5)
{
    const char *filename = "./test/data/fib_out_5";

    if(elfio_manager(filename, *cpu)) {return;};

    if(run_simulation(*cpu)) {return;};

    EXPECT_EQ(cpu->getReg(0), 0);
    EXPECT_EQ(cpu->getReg(15), 5);
}

