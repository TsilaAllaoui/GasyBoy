#include "cpu.h"

namespace gasyboy
{
    void Cpu::ADD_A_r(const Register::RegisterName &reg)
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
        checkAddHalfCarry(value, _registers->AF.getLeftRegister()) ? _registers->AF.setFlag(Register::FlagName::H) : _registers->AF.clearFlag(Register::FlagName::H);
        checkAddCarry(value, _registers->AF.getLeftRegister()) ? _registers->AF.setFlag(Register::FlagName::C) : _registers->AF.clearFlag(Register::FlagName::C);
        uint8_t result = _registers->AF.getLeftRegister() + value;
        (result == 0) ? _registers->AF.setFlag(Register::FlagName::Z) : _registers->AF.clearFlag(Register::FlagName::Z);
        _registers->AF.clearFlag(Register::FlagName::N);
        _registers->AF.setLeftRegister(result);
    }

    void Cpu::ADD_A_n(const uint8_t &value)
    {
        checkAddHalfCarry(value, _registers->AF.getLeftRegister()) ? _registers->AF.setFlag(Register::FlagName::H) : _registers->AF.clearFlag(Register::FlagName::H);
        checkAddCarry(value, _registers->AF.getLeftRegister()) ? _registers->AF.setFlag(Register::FlagName::C) : _registers->AF.clearFlag(Register::FlagName::C);
        uint8_t result = _registers->AF.getLeftRegister() + value;
        (result == 0) ? _registers->AF.setFlag(Register::FlagName::Z) : _registers->AF.clearFlag(Register::FlagName::Z);
        _registers->AF.clearFlag(Register::FlagName::N);
        _registers->AF.setLeftRegister(result);
    }

    void Cpu::ADD_A_16()
    {
        uint8_t value = _mmu->readRam(_registers->HL.get());
        checkAddHalfCarry(value, _registers->AF.getLeftRegister()) ? _registers->AF.setFlag(Register::FlagName::H) : _registers->AF.clearFlag(Register::FlagName::H);
        ((uint16_t)value + (uint16_t)_registers->AF.getLeftRegister() >= 0x100) ? _registers->AF.setFlag(Register::FlagName::C) : _registers->AF.clearFlag(Register::FlagName::C);
        uint8_t result = _registers->AF.getLeftRegister() + value;
        (result == 0) ? _registers->AF.setFlag(Register::FlagName::Z) : _registers->AF.clearFlag(Register::FlagName::Z);
        _registers->AF.clearFlag(Register::FlagName::N);
        _registers->AF.setLeftRegister(result);
    }

    void Cpu::ADD_HL_rr(const Register::RegisterPairName &reg)
    {
        uint16_t value;
        if (reg == Register::RegisterPairName::AF)
            value = _registers->AF.get();
        else if (reg == Register::RegisterPairName::BC)
            value = _registers->BC.get();
        else if (reg == Register::RegisterPairName::DE)
            value = _registers->DE.get();
        else if (reg == Register::RegisterPairName::HL)
            value = _registers->HL.get();
        else if (reg == Register::RegisterPairName::SP)
            value = _registers->SP;
        else
            exit(2);
        uint16_t operand = _registers->HL.get();
        _registers->HL.set(operand + value);
        _registers->AF.clearFlag(Register::FlagName::N);
        ((operand + value) >= 0x10000) ? _registers->AF.setFlag(Register::FlagName::C) : _registers->AF.clearFlag(Register::FlagName::C);
        ((operand & 0xFFF) + (value & 0xFFF) >= 0x1000) ? _registers->AF.setFlag(Register::FlagName::H) : _registers->AF.clearFlag(Register::FlagName::H);
    }

    void Cpu::ADD_SP_n()
    {
        int8_t value = static_cast<int8_t>(_mmu->readRam(_registers->PC + 1));
        uint16_t result = (_registers->SP + value);
        (((_registers->SP ^ value ^ (result & 0xFFFF)) & 0x10) == 0x10) ? _registers->AF.setFlag(Register::FlagName::H) : _registers->AF.clearFlag(Register::FlagName::H);
        (((_registers->SP ^ value ^ (result & 0xFFFF)) & 0x100) == 0x100) ? _registers->AF.setFlag(Register::FlagName::C) : _registers->AF.clearFlag(Register::FlagName::C);
        _registers->SP = (result);
        _registers->AF.clearFlag(Register::FlagName::Z);
        _registers->AF.clearFlag(Register::FlagName::N);
    }
}