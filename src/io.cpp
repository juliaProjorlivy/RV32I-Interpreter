#include "io.hpp"
#include "rv32i.hpp"
#include <elfio/elfio.hpp>
#include <elfio/elf_types.hpp>
#include <elfio/elfio_segment.hpp>

void write_to_mem(Cpu &cpu, int Ninstr, const char *data_ptr, addr_t entry)
{
    for (int j = 0; j < Ninstr; ++j)
    {
        cpu.store<addr_t>(entry + j * sizeof(addr_t), *(reinterpret_cast<const reg_t *>(data_ptr + j * sizeof(reg_t))));
    }
}

int elfio_manager(const char *filename, Cpu &cpu)
{
    ELFIO::elfio reader;
    if(!reader.load(filename))
    {
        std::cout << "Can't find or process ELF file " << filename << std::endl;
        return 1;
    }
    if(reader.get_machine() != ELFIO::EM_RISCV)
    {
        std::cout << "Not RISC-V32" << std::endl;
        return 1;
    }
    if(reader.get_class() != ELFIO::ELFCLASS32)
    {
        std::cout << "Not ELF32" << std::endl;
        return 1;
    }

    ELFIO::Elf_Half seg_num = reader.segments.size();
    ELFIO::Elf64_Addr entry_point = reader.get_entry();

    addr_t seg_offset = reader.get_segments_offset();
    addr_t seg_header_size = reader.get_segment_entry_size();
    addr_t code_start_offset = seg_offset + seg_num * seg_header_size;

    int Ninstr = 0;

    for(int i = 0; i < seg_num; i++)
    {
        const ELFIO::segment *seg = reader.segments[i];
        if(seg->get_type() == ELFIO::PT_LOAD && seg->get_flags() == (ELFIO::PF_X | ELFIO::PF_R))
        {
            Ninstr = (seg->get_file_size() - code_start_offset) / sizeof(addr_t);
            addr_t main_entry_offset = entry_point - seg->get_virtual_address() - code_start_offset;
            //TODO: LOAD ALL
            const char *start = seg->get_data() + code_start_offset;

            cpu.setPc(main_entry_offset);

            write_to_mem(cpu, Ninstr, start);
        }
    }

    return 0;
}

int run_simulation(Cpu &cpu)
{
    while(!cpu.isdone())
    {
        // if(auto basic_block= cpu.bb_translated.find(cpu.getPc()); basic_block != cpu.bb_translated.end())
        // {
        //     (basic_block->second)();
        //     continue;
        // }
        // else if(cpu.bb_cache.count(cpu.getPc()))
        // {
        //     auto cache_block= cpu.bb_cache.find(cpu.getPc());
        //     if(cache_block->second.size() >= BB_THRESHOLD)
        //     {
        //         auto func = translate(cpu, cache_block->second);
        //         if(func)
        //         {
        //             cpu.bb_translated.emplace(cpu.getPc(), func);
        //             func();
        //             continue;
        //         }
        //         else
        //         {
        //             std::cout << "TRNASLATION ERROR\n";
        //             return 1;
        //         }
        //     }
        //     //TODO: HANDLE AN ERROR
        // }
        auto instrs = lookup(cpu, cpu.getPc());
        interpret_block (cpu, instrs);
    }

    return 0;
}


