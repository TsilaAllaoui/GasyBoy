#include "cpu.h"

namespace gasyboy
{
    void Cpu::RLCA()
    {
        uint8_t reg = _registers->AF.getLeftRegister();
        ((reg & 0x80) == 0x80) ? _registers->AF.setFlag(Register::FlagName::C) : _registers->AF.clearFlag(Register::FlagName::C);
        int old7bit = ((reg & 0x80) == 0x80) ? 1 : 0;
        uint8_t value = static_cast<uint8_t>((reg << 1) | (old7bit));
        _registers->AF.setLeftRegister(value);
        _registers->AF.clearFlag(Register::FlagName::Z);
        _registers->AF.clearFlag(Register::FlagName::H);
        _registers->AF.clearFlag(Register::FlagName::N);
    }

    void Cpu::RLA()
    {
        int oldCarry = _registers->AF.getFlag(Register::FlagName::C);
        (_registers->AF.getLeftRegister() & 0x80) ? _registers->AF.setFlag(Register::FlagName::C) : _registers->AF.clearFlag(Register::FlagName::C);
        uint8_t value = static_cast<uint8_t>((_registers->AF.getLeftRegister() << 1) | (oldCarry << 0));
        _registers->AF.setLeftRegister(value);
        _registers->AF.clearFlag(Register::FlagName::Z);
        _registers->AF.clearFlag(Register::FlagName::H);
        _registers->AF.clearFlag(Register::FlagName::N);
    }

    void Cpu::RLC_r(const Register::RegisterName &reg)
    {
        uint8_t value = 0;
        switch (reg)
        {
        case Register::RegisterName::A:
            (_registers->AF.getLeftRegister() & 0x80) ? _registers->AF.setFlag(Register::FlagName::C) : _registers->AF.clearFlag(Register::FlagName::C);
            _registers->AF.setLeftRegister((_registers->AF.getLeftRegister() << 1) | static_cast<uint8_t>(_registers->AF.getFlag(Register::FlagName::C)));
            _registers->AF.clearFlag(Register::FlagName::H);
            _registers->AF.clearFlag(Register::FlagName::N);
            value = _registers->AF.getLeftRegister();
            break;
        case Register::RegisterName::B:
            (_registers->BC.getLeftRegister() & 0x80) ? _registers->AF.setFlag(Register::FlagName::C) : _registers->AF.clearFlag(Register::FlagName::C);
            _registers->BC.setLeftRegister((_registers->BC.getLeftRegister() << 1) | static_cast<uint8_t>(_registers->AF.getFlag(Register::FlagName::C)));
            _registers->AF.clearFlag(Register::FlagName::H);
            _registers->AF.clearFlag(Register::FlagName::N);
            value = _registers->BC.getLeftRegister();
            break;
        case Register::RegisterName::C:
            (_registers->BC.getRightRegister() & 0x80) ? _registers->AF.setFlag(Register::FlagName::C) : _registers->AF.clearFlag(Register::FlagName::C);
            _registers->BC.setRightRegister((_registers->BC.getRightRegister() << 1) | static_cast<uint8_t>(_registers->AF.getFlag(Register::FlagName::C)));
            _registers->AF.clearFlag(Register::FlagName::H);
            _registers->AF.clearFlag(Register::FlagName::N);
            value = _registers->BC.getRightRegister();
            break;
        case Register::RegisterName::D:
            (_registers->DE.getLeftRegister() & 0x80) ? _registers->AF.setFlag(Register::FlagName::C) : _registers->AF.clearFlag(Register::FlagName::C);
            _registers->DE.setLeftRegister((_registers->DE.getLeftRegister() << 1) | static_cast<uint8_t>(_registers->AF.getFlag(Register::FlagName::C)));
            _registers->AF.clearFlag(Register::FlagName::H);
            _registers->AF.clearFlag(Register::FlagName::N);
            value = _registers->DE.getLeftRegister();
            break;
        case Register::RegisterName::E:
            (_registers->DE.getRightRegister() & 0x80) ? _registers->AF.setFlag(Register::FlagName::C) : _registers->AF.clearFlag(Register::FlagName::C);
            _registers->DE.setRightRegister((_registers->DE.getRightRegister() << 1) | static_cast<uint8_t>(_registers->AF.getFlag(Register::FlagName::C)));
            _registers->AF.clearFlag(Register::FlagName::H);
            _registers->AF.clearFlag(Register::FlagName::N);
            value = _registers->DE.getRightRegister();
            break;
        case Register::RegisterName::H:
            (_registers->HL.getLeftRegister() & 0x80) ? _registers->AF.setFlag(Register::FlagName::C) : _registers->AF.clearFlag(Register::FlagName::C);
            _registers->HL.setLeftRegister((_registers->HL.getLeftRegister() << 1) | static_cast<uint8_t>(_registers->AF.getFlag(Register::FlagName::C)));
            _registers->AF.clearFlag(Register::FlagName::H);
            _registers->AF.clearFlag(Register::FlagName::N);
            value = _registers->HL.getLeftRegister();
            break;
        case Register::RegisterName::L:
            (_registers->HL.getRightRegister() & 0x80) ? _registers->AF.setFlag(Register::FlagName::C) : _registers->AF.clearFlag(Register::FlagName::C);
            _registers->HL.setRightRegister((_registers->HL.getRightRegister() << 1) | static_cast<uint8_t>(_registers->AF.getFlag(Register::FlagName::C)));
            _registers->AF.clearFlag(Register::FlagName::H);
            _registers->AF.clearFlag(Register::FlagName::N);
            value = _registers->HL.getRightRegister();
            break;
        default:
            cout << "Flag error.";
            exit(0);
            break;
        }
        (value == 0) ? _registers->AF.setFlag(Register::FlagName::Z) : _registers->AF.clearFlag(Register::FlagName::Z);
    }

    void Cpu::RLC_16()
    {
        uint8_t value = _mmu->readRam(_registers->HL.get());
        (value & 0x80) ? _registers->AF.setFlag(Register::FlagName::C) : _registers->AF.clearFlag(Register::FlagName::C);
        uint8_t newValue = (value << 1) | static_cast<uint8_t>(_registers->AF.getFlag(Register::FlagName::C));
        _mmu->writeRam(_registers->HL.get(), newValue);
        _registers->AF.clearFlag(Register::FlagName::H);
        _registers->AF.clearFlag(Register::FlagName::N);
        (newValue == 0) ? _registers->AF.setFlag(Register::FlagName::Z) : _registers->AF.clearFlag(Register::FlagName::Z);
    }

    void Cpu::RL_16()
    {
        uint8_t value = _mmu->readRam(_registers->HL.get());
        int oldCarry = _registers->AF.getFlag(Register::FlagName::C);
        (value & 0x80) ? _registers->AF.setFlag(Register::FlagName::C) : _registers->AF.clearFlag(Register::FlagName::C);
        value = static_cast<uint8_t>((value << 1) | (oldCarry));
        (value == 0) ? _registers->AF.setFlag(Register::FlagName::Z) : _registers->AF.clearFlag(Register::FlagName::Z);
        _registers->AF.clearFlag(Register::FlagName::H);
        _registers->AF.clearFlag(Register::FlagName::N);
        _mmu->writeRam(_registers->HL.get(), value);
    }

    void Cpu::RL_r(const Register::RegisterName &reg)
    {
        uint8_t value = 0, oldCarry = _registers->AF.getFlag(Register::FlagName::C);
        switch (reg)
        {
        case Register::RegisterName::A:
            (_registers->AF.getLeftRegister() & 0x80) ? _registers->AF.setFlag(Register::FlagName::C) : _registers->AF.clearFlag(Register::FlagName::C);
            _registers->AF.setLeftRegister((_registers->AF.getLeftRegister() << 1) | oldCarry);
            _registers->AF.clearFlag(Register::FlagName::H);
            _registers->AF.clearFlag(Register::FlagName::N);
            value = _registers->AF.getLeftRegister();
            break;
        case Register::RegisterName::B:
            (_registers->BC.getLeftRegister() & 0x80) ? _registers->AF.setFlag(Register::FlagName::C) : _registers->AF.clearFlag(Register::FlagName::C);
            _registers->BC.setLeftRegister((_registers->BC.getLeftRegister() << 1) | oldCarry);
            _registers->AF.clearFlag(Register::FlagName::H);
            _registers->AF.clearFlag(Register::FlagName::N);
            value = _registers->BC.getLeftRegister();
            break;
        case Register::RegisterName::C:
            (_registers->BC.getRightRegister() & 0x80) ? _registers->AF.setFlag(Register::FlagName::C) : _registers->AF.clearFlag(Register::FlagName::C);
            _registers->BC.setRightRegister((_registers->BC.getRightRegister() << 1) | oldCarry);
            _registers->AF.clearFlag(Register::FlagName::H);
            _registers->AF.clearFlag(Register::FlagName::N);
            value = _registers->BC.getRightRegister();
            break;
        case Register::RegisterName::D:
            (_registers->DE.getLeftRegister() & 0x80) ? _registers->AF.setFlag(Register::FlagName::C) : _registers->AF.clearFlag(Register::FlagName::C);
            _registers->DE.setLeftRegister((_registers->DE.getLeftRegister() << 1) | oldCarry);
            _registers->AF.clearFlag(Register::FlagName::H);
            _registers->AF.clearFlag(Register::FlagName::N);
            value = _registers->DE.getLeftRegister();
            break;
        case Register::RegisterName::E:
            (_registers->DE.getRightRegister() & 0x80) ? _registers->AF.setFlag(Register::FlagName::C) : _registers->AF.clearFlag(Register::FlagName::C);
            _registers->DE.setRightRegister((_registers->DE.getRightRegister() << 1) | oldCarry);
            _registers->AF.clearFlag(Register::FlagName::H);
            _registers->AF.clearFlag(Register::FlagName::N);
            value = _registers->DE.getRightRegister();
            break;
        case Register::RegisterName::H:
            (_registers->HL.getLeftRegister() & 0x80) ? _registers->AF.setFlag(Register::FlagName::C) : _registers->AF.clearFlag(Register::FlagName::C);
            _registers->HL.setLeftRegister((_registers->HL.getLeftRegister() << 1) | oldCarry);
            _registers->AF.clearFlag(Register::FlagName::H);
            _registers->AF.clearFlag(Register::FlagName::N);
            value = _registers->HL.getLeftRegister();
            break;
        case Register::RegisterName::L:
            (_registers->HL.getRightRegister() & 0x80) ? _registers->AF.setFlag(Register::FlagName::C) : _registers->AF.clearFlag(Register::FlagName::C);
            _registers->HL.setRightRegister((_registers->HL.getRightRegister() << 1) | oldCarry);
            _registers->AF.clearFlag(Register::FlagName::H);
            _registers->AF.clearFlag(Register::FlagName::N);
            value = _registers->HL.getRightRegister();
            break;
        default:
            cout << "Flag error.";
            exit(0);
            break;
        }
        (value == 0) ? _registers->AF.setFlag(Register::FlagName::Z) : _registers->AF.clearFlag(Register::FlagName::Z);
    }
}