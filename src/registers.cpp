#include "gbException.h"
#include "registers.h"
#include "logger.h"
#include "defs.h"

namespace gasyboy
{
    Registers::Registers()
        : AF(0),
          BC(0),
          DE(0),
          HL(0),
          PC(0),
          SP(0)
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
            return BC.getRigthRegister();
            break;
        case 'D':
            return DE.getLeftRegister();
            break;
        case 'E':
            return DE.getRigthRegister();
            break;
        case 'H':
            return HL.getLeftRegister();
            break;
        case 'L':
            return HL.getRigthRegister();
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
            BC.setRigthRegister(value);
            break;
        case 'D':
            DE.setLeftRegister(value);
            break;
        case 'E':
            DE.setRigthRegister(value);
            break;
        case 'H':
            HL.setLeftRegister(value);
            break;
        case 'L':
            HL.setRigthRegister(value);
            break;
        default:
            std::string message("Invalid register: \"");
            message += reg;
            message += "\"";
            throw exception::GbException(message);
            exit(ExitState::CRITICAL_ERROR);
        }
    }
}