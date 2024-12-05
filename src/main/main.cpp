#include "io.hpp"
#include <elfio/elfio.hpp>
#include <elfio/elf_types.hpp>
#include <elfio/elfio_segment.hpp>

int main(int argc, char* argv[])
{
    if(argc < 2)
    {
        std::cout << "Too little arguments" << std::endl;
        return 1;
    }

    Memory mem{};
    addr_t main_entry_offset = 0;
    if(elfio_manager(argv[1], mem, main_entry_offset)) {return 1;}

    Cpu cpu(&mem, main_entry_offset);
    if(run_simulation(cpu)) {return 1;}

    cpu.dump(std::cout);
    return 0;
}

