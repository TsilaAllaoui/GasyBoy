#include "cpu.h"

namespace gasyboy
{
    void Cpu::RET()
    {
        uint8_t firstByte = _mmu->readRam(_registers->SP + 1);
        uint16_t secondByte = (_mmu->readRam(_registers->SP));
        _registers->SP += 2;
        _registers->PC = ((firstByte << 8) | secondByte);
    }

    void Cpu::RET_c(const Register::FlagName &condition)
    {
        if (condition == Register::FlagName::Z)
        {
            if (_registers->AF.getFlag(Register::FlagName::Z))
            {
                uint16_t leftNibble = (_mmu->readRam(_registers->SP + 1) << 8);
                uint8_t rightNibble = _mmu->readRam(_registers->SP);
                _registers->SP += 2;
                _registers->PC = (leftNibble | rightNibble);
                return;
            }
            else
            {
                _registers->PC++;
                return;
            }
        }

        if (condition == Register::FlagName::NZ)
        {
            if (!_registers->AF.getFlag(Register::FlagName::Z))
            {
                uint16_t leftNibble = (_mmu->readRam(_registers->SP + 1) << 8);
                uint8_t rightNibble = _mmu->readRam(_registers->SP);
                _registers->SP += 2;
                _registers->PC = (leftNibble | rightNibble);
                return;
            }
            else
            {
                _registers->PC++;
                return;
            }
        }
        if (condition == Register::FlagName::C)
        {
            if (_registers->AF.getFlag(Register::FlagName::C))
            {
                uint16_t leftNibble = (_mmu->readRam(_registers->SP + 1) << 8);
                uint8_t rightNibble = _mmu->readRam(_registers->SP);
                _registers->SP += 2;
                _registers->PC = (leftNibble | rightNibble);
                return;
            }
            else
            {
                _registers->PC++;
                return;
            }
        }
        if (condition == Register::FlagName::NC)
        {
            if (!_registers->AF.getFlag(Register::FlagName::C))
            {
                uint16_t leftNibble = (_mmu->readRam(_registers->SP + 1) << 8);
                uint8_t rightNibble = _mmu->readRam(_registers->SP);
                _registers->SP += 2;
                _registers->PC = (leftNibble | rightNibble);
                return;
            }
            else
            {
                _registers->PC++;
                return;
            }
        }
    }

    void Cpu::RETI() // TODO  Unknown if the IME is enabled after execution of this
    {
        _registers->setInterruptEnabled(true);
        uint16_t leftNibble = (_mmu->readRam(_registers->SP + 1) << 8);
        uint8_t rightNibble = _mmu->readRam(_registers->SP);
        _registers->SP += 2;
        _registers->PC = (leftNibble | rightNibble);
        _interruptManager->setMasterInterrupt(true);
    }
}