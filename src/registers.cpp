#include "gbException.h"
#include "registers.h"
#include "logger.h"
#include "defs.h"

namespace gasyboy
{
    Registers::Registers(Mmu &mmu)
        : _mmu(mmu),
          AF(0),
          BC(0),
          DE(0),
          HL(0),
          PC(0),
          SP(0),
          _interruptEnabled(false),
          _halted(false)
    {
        _registersMap = {
            {"AF", AF},
            {"BC", BC},
            {"DE", DE},
            {"HL", HL},
        };
    }

    Register Registers::getRegister(const std::string &reg)
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

    uint8_t Registers::getRegister(const char &reg)
    {
        switch (reg)
        {
        case 'A':
            return AF.getLeftRegister();
            break;
        case 'B':
            return BC.getLeftRegister();
            break;
        case 'C':
            return BC.getRightRegister();
            break;
        case 'D':
            return DE.getLeftRegister();
            break;
        case 'E':
            return DE.getRightRegister();
            break;
        case 'H':
            return HL.getLeftRegister();
            break;
        case 'L':
            return HL.getRightRegister();
            break;
        default:
            std::string message("Invalid register: \"");
            message += reg;
            message += "\"";
            throw exception::GbException(message);
            exit(ExitState::CRITICAL_ERROR);
        }
    }

    void Registers::setRegister(const std::string &reg, const uint16_t &value)
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

    void Registers::setRegister(const char &reg, const uint8_t &value)
    {
        switch (reg)
        {
        case 'A':
            AF.setLeftRegister(value);
            break;
        case 'B':
            BC.setLeftRegister(value);
            break;
        case 'C':
            BC.setRightRegister(value);
            break;
        case 'D':
            DE.setLeftRegister(value);
            break;
        case 'E':
            DE.setRightRegister(value);
            break;
        case 'H':
            HL.setLeftRegister(value);
            break;
        case 'L':
            HL.setRightRegister(value);
            break;
        default:
            std::string message("Invalid register: \"");
            message += reg;
            message += "\"";
            throw exception::GbException(message);
            exit(ExitState::CRITICAL_ERROR);
        }
    }

    void Registers::pushSP(const uint16_t &address)
    {
        uint8_t firstByte = static_cast<uint8_t>(address >> 8);
        uint8_t secondByte = static_cast<uint8_t>(address & 0xFF);

        SP--;
        _mmu.writeRam(SP, firstByte);
        SP--;
        _mmu.writeRam(SP, secondByte);
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
}