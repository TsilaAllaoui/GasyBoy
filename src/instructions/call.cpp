#include "cpu.h"

namespace gasyboy
{
    void Cpu::CALL()
    {
        uint8_t leftValue = _mmu->readRam(_registers->PC + 2);
        uint8_t rightValue = _mmu->readRam(_registers->PC + 1);
        _registers->SP--;
        _mmu->writeRam(_registers->SP, (((_registers->PC + 3) & 0xFF00) >> 8));
        _registers->SP--;
        _mmu->writeRam(_registers->SP, static_cast<uint8_t>((_registers->PC + 3) & 0xFF));
        _registers->PC = ((leftValue << 8) | rightValue);
    }

    void Cpu::CALL_c(const Register::FlagName &condition)
    {
        uint8_t leftValue = _mmu->readRam(_registers->PC + 2);
        uint8_t rightValue = _mmu->readRam(_registers->PC + 1);
        if (condition == Register::FlagName::Z)
        {
            if (_registers->AF.getFlag(Register::FlagName::Z))
            {

                _registers->SP--;
                _mmu->writeRam(_registers->SP, (((_registers->PC + 3) & 0xFF00) >> 8));
                _registers->SP--;
                _mmu->writeRam(_registers->SP, static_cast<uint8_t>((_registers->PC + 3) & 0xFF));
                _registers->PC = ((leftValue << 8) | rightValue);
                return;
            }
            else
            {
                _registers->PC += 3;
                return;
            }
        }

        else if (condition == Register::FlagName::NZ)
        {
            if (!_registers->AF.getFlag(Register::FlagName::Z))
            {

                _registers->SP--;
                _mmu->writeRam(_registers->SP, (((_registers->PC + 3) & 0xFF00) >> 8));
                _registers->SP--;
                _mmu->writeRam(_registers->SP, static_cast<uint8_t>((_registers->PC + 3) & 0xFF));
                _registers->PC = ((leftValue << 8) | rightValue);
                return;
            }
            else
            {
                _registers->PC += 3;
                return;
            }
        }
        else if (condition == Register::FlagName::C)
        {
            if (_registers->AF.getFlag(Register::FlagName::C))
            {

                _registers->SP--;
                _mmu->writeRam(_registers->SP, (((_registers->PC + 3) & 0xFF00) >> 8));
                _registers->SP--;
                _mmu->writeRam(_registers->SP, static_cast<uint8_t>((_registers->PC + 3) & 0xFF));
                _registers->PC = ((leftValue << 8) | rightValue);
                return;
            }
            else
            {
                _registers->PC += 3;
                return;
            }
        }
        else if (condition == Register::FlagName::NC)
        {
            if (!_registers->AF.getFlag(Register::FlagName::C))
            {

                _registers->SP--;
                _mmu->writeRam(_registers->SP, (((_registers->PC + 3) & 0xFF00) >> 8));
                _registers->SP--;
                _mmu->writeRam(_registers->SP, static_cast<uint8_t>((_registers->PC + 3) & 0xFF));
                _registers->PC = ((leftValue << 8) | rightValue);
                return;
            }
            else
            {
                _registers->PC += 3;
                return;
            }
        }
    }
}