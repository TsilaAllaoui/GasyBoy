#include "cpu.h"

namespace gasyboy
{
    void Cpu::CP_r(const Register::RegisterName &reg)
    {
        uint8_t value = 0;
        switch (reg)
        {
        case Register::RegisterName::A:
            value = _registers->AF.getLeftRegister();
            break;
        case Register::RegisterName::F:
            value = _registers->AF.getRightRegister();
            break;
        case Register::RegisterName::B:
            value = _registers->BC.getLeftRegister();
            break;
        case Register::RegisterName::C:
            value = _registers->BC.getRightRegister();
            break;
        case Register::RegisterName::D:
            value = _registers->DE.getLeftRegister();
            break;
        case Register::RegisterName::E:
            value = _registers->DE.getRightRegister();
            break;
        case Register::RegisterName::H:
            value = _registers->HL.getLeftRegister();
            break;
        case Register::RegisterName::L:
            value = _registers->HL.getRightRegister();
            break;
        default:
            cout << "Flag error.";
            exit(0);
            break;
        }
        ((uint16_t)_registers->AF.getLeftRegister() - (uint16_t)value < 0) ? _registers->AF.setFlag(Register::FlagName::C) : _registers->AF.clearFlag(Register::FlagName::C);
        (checkSubHalfCarry(_registers->AF.getLeftRegister(), value)) ? _registers->AF.setFlag(Register::FlagName::H) : _registers->AF.clearFlag(Register::FlagName::H);
        (_registers->AF.getLeftRegister() - value == 0) ? _registers->AF.setFlag(Register::FlagName::Z) : _registers->AF.clearFlag(Register::FlagName::Z);
        _registers->AF.setFlag(Register::FlagName::N);
    }

    void Cpu::CP_n(const uint8_t &value)
    {
        uint8_t reg = _registers->AF.getLeftRegister();
        uint8_t result = static_cast<uint8_t>(reg - value);

        (result == 0) ? _registers->AF.setFlag(Register::FlagName::Z) : _registers->AF.clearFlag(Register::FlagName::Z);
        _registers->AF.setFlag(Register::FlagName::N);
        (((reg & 0xf) - (value & 0xf)) < 0) ? _registers->AF.setFlag(Register::FlagName::H) : _registers->AF.clearFlag(Register::FlagName::H);
        (reg < value) ? _registers->AF.setFlag(Register::FlagName::C) : _registers->AF.clearFlag(Register::FlagName::C);
    }

    void Cpu::CP_16()
    {
        uint8_t value = _mmu->readRam(_registers->HL.get());
        ((uint16_t)_registers->AF.getLeftRegister() - (uint16_t)value < 0) ? _registers->AF.setFlag(Register::FlagName::C) : _registers->AF.clearFlag(Register::FlagName::C);
        (checkSubHalfCarry(_registers->AF.getLeftRegister(), value)) ? _registers->AF.setFlag(Register::FlagName::H) : _registers->AF.clearFlag(Register::FlagName::H);
        (_registers->AF.getLeftRegister() - value == 0) ? _registers->AF.setFlag(Register::FlagName::Z) : _registers->AF.clearFlag(Register::FlagName::Z);
        _registers->AF.setFlag(Register::FlagName::N);
    }
}
