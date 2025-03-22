#include <cstring>
#include "utilitiesProvider.h"
#include "gamepadProvider.h"
#include "gbException.h"
#include "logger.h"
#include "timer.h"
#include "mmu.h"
#include "timerProvider.h"

namespace gasyboy
{
    Mmu::Mmu(const uint8_t *bytes, const size_t &romSize)
        : _memory(0x10000, 0),
          _biosEnabled(provider::UtilitiesProvider::getInstance()->executeBios),
          _gamepad(provider::GamepadProvider::getInstance()),
          _cartridge()
    {
        // setting joypad to off
        _memory[0xFF00] = 0xFF;

        // loading rom file
        try
        {
            _cartridge.loadRomFromByteArray(romSize, const_cast<uint8_t *>(bytes));
            utils::Logger::getInstance()->log(utils::Logger::LogType::FUNCTIONAL,
                                              "Rom file loaded successfully from byte array");
        }
        catch (const exception::GbException &e)
        {
            utils::Logger::getInstance()->log(utils::Logger::LogType::CRITICAL,
                                              e.what());
        }

        // Load RAM file to ram
        this->loadRam();
    }

    Mmu &Mmu::operator=(const gasyboy::Mmu &other)
    {
        {
            for (int i = 0; i < 0x10000; i++)
            {
                _memory[i] = other._memory[i];
            }
            _biosEnabled = provider::UtilitiesProvider::getInstance()->executeBios;
            _gamepad = other._gamepad;
            _cartridge = other._cartridge;
            return *this;
        }
    }

    Mmu::Mmu()
        : _memory(0x10000, 0),
          _biosEnabled(provider::UtilitiesProvider::getInstance()->executeBios),
          _cartridge(),
          _gamepad(provider::GamepadProvider::getInstance())

    {
        if (!_biosEnabled)
        {
            _memory[0xFF40] = 0x91;
            _memory[0xFF47] = 0xFC;
            _memory[0xFF48] = 0xFF;
        }

        // setting joypad to off
        _memory[0xFF00] = 0xFF;

        // loading rom file
        try
        {
            if (provider::UtilitiesProvider::getInstance()->romFilePath.empty())
            {
                throw exception::GbException("Invalid rom file path");
            }

            _cartridge.loadRom(provider::UtilitiesProvider::getInstance()->romFilePath);
            utils::Logger::getInstance()->log(utils::Logger::LogType::FUNCTIONAL,
                                              "Rom file : \"" +
                                                  provider::UtilitiesProvider::getInstance()->romFilePath + "\" loaded successfully");
        }
        catch (const exception::GbException &e)
        {
            utils::Logger::getInstance()->log(utils::Logger::LogType::CRITICAL,
                                              e.what());
        }

        // Load RAM file to ram
        this->loadRam();
    }

    void Mmu::reset()
    {
        _biosEnabled = provider::UtilitiesProvider::getInstance()->executeBios;
        _cartridge.reset();
        _memory.assign(0x10000, 0);

        if (!_biosEnabled)
        {
            _memory[0xFF40] = 0x91;
            _memory[0xFF47] = 0xFC;
            _memory[0xFF48] = 0xFF;
        }

        // setting joypad to off
        _memory[0xFFFF] = 0xFF;

        // loading rom file
        try
        {
            if (provider::UtilitiesProvider::getInstance()->romFilePath.empty())
            {
                throw exception::GbException("Invalid rom file path");
            }

            _cartridge.loadRom(provider::UtilitiesProvider::getInstance()->romFilePath);
            utils::Logger::getInstance()->log(utils::Logger::LogType::FUNCTIONAL,
                                              "Rom file : \"" +
                                                  provider::UtilitiesProvider::getInstance()->romFilePath + "\" loaded successfully");
        }
        catch (const exception::GbException &e)
        {
            utils::Logger::getInstance()->log(utils::Logger::LogType::CRITICAL,
                                              e.what());
        }

        // Load RAM file to ram
        this->loadRam();
    }

    void Mmu::disableBios()
    {
        _biosEnabled = false;
    }

    bool Mmu::isInBios()
    {
        return _biosEnabled;
    }

    uint8_t Mmu::readRam(const uint16_t &address)
    {
        if (address == 0xff00)
        {
            return _gamepad->getState();
        }

        // Timers
        else if (address == 0xff04)
            return provider::TimerProvider::getInstance()->DIV();
        else if (address == 0xff05)
            return provider::TimerProvider::getInstance()->TIMA();
        else if (address == 0xff06)
            return provider::TimerProvider::getInstance()->TMA();
        else if (address == 0xff07)
            return provider::TimerProvider::getInstance()->TAC();

        if (address == 0xff0f)
            return _memory[0xFF0F];

        // Switchable ROM banks
        if (address < 0x8000)
        {
            if (address < 0x100 && _biosEnabled)
                return _bios[address];
            return _cartridge.mbcRomRead(address);
        }

        // Switchable RAM banks
        if (address >= 0xA000 && address <= 0xBFFF)
            return _cartridge.mbcRamRead(address);

        return _memory[address];
    }

    void Mmu::writeRam(const uint16_t &address, const uint8_t &value)
    {
        // if writing to ROM, manage memory banks
        if (address < 0x8000)
        {
            _cartridge.mbcRomWrite(address, value);
        }

        // if writing to _vRam
        else if (address >= 0x8000 && address < 0xA000)
        {
            _memory[address] = value;
            if (address >= 0x8000 && address < 0x9800)
            {
                updateTile(address);
                return;
            }
        }

        // writing to _extRam
        else if (address >= 0xA000 && address < 0xC000)
        {
            _cartridge.mbcRamWrite(address, value);
        }

        // writing to _memory && HighRAM
        else if (address >= 0xC000 && address <= 0xFFFF)
        {
            // writing to _memory
            if (address >= 0xE000 && address < 0xFE00)
            {
                _memory[address] = value;
                _memory[address - 0x2000] = value;
            }

            else if (address >= 0xFE00 && address <= 0xFE9F)
            {
                updateSprite(address, value);
                return;
            }

            // for unsupported write in _memory
            else if (address >= 0xFEA0 && address <= 0xFEFF)
            {
                return;
            }

            // I/O && HighRAM Region

            // Joypad register
            else if (address == 0xFF00)
            {
                _gamepad->setState(value);
            }

            // for Serial IN/OUT
            else if (address == 0xFF02)
            { // only for Blargg Test roms debugging, TODO: implement serial transfer protocol
                if (value == 0x81)
                {
                    std::string serialCharOutput(1, static_cast<char>(_memory[0xFF01]));
                    utils::Logger::getInstance()->log(utils::Logger::LogType::DEBUG, serialCharOutput);
                }
            }

            // writing to DIV register reset its counter
            else if (address == 0xFF04)
            {
                provider::TimerProvider::getInstance()->setDIV(0);
            }

            // TIMA timer
            else if (address == 0xFF05)
            {
                provider::TimerProvider::getInstance()->setTIMA(value);
            }

            // TMA timer
            else if (address == 0xFF06)
            {
                provider::TimerProvider::getInstance()->setTMA(value);
            }

            // TAC Timer control
            else if (address == 0xFF07)
            {
                provider::TimerProvider::getInstance()->setTAC(value);
            }

            // writing to LY register reset it
            else if (address == 0xFF44)
            {
                _memory[address] = 0;
            }

            // LYC write
            else if (address == 0xFF45)
            {
                _memory[address] = value;
            }

            // OAM DMA Transfer
            else if (address == 0xFF46)
            {
                for (uint16_t i = 0; i < 160; i++)
                {
                    writeRam(0xFE00 + i, readRam((value << 8) + i));
                }
                return;
            }

            else if (address == 0xFF47)
            {
                updatePalette(palette_BGP, value);
                return;
            }
            else if (address == 0xFF48)
            {
                updatePalette(palette_OBP0, value);
                return;
            }
            else if (address == 0xFF49)
            {
                updatePalette(palette_OBP1, value);
                return;
            }
            else if (address == 0xFF50 && value != 0)
            {
                // Bios lockout
                disableBios();
                return;
            }

            // other write
            else
            {
                _memory[address] = value;
            }
        }

        // for unsupported writes
        else
        {
            exit(78);
        }
    }

    uint8_t Mmu::getPaletteColor(const uint8_t &index)
    {
        uint8_t palette[4] = {0};
        palette[0] = readRam(0xFF47) & 0b00000011;
        palette[1] = (readRam(0xFF47) & 0b0000110) >> 2;
        palette[2] = (readRam(0xFF47) & 0b00110000) >> 4;
        palette[3] = (readRam(0xFF47) & 0b11000000) >> 6;
        return palette[index];
    }

    Cartridge &Mmu::getCartridge()
    {
        return _cartridge;
    }

    void Mmu::saveRam()
    {
        _cartridge.saveRam();
    }

    void Mmu::loadRam()
    {
        _cartridge.loadRam();
    }

    void Mmu::updateTile(const uint16_t &laddress)
    {
        uint16_t address = laddress & 0xFFFE;

        uint16_t tile = (address >> 4) & 511;
        uint16_t y = (address >> 1) & 7;

        uint8_t bitIndex;
        for (uint8_t x = 0; x < 8; x++)
        {
            bitIndex = 1 << (7 - x);

            tiles[tile].pixels[y][x] =
                ((readRam(address) & bitIndex) ? 1 : 0) + ((readRam(address + 1) & bitIndex) ? 2 : 0);
        }
    }

    void Mmu::updateSprite(const uint16_t &laddress, const uint8_t &value)
    {
        uint16_t address = laddress - 0xFE00;
        Sprite *sprite = &sprites[address >> 2];
        sprite->ready = false;
        switch (address & 3)
        {
        case 0:
            sprite->y = value - 16;
            break;
        case 1:
            sprite->x = value - 8;
            break;
        case 2:
            sprite->tile = value;
            break;
        case 3:
            sprite->options.value = value;
            sprite->colourPalette = (sprite->options.paletteNumber) ? palette_OBP1 : palette_OBP0;
            sprite->ready = true;
            break;
        }
    }

    void Mmu::updatePalette(Colour *palette, uint8_t value)
    {
        palette[0] = palette_colours[value & 0x3];
        palette[1] = palette_colours[(value >> 2) & 0x3];
        palette[2] = palette_colours[(value >> 4) & 0x3];
        palette[3] = palette_colours[(value >> 6) & 0x3];
    }
}