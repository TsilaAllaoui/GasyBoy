#include "cpu.h"

namespace gasyboy
{
    void Cpu::SUB_r(const Register::RegisterName &reg)
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
        (_registers->AF.getLeftRegister() - value < 0) ? _registers->AF.setFlag(Register::FlagName::C) : _registers->AF.clearFlag(Register::FlagName::C);
        (checkSubHalfCarry(_registers->AF.getLeftRegister(), value)) ? _registers->AF.setFlag(Register::FlagName::H) : _registers->AF.clearFlag(Register::FlagName::H);
        _registers->AF.setLeftRegister(_registers->AF.getLeftRegister() - value);
        (_registers->AF.getLeftRegister() == 0) ? _registers->AF.setFlag(Register::FlagName::Z) : _registers->AF.clearFlag(Register::FlagName::Z);
        _registers->AF.setFlag(Register::FlagName::N);
    }

    void Cpu::SUB_n(const uint8_t &value)
    {
        (_registers->AF.getLeftRegister() - value < 0) ? _registers->AF.setFlag(Register::FlagName::C) : _registers->AF.clearFlag(Register::FlagName::C);
        (checkSubHalfCarry(_registers->AF.getLeftRegister(), value)) ? _registers->AF.setFlag(Register::FlagName::H) : _registers->AF.clearFlag(Register::FlagName::H);
        _registers->AF.setLeftRegister(_registers->AF.getLeftRegister() - value);
        (_registers->AF.getLeftRegister() == 0) ? _registers->AF.setFlag(Register::FlagName::Z) : _registers->AF.clearFlag(Register::FlagName::Z);
        _registers->AF.setFlag(Register::FlagName::N);
    }

    void Cpu::SUB_16()
    {
        uint8_t value = _mmu->readRam(_registers->HL.get());
        (_registers->AF.getLeftRegister() - value < 0) ? _registers->AF.setFlag(Register::FlagName::C) : _registers->AF.clearFlag(Register::FlagName::C);
        (checkSubHalfCarry(_registers->AF.getLeftRegister(), value)) ? _registers->AF.setFlag(Register::FlagName::H) : _registers->AF.clearFlag(Register::FlagName::H);
        _registers->AF.setLeftRegister(_registers->AF.getLeftRegister() - value);
        (_registers->AF.getLeftRegister() == 0) ? _registers->AF.setFlag(Register::FlagName::Z) : _registers->AF.clearFlag(Register::FlagName::Z);
        _registers->AF.setFlag(Register::FlagName::N);
    }
}