#include "utils.h"

namespace gasyboy
{
    namespace utils
    {
        gasyboy::Cartridge::CartridgeType uint8ToCartridgeType(const uint8_t &value)
        {
            switch (value)
            {
            case 0x00:
                return Cartridge::CartridgeType::ROM_ONLY;
            case 0x01:
                return Cartridge::CartridgeType::MBC1;
            case 0x02:
                return Cartridge::CartridgeType::MBC1_RAM;
            case 0x03:
                return Cartridge::CartridgeType::MBC1_RAM_BATT;
            case 0x05:
                return Cartridge::CartridgeType::MBC2;
            case 0x06:
                return Cartridge::CartridgeType::MBC2_BATT;
            case 0x08:
                return Cartridge::CartridgeType::RAM;
            case 0x09:
                return Cartridge::CartridgeType::RAM_BATT;
            case 0x0B:
                return Cartridge::CartridgeType::MMM01;
            case 0x0C:
                return Cartridge::CartridgeType::MMM01_RAM;
            case 0x0D:
                return Cartridge::CartridgeType::MMM01_RAM_BATT;
            case 0x0F:
                return Cartridge::CartridgeType::MBC3_TIMER_BATT;
            case 0x10:
                return Cartridge::CartridgeType::MBC3_TIMER_RAM_BATT;
            case 0x11:
                return Cartridge::CartridgeType::MBC3;
            case 0x12:
                return Cartridge::CartridgeType::MBC3_RAM;
            case 0x13:
                return Cartridge::CartridgeType::MBC3_RAM_BATT;
            case 0x19:
                return Cartridge::CartridgeType::MBC5;
            case 0x1A:
                return Cartridge::CartridgeType::MBC5_RAM;
            case 0x1B:
                return Cartridge::CartridgeType::MBC5_RAM_BATT;
            case 0x1C:
                return Cartridge::CartridgeType::MBC5_RUMBLE;
            case 0x1D:
                return Cartridge::CartridgeType::MBC5_RUMBLE_RAM;
            case 0x1E:
                return Cartridge::CartridgeType::MBC5_RUMBLE_RAM_BATT;
            case 0xFC:
                return Cartridge::CartridgeType::POCKET_CAMERA;
            case 0xFD:
                return Cartridge::CartridgeType::BANDAI_TAMA5;
            case 0xFE:
                return Cartridge::CartridgeType::HuC3;
            case 0xFF:
                return Cartridge::CartridgeType::HuC1_RAM_BATT;
            default:
                throw std::invalid_argument("Invalid Cartridge::CartridgeType value");
            }
        }
    }
}