#include "cpu.h"

namespace gasyboy
{
    void Cpu::DAA()
    {
        if (!_registers->AF.getFlag(Register::FlagName::N))
        {
            if (_registers->AF.getFlag(Register::FlagName::C) || (_registers->AF.getLeftRegister() > 0x99))
            {
                uint8_t value = _registers->AF.getLeftRegister();
                _registers->AF.setLeftRegister(value + 0x60);
                _registers->AF.setFlag(Register::FlagName::C);
            }
            if (_registers->AF.getFlag(Register::FlagName::H) || ((_registers->AF.getLeftRegister() & 0x0F) > 0x09))
            {
                uint8_t value = _registers->AF.getLeftRegister();
                _registers->AF.setLeftRegister(value + 0x06);
            }
        }
        else
        {
            if (_registers->AF.getFlag(Register::FlagName::C))
            {
                uint8_t value = _registers->AF.getLeftRegister();
                _registers->AF.setLeftRegister(value - 0x60);
            }
            if (_registers->AF.getFlag(Register::FlagName::H))
            {
                uint8_t value = _registers->AF.getLeftRegister();
                _registers->AF.setLeftRegister(value - 0x06);
            }
        }
        (_registers->AF.getLeftRegister() == 0) ? _registers->AF.setFlag(Register::FlagName::Z) : _registers->AF.clearFlag(Register::FlagName::Z);
        _registers->AF.clearFlag(Register::FlagName::H);
    }
}