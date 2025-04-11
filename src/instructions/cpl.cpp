#include "cpu.h"

namespace gasyboy
{
    void Cpu::CPL()
    {
        uint8_t value = _registers->AF.getLeftRegister();
        _registers->AF.setLeftRegister(~value);
        _registers->AF.setFlag(Register::FlagName::H);
        _registers->AF.setFlag(Register::FlagName::N);
    }
}