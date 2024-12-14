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

    const char *tracing = "trace.txt";
    if(argc > 2)
    {
        tracing = argv[2];
    }

    Memory mem{};

    Cpu cpu(&mem, tracing);

    if(elfio_manager(argv[1], cpu)) {return 1;}

    if(run_simulation(cpu)) {return 1;}

    cpu.dump(std::cout);
    return 0;
}

