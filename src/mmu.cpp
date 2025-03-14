#include <cstring>
#include "utilitiesProvider.h"
#include "gamepadProvider.h"
#include "gbException.h"
#include "logger.h"
#include "timer.h"
#include "mmu.h"

namespace gasyboy
{
    Mmu::Mmu(const uint8_t *bytes, const size_t &romSize)
        : _vRam(std::vector<uint8_t>(0x2000, 0)),
          _extRam(std::vector<uint8_t>(0x2000, 0)),
          _workingRam(std::vector<uint8_t>(0x4000, 0)),
          _executeBios(provider::UtilitiesProvider::getInstance().executeBios),
          _cartridge(),
          _gamepad(provider::GamepadProvider::getInstance()),
          _currModifiedTile(-1),
          _dmaRegionWritten(false)
    {
        // setting joypad to off
        _workingRam[0xFFFF - 0xC000] = 0xFF;

        // loading rom file
        try
        {
            _cartridge.loadRomFromByteArray(romSize, const_cast<uint8_t *>(bytes));
            utils::Logger::getInstance()->log(utils::Logger::LogType::FUNCTIONAL,
                                              "Rom file : \"" +
                                                  _romFilePath + "\" loaded successfully");
        }
        catch (const exception::GbException &e)
        {
            utils::Logger::getInstance()->log(utils::Logger::LogType::CRITICAL,
                                              e.what());
        }
    }

    Mmu::Mmu()
        : _vRam(std::vector<uint8_t>(0x2000, 0)),
          _extRam(std::vector<uint8_t>(0x2000, 0)),
          _workingRam(std::vector<uint8_t>(0x4000, 0)),
          _executeBios(provider::UtilitiesProvider::getInstance().executeBios),
          _cartridge(),
          _gamepad(provider::GamepadProvider::getInstance()),
          _currModifiedTile(-1),
          _dmaRegionWritten(false),
          _romFilePath(provider::UtilitiesProvider::getInstance().romFilePath)

    {
        if (!_executeBios)
        {
            writeRam(0xFF40, 0x91);
            writeRam(0xFF47, 0xFC);
            writeRam(0xFF48, 0xFF);
        }

        // setting joypad to off
        _workingRam[0xFFFF - 0xC000] = 0xFF;

        // loading rom file
        try
        {
            if (_romFilePath.empty())
            {
                throw exception::GbException("Invalid rom file path");
            }

            _cartridge.loadRom(_romFilePath);
            utils::Logger::getInstance()->log(utils::Logger::LogType::FUNCTIONAL,
                                              "Rom file : \"" +
                                                  _romFilePath + "\" loaded successfully");
        }
        catch (const exception::GbException &e)
        {
            utils::Logger::getInstance()->log(utils::Logger::LogType::CRITICAL,
                                              e.what());
        }
    }

    void Mmu::reset()
    {
        _vRam = std::vector<uint8_t>(0x2000, 0);
        _extRam = std::vector<uint8_t>(0x2000, 0);
        _workingRam = std::vector<uint8_t>(0x4000, 0);
        _currModifiedTile = -1;
        _dmaRegionWritten = false;
        _romFilePath = provider::UtilitiesProvider::getInstance().romFilePath;
        _executeBios = provider::UtilitiesProvider::getInstance().executeBios;
        _cartridge.reset();

        if (!_executeBios)
        {
            writeRam(0xFF40, 0x91);
            writeRam(0xFF47, 0xFC);
            writeRam(0xFF48, 0xFF);
        }

        // setting joypad to off
        _workingRam[0xFFFF - 0xC000] = 0xFF;

        // loading rom file
        try
        {
            if (_romFilePath.empty())
            {
                throw exception::GbException("Invalid rom file path");
            }

            _cartridge.loadRom(_romFilePath);
            utils::Logger::getInstance()->log(utils::Logger::LogType::FUNCTIONAL,
                                              "Rom file : \"" +
                                                  _romFilePath + "\" loaded successfully");
        }
        catch (const exception::GbException &e)
        {
            utils::Logger::getInstance()->log(utils::Logger::LogType::CRITICAL,
                                              e.what());
        }
    }

    void gasyboy::Mmu::setRomFile(const std::string &filePath)
    {
        _romFilePath = filePath;
    }

    void Mmu::disableBios()
    {
        _executeBios = false;
    }

    bool Mmu::isInBios()
    {
        return _executeBios;
    }

    uint8_t Mmu::readRam(const uint16_t &address)
    {
        try
        {
            if (address < 0x100)
            {
                // if the gameboy is in internal BIOS, in should return BIOS content
                if (_executeBios)
                    return _bios[address];

                // return ROM content otherwise
                else
                    return _cartridge.mbcRomRead(address);
            }

            // for reading ROM
            else if (address >= 0x100 && address < 0x8000)
                return _cartridge.mbcRomRead(address);

            // fro reading vram
            else if (address >= 0x8000 && address < 0xA000)
                return _vRam[address - 0x8000];

            // for reading _extRam
            else if (address >= 0xA000 && address < 0xC000)
                return _cartridge.mbcRamRead(address);

            // for reading high RAM
            else if (address >= 0xC000 && address < 0xFF00)
                return _workingRam[address - 0xC000];

            // for reading I/O region
            else if (address >= 0xFF00 && address <= 0xFFFF)
            {
                // Gamepad register
                if (address == 0xFF00)
                    return _gamepad.getState();

                // DIV register
                else if (address == 0xFF04)
                {
                    return Timer::DIV();
                }
                // TIMA register
                else if (address == 0xFF05)
                {
                    return Timer::TIMA();
                }
                // TMA register
                else if (address == 0xFF06)
                {
                    return Timer::TMA();
                }
                // TAC register
                else if (address == 0xFF07)
                {
                    return Timer::TAC();
                }
                // Boot register (Read only)
                else if (address == 0xFF50)
                {
                    return 0xFF;
                }
                else
                {
                    return _workingRam[address - 0xC000];
                }
            }

            else
            {
                throw exception::GbException("Invalid ram read");
            }
        }
        catch (const exception::GbException &e)
        {
            utils::Logger::getInstance()->log(utils::Logger::LogType::CRITICAL,
                                              e.what());
            exit(ExitState::CRITICAL_ERROR);
        }
    }

    void Mmu::writeRam(const uint16_t &address, const uint8_t &value)
    {
        // if writing to ROM, manage memory banks
        if (address < 0x8000)
        {
            if (_cartridge._cartridgeType == Cartridge::CartridgeType::ROM_ONLY)
            {
                return;
            }

            _cartridge.mbcRomWrite(address, value);
        }

        // if writing to _vRam
        else if (address >= 0x8000 && address < 0xA000)
        {
            // for notifying gpu that _vRam was modified && need t
            if (address <= 0x97FF)
            {
                int MSB;

                if (((address & 0xF000)) == 0x8000)
                    MSB = 0;
                else
                    MSB = 1;

                int multiplier = ((address - 0x8000) & 0xF00) >> 8;
                int index = ((address - 0x8000) & 0xF0) >> 4;
                _currModifiedTile = (index + multiplier * 16 + 256 * MSB);
            }

            _vRam[address - 0x8000] = value;
            if (address >= 0x8000 && address < 0x9800)
            {
                updateTile(address);
                return;
            }
        }

        // writing to _extRam
        else if (address >= 0xA000 && address < 0xC000)
            _cartridge.mbcRamWrite(address, value);

        // writing to _workingRam && HighRAM
        else if (address >= 0xC000 && address <= 0xFFFF)
        {
            // writing to _workingRam
            if (address >= 0xE000 && address < 0xFE00)
            {
                _workingRam[address - 0xC000] = value;
                _workingRam[address - 0x2000 - 0xC000] = value;
            }

            else if (address >= 0xFE00 && address <= 0xFE9F)
            {
                updateSprite(address, value);
                return;
            }

            // for unsupported write in _workingRam
            else if (address >= 0xFEA0 && address <= 0xFEFF)
                return;

            // I/O && HighRAM Region

            // Joypad register
            else if (address == 0xFF00)
                _gamepad.setState(value);

            // for Serail IN/OUT
            else if (address == 0xFF02) // only for Blargg Test roms debugging, TODO: implement serial transfert protocol
            {
                if (value == 0x81)
                {
                    std::string serialCharOutput(1, static_cast<char>(_workingRam[0xFF01 - 0xC000]));
                    utils::Logger::getInstance()->log(utils::Logger::LogType::DEBUG,
                                                      serialCharOutput);
                }
            }

            // writing to DIV register reset its counter
            else if (address == 0xFF04)
            {
                Timer::resetDIV();
            }

            // TIMA timer
            else if (address == 0xFF05)
            {
                // Do nothing
                return;
            }

            // TMA timer
            else if (address == 0xFF06)
            {
                Timer::updateTMA(value);
            }

            // TAC Timer control
            else if (address == 0xFF07)
            {
                Timer::updateTAC(value);
            }

            // writing to LY register reset it
            else if (address == 0xFF44)
                _workingRam[address - 0xC000] = 0;

            // LYC write
            else if (address == 0xFF45)
            {
                _workingRam[address - 0xC000] = value;
            }

            // OAM DMA Transfert
            else if (address == 0xFF46)
            {
                // doDmaTransfert(value);
                if (address == 0xFF46)
                    for (uint16_t i = 0; i < 160; i++)
                        writeRam(0xFE00 + i, readRam((value << 8) + i));
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
                _executeBios = false;
                return;
            }

            // other write
            else
                _workingRam[address - 0xC000] = value;
        }

        // for unsupported writes
        else
            exit(78);
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

    void Mmu::directSet(const uint16_t &address, const uint8_t &value)
    {
        _workingRam[address - 0xC000] = value;
    }

    void Mmu::doDmaTransfert(const uint8_t &value)
    {
        uint16_t data = (value << 8);

        for (uint16_t i = 0; i < 0xA0; i++)
        {
            uint8_t tmp = readRam(data + i);
            directSet(0xFE00 + i, tmp);
        }

        _dmaRegionWritten = true;
    }

    uint16_t Mmu::getNext2Bytes(const uint16_t &adress)
    {
        return (readRam(adress + 2) << 8) | readRam(adress + 1);
    }

    std::vector<uint8_t> Mmu::getVram()
    {
        return _vRam;
    }

    bool Mmu::isVramWritten()
    {
        return _vramWritten;
    }

    void Mmu::setVramWriteStatus(const bool &value)
    {
        _vramWritten = value;
    }

    void Mmu::unsetDMAWritten()
    {
        _dmaRegionWritten = false;
    }

    std::string Mmu::getCartridgeTitle()
    {
        return "TODO: get title"; // TODO
    }

    Cartridge &Mmu::getCartridge()
    {
        return _cartridge;
    }

    void *Mmu::ramCellptr(const uint16_t &pos)
    {
        return &_workingRam[pos - 0xC000];
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