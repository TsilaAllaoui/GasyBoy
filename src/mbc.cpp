#include "MBC.h"

namespace gasyboy
{

    MBC0::MBC0(std::vector<uint8_t> rom) : _rom(rom)
    {
        int a = 0;
    }

    uint8_t MBC0::readByte(const uint16_t &address)
    {
        if (address < 0x8000)
            return _rom[address];

        return 0;
    }

    MBC1::MBC1(const std::vector<uint8_t> &rom, const std::vector<uint8_t> &ram, int romBanksCount, int ramBanksCount)
        : _rom(rom),
          _ram(ram),
          _romBanksCount(romBanksCount),
          _ramBanksCount(ramBanksCount),
          _ramEnabled(false),
          _mode(false),
          _romBank(1),
          _ramBank(0)
    {
    }

    uint8_t MBC1::readByte(const uint16_t &address)
    {
        if (address < 0x4000)
        {
            int bank = _mode * (_ramBank << 5) % _romBanksCount;
            return _rom[bank * 0x4000 + address];
        }
        else if (address < 0x8000)
        {
            int bank = ((_ramBank << 5) | _romBank) % _romBanksCount;
            return _rom[bank * 0x4000 + address - 0x4000];
        }
        else if (address >= 0xA000 && address < 0xC000)
        {
            if (_ramEnabled)
            {
                int bank = _mode * _ramBank % _ramBanksCount;
                return _ram[bank * 0x2000 + address - 0xA000];
            }
        }
        return 0xFF;
    }

    void MBC1::writeByte(const uint16_t &address, const uint8_t &value)
    {
        if (address < 0x2000)
        {
            _ramEnabled = (value & 0x0F) == 0x0A;
        }
        else if (address < 0x4000)
        {
            auto shiftedValue = value & 0x1F;
            if (shiftedValue == 0)
                shiftedValue = 1;
            _romBank = shiftedValue;
        }
        else if (address < 0x6000)
        {
            _ramBank = value & 0x3;
        }
        else if (address < 0x8000)
        {
            _mode = value & 0x1;
        }
        else if (address >= 0xA000 && address < 0xC000)
        {
            if (_ramEnabled)
            {
                int bank = (_ramBank * _mode) % _ramBanksCount;
                _ram[bank * 0x2000 + address - 0xA000] = value;
            }
        }
    }

    uint8_t MBC2::readByte(const uint16_t &address)
    {
        if (address < 0x4000)
            return _rom[address];
        else if (address < 0x8000)
            return _rom[_romBank * 0x4000 + address - 0x4000];
        else if (address >= 0xA000 && address < 0xC000)
        {
            if (_ramEnabled)
                return _ram[_ramBank * 0x2000 + address - 0xA000];
        }

        return 0;
    }

    void MBC2::writeByte(const uint16_t &address, const uint8_t &value)
    {
        if (address < 0x2000)
        {
            if ((address & 0x0100) == 0)
                _ramEnabled = value == 0x0a;
        }
        else if (address < 0x4000)
        {
            if ((address & 0x0100) != 0)
                _romBank = value;
        }
        else if (address >= 0xA000 && address < 0xC000)
        {
            if (_ramEnabled)
                _ram[_ramBank * 0x2000 + address - 0xA000] = value;
        }
    }

    uint8_t MBC3::readByte(const uint16_t &address)
    {
        if (address < 0x4000)
            return _rom[address];
        else if (address < 0x8000)
            return _rom[_romBank * 0x4000 + address - 0x4000];
        else if (address >= 0xA000 && address < 0xC000)
        {
            if (_ramEnabled)
            {
                if (_ramBank <= 0x03)
                    return _ram[_ramBank * 0x2000 + address - 0xA000];
            }
        }

        return 0;
    }

    void MBC3::writeByte(const uint16_t &address, const uint8_t &value)
    {
        if (address < 0x2000)
            _ramEnabled = (value & 0x0f) == 0x0a;
        else if (address < 0x4000)
        {
            _romBank = value;
            if (_romBank == 0x00)
                _romBank = 0x01;
        }
        else if (address < 0x6000)
            _ramBank = value & 0x0f;
        else if (address >= 0xA000 && address < 0xC000)
        {
            if (_ramEnabled)
            {
                if (_ramBank <= 0x03)
                    _ram[_ramBank * 0x2000 + address - 0xA000] = value;
            }
        }
    }

    uint8_t MBC5::readByte(const uint16_t &address)
    {
        if (address < 0x4000)
            return _rom[address];
        else if (address < 0x8000)
            return _rom[_romBank * 0x4000 + address - 0x4000];
        else if (address >= 0xA000 && address < 0xC000)
        {
            if (_ramEnabled)
                return _ram[_ramBank * 0x2000 + address - 0xA000];
        }

        return 0;
    }

    void MBC5::writeByte(const uint16_t &address, const uint8_t &value)
    {
        if (address < 0x2000)
            _ramEnabled = (value & 0x0f) == 0x0a;
        else if (address < 0x3000)
            _romBank = (_romBank & 0x100) | value;
        else if (address < 0x4000)
            _romBank = (_romBank & 0xff) | ((value & 0x01) << 8);
        else if (address < 0x6000)
            _ramBank = (value & 0x0f) % _ramBanksCount;
        else if (address >= 0xA000 && address < 0xC000)
        {
            if (_ramEnabled)
                _ram[_ramBank * 0x2000 + address - 0xA000] = value;
        }
    }

}