#include "cpu.h"

namespace gasyboy
{
    void Cpu::SBC_r(const Register::RegisterName &reg)
    {
        uint8_t value = 0;
        switch (reg)
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
            cout << "Flag error.";
            exit(0);
            break;
        }
        uint8_t carry = _registers->AF.getFlag(Register::FlagName::C) ? 1 : 0;
        uint8_t A = _registers->AF.getLeftRegister();

        int result_full = A - value - carry;
        uint8_t result = static_cast<uint8_t>(result_full);

        (result == 0) ? _registers->AF.setFlag(Register::FlagName::Z) : _registers->AF.clearFlag(Register::FlagName::Z);
        _registers->AF.setFlag(Register::FlagName::N);
        (result_full < 0) ? _registers->AF.setFlag(Register::FlagName::C) : _registers->AF.clearFlag(Register::FlagName::C);
        (((A & 0xF) - (value & 0xF) - carry) < 0) ? _registers->AF.setFlag(Register::FlagName::H) : _registers->AF.clearFlag(Register::FlagName::H);

        _registers->AF.setLeftRegister(result);
    }

    void Cpu::SBC_n(const uint8_t &value)
    {
        uint8_t carry = _registers->AF.getFlag(Register::FlagName::C) ? 1 : 0;
        uint8_t A = _registers->AF.getLeftRegister();

        int result_full = A - value - carry;
        uint8_t result = static_cast<uint8_t>(result_full);

        (result == 0) ? _registers->AF.setFlag(Register::FlagName::Z) : _registers->AF.clearFlag(Register::FlagName::Z);
        _registers->AF.setFlag(Register::FlagName::N);
        (result_full < 0) ? _registers->AF.setFlag(Register::FlagName::C) : _registers->AF.clearFlag(Register::FlagName::C);
        (((A & 0xF) - (value & 0xF) - carry) < 0) ? _registers->AF.setFlag(Register::FlagName::H) : _registers->AF.clearFlag(Register::FlagName::H);

        _registers->AF.setLeftRegister(result);
    }

    void Cpu::SBC_16()
    {
        uint8_t value = _mmu->readRam(_registers->HL.get());
        uint8_t carry = _registers->AF.getFlag(Register::FlagName::C) ? 1 : 0;
        uint8_t A = _registers->AF.getLeftRegister();

        int result_full = A - value - carry;
        uint8_t result = static_cast<uint8_t>(result_full);

        (result == 0) ? _registers->AF.setFlag(Register::FlagName::Z) : _registers->AF.clearFlag(Register::FlagName::Z);
        _registers->AF.setFlag(Register::FlagName::N);
        (result_full < 0) ? _registers->AF.setFlag(Register::FlagName::C) : _registers->AF.clearFlag(Register::FlagName::C);
        (((A & 0xF) - (value & 0xF) - carry) < 0) ? _registers->AF.setFlag(Register::FlagName::H) : _registers->AF.clearFlag(Register::FlagName::H);

        _registers->AF.setLeftRegister(result);
    }

    void Cpu::SBC_HL_rr(const Register::RegisterPairName &reg)
    {
        uint16_t value = 0;
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
        value += _registers->AF.getFlag(Register::FlagName::C) ? 1 : 0;
        uint16_t operand = _registers->HL.get();
        _registers->HL.set(operand - value);
        _registers->AF.setFlag(Register::FlagName::N);
        checkAddCarry(operand, value) ? _registers->AF.setFlag(Register::FlagName::C) : _registers->AF.clearFlag(Register::FlagName::C);
        checkSubHalfCarry(operand, value) ? _registers->AF.setFlag(Register::FlagName::H) : _registers->AF.clearFlag(Register::FlagName::H);
    }
}