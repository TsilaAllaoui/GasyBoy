#include "cpu.h"

namespace gasyboy
{
    void Cpu::SLA_16()
    {
        uint8_t value = _mmu->readRam(_registers->HL.get());
        (value & 0x80) ? _registers->AF.setFlag(Register::FlagName::C) : _registers->AF.clearFlag(Register::FlagName::C);
        _mmu->writeRam(_registers->HL.get(), (value << 1));
        _registers->AF.clearFlag(Register::FlagName::H);
        _registers->AF.clearFlag(Register::FlagName::N);
        value = _mmu->readRam(_registers->HL.get());
        (value == 0) ? _registers->AF.setFlag(Register::FlagName::Z) : _registers->AF.clearFlag(Register::FlagName::Z);
    }

    void Cpu::SRA_r(const Register::RegisterName &reg)
    {
        uint8_t value = 0;
        uint8_t old7thbit = 0;
        switch (reg)
        {
        case Register::RegisterName::A:
            (_registers->AF.getLeftRegister() & 0x1) ? _registers->AF.setFlag(Register::FlagName::C) : _registers->AF.clearFlag(Register::FlagName::C);
            old7thbit = (_registers->AF.getLeftRegister() & 0x80);
            _registers->AF.setLeftRegister((_registers->AF.getLeftRegister() >> 1) | (old7thbit));
            _registers->AF.clearFlag(Register::FlagName::H);
            _registers->AF.clearFlag(Register::FlagName::N);
            value = _registers->AF.getLeftRegister();
            break;
        case Register::RegisterName::B:
            (_registers->BC.getLeftRegister() & 0x1) ? _registers->AF.setFlag(Register::FlagName::C) : _registers->AF.clearFlag(Register::FlagName::C);
            old7thbit = (_registers->BC.getLeftRegister() & 0x80);
            _registers->BC.setLeftRegister((_registers->BC.getLeftRegister() >> 1) | (old7thbit));
            _registers->AF.clearFlag(Register::FlagName::H);
            _registers->AF.clearFlag(Register::FlagName::N);
            value = _registers->BC.getLeftRegister();
            break;
        case Register::RegisterName::C:
            (_registers->BC.getRightRegister() & 0x1) ? _registers->AF.setFlag(Register::FlagName::C) : _registers->AF.clearFlag(Register::FlagName::C);
            old7thbit = (_registers->BC.getRightRegister() & 0x80);
            _registers->BC.setRightRegister((_registers->BC.getRightRegister() >> 1) | (old7thbit));
            _registers->AF.clearFlag(Register::FlagName::H);
            _registers->AF.clearFlag(Register::FlagName::N);
            value = _registers->BC.getRightRegister();
            break;
        case Register::RegisterName::D:
            (_registers->DE.getLeftRegister() & 0x1) ? _registers->AF.setFlag(Register::FlagName::C) : _registers->AF.clearFlag(Register::FlagName::C);
            old7thbit = (_registers->DE.getLeftRegister() & 0x80);
            _registers->DE.setLeftRegister((_registers->DE.getLeftRegister() >> 1) | (old7thbit));
            _registers->AF.clearFlag(Register::FlagName::H);
            _registers->AF.clearFlag(Register::FlagName::N);
            value = _registers->DE.getLeftRegister();
            break;
        case Register::RegisterName::E:
            (_registers->DE.getRightRegister() & 0x1) ? _registers->AF.setFlag(Register::FlagName::C) : _registers->AF.clearFlag(Register::FlagName::C);
            old7thbit = (_registers->DE.getRightRegister() & 0x80);
            _registers->DE.setRightRegister((_registers->DE.getRightRegister() >> 1) | (old7thbit));
            _registers->AF.clearFlag(Register::FlagName::H);
            _registers->AF.clearFlag(Register::FlagName::N);
            value = _registers->DE.getRightRegister();
            break;
        case Register::RegisterName::H:
            (_registers->HL.getLeftRegister() & 0x1) ? _registers->AF.setFlag(Register::FlagName::C) : _registers->AF.clearFlag(Register::FlagName::C);
            old7thbit = (_registers->HL.getLeftRegister() & 0x80);
            _registers->HL.setLeftRegister((_registers->HL.getLeftRegister() >> 1) | (old7thbit));
            _registers->AF.clearFlag(Register::FlagName::H);
            _registers->AF.clearFlag(Register::FlagName::N);
            value = _registers->HL.getLeftRegister();
            break;
        case Register::RegisterName::L:
            (_registers->HL.getRightRegister() & 0x1) ? _registers->AF.setFlag(Register::FlagName::C) : _registers->AF.clearFlag(Register::FlagName::C);
            old7thbit = (_registers->HL.getRightRegister() & 0x80);
            _registers->HL.setRightRegister((_registers->HL.getRightRegister() >> 1) | (old7thbit));
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

    void Cpu::SRA_16()
    {
        uint8_t value = _mmu->readRam(_registers->HL.get());
        int old7bit = (value & 0x80);
        (value & 0x1) ? _registers->AF.setFlag(Register::FlagName::C) : _registers->AF.clearFlag(Register::FlagName::C);
        value = static_cast<uint8_t>((value >> 1) | (old7bit));
        _mmu->writeRam(_registers->HL.get(), value);
        (value == 0) ? _registers->AF.setFlag(Register::FlagName::Z) : _registers->AF.clearFlag(Register::FlagName::Z);
        _registers->AF.clearFlag(Register::FlagName::H);
        _registers->AF.clearFlag(Register::FlagName::N);
    }

    void Cpu::SRL_r(const Register::RegisterName &reg)
    {
        switch (reg)
        {
        case Register::RegisterName::A:
            (_registers->AF.getLeftRegister() & 0x1) ? _registers->AF.setFlag(Register::FlagName::C) : _registers->AF.clearFlag(Register::FlagName::C);
            _registers->AF.setLeftRegister((_registers->AF.getLeftRegister() >> 1));
            (_registers->AF.getLeftRegister() == 0) ? _registers->AF.setFlag(Register::FlagName::Z) : _registers->AF.clearFlag(Register::FlagName::Z);
            _registers->AF.clearFlag(Register::FlagName::H);
            _registers->AF.clearFlag(Register::FlagName::N);
            break;
        case Register::RegisterName::B:
            (_registers->BC.getLeftRegister() & 0x1) ? _registers->AF.setFlag(Register::FlagName::C) : _registers->AF.clearFlag(Register::FlagName::C);
            _registers->BC.setLeftRegister((_registers->BC.getLeftRegister() >> 1));
            _registers->AF.clearFlag(Register::FlagName::H);
            _registers->AF.clearFlag(Register::FlagName::N);
            (_registers->BC.getLeftRegister() == 0) ? _registers->AF.setFlag(Register::FlagName::Z) : _registers->AF.clearFlag(Register::FlagName::Z);
            break;
        case Register::RegisterName::C:
            (_registers->BC.getRightRegister() & 0x1) ? _registers->AF.setFlag(Register::FlagName::C) : _registers->AF.clearFlag(Register::FlagName::C);
            _registers->BC.setRightRegister((_registers->BC.getRightRegister() >> 1));
            _registers->AF.clearFlag(Register::FlagName::H);
            _registers->AF.clearFlag(Register::FlagName::N);
            (_registers->BC.getRightRegister() == 0) ? _registers->AF.setFlag(Register::FlagName::Z) : _registers->AF.clearFlag(Register::FlagName::Z);
            break;
        case Register::RegisterName::D:
            (_registers->DE.getLeftRegister() & 0x1) ? _registers->AF.setFlag(Register::FlagName::C) : _registers->AF.clearFlag(Register::FlagName::C);
            _registers->DE.setLeftRegister((_registers->DE.getLeftRegister() >> 1));
            _registers->AF.clearFlag(Register::FlagName::H);
            _registers->AF.clearFlag(Register::FlagName::N);
            (_registers->DE.getLeftRegister() == 0) ? _registers->AF.setFlag(Register::FlagName::Z) : _registers->AF.clearFlag(Register::FlagName::Z);
            break;
        case Register::RegisterName::E:
            (_registers->DE.getRightRegister() & 0x1) ? _registers->AF.setFlag(Register::FlagName::C) : _registers->AF.clearFlag(Register::FlagName::C);
            _registers->DE.setRightRegister((_registers->DE.getRightRegister() >> 1));
            _registers->AF.clearFlag(Register::FlagName::H);
            _registers->AF.clearFlag(Register::FlagName::N);
            (_registers->DE.getRightRegister() == 0) ? _registers->AF.setFlag(Register::FlagName::Z) : _registers->AF.clearFlag(Register::FlagName::Z);
            break;
        case Register::RegisterName::H:
            (_registers->HL.getLeftRegister() & 0x1) ? _registers->AF.setFlag(Register::FlagName::C) : _registers->AF.clearFlag(Register::FlagName::C);
            _registers->HL.setLeftRegister((_registers->HL.getLeftRegister() >> 1));
            _registers->AF.clearFlag(Register::FlagName::H);
            _registers->AF.clearFlag(Register::FlagName::N);
            (_registers->HL.getLeftRegister() == 0) ? _registers->AF.setFlag(Register::FlagName::Z) : _registers->AF.clearFlag(Register::FlagName::Z);
            break;
        case Register::RegisterName::L:
            (_registers->HL.getRightRegister() & 0x1) ? _registers->AF.setFlag(Register::FlagName::C) : _registers->AF.clearFlag(Register::FlagName::C);
            _registers->HL.setRightRegister((_registers->HL.getRightRegister() >> 1));
            _registers->AF.clearFlag(Register::FlagName::H);
            _registers->AF.clearFlag(Register::FlagName::N);
            (_registers->HL.getRightRegister() == 0) ? _registers->AF.setFlag(Register::FlagName::Z) : _registers->AF.clearFlag(Register::FlagName::Z);
            break;
        default:
            cout << "Flag error.";
            exit(0);
            break;
        }
    }

    void Cpu::SRL_16()
    {
        uint8_t value = 0;
        (_mmu->readRam(_registers->HL.get()) & 0x1) ? _registers->AF.setFlag(Register::FlagName::C) : _registers->AF.clearFlag(Register::FlagName::C);
        _mmu->writeRam(_registers->HL.get(), (_mmu->readRam(_registers->HL.get()) >> 1));
        _registers->AF.clearFlag(Register::FlagName::H);
        _registers->AF.clearFlag(Register::FlagName::N);
        value = _mmu->readRam(_registers->HL.get());
        (value == 0) ? _registers->AF.setFlag(Register::FlagName::Z) : _registers->AF.clearFlag(Register::FlagName::Z);
    }
}