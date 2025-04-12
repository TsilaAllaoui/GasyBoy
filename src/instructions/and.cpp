#include "cpu.h"

namespace gasyboy
{
    void Cpu::AND_r(const Register::RegisterName &reg)
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
        uint8_t result = static_cast<uint8_t>(_registers->AF.getLeftRegister() & value);
        _registers->AF.setLeftRegister(result);
        (result == 0) ? _registers->AF.setFlag(Register::FlagName::Z) : _registers->AF.clearFlag(Register::FlagName::Z);
        _registers->AF.setFlag(Register::FlagName::H);
        _registers->AF.clearFlag(Register::FlagName::N);
        _registers->AF.clearFlag(Register::FlagName::C);
    }

    void Cpu::AND_n(const uint8_t &value)
    {
        uint8_t result = static_cast<uint8_t>(_registers->AF.getLeftRegister() & value);
        _registers->AF.setLeftRegister(result);
        (result == 0) ? _registers->AF.setFlag(Register::FlagName::Z) : _registers->AF.clearFlag(Register::FlagName::Z);
        _registers->AF.setFlag(Register::FlagName::H);
        _registers->AF.clearFlag(Register::FlagName::N);
        _registers->AF.clearFlag(Register::FlagName::C);
    }

    void Cpu::AND_16()
    {
        uint8_t value = _mmu->readRam(_registers->HL.get());
        uint8_t result = static_cast<uint8_t>(_registers->AF.getLeftRegister() & value);
        _registers->AF.setLeftRegister(result);
        (result == 0) ? _registers->AF.setFlag(Register::FlagName::Z) : _registers->AF.clearFlag(Register::FlagName::Z);
        _registers->AF.setFlag(Register::FlagName::H);
        _registers->AF.clearFlag(Register::FlagName::N);
        _registers->AF.clearFlag(Register::FlagName::C);
    }
}