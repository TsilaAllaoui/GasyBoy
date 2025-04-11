#include "cpu.h"

namespace gasyboy
{
    void Cpu::RST_p(const uint16_t &p)
    {
        _registers->SP--;
        _mmu->writeRam(_registers->SP, (((_registers->PC + 1) & 0xFF00) >> 8));
        _registers->SP--;
        _mmu->writeRam(_registers->SP, ((_registers->PC + 1) & 0x00FF));
        _registers->PC = p;
    }
}