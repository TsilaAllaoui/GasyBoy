#include "gbException.h"
#include "logger.h"
#include "utils.h"
#include "defs.h"

namespace gasyboy
{
    namespace utils
    {
        gasyboy::Cartridge::Cartridge::CartridgeType uint8ToCartridgeType(const uint8_t &value)
        {
            switch (value)
            {
            case 0x00:
                return Cartridge::Cartridge::CartridgeType::ROM_ONLY;
            case 0x01:
                return Cartridge::Cartridge::CartridgeType::MBC1;
            case 0x02:
                return Cartridge::Cartridge::CartridgeType::MBC1_RAM;
            case 0x03:
                return Cartridge::Cartridge::CartridgeType::MBC1_RAM_BATT;
            case 0x05:
                return Cartridge::Cartridge::CartridgeType::MBC2;
            case 0x06:
                return Cartridge::Cartridge::CartridgeType::MBC2_BATT;
            case 0x08:
                return Cartridge::Cartridge::CartridgeType::RAM;
            case 0x09:
                return Cartridge::Cartridge::CartridgeType::RAM_BATT;
            case 0x0B:
                return Cartridge::Cartridge::CartridgeType::MMM01;
            case 0x0C:
                return Cartridge::Cartridge::CartridgeType::MMM01_RAM;
            case 0x0D:
                return Cartridge::Cartridge::CartridgeType::MMM01_RAM_BATT;
            case 0x0F:
                return Cartridge::Cartridge::CartridgeType::MBC3_TIMER_BATT;
            case 0x10:
                return Cartridge::Cartridge::CartridgeType::MBC3_TIMER_RAM_BATT;
            case 0x11:
                return Cartridge::Cartridge::CartridgeType::MBC3;
            case 0x12:
                return Cartridge::Cartridge::CartridgeType::MBC3_RAM;
            case 0x13:
                return Cartridge::Cartridge::CartridgeType::MBC3_RAM_BATT;
            case 0x19:
                return Cartridge::Cartridge::CartridgeType::MBC5;
            case 0x1A:
                return Cartridge::Cartridge::CartridgeType::MBC5_RAM;
            case 0x1B:
                return Cartridge::Cartridge::CartridgeType::MBC5_RAM_BATT;
            case 0x1C:
                return Cartridge::Cartridge::CartridgeType::MBC5_RUMBLE;
            case 0x1D:
                return Cartridge::Cartridge::CartridgeType::MBC5_RUMBLE_RAM;
            case 0x1E:
                return Cartridge::Cartridge::CartridgeType::MBC5_RUMBLE_RAM_BATT;
            case 0xFC:
                return Cartridge::Cartridge::CartridgeType::POCKET_CAMERA;
            case 0xFD:
                return Cartridge::Cartridge::CartridgeType::BANDAI_TAMA5;
            case 0xFE:
                return Cartridge::Cartridge::CartridgeType::HuC3;
            case 0xFF:
                return Cartridge::Cartridge::CartridgeType::HuC1_RAM_BATT;
            default:
                throw std::invalid_argument("Invalid Cartridge::Cartridge::CartridgeType value");
            }
        }

        std::string XToString::toString(const Cartridge::Cartridge::CartridgeType &cartridgeType)
        {
            try
            {

                switch (cartridgeType)
                {
                case Cartridge::CartridgeType::ROM_ONLY:
                    return "ROM_ONLY";
                case Cartridge::CartridgeType::MBC1:
                    return "MBC1";
                case Cartridge::CartridgeType::MBC1_RAM:
                    return "MBC1_RAM";
                case Cartridge::CartridgeType::MBC1_RAM_BATT:
                    return "MBC1_RAM_BATT";
                case Cartridge::CartridgeType::MBC2:
                    return "MBC2";
                case Cartridge::CartridgeType::MBC2_BATT:
                    return "MBC2_BATT";
                case Cartridge::CartridgeType::RAM:
                    return "RAM";
                case Cartridge::CartridgeType::RAM_BATT:
                    return "RAM_BATT";
                case Cartridge::CartridgeType::MMM01:
                    return "MMM01";
                case Cartridge::CartridgeType::MMM01_RAM:
                    return "MMM01_RAM";
                case Cartridge::CartridgeType::MMM01_RAM_BATT:
                    return "MMM01_RAM_BATT";
                case Cartridge::CartridgeType::MBC3_TIMER_BATT:
                    return "MBC3_TIMER_BATT";
                case Cartridge::CartridgeType::MBC3_TIMER_RAM_BATT:
                    return "MBC3_TIMER_RAM_BATT";
                case Cartridge::CartridgeType::MBC3:
                    return "MBC3";
                case Cartridge::CartridgeType::MBC3_RAM:
                    return "MBC3_RAM";
                case Cartridge::CartridgeType::MBC3_RAM_BATT:
                    return "MBC3_RAM_BATT";
                case Cartridge::CartridgeType::MBC5:
                    return "MBC5";
                case Cartridge::CartridgeType::MBC5_RAM:
                    return "MBC5_RAM";
                case Cartridge::CartridgeType::MBC5_RAM_BATT:
                    return "MBC5_RAM_BATT";
                case Cartridge::CartridgeType::MBC5_RUMBLE:
                    return "MBC5_RUMBLE";
                case Cartridge::CartridgeType::MBC5_RUMBLE_RAM:
                    return "MBC5_RUMBLE_RAM";
                case Cartridge::CartridgeType::MBC5_RUMBLE_RAM_BATT:
                    return "MBC5_RUMBLE_RAM_BATT";
                case Cartridge::CartridgeType::POCKET_CAMERA:
                    return "POCKET_CAMERA";
                case Cartridge::CartridgeType::BANDAI_TAMA5:
                    return "BANDAI_TAMA5";
                case Cartridge::CartridgeType::HuC3:
                    return "HuC3";
                case Cartridge::CartridgeType::HuC1_RAM_BATT:
                    return "HuC1_RAM_BATT";
                default:
                    throw exception::GbException("Unknown cartridge type");
                }
            }
            catch (const exception::GbException &e)
            {
                utils::Logger::getInstance()->log(utils::Logger::LogType::CRITICAL,
                                                  e.what());
                exit(ExitState::CRITICAL_ERROR);
            }
        }
    }
}