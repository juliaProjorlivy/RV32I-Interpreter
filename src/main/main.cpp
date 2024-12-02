#include "cpu.hpp"
#include "rv32i.hpp"
#include <elfio/elfio.hpp>
#include <elfio/elf_types.hpp>
#include <elfio/elfio_segment.hpp>

void write_to_mem(Memory &mem, int Ninstr, const char *data_ptr, addr_t entry = 0)
{
    for (int j = 0; j < Ninstr; ++j)
    {
        mem.store<addr_t>(entry + j * sizeof(addr_t), *(reinterpret_cast<const reg_t *>(data_ptr + j * sizeof(reg_t))));
    }
}

void do_program(int Ninstr, Memory *mem, addr_t entry_point = 0)
{
    Cpu cpu{mem, entry_point};

    while(!cpu.isdone())
    {
        // reg_t command = cpu.fetch();
        if(auto basic_block= cpu.bb_translated.find(cpu.getPc()); basic_block != cpu.bb_translated.end())
        {
            (basic_block->second)();
            continue;
        }
        else if(cpu.bb_cache.count(cpu.getPc()))
        {
            auto cache_block= cpu.bb_cache.find(cpu.getPc());
            if(cache_block->second.size() >= BB_THRESHOLD)
            {
                auto func = translate(cpu, cache_block->second);
                if(func)
                {
                    cpu.bb_translated.emplace(cpu.getPc(), func);
                    func();
                    continue;
                }
                else
                {
                    std::cout << "TRNASLATION ERROR\n";
                    return;
                }
            }
            //TODO: HANDLE AN ERROR
        }
        auto instrs = lookup(cpu, cpu.getPc());
        interpret_block (cpu, instrs);
    }

    cpu.dump(std::cout);
}

int main(int argc, char* argv[])
{
    if(argc < 2)
    {
        std::cout << "Too little arguments" << std::endl;
        return 1;
    }

    ELFIO::elfio reader;
    if(!reader.load( argv[1]))
    {
        std::cout << "Can't find or process ELF file " << argv[1] << std::endl;
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

    Memory mem {};

    int Ninstr = 0;
    addr_t main_entry_offset = 0;

    for(int i = 0; i < seg_num; i++)
    {
        const ELFIO::segment *seg = reader.segments[i];
        if(seg->get_type() == ELFIO::PT_LOAD && seg->get_flags() == (ELFIO::PF_X | ELFIO::PF_R))
        {
            Ninstr = (seg->get_file_size() - code_start_offset) / sizeof(addr_t);
            main_entry_offset = entry_point - seg->get_virtual_address() - code_start_offset;
            const char *start = seg->get_data() + code_start_offset;

            write_to_mem(mem, Ninstr, start);
        }
    }

    do_program(Ninstr, &mem, main_entry_offset);
    return 0;
}

