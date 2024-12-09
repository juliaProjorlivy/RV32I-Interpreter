#ifndef RV32I_IO_HPP
#define RV32I_IO_HPP

#include "cpu.hpp"
#include "rv32i.hpp"

int elfio_manager(const char *filename, Cpu &cpu);

void write_to_mem(Cpu &cpu, int Ninstr, const char *data_ptr, addr_t entry = 0);

int run_simulation(Cpu &cpu);

#endif

