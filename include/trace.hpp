#include "rv32i.hpp"
#include <fstream>

class Trace
{
private:
    std::ofstream trace_out_file;
public:
    Trace(const char *filename = "trace.log") : trace_out_file(filename, std::ios::out) {}
    ~Trace() {trace_out_file.close();}

    void MemoryWrite(addr_t addr, reg_t val)
    {
        trace_out_file << "Write to addr=" << std::ofstream::hex << addr << ";val=" << val << std::endl;
    }
    void MemoryRead(addr_t addr, reg_t val)
    {
        trace_out_file << "Read from addr=" << std::ofstream::hex << addr << ";val=" << val << std::endl;
    }
    void RegisterChange(int reg_id, reg_t val)
    {
        trace_out_file << "Write to reg=x" << reg_id << ";val=" << val << std::endl;
    }
    void Interrupt(int interrupt_number)
    {
        trace_out_file << "Interrupt number=" << interrupt_number << std::endl;
    }
};
