#ifndef RV32I_IO_HPP
#define RV32I_IO_HPP

#include "cpu.hpp"
#include "rv32i.hpp"

int elfio_manager(const char *filename, Memory &mem, addr_t &main_entry_offset);

void write_to_mem(Memory &mem, int Ninstr, const char *data_ptr, addr_t entry = 0);

int run_simulation(Cpu &cpu);

#endif

