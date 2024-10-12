#include "rv32i.hpp"

int main()
{
    std::uint32_t prog[] = { 0x14d00093, 0x40100133, 0x00209463, 0x3e700293, 0x29a00313, 0x00100073};

    Memory mem;
    addr_t entry = 0;

    for (int i = 0; i < std::size(prog); i++)
    {
        mem.store<addr_t>(entry + i * sizeof(addr_t), prog[i]);
    }

    Cpu cpu{&mem};

    while (!cpu.isdone())
    {
        reg_t command = cpu.fetch();
        Instr instr = decode(command);
        execute (cpu, instr);
    }

    cpu.dump(std::cout);

    return 0;
}


