#include "utilitiesProvider.h"
#include "mmuProvider.h"
#include "gbException.h"
#include "registers.h"
#include "logger.h"
#include "defs.h"

namespace gasyboy
{
    Registers::Registers()
        : _mmu(provider::MmuProvider::getInstance()),
          AF(0),
          BC(0),
          DE(0),
          HL(0),
          PC(0),
          SP(0),
          _interruptEnabled(false),
          _halted(false),
          _stopMode(false),
          _executeBios(provider::UtilitiesProvider::getInstance()->executeBios)
    {
        if (!_executeBios)
        {
            AF.set(0x01B0);
            BC.set(0x0013);
            DE.set(0x00D8);
            HL.set(0x014D);
            PC = 0x100;
            SP = 0xFFFE;

            _registersMap = {
                {Register::RegisterPairName::AF, AF},
                {Register::RegisterPairName::BC, BC},
                {Register::RegisterPairName::DE, DE},
                {Register::RegisterPairName::HL, HL},
            };
        }
    }

    Registers &Registers::operator=(const Registers &other)
    {
        AF = other.AF;
        BC = other.BC;
        DE = other.DE;
        HL = other.HL;
        PC = other.PC;
        SP = other.SP;
        _interruptEnabled = other._interruptEnabled;
        _halted = other._halted;
        _executeBios = other._executeBios;
        _registersMap = other._registersMap;
        return *this;
    }

    void Registers::reset()
    {
        AF = 0;
        BC = 0;
        DE = 0;
        HL = 0;
        PC = 0;
        SP = 0;
        _interruptEnabled = false;
        _halted = false;

        if (!_executeBios)
        {
            AF.set(0x01B0);
            BC.set(0x0013);
            DE.set(0x00D8);
            HL.set(0x014D);
            PC = 0x100;
            SP = 0xFFFE;

            _registersMap = {
                {Register::RegisterPairName::AF, AF},
                {Register::RegisterPairName::BC, BC},
                {Register::RegisterPairName::DE, DE},
                {Register::RegisterPairName::HL, HL},
            };
        }
    }

    Register Registers::getRegister(const Register::RegisterPairName &reg)
    {
        try
        {
            auto result = _registersMap.find(reg);

            if (result == _registersMap.end())
            {
                throw exception::GbException("Invalid register to get");
            }

            return _registersMap[reg];
        }
        catch (const exception::GbException &e)
        {
            utils::Logger::getInstance()->log(utils::Logger::LogType::CRITICAL,
                                              e.what());
            exit(ExitState::CRITICAL_ERROR);
        }
    }

    uint8_t Registers::getRegister(const Register::RegisterName &reg)
    {
        switch (reg)
        {
        case Register::RegisterName::A:
            return AF.getLeftRegister();
            break;
        case Register::RegisterName::B:
            return BC.getLeftRegister();
            break;
        case Register::RegisterName::C:
            return BC.getRightRegister();
            break;
        case Register::RegisterName::D:
            return DE.getLeftRegister();
            break;
        case Register::RegisterName::E:
            return DE.getRightRegister();
            break;
        case Register::RegisterName::H:
            return HL.getLeftRegister();
            break;
        case Register::RegisterName::L:
            return HL.getRightRegister();
            break;
        default:
            throw exception::GbException("Invalid register: \"");
        }
    }

    void Registers::setRegister(const Register::RegisterPairName &reg, const uint16_t &value)
    {
        try
        {
            auto result = _registersMap.find(reg);

            if (result == _registersMap.end())
            {
                throw exception::GbException("Invalid register to get");
            }

            return _registersMap[reg].set(value);
        }
        catch (const exception::GbException &e)
        {
            utils::Logger::getInstance()->log(utils::Logger::LogType::CRITICAL,
                                              e.what());
            exit(ExitState::CRITICAL_ERROR);
        }
    }

    void Registers::setRegister(const Register::RegisterName &reg, const uint8_t &value)
    {
        switch (reg)
        {
        case Register::RegisterName::A:
            AF.setLeftRegister(value);
            break;
        case Register::RegisterName::B:
            BC.setLeftRegister(value);
            break;
        case Register::RegisterName::C:
            BC.setRightRegister(value);
            break;
        case Register::RegisterName::D:
            DE.setLeftRegister(value);
            break;
        case Register::RegisterName::E:
            DE.setRightRegister(value);
            break;
        case Register::RegisterName::H:
            HL.setLeftRegister(value);
            break;
        case Register::RegisterName::L:
            HL.setRightRegister(value);
            break;
        default:
            throw exception::GbException("Invalid register: \"");
        }
    }

    void Registers::pushSP(const uint16_t &address)
    {
        uint8_t firstByte = static_cast<uint8_t>(address >> 8);
        uint8_t secondByte = static_cast<uint8_t>(address & 0xFF);

        SP--;
        _mmu->writeRam(SP, firstByte);
        SP--;
        _mmu->writeRam(SP, secondByte);
    }

    void Registers::setInterruptEnabled(const bool &value)
    {
        _interruptEnabled = value;
    }

    bool Registers::getInterruptEnabled()
    {
        return _interruptEnabled;
    }

    void Registers::setHalted(const bool &value)
    {
        _halted = value;
    }

    bool Registers::getHalted()
    {
        return _halted;
    }

    void Registers::setStopMode(const bool &value)
    {
        _stopMode = value;
    }

    bool Registers::getStopMode()
    {
        return _stopMode;
    }
}