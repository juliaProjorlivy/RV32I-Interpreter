#include "io.hpp"
#include "rv32i.hpp"
#include <elfio/elfio.hpp>
#include <elfio/elf_types.hpp>
#include <elfio/elfio_segment.hpp>
#include <vector>

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
    //virtual main address
    ELFIO::Elf64_Addr entry_point = reader.get_entry();

    for(int i = 0; i < seg_num; i++)
    {
        const ELFIO::segment *seg = reader.segments[i];
        if(seg->get_type() == ELFIO::PT_LOAD && seg->get_flags() == (ELFIO::PF_X | ELFIO::PF_R))
        {
            const char *start = seg->get_data();

            cpu.setPc(entry_point - seg->get_virtual_address());

            cpu.store(0, (const void *)start, seg->get_file_size());
        }
    }

    return 0;
}

bool is_bb_end(Instr &instr)
{
    switch (instr.opcode)
    {
        case Opcode::System:
            {
                if(static_cast<I::System::funct3>(instr.funct3) == I::System::funct3::EBREAK) {return true;}
                return false;
            }
        case Opcode::Branch:
        case Opcode::Jal:
        case Opcode::Jalr:
            return true;
        default:
            return false;
    }
    return false;
}

std::vector<Instr> lookup(Cpu &cpu, addr_t addr)
{
    auto basic_block_res = cpu.bb_cache.find(addr);

    //if bb was not found -> update bb_cache
    if(basic_block_res == cpu.bb_cache.end())
    {
        Instr cur_instr {};
        addr_t cur_addr = addr;
        std::vector<Instr> bb;
        bb.reserve(BB_AVERAGE_SIZE);

        do
        {
            reg_t command = cpu.fetch(cur_addr);
            cur_instr = decode(command);
            bb.push_back(cur_instr);
            cur_addr += cur_instr.size;
        } while (!is_bb_end(cur_instr));

        basic_block_res = cpu.bb_cache.emplace(addr, bb).first;
    }

    return basic_block_res->second;
}

int run_simulation(Cpu &cpu)
{
    while(!cpu.isdone())
    {
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
                    return 1;
                }
            }
            //TODO: HANDLE AN ERROR
        }
        std::vector<Instr> instrs = lookup(cpu, cpu.getPc());
        interpret_block (cpu, instrs);
    }

    return 0;
}

