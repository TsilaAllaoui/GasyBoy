#ifndef MBC_H
#define MBC_H

#include <iostream>
#include <cstdint>
#include <vector>
#include "gbException.h"

namespace gasyboy
{
    class IMBC
    {
    public:
        virtual uint8_t readByte(const uint16_t &address) = 0;
        virtual void writeByte(const uint16_t &address, const uint8_t &value) = 0;
        virtual std::vector<uint8_t> &getRom() = 0;
        virtual std::vector<uint8_t> &getRam() = 0;
        virtual ~IMBC() = default;
    };

    class MBC0 : public IMBC
    {
    public:
        std::vector<uint8_t> _rom;

        MBC0(const std::vector<uint8_t> &rom);
        virtual uint8_t readByte(const uint16_t &address) override;
        virtual void writeByte(const uint16_t &address, const uint8_t &value) override {}
        virtual std::vector<uint8_t> &getRom() override { return _rom; }
        virtual std::vector<uint8_t> &getRam() override { throw exception::GbException("MBC0 does not have RAM"); }
    };

    class MBC1 : public IMBC
    {
    public:
        std::vector<uint8_t> _rom;
        std::vector<uint8_t> _ram;
        int _romBanksCount = 1;
        int _ramBanksCount = 1;
        bool _ramEnabled;
        bool _mode;
        uint8_t _romBank;
        uint8_t _ramBank;

        MBC1(const std::vector<uint8_t> &rom, const std::vector<uint8_t> &ram, int romBanksCount, int ramBanksCount);
        virtual uint8_t readByte(const uint16_t &address) override;
        virtual void writeByte(const uint16_t &address, const uint8_t &value) override;
        virtual std::vector<uint8_t> &getRom() override { return _rom; }
        virtual std::vector<uint8_t> &getRam() override { return _ram; }
    };

    class MBC2 : public MBC1
    {
    public:
        MBC2(const std::vector<uint8_t> &rom, const std::vector<uint8_t> &ram, int romBanksCount, int ramBanksCount) : MBC1(rom, ram, romBanksCount, ramBanksCount) {}
        uint8_t readByte(const uint16_t &address);
        void writeByte(const uint16_t &address, const uint8_t &value);
    };

    class MBC3 : public MBC1
    {
    public:
        MBC3(const std::vector<uint8_t> &rom, const std::vector<uint8_t> &ram, int romBanksCount, int ramBanksCount) : MBC1(rom, ram, romBanksCount, ramBanksCount) {}
        uint8_t readByte(const uint16_t &address);
        void writeByte(const uint16_t &address, const uint8_t &value);
    };

    class MBC5 : public MBC1
    {
    public:
        MBC5(const std::vector<uint8_t> &rom, const std::vector<uint8_t> &ram, int romBanksCount, int ramBanksCount) : MBC1(rom, ram, romBanksCount, ramBanksCount) {}
        uint8_t readByte(const uint16_t &address);
        void writeByte(const uint16_t &address, const uint8_t &value);
    };
}

#endif