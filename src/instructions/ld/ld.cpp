#ifndef LD_INSTRUCTION_H
#define LD_INSTRUCTION_H

#include "cpu.h"

namespace gasyboy
{
    void Cpu::LD_HL_SP_n()
    {
        int8_t value = static_cast<int8_t>(_mmu->readRam(_registers->PC + 1));
        uint16_t result = (_registers->SP + value);
        (((_registers->SP ^ value ^ (result & 0xFFFF)) & 0x10) == 0x10) ? _registers->AF.setFlag(Register::FlagName::H) : _registers->AF.clearFlag(Register::FlagName::H);
        (((_registers->SP ^ value ^ (result & 0xFFFF)) & 0x100) == 0x100) ? _registers->AF.setFlag(Register::FlagName::C) : _registers->AF.clearFlag(Register::FlagName::C);
        _registers->HL.set(result);
        _registers->AF.clearFlag(Register::FlagName::Z);
        _registers->AF.clearFlag(Register::FlagName::N);
    }

    // TODO: use Registers method
    void Cpu::LD_r_r(const Register::RegisterName &from, const Register::RegisterName &to)
    {
        uint8_t regFrom = 0;
        switch (from)
        {
        case Register::RegisterName::A:
            regFrom = _registers->AF.getLeftRegister();
            break;
        case Register::RegisterName::B:
            regFrom = _registers->BC.getLeftRegister();
            break;
        case Register::RegisterName::C:
            regFrom = _registers->BC.getRightRegister();
            break;
        case Register::RegisterName::D:
            regFrom = _registers->DE.getLeftRegister();
            break;
        case Register::RegisterName::E:
            regFrom = _registers->DE.getRightRegister();
            break;
        case Register::RegisterName::H:
            regFrom = _registers->HL.getLeftRegister();
            break;
        case Register::RegisterName::L:
            regFrom = _registers->HL.getRightRegister();
            break;
        default:
            throw exception::GbException("Invalid register");
            break;
        }
        switch (to)
        {
        case Register::RegisterName::A:
            _registers->AF.setLeftRegister(regFrom);
            break;
        case Register::RegisterName::B:
            _registers->BC.setLeftRegister(regFrom);
            break;
        case Register::RegisterName::C:
            _registers->BC.setRightRegister(regFrom);
            break;
        case Register::RegisterName::D:
            _registers->DE.setLeftRegister(regFrom);
            break;
        case Register::RegisterName::E:
            _registers->DE.setRightRegister(regFrom);
            break;
        case Register::RegisterName::H:
            _registers->HL.setLeftRegister(regFrom);
            break;
        case Register::RegisterName::L:
            _registers->HL.setRightRegister(regFrom);
            break;
        default:
            cout << "Flag error.";
            exit(0);
            break;
        }
    }

    void Cpu::LD_r_n(const uint8_t &from, const Register::RegisterName &to)
    {
        switch (to)
        {
        case Register::RegisterName::A:
            _registers->AF.setLeftRegister(from);
            break;
        case Register::RegisterName::F:
            _registers->AF.setRightRegister(from);
            break;
        case Register::RegisterName::B:
            _registers->BC.setLeftRegister(from);
            break;
        case Register::RegisterName::C:
            _registers->BC.setRightRegister(from);
            break;
        case Register::RegisterName::D:
            _registers->DE.setLeftRegister(from);
            break;
        case Register::RegisterName::E:
            _registers->DE.setRightRegister(from);
            break;
        case Register::RegisterName::H:
            _registers->HL.setLeftRegister(from);
            break;
        case Register::RegisterName::L:
            _registers->HL.setRightRegister(from);
            break;
        default:
            cout << "Flag error.";
            exit(0);
            break;
        }
    }

    void Cpu::LD_r_16(const uint16_t &adress, const Register::RegisterName &to)
    {
        uint8_t from = _mmu->readRam(adress);
        switch (to)
        {
        case Register::RegisterName::A:
            _registers->AF.setLeftRegister(from);
            break;
        case Register::RegisterName::B:
            _registers->BC.setLeftRegister(from);
            break;
        case Register::RegisterName::C:
            _registers->BC.setRightRegister(from);
            break;
        case Register::RegisterName::D:
            _registers->DE.setLeftRegister(from);
            break;
        case Register::RegisterName::E:
            _registers->DE.setRightRegister(from);
            break;
        case Register::RegisterName::H:
            _registers->HL.setLeftRegister(from);
            break;
        case Register::RegisterName::L:
            _registers->HL.setRightRegister(from);
            break;
        default:
            cout << "Flag error.";
            exit(0);
            break;
        }
    }

    void Cpu::LD_16_r(const uint16_t &adress, const Register::RegisterName &from)
    {
        uint8_t value = 0;
        switch (from)
        {
        case Register::RegisterName::A:
            value = _registers->AF.getLeftRegister();
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
            cout << "Flag error."; // TODO: add gbException here
            exit(0);
            break;
        }
        _mmu->writeRam(adress, value);
    }

    void Cpu::LD_16_n(const uint16_t &adress, const uint8_t &value)
    {
        _mmu->writeRam(adress, value);
    }

    void Cpu::LD_rr_nn(const uint16_t &value, const Register::RegisterPairName &reg)
    {
        if (reg == Register::RegisterPairName::AF)
            _registers->AF.set(value);
        else if (reg == Register::RegisterPairName::BC)
            _registers->BC.set(value);
        else if (reg == Register::RegisterPairName::DE)
            _registers->DE.set(value);
        else if (reg == Register::RegisterPairName::HL)
            _registers->HL.set(value);
        else if (reg == Register::RegisterPairName::SP)
            _registers->SP = value;
        else
            exit(2); // TODO: add gbException here
    }

    void Cpu::LD_rr_16(const uint16_t &adress, const Register::RegisterPairName &reg)
    {
        uint8_t leftValue = _mmu->readRam(adress + 1);
        uint8_t rightValue = _mmu->readRam(adress);
        uint16_t value = ((uint16_t)(leftValue << 8) | rightValue);
        if (reg == Register::RegisterPairName::AF)
            _registers->AF.set(value);
        else if (reg == Register::RegisterPairName::BC)
            _registers->BC.set(value);
        else if (reg == Register::RegisterPairName::DE)
            _registers->DE.set(value);
        else if (reg == Register::RegisterPairName::HL)
            _registers->HL.set(value);
        else if (reg == Register::RegisterPairName::SP)
            _registers->SP = value;
        else
            exit(2);
    }

    void Cpu::LD_16_rr(const uint16_t &adress, const Register::RegisterPairName &reg)
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
        uint8_t firstByte = static_cast<uint8_t>(value & 0xFF);
        uint8_t secondByte = static_cast<uint8_t>((value & 0xFF00) >> 8);
        _mmu->writeRam(adress, firstByte);
        _mmu->writeRam(adress + 1, secondByte);
    }

    void Cpu::LD_SP_HL()
    {
        _registers->SP = _registers->HL.get();
    }

    void Cpu::PUSH(const Register::RegisterPairName &reg)
    {
        uint16_t value = 0xFFFF;
        if (reg == Register::RegisterPairName::AF)
            value = _registers->AF.get();
        else if (reg == Register::RegisterPairName::BC)
            value = _registers->BC.get();
        else if (reg == Register::RegisterPairName::DE)
            value = _registers->DE.get();
        else if (reg == Register::RegisterPairName::HL)
            value = _registers->HL.get();
        else
            exit(2);
        uint8_t firstByte = static_cast<uint8_t>((value & 0xFF00) >> 8);
        uint8_t secondByte = static_cast<uint8_t>(value & 0xFF);
        _registers->SP--;
        _mmu->writeRam(_registers->SP, firstByte);
        _registers->SP--;
        _mmu->writeRam(_registers->SP, secondByte);
    }

    void Cpu::POP(const Register::RegisterPairName &reg)
    {
        uint8_t firstByte = _mmu->readRam(_registers->SP + 1);
        uint16_t secondByte = (_mmu->readRam(_registers->SP));
        _registers->SP += 2;
        uint16_t value = ((firstByte << 8) | secondByte);
        if (reg == Register::RegisterPairName::AF)
        {
            value &= 0xFFF0;
            _registers->AF.set(value);
        }
        // _registers->AF.set(value);
        else if (reg == Register::RegisterPairName::BC)
        {
            _registers->BC.set(value);
        }
        else if (reg == Register::RegisterPairName::DE)
        {
            _registers->DE.set(value);
        }
        else if (reg == Register::RegisterPairName::HL)
        {
            _registers->HL.set(value);
        }
        else
            exit(2);
    }
}

#endif