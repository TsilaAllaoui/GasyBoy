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
        std::vector<std::vector<uint8_t>> _rom;

        // the current ROM bank
        uint8_t _currRomBank;

        // RAM banks
        std::vector<uint8_t> _ramBanks;

        // the current RAM Bank
        uint8_t _currRamBank;

        // for checking if RAM is writable
        bool _enabledRAM;

        // RTC registers
        uint8_t RTCS, RTCM, RTCH, RTCDL, RTCDH;

        // curren tused RTC register mapped in 0xA000-0xBFFFF
        uint8_t _currRTCReg;

        // MBC mode
        bool _mode;

        // number of banks of the ROM
        int _banksNumber;

    public:
        // constructor/destructor
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

        // cartridge type
        CartridgeType _cartridgeType;

        // loading ROM
        void loadRom(const std::string &filename);

        // For debugging
        void loadRom(uint8_t size, uint8_t *mem);

        // set MBC type
        void setMBCType(const uint8_t &value);

        // set rom banks number
        void setBankNumber(const uint8_t &value);

        // getters && setters
        uint8_t getCurrRomBanks();
        uint8_t getCurrRamBanks();
        bool isRamWriteEnabled();

        // ROM Bank reading
        uint8_t romBankRead(const uint16_t &adrr);

        // RAM Bank reading
        uint8_t ramBankRead(const uint16_t &adrr);

        // to handle all bank changes
        void handleRomMemory(const uint16_t &adrr, const uint8_t &value);
        void handleRamMemory(const uint16_t &adrr, const uint8_t &value);
    };
}

#endif