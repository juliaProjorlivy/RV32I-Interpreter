#include "rv32i.hpp"
#include "elfio/elfio.hpp"
#include <elfio/elf_types.hpp>
#include <elfio/elfio_segment.hpp>

int main(int argc, char* argv[])
{
    using namespace ELFIO;
    elfio reader;

    if(argc < 2)
    {
        std::cout << "Too little arguments" << std::endl;
        return 1;
    }
    if(!reader.load( argv[1]))
    {
        std::cout << "Can't find or process ELF file " << argv[1] << std::endl;
        return 1;
    }
    if(reader.get_machine() != EM_RISCV)
    {
        std::cout << "Not RISC-V32" << std::endl;
        return 1;
    }
    if(reader.get_class() != ELFCLASS32)
    {
        std::cout << "Not ELF32" << std::endl;
        return 1;
    }

    Elf_Half seg_num = reader.segments.size();
    Elf64_Addr entry_point = reader.get_entry();
    for(int i = 0; i < seg_num; i++)
    {
        const segment *seg = reader.segments[i];
        if(seg->get_type() == PT_LOAD && seg->get_flags() == (PF_X | PF_R))
        {
            // std::cout << "was found" << std::endl;

            Elf64_Addr vaddr = seg->get_virtual_address();
            // Elf_Xword size = seg->get_memory_size();
            // std::cout << std::hex << size << std::endl;
            const char *code = seg->get_data() + entry_point - vaddr;

            Memory mem;
            Cpu cpu{&mem};

            int N = 26;
            for(int j = 0; j < N; j++)
            {
                std::cout << j << std::endl;
                reg_t command = *(reg_t *)(code + j * sizeof(reg_t));
                // reg_t command = cpu.fetch();
                Instr instr = decode(command);
                execute (cpu, instr);
            }


            // int i_instr = 0;
            // for (int i = 0; i < 4; i++)
            // {
            // }
            // // std::cout << offset << std::endl;
            cpu.dump(std::cout);

        }
    }

    return 0;

}

// int main()
// {
//     std::uint32_t prog[] = { 0x14d00093, 0x40100133, 0x00209463, 0x3e700293, 0x29a00313, 0x00100073};
//
//     Memory mem;
//     addr_t entry = 0;
//
//     for (int i = 0; i < std::size(prog); i++)
//     {
//         mem.store<addr_t>(entry + i * sizeof(addr_t), prog[i]);
//     }
//
//     Cpu cpu{&mem};
//
//     while (!cpu.isdone())
//     {
//         reg_t command = cpu.fetch();
//         Instr instr = decode(command);
//         execute (cpu, instr);
//     }
//
//     cpu.dump(std::cout);
//
//     return 0;
// }


