#include "cpu.h"

namespace gasyboy
{
    void Cpu::DEC_r(const Register::RegisterName &reg)
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
        switch (reg)
        {
        case Register::RegisterName::A:
            _registers->AF.setLeftRegister(value - 1);
            break;
        case Register::RegisterName::F:
            _registers->AF.setRightRegister(value - 1);
            break;
        case Register::RegisterName::B:
            _registers->BC.setLeftRegister(value - 1);
            break;
        case Register::RegisterName::C:
            _registers->BC.setRightRegister(value - 1);
            break;
        case Register::RegisterName::D:
            _registers->DE.setLeftRegister(value - 1);
            break;
        case Register::RegisterName::E:
            _registers->DE.setRightRegister(value - 1);
            break;
        case Register::RegisterName::H:
            _registers->HL.setLeftRegister(value - 1);
            break;
        case Register::RegisterName::L:
            _registers->HL.setRightRegister(value - 1);
            break;
        default:
            cout << "Flag error.";
            exit(0);
            break;
        }
        ((value - 1) == 0) ? _registers->AF.setFlag(Register::FlagName::Z) : _registers->AF.clearFlag(Register::FlagName::Z);
        (checkSubHalfCarry(value, 1)) ? _registers->AF.setFlag(Register::FlagName::H) : _registers->AF.clearFlag(Register::FlagName::H);
        _registers->AF.setFlag(Register::FlagName::N);
    }

    void Cpu::DEC_16()
    {
        uint8_t value = _mmu->readRam(_registers->HL.get());
        _mmu->writeRam(_registers->HL.get(), value - 1);
        ((value - 1) == 0) ? _registers->AF.setFlag(Register::FlagName::Z) : _registers->AF.clearFlag(Register::FlagName::Z);
        (checkSubHalfCarry(value, 1)) ? _registers->AF.setFlag(Register::FlagName::H) : _registers->AF.clearFlag(Register::FlagName::H);
        _registers->AF.setFlag(Register::FlagName::N);
    }

    void Cpu::DEC_rr(const Register::RegisterPairName &reg)
    {
        uint16_t value = 0;
        if (reg == Register::RegisterPairName::AF)
        {
            value = _registers->AF.get() - 1;
            _registers->AF.set(value);
        }
        else if (reg == Register::RegisterPairName::BC)
        {
            value = _registers->BC.get() - 1;
            _registers->BC.set(value);
        }
        else if (reg == Register::RegisterPairName::DE)
        {
            value = _registers->DE.get() - 1;
            _registers->DE.set(value);
        }
        else if (reg == Register::RegisterPairName::HL)
        {
            value = _registers->HL.get() - 1;
            _registers->HL.set(value);
        }
        else if (reg == Register::RegisterPairName::SP)
        {
            value = _registers->SP - 1;
            _registers->SP--;
        }
        else
            exit(2);
    }
}