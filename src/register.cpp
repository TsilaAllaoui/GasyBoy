#include "gbException.h"
#include "register.h"
#include "logger.h"
#include "defs.h"

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

    bool SpecialRegister::getFlag(const char &reg)
    {
        try
        {
            bool value;
            switch (reg)
            {
            case 'Z':
                value = getRightRegister() & (1 << 7);
                break;
            case 'N':
                value = getRightRegister() & (1 << 6);
                break;
            case 'H':
                value = getRightRegister() & (1 << 5);
                break;
            case 'C':
                value = getRightRegister() & (1 << 4);
                break;
            default:
                std::string message("Invalid register: \"");
                message += reg + "\"";
                throw exception::GbException(message);
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

    void SpecialRegister::setFlag(const char &reg)
    {
        try
        {
            uint8_t bit = 0;
            switch (reg)
            {
            case 'Z':
                bit = 0x80;
                break;
            case 'N':
                bit = 0x40;
                break;
            case 'H':
                bit = 0x20;
                break;
            case 'C':
                bit = 0x10;
                break;
            default:
                std::string message("Invalid register: \"");
                message += reg + "\"";
                throw exception::GbException(message);
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

    void SpecialRegister::clearFlag(const char &reg)
    {
        try
        {
            int bit = 0;
            switch (reg)
            {
            case 'Z':
                bit = (~0x80);
                break;
            case 'N':
                bit = (~0x40);
                break;
            case 'H':
                bit = (~0x20);
                break;
            case 'C':
                bit = (~0x10);
                break;
            default:
                std::string message("Invalid register: \"");
                message += reg + "\"";
                throw exception::GbException(message);
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