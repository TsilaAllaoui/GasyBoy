#include "cpu.h"

namespace gasyboy
{
    void Cpu::CCF()
    {
        uint8_t value = _registers->AF.getRightRegister();
        value ^= 0x10;
        _registers->AF.setRightRegister(value);
        _registers->AF.clearFlag(Register::FlagName::N);
        _registers->AF.clearFlag(Register::FlagName::H);
    }
}