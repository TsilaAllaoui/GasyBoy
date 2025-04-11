#include "cpu.h"

namespace gasyboy
{
    void Cpu::SWAP_r(const Register::RegisterName &reg)
    {
        uint8_t left = 0, rigth = 0, value = 0;
        switch (reg)
        {
        case Register::RegisterName::A:
            left = _registers->AF.getLeftRegister() & 0xF0;
            rigth = _registers->AF.getLeftRegister() & 0xF;
            value = ((left >> 4) | (rigth << 4));
            _registers->AF.setLeftRegister(value);
            break;
        case Register::RegisterName::B:
            left = _registers->BC.getLeftRegister() & 0xF0;
            rigth = _registers->BC.getLeftRegister() & 0xF;
            value = ((left >> 4) | (rigth << 4));
            _registers->BC.setLeftRegister(value);
            break;
        case Register::RegisterName::C:
            left = _registers->BC.getRightRegister() & 0xF0;
            rigth = _registers->BC.getRightRegister() & 0xF;
            value = ((left >> 4) | (rigth << 4));
            _registers->BC.setRightRegister(value);
            break;
        case Register::RegisterName::D:
            left = _registers->DE.getLeftRegister() & 0xF0;
            rigth = _registers->DE.getLeftRegister() & 0xF;
            value = ((left >> 4) | (rigth << 4));
            _registers->DE.setLeftRegister(value);
            break;
        case Register::RegisterName::E:
            left = _registers->DE.getRightRegister() & 0xF0;
            rigth = _registers->DE.getRightRegister() & 0xF;
            value = ((left >> 4) | (rigth << 4));
            _registers->DE.setRightRegister(value);
            break;
        case Register::RegisterName::H:
            left = _registers->HL.getLeftRegister() & 0xF0;
            rigth = _registers->HL.getLeftRegister() & 0xF;
            value = ((left >> 4) | (rigth << 4));
            _registers->HL.setLeftRegister(value);
            break;
        case Register::RegisterName::L:
            left = _registers->HL.getRightRegister() & 0xF0;
            rigth = _registers->HL.getRightRegister() & 0xF;
            value = ((left >> 4) | (rigth << 4));
            _registers->HL.setRightRegister(value);
            break;
        }
        _registers->AF.clearFlag(Register::FlagName::N);
        _registers->AF.clearFlag(Register::FlagName::C);
        _registers->AF.clearFlag(Register::FlagName::H);
        (value == 0) ? _registers->AF.setFlag(Register::FlagName::Z) : _registers->AF.clearFlag(Register::FlagName::Z);
    }

    void Cpu::SWAP_16()
    {
        uint8_t left = 0, rigth = 0, value = _mmu->readRam(_registers->HL.get());
        left = (value & 0xF0);
        rigth = (value & 0xF);
        value = ((left >> 4) | (rigth << 4));
        _mmu->writeRam(_registers->HL.get(), value);
        (value == 0) ? _registers->AF.setFlag(Register::FlagName::Z) : _registers->AF.clearFlag(Register::FlagName::Z);
        _registers->AF.clearFlag(Register::FlagName::N);
        _registers->AF.clearFlag(Register::FlagName::H);
        _registers->AF.clearFlag(Register::FlagName::C);
    }
}