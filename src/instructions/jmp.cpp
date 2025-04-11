#include "cpu.h"

namespace gasyboy
{
    void Cpu::JP_16(const uint16_t &adress)
    {
        uint16_t leftValue = _mmu->readRam(adress + 1);
        uint8_t rigthValue = _mmu->readRam(adress);
        _registers->PC = ((leftValue << 8) | rigthValue);
    }

    void Cpu::JP_c_16(const Register::FlagName &condition, const uint16_t &adress)
    {
        if (condition == Register::FlagName::Z)
            (_registers->AF.getFlag(Register::FlagName::Z)) ? _registers->PC = adress : _registers->PC += 3;
        else if (condition == Register::FlagName::NZ)
            (!_registers->AF.getFlag(Register::FlagName::Z)) ? _registers->PC = adress : _registers->PC += 3;
        else if (condition == Register::FlagName::C)
            (_registers->AF.getFlag(Register::FlagName::C)) ? _registers->PC = adress : _registers->PC += 3;
        else if (condition == Register::FlagName::NC)
            (!_registers->AF.getFlag(Register::FlagName::C)) ? _registers->PC = adress : _registers->PC += 3;
    }

    void Cpu::JR_e(const uint8_t &value)
    {
        _registers->PC += 2;
        _registers->PC += static_cast<int8_t>(value);
    }

    void Cpu::JR_C_e(const uint8_t &value)
    {
        _registers->PC += 2;
        if (_registers->AF.getFlag(Register::FlagName::C))
            _registers->PC += static_cast<int8_t>(value);
    }

    void Cpu::JR_NC_e(const uint8_t &value)
    {
        _registers->PC += 2;
        if (!_registers->AF.getFlag(Register::FlagName::C))
            _registers->PC += static_cast<int8_t>(value);
    }

    void Cpu::JR_Z_e(const uint8_t &value)
    {
        _registers->PC += 2;
        if (_registers->AF.getFlag(Register::FlagName::Z))
            _registers->PC += static_cast<int8_t>(value);
    }

    void Cpu::JR_NZ_e(const uint8_t &value)
    {
        _registers->PC += 2;
        if (!_registers->AF.getFlag(Register::FlagName::Z))
            _registers->PC += static_cast<int8_t>(value);
    }

    void Cpu::JP_16()
    {
        _registers->PC = _registers->HL.get();
    }
}