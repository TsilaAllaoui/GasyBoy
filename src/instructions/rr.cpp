#include "cpu.h"

namespace gasyboy
{
    void Cpu::RRCA()
    {
        (_registers->AF.getLeftRegister() & 0x1) ? _registers->AF.setFlag(Register::FlagName::C) : _registers->AF.clearFlag(Register::FlagName::C);
        _registers->AF.setLeftRegister((_registers->AF.getLeftRegister() >> 1) | (_registers->AF.getFlag(Register::FlagName::C) << 7));
        _registers->AF.clearFlag(Register::FlagName::H);
        _registers->AF.clearFlag(Register::FlagName::N);
        _registers->AF.clearFlag(Register::FlagName::Z);
    }

    void Cpu::RRA()
    {
        int oldCarry = _registers->AF.getFlag(Register::FlagName::C);
        (_registers->AF.getLeftRegister() & 0x1) ? _registers->AF.setFlag(Register::FlagName::C) : _registers->AF.clearFlag(Register::FlagName::C);
        uint8_t value = static_cast<uint8_t>((_registers->AF.getLeftRegister() >> 1) | (oldCarry << 7));
        _registers->AF.setLeftRegister(value);
        _registers->AF.clearFlag(Register::FlagName::Z);
        _registers->AF.clearFlag(Register::FlagName::H);
        _registers->AF.clearFlag(Register::FlagName::N);
    }

    void Cpu::RR_r(const Register::RegisterName &reg)
    {
        uint8_t value = 0;
        int oldCarry = _registers->AF.getFlag(Register::FlagName::C);
        switch (reg)
        {
        case Register::RegisterName::A:
            (_registers->AF.getLeftRegister() & 0x1) ? _registers->AF.setFlag(Register::FlagName::C) : _registers->AF.clearFlag(Register::FlagName::C);
            _registers->AF.setLeftRegister(static_cast<uint8_t>((_registers->AF.getLeftRegister() >> 1) | (oldCarry << 7)));
            _registers->AF.clearFlag(Register::FlagName::H);
            _registers->AF.clearFlag(Register::FlagName::N);
            value = _registers->AF.getLeftRegister();
            break;
        case Register::RegisterName::B:
            (_registers->BC.getLeftRegister() & 0x1) ? _registers->AF.setFlag(Register::FlagName::C) : _registers->AF.clearFlag(Register::FlagName::C);
            _registers->BC.setLeftRegister(static_cast<uint8_t>((_registers->BC.getLeftRegister() >> 1) | (oldCarry << 7)));
            _registers->AF.clearFlag(Register::FlagName::H);
            _registers->AF.clearFlag(Register::FlagName::N);
            value = _registers->BC.getLeftRegister();
            break;
        case Register::RegisterName::C:
            (_registers->BC.getRightRegister() & 0x1) ? _registers->AF.setFlag(Register::FlagName::C) : _registers->AF.clearFlag(Register::FlagName::C);
            _registers->BC.setRightRegister(static_cast<uint8_t>((_registers->BC.getRightRegister() >> 1) | (oldCarry << 7)));
            _registers->AF.clearFlag(Register::FlagName::H);
            _registers->AF.clearFlag(Register::FlagName::N);
            value = _registers->BC.getRightRegister();
            break;
        case Register::RegisterName::D:
            (_registers->DE.getLeftRegister() & 0x1) ? _registers->AF.setFlag(Register::FlagName::C) : _registers->AF.clearFlag(Register::FlagName::C);
            _registers->DE.setLeftRegister(static_cast<uint8_t>((_registers->DE.getLeftRegister() >> 1) | (oldCarry << 7)));
            _registers->AF.clearFlag(Register::FlagName::H);
            _registers->AF.clearFlag(Register::FlagName::N);
            value = _registers->DE.getLeftRegister();
            break;
        case Register::RegisterName::E:
            (_registers->DE.getRightRegister() & 0x1) ? _registers->AF.setFlag(Register::FlagName::C) : _registers->AF.clearFlag(Register::FlagName::C);
            _registers->DE.setRightRegister(static_cast<uint8_t>((_registers->DE.getRightRegister() >> 1) | (oldCarry << 7)));
            _registers->AF.clearFlag(Register::FlagName::H);
            _registers->AF.clearFlag(Register::FlagName::N);
            value = _registers->DE.getRightRegister();
            break;
        case Register::RegisterName::H:
            (_registers->HL.getLeftRegister() & 0x1) ? _registers->AF.setFlag(Register::FlagName::C) : _registers->AF.clearFlag(Register::FlagName::C);
            _registers->HL.setLeftRegister(static_cast<uint8_t>((_registers->HL.getLeftRegister() >> 1) | (oldCarry << 7)));
            _registers->AF.clearFlag(Register::FlagName::H);
            _registers->AF.clearFlag(Register::FlagName::N);
            value = _registers->HL.getLeftRegister();
            break;
        case Register::RegisterName::L:
            (_registers->HL.getRightRegister() & 0x1) ? _registers->AF.setFlag(Register::FlagName::C) : _registers->AF.clearFlag(Register::FlagName::C);
            _registers->HL.setRightRegister(static_cast<uint8_t>((_registers->HL.getRightRegister() >> 1) | (oldCarry << 7)));
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

    void Cpu::RR_16() // TODO may be innacurate
    {
        int oldCarry = _registers->AF.getFlag(Register::FlagName::C);
        uint8_t value = _mmu->readRam(_registers->HL.get());
        (value & 0x1) ? _registers->AF.setFlag(Register::FlagName::C) : _registers->AF.clearFlag(Register::FlagName::C);
        value = static_cast<uint8_t>((value >> 1) | (oldCarry << 7));
        _mmu->writeRam(_registers->HL.get(), value);
        (value == 0) ? _registers->AF.setFlag(Register::FlagName::Z) : _registers->AF.clearFlag(Register::FlagName::Z);
        _registers->AF.clearFlag(Register::FlagName::H);
        _registers->AF.clearFlag(Register::FlagName::N);
    }

    void Cpu::RRC_r(const Register::RegisterName &reg)
    {
        uint8_t value = 0;
        switch (reg)
        {
        case Register::RegisterName::A:
            (_registers->AF.getLeftRegister() & 0x1) ? _registers->AF.setFlag(Register::FlagName::C) : _registers->AF.clearFlag(Register::FlagName::C);
            _registers->AF.setLeftRegister((_registers->AF.getLeftRegister() >> 1) | (_registers->AF.getFlag(Register::FlagName::C) << 7));
            _registers->AF.clearFlag(Register::FlagName::H);
            _registers->AF.clearFlag(Register::FlagName::N);
            value = _registers->AF.getLeftRegister();
            break;
        case Register::RegisterName::B:
            (_registers->BC.getLeftRegister() & 0x1) ? _registers->AF.setFlag(Register::FlagName::C) : _registers->AF.clearFlag(Register::FlagName::C);
            _registers->BC.setLeftRegister((_registers->BC.getLeftRegister() >> 1) | (_registers->AF.getFlag(Register::FlagName::C) << 7));
            _registers->AF.clearFlag(Register::FlagName::H);
            _registers->AF.clearFlag(Register::FlagName::N);
            value = _registers->BC.getLeftRegister();
            break;
        case Register::RegisterName::C:
            (_registers->BC.getRightRegister() & 0x1) ? _registers->AF.setFlag(Register::FlagName::C) : _registers->AF.clearFlag(Register::FlagName::C);
            _registers->BC.setRightRegister((_registers->BC.getRightRegister() >> 1) | (_registers->AF.getFlag(Register::FlagName::C) << 7));
            _registers->AF.clearFlag(Register::FlagName::H);
            _registers->AF.clearFlag(Register::FlagName::N);
            value = _registers->BC.getRightRegister();
            break;
        case Register::RegisterName::D:
            (_registers->DE.getLeftRegister() & 0x1) ? _registers->AF.setFlag(Register::FlagName::C) : _registers->AF.clearFlag(Register::FlagName::C);
            _registers->DE.setLeftRegister((_registers->DE.getLeftRegister() >> 1) | (_registers->AF.getFlag(Register::FlagName::C) << 7));
            _registers->AF.clearFlag(Register::FlagName::H);
            _registers->AF.clearFlag(Register::FlagName::N);
            value = _registers->DE.getLeftRegister();
            break;
        case Register::RegisterName::E:
            (_registers->DE.getRightRegister() & 0x1) ? _registers->AF.setFlag(Register::FlagName::C) : _registers->AF.clearFlag(Register::FlagName::C);
            _registers->DE.setRightRegister((_registers->DE.getRightRegister() >> 1) | (_registers->AF.getFlag(Register::FlagName::C) << 7));
            _registers->AF.clearFlag(Register::FlagName::H);
            _registers->AF.clearFlag(Register::FlagName::N);
            value = _registers->DE.getRightRegister();
            break;
        case Register::RegisterName::H:
            (_registers->HL.getLeftRegister() & 0x1) ? _registers->AF.setFlag(Register::FlagName::C) : _registers->AF.clearFlag(Register::FlagName::C);
            _registers->HL.setLeftRegister((_registers->HL.getLeftRegister() >> 1) | (_registers->AF.getFlag(Register::FlagName::C) << 7));
            _registers->AF.clearFlag(Register::FlagName::H);
            _registers->AF.clearFlag(Register::FlagName::N);
            value = _registers->HL.getLeftRegister();
            break;
        case Register::RegisterName::L:
            (_registers->HL.getRightRegister() & 0x1) ? _registers->AF.setFlag(Register::FlagName::C) : _registers->AF.clearFlag(Register::FlagName::C);
            _registers->HL.setRightRegister((_registers->HL.getRightRegister() >> 1) | (_registers->AF.getFlag(Register::FlagName::C) << 7));
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

    void Cpu::RRC_16()
    {
        uint8_t value = _mmu->readRam(_registers->HL.get());
        (value & 0x1) ? _registers->AF.setFlag(Register::FlagName::C) : _registers->AF.clearFlag(Register::FlagName::C);
        value = ((value >> 1) | (_registers->AF.getFlag(Register::FlagName::C) << 7));
        (value == 0) ? _registers->AF.setFlag(Register::FlagName::Z) : _registers->AF.clearFlag(Register::FlagName::Z);
        _registers->AF.clearFlag(Register::FlagName::H);
        _registers->AF.clearFlag(Register::FlagName::N);
        _mmu->writeRam(_registers->HL.get(), value);
    }

    void Cpu::SLA_r(const Register::RegisterName &reg)
    {
        uint8_t value = 0;
        switch (reg)
        {
        case Register::RegisterName::A:
            (_registers->AF.getLeftRegister() & 0x80) ? _registers->AF.setFlag(Register::FlagName::C) : _registers->AF.clearFlag(Register::FlagName::C);
            _registers->AF.setLeftRegister((_registers->AF.getLeftRegister() << 1));
            _registers->AF.clearFlag(Register::FlagName::H);
            _registers->AF.clearFlag(Register::FlagName::N);
            value = _registers->AF.getLeftRegister();
            break;
        case Register::RegisterName::B:
            (_registers->BC.getLeftRegister() & 0x80) ? _registers->AF.setFlag(Register::FlagName::C) : _registers->AF.clearFlag(Register::FlagName::C);
            _registers->BC.setLeftRegister((_registers->BC.getLeftRegister() << 1));
            _registers->AF.clearFlag(Register::FlagName::H);
            _registers->AF.clearFlag(Register::FlagName::N);
            value = _registers->BC.getLeftRegister();
            break;
        case Register::RegisterName::C:
            (_registers->BC.getRightRegister() & 0x80) ? _registers->AF.setFlag(Register::FlagName::C) : _registers->AF.clearFlag(Register::FlagName::C);
            _registers->BC.setRightRegister((_registers->BC.getRightRegister() << 1));
            _registers->AF.clearFlag(Register::FlagName::H);
            _registers->AF.clearFlag(Register::FlagName::N);
            value = _registers->BC.getRightRegister();
            break;
        case Register::RegisterName::D:
            (_registers->DE.getLeftRegister() & 0x80) ? _registers->AF.setFlag(Register::FlagName::C) : _registers->AF.clearFlag(Register::FlagName::C);
            _registers->DE.setLeftRegister((_registers->DE.getLeftRegister() << 1));
            _registers->AF.clearFlag(Register::FlagName::H);
            _registers->AF.clearFlag(Register::FlagName::N);
            value = _registers->DE.getLeftRegister();
            break;
        case Register::RegisterName::E:
            (_registers->DE.getRightRegister() & 0x80) ? _registers->AF.setFlag(Register::FlagName::C) : _registers->AF.clearFlag(Register::FlagName::C);
            _registers->DE.setRightRegister((_registers->DE.getRightRegister() << 1));
            _registers->AF.clearFlag(Register::FlagName::H);
            _registers->AF.clearFlag(Register::FlagName::N);
            value = _registers->DE.getRightRegister();
            break;
        case Register::RegisterName::H:
            (_registers->HL.getLeftRegister() & 0x80) ? _registers->AF.setFlag(Register::FlagName::C) : _registers->AF.clearFlag(Register::FlagName::C);
            _registers->HL.setLeftRegister((_registers->HL.getLeftRegister() << 1));
            _registers->AF.clearFlag(Register::FlagName::H);
            _registers->AF.clearFlag(Register::FlagName::N);
            value = _registers->HL.getLeftRegister();
            break;
        case Register::RegisterName::L:
            (_registers->HL.getRightRegister() & 0x80) ? _registers->AF.setFlag(Register::FlagName::C) : _registers->AF.clearFlag(Register::FlagName::C);
            _registers->HL.setRightRegister((_registers->HL.getRightRegister() << 1));
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