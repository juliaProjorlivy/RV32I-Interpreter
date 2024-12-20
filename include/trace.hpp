#include "rv32i.hpp"
#include <fstream>
#include <iomanip>

class Trace
{
private:
    std::ofstream trace_out_file;
public:
    Trace(const char *filename = "trace.txt") : trace_out_file(filename, std::ios::trunc) {}
    ~Trace() {trace_out_file.close();}

    void MemoryWrite(addr_t addr, reg_t val)
    {
        trace_out_file << "[0x" << std::setw(8) << std::setfill('0') << std::hex << addr << "] <== <" << std::dec << val << ">" << std::endl;
        trace_out_file << "mem change (store)" << std::endl;
    }
    void MemoryRead(addr_t addr, reg_t val)
    {
        trace_out_file << "[0x" << std::setw(8) << std::setfill('0') << std::hex << addr << "] ==> <" << std::dec << val << ">" << std::endl;
        trace_out_file << "mem load" << std::endl;
    }
    void RegisterChange(int reg_id, reg_t val)
    {
        trace_out_file << "x" << std::dec << reg_id << " <== <" << std::dec << val << ">" <<  std::endl;
        trace_out_file << "reg write"<< std::endl;
    }
    void PcChange(reg_t val)
    {
        trace_out_file << "pc=0x" << std::setw(8) << std::setfill('0') << std::hex << val << std::endl;
    }
    void Interrupt(int interrupt_number)
    {
        trace_out_file << "interrupt " << std::dec << interrupt_number << std::endl;
    }
};
