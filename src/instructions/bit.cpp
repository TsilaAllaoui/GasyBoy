#include "cpu.h"

namespace gasyboy
{
    void Cpu::BIT_b_r(const int &bit, const Register::RegisterName &reg)
    {
        if (bit > 7 || bit < 0)
        {
            cout << "Bit to check out of bound" << endl;
            exit(3);
        }
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
        (value & (1 << bit)) ? _registers->AF.clearFlag(Register::FlagName::Z) : _registers->AF.setFlag(Register::FlagName::Z);
        _registers->AF.clearFlag(Register::FlagName::N);
        _registers->AF.setFlag(Register::FlagName::H);
    }

    void Cpu::BIT_b_16(const int &bit)
    {
        if (bit > 7 || bit < 0)
        {
            cout << "Bit to check out of bound" << endl;
            exit(3);
        }
        uint8_t value = _mmu->readRam(_registers->HL.get());
        (value & (1 << bit)) ? _registers->AF.clearFlag(Register::FlagName::Z) : _registers->AF.setFlag(Register::FlagName::Z);
        _registers->AF.clearFlag(Register::FlagName::N);
        _registers->AF.setFlag(Register::FlagName::H);
    }

    void Cpu::SET_b_r(const int &bit, const Register::RegisterName &reg)
    {
        if (bit > 7 || bit < 0)
        {
            cout << "Bit to check out of bound" << endl;
            exit(3);
        }
        uint8_t value = (1 << bit);
        switch (reg)
        {
        case Register::RegisterName::A:
            _registers->AF.setLeftRegister(_registers->AF.getLeftRegister() | value);
            break;
        case Register::RegisterName::F:
            _registers->AF.setRightRegister(_registers->AF.getRightRegister() | value);
            break;
        case Register::RegisterName::B:
            _registers->BC.setLeftRegister(_registers->BC.getLeftRegister() | value);
            break;
        case Register::RegisterName::C:
            _registers->BC.setRightRegister(_registers->BC.getRightRegister() | value);
            break;
        case Register::RegisterName::D:
            _registers->DE.setLeftRegister(_registers->DE.getLeftRegister() | value);
            break;
        case Register::RegisterName::E:
            _registers->DE.setRightRegister(_registers->DE.getRightRegister() | value);
            break;
        case Register::RegisterName::H:
            _registers->HL.setLeftRegister(_registers->HL.getLeftRegister() | value);
            break;
        case Register::RegisterName::L:
            _registers->HL.setRightRegister(_registers->HL.getRightRegister() | value);
            break;
        default:
            cout << "Flag error.";
            exit(0);
            break;
        }
    }

    void Cpu::SET_b_16(const int &bit)
    {
        if (bit > 7 || bit < 0)
        {
            cout << "Bit to check out of bound" << endl;
            exit(3);
        }
        uint8_t value = _mmu->readRam(_registers->HL.get());
        _mmu->writeRam(_registers->HL.get(), (value | (1 << bit)));
    }

    void Cpu::RES_b_r(const int &bit, const Register::RegisterName &reg)
    {
        if (bit > 7 || bit < 0)
        {
            cout << "Bit to check out of bound" << endl;
            exit(3);
        }
        uint8_t value = (1 << bit);
        switch (reg)
        {
        case Register::RegisterName::A:
            _registers->AF.setLeftRegister(_registers->AF.getLeftRegister() & ~value);
            break;
        case Register::RegisterName::B:
            _registers->BC.setLeftRegister(_registers->BC.getLeftRegister() & ~value);
            break;
        case Register::RegisterName::C:
            _registers->BC.setRightRegister(_registers->BC.getRightRegister() & ~value);
            break;
        case Register::RegisterName::D:
            _registers->DE.setLeftRegister(_registers->DE.getLeftRegister() & ~value);
            break;
        case Register::RegisterName::E:
            _registers->DE.setRightRegister(_registers->DE.getRightRegister() & ~value);
            break;
        case Register::RegisterName::H:
            _registers->HL.setLeftRegister(_registers->HL.getLeftRegister() & ~value);
            break;
        case Register::RegisterName::L:
            _registers->HL.setRightRegister(_registers->HL.getRightRegister() & ~value);
            break;
        default:
            cout << "Flag error.";
            exit(0);
            break;
        }
    }

    void Cpu::RES_b_16(const int &bit)
    {
        if (bit > 7 || bit < 0)
        {
            cout << "Bit to check out of bound" << endl;
            exit(3);
        }
        uint8_t value = ~(1 << bit);
        _mmu->writeRam(_registers->HL.get(), _mmu->readRam(_registers->HL.get()) & value);
    }
}