#ifndef _CARTRIDGE_H_
#define _CARTRIDGE_H_

#include <iostream>
#include <fstream>
#include <cstring>
#include <vector>
#include <cmath>

namespace gasyboy
{
    class Cartridge
    {
    private:
        // ROM memory
        std::vector<std::vector<uint8_t>> _romBanks;

        // The current ROM bank
        uint8_t _currentRomBank;

        // RAM banks
        std::vector<std::vector<uint8_t>> _ramBanks;

        // The current RAM Bank
        uint8_t _currentRamBank;

        // For checking if RAM is writable
        bool _isRamEnabled;

        // RTC registers
        uint8_t RTCS, RTCM, RTCH, RTCDL, RTCDH;

        // Current used RTC register mapped in 0xA000-0xBFFFF
        uint8_t _currentRtcReg;

        // ROM/RAM Banking mode
        enum class BankingMode
        {
            MODE_0,
            MODE_1
        };

        // MBC mode
        BankingMode _bankingMode;

        // Number of banks of the ROM
        int _romBanksCount;

        // Number of banks of the ext RAM
        int _ramBanksCount;

    public:
        // Constructor/destructor
        Cartridge();
        ~Cartridge() = default;

        // MBC type
        enum class CartridgeType
        {
            ROM_ONLY = 0x00,
            MBC1 = 0x01,
            MBC1_RAM = 0x02,
            MBC1_RAM_BATT = 0x03,
            MBC2 = 0x05,
            MBC2_BATT = 0x06,
            RAM = 0x08,
            RAM_BATT = 0x09,
            MMM01 = 0x0B,
            MMM01_RAM = 0x0C,
            MMM01_RAM_BATT = 0x0D,
            MBC3_TIMER_BATT = 0x0F,
            MBC3_TIMER_RAM_BATT = 0x10,
            MBC3 = 0x11,
            MBC3_RAM = 0x12,
            MBC3_RAM_BATT = 0x13,
            MBC5 = 0x19,
            MBC5_RAM = 0x1A,
            MBC5_RAM_BATT = 0x1B,
            MBC5_RUMBLE = 0x1C,
            MBC5_RUMBLE_RAM = 0x1D,
            MBC5_RUMBLE_RAM_BATT = 0x1E,
            POCKET_CAMERA = 0xFC,
            BANDAI_TAMA5 = 0xFD,
            HuC3 = 0xFE,
            HuC1_RAM_BATT = 0xFF
        };

        // Cartridge type
        CartridgeType _cartridgeType;

        // Loading ROM
        void loadRom(const std::string &filename);

        // For debugging
        void loadRom(uint8_t size, uint8_t *mem);

        // Set MBC type
        void setMBCType(const uint8_t &value);

        // Get/Set rom banks number
        void setRomBankNumber(const uint8_t &value);
        uint8_t getRomBanksNumber();

        // Get/Set rom banks number
        void setRamBankNumber(const uint8_t &value);
        uint8_t getRamBanksNumber();

        // ROM Bank reading
        uint8_t romBankRead(const uint16_t &adrr);

        // RAM Bank reading
        uint8_t ramBankRead(const uint16_t &adrr);

        // To handle all bank changes
        void mbcRomWrite(const uint16_t &adrr, const uint8_t &value);
        void mbcRamWrite(const uint16_t &adrr, const uint8_t &value);

        // To load rom from byte array
        void loadRomFromByteArray(const std::vector<uint8_t> &byteArray);

        // Get rom content
        std::vector<std::vector<uint8_t>> getRomBanks();
    };
}

#endif