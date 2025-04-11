#include "cpu.h"

namespace gasyboy
{
    void Cpu::INC_r(const Register::RegisterName &reg)
    {
        uint8_t value = 0, oldReg = 0;
        switch (reg)
        {
        case Register::RegisterName::A:
            oldReg = _registers->AF.getLeftRegister();
            value = _registers->AF.getLeftRegister() + 1;
            _registers->AF.setLeftRegister(value);
            break;
        case Register::RegisterName::F:
            oldReg = _registers->AF.getRightRegister();
            value = _registers->AF.getRightRegister() + 1;
            _registers->AF.setRightRegister(value);
            break;
        case Register::RegisterName::B:
            oldReg = _registers->BC.getLeftRegister();
            value = _registers->BC.getLeftRegister() + 1;
            _registers->BC.setLeftRegister(value);
            break;
        case Register::RegisterName::C:
            oldReg = _registers->BC.getRightRegister();
            value = _registers->BC.getRightRegister() + 1;
            _registers->BC.setRightRegister(value);
            break;
        case Register::RegisterName::D:
            oldReg = _registers->DE.getLeftRegister();
            value = _registers->DE.getLeftRegister() + 1;
            _registers->DE.setLeftRegister(value);
            break;
        case Register::RegisterName::E:
            oldReg = _registers->DE.getRightRegister();
            value = _registers->DE.getRightRegister() + 1;
            _registers->DE.setRightRegister(value);
            break;
        case Register::RegisterName::H:
            oldReg = _registers->HL.getLeftRegister();
            value = _registers->HL.getLeftRegister() + 1;
            _registers->HL.setLeftRegister(value);
            break;
        case Register::RegisterName::L:
            oldReg = _registers->HL.getRightRegister();
            value = _registers->HL.getRightRegister() + 1;
            _registers->HL.setRightRegister(value);
            break;
        default:
            cout << "Flag error.";
            exit(0);
            break;
        }
        (value == 0) ? _registers->AF.setFlag(Register::FlagName::Z) : _registers->AF.clearFlag(Register::FlagName::Z);
        checkAddHalfCarry(oldReg, 1) ? _registers->AF.setFlag(Register::FlagName::H) : _registers->AF.clearFlag(Register::FlagName::H);
        _registers->AF.clearFlag(Register::FlagName::N);
    }

    void Cpu::INC_16()
    {
        uint8_t oldValue = _mmu->readRam(_registers->HL.get());
        uint8_t value = _mmu->readRam(_registers->HL.get()) + 1;
        _mmu->writeRam(_registers->HL.get(), value);
        (value == 0) ? _registers->AF.setFlag(Register::FlagName::Z) : _registers->AF.clearFlag(Register::FlagName::Z);
        checkAddHalfCarry(oldValue, 1) ? _registers->AF.setFlag(Register::FlagName::H) : _registers->AF.clearFlag(Register::FlagName::H);
        _registers->AF.clearFlag(Register::FlagName::N);
    }

    void Cpu::INC_rr(const Register::RegisterPairName &reg)
    {
        uint16_t value = 0;
        if (reg == Register::RegisterPairName::AF)
        {
            value = _registers->AF.get() + 1;
            _registers->AF.set(value);
        }
        else if (reg == Register::RegisterPairName::BC)
        {
            value = _registers->BC.get() + 1;
            _registers->BC.set(value);
        }
        else if (reg == Register::RegisterPairName::DE)
        {
            value = _registers->DE.get() + 1;
            _registers->DE.set(value);
        }
        else if (reg == Register::RegisterPairName::HL)
        {
            value = _registers->HL.get() + 1;
            _registers->HL.set(value);
        }
        else if (reg == Register::RegisterPairName::SP)
        {
            value = _registers->SP + 1;
            _registers->SP++;
        }
        else
            exit(2);
    }
}