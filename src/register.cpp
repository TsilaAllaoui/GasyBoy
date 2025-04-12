#include "gbException.h"
#include "register.h"
#include "logger.h"
#include "defs.h"
#include <iostream>

namespace gasyboy
{
    Register::Register()
        : _registerValue(0)
    {
    }

    Register::Register(const uint8_t &value)
        : _registerValue(value)
    {
    }

    Register::~Register()
    {
    }

    void Register::set(const uint16_t &value)
    {
        _registerValue = value;
    }

    uint16_t Register::get()
    {
        return _registerValue;
    }

    uint8_t Register::getRightRegister()
    {
        return static_cast<uint8_t>(_registerValue & 0xFF);
    }

    uint8_t Register::getLeftRegister()
    {
        return static_cast<uint8_t>(_registerValue >> 8);
    }

    void Register::setLeftRegister(const uint8_t &value)
    {
        _registerValue = (value << 8) | (_registerValue & 0x00FF);
    }

    void Register::setRightRegister(const uint8_t &value)
    {
        _registerValue = (_registerValue & 0xFF00) | value;
    }

    void Register::show()
    {
        std::cout << std::hex << _registerValue << endl;
    }

    SpecialRegister::SpecialRegister() : Register()
    {
    }

    SpecialRegister::SpecialRegister(const uint8_t &value)
        : Register(value)
    {
    }

    SpecialRegister::~SpecialRegister()
    {
    }

    bool SpecialRegister::getFlag(const FlagName &reg)
    {
        try
        {
            bool value;
            switch (reg)
            {
            case FlagName::Z:
                value = getRightRegister() & (1 << 7);
                break;
            case FlagName::N:
                value = getRightRegister() & (1 << 6);
                break;
            case FlagName::H:
                value = getRightRegister() & (1 << 5);
                break;
            case FlagName::C:
                value = getRightRegister() & (1 << 4);
                break;
            default:
                std::stringstream message;
                message << "Invalid register\n";
                throw exception::GbException(message.str());
            }
            return value;
        }
        catch (const exception::GbException &e)
        {
            utils::Logger::getInstance()->log(utils::Logger::LogType::CRITICAL,
                                              e.what());
            exit(ExitState::CRITICAL_ERROR);
        }
    }

    void SpecialRegister::setFlag(const FlagName &reg)
    {
        try
        {
            uint8_t bit = 0;
            switch (reg)
            {
            case FlagName::Z:
                bit = 0x80;
                break;
            case FlagName::N:
                bit = 0x40;
                break;
            case FlagName::H:
                bit = 0x20;
                break;
            case FlagName::C:
                bit = 0x10;
                break;
            default:
                std::stringstream message;
                message << "Invalid register\n";
                throw exception::GbException(message.str());
            }
            bit |= getRightRegister();
            setRightRegister(bit);
        }
        catch (const std::exception &e)
        {
            utils::Logger::getInstance()->log(utils::Logger::LogType::CRITICAL,
                                              e.what());
            exit(ExitState::CRITICAL_ERROR);
        }
    }

    void SpecialRegister::clearFlag(const FlagName &reg)
    {
        try
        {
            int bit = 0;
            switch (reg)
            {
            case FlagName::Z:
                bit = (~0x80);
                break;
            case FlagName::N:
                bit = (~0x40);
                break;
            case FlagName::H:
                bit = (~0x20);
                break;
            case FlagName::C:
                bit = (~0x10);
                break;
            default:
                std::stringstream message;
                message << "Invalid register\n";
                throw exception::GbException(message.str());
            }

            bit = getRightRegister() & bit;
            setRightRegister(bit);
        }
        catch (const std::exception &e)
        {
            utils::Logger::getInstance()->log(utils::Logger::LogType::CRITICAL,
                                              e.what());
            exit(ExitState::CRITICAL_ERROR);
        }
    }
}