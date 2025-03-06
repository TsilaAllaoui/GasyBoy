#include <cstring>
#include <vector>
#include <string>
#include <stdexcept>
#include "utilitiesProvider.h"
#include "gamepadProvider.h"
#include "gbException.h"
#include "logger.h"
#include "timer.h"
#include "mmu.h"

namespace gasyboy
{

    // Optimized constructor using iterator initialization for ROM bytes.
    Mmu::Mmu(const uint8_t *bytes, const size_t &romSize)
        : _vRam(0x2000, 0),
          _extRam(0x2000, 0),
          _workingRam(0x4000, 0),
          _executeBios(provider::UtilitiesProvider::getInstance().executeBios),
          _cartridge(),
          _gamepad(provider::GamepadProvider::getInstance()),
          _currModifiedTile(-1),
          _dmaRegionWritten(false)
    {
        // Setting joypad register to off (assuming High RAM offset 0xFFFF corresponds to index 0xFFFF-0xC000)
        _workingRam[0xFFFF - 0xC000] = 0xFF;

        // Load ROM file using vector iterator initialization.
        try
        {
            std::vector<uint8_t> byteList(bytes, bytes + romSize);
            _cartridge.loadRomFromByteArray(byteList);
            utils::Logger::getInstance()->log(utils::Logger::LogType::FUNCTIONAL,
                                              "Rom file : \"" + _romFilePath + "\" loaded successfully");
        }
        catch (const exception::GbException &e)
        {
            utils::Logger::getInstance()->log(utils::Logger::LogType::CRITICAL, e.what());
        }
    }

    // Default constructor: loads ROM using file path from provider.
    Mmu::Mmu()
        : _vRam(0x2000, 0),
          _extRam(0x2000, 0),
          _workingRam(0x4000, 0),
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

        // Setting joypad register to off.
        _workingRam[0xFFFF - 0xC000] = 0xFF;

        // Load ROM file.
        try
        {
            if (_romFilePath.empty())
                throw exception::GbException("Invalid rom file path");
            _cartridge.loadRom(_romFilePath);
            utils::Logger::getInstance()->log(utils::Logger::LogType::FUNCTIONAL,
                                              "Rom file : \"" + _romFilePath + "\" loaded successfully");
        }
        catch (const exception::GbException &e)
        {
            utils::Logger::getInstance()->log(utils::Logger::LogType::CRITICAL, e.what());
        }
    }

    void Mmu::reset()
    {
        // Reinitialize RAM regions using vector assignment.
        _vRam.assign(0x2000, 0);
        _extRam.assign(0x2000, 0);
        _workingRam.assign(0x4000, 0);
        _currModifiedTile = -1;
        _dmaRegionWritten = false;
        _romFilePath = provider::UtilitiesProvider::getInstance().romFilePath;
        _cartridge.reset();

        if (!_executeBios)
        {
            writeRam(0xFF40, 0x91);
            writeRam(0xFF47, 0xFC);
            writeRam(0xFF48, 0xFF);
        }

        // Setting joypad register to off.
        _workingRam[0xFFFF - 0xC000] = 0xFF;

        // Load ROM file.
        try
        {
            if (_romFilePath.empty())
                throw exception::GbException("Invalid rom file path");
            _cartridge.loadRom(_romFilePath);
            utils::Logger::getInstance()->log(utils::Logger::LogType::FUNCTIONAL,
                                              "Rom file : \"" + _romFilePath + "\" loaded successfully");
        }
        catch (const exception::GbException &e)
        {
            utils::Logger::getInstance()->log(utils::Logger::LogType::CRITICAL, e.what());
        }
    }

    void Mmu::setRomFile(const std::string &filePath)
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
                // If BIOS is still active, return BIOS content.
                if (_executeBios)
                    return _bios[address];
                else
                    return _cartridge.romBankRead(address);
            }
            else if (address < 0x8000)
                return _cartridge.romBankRead(address);
            else if (address < 0xA000)
                return _vRam[address - 0x8000];
            else if (address < 0xC000)
                return _cartridge.ramBankRead(address);
            else if (address < 0xFF00)
                return _workingRam[address - 0xC000];
            else if (address <= 0xFFFF)
            {
                switch (address)
                {
                case 0xFF00:
                    return _gamepad.getState();
                case 0xFF04:
                    return Timer::DIV();
                case 0xFF05:
                    return Timer::TIMA();
                case 0xFF06:
                    return Timer::TMA();
                case 0xFF07:
                    return Timer::TAC();
                case 0xFF50:
                    return 0xFF;
                default:
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
            utils::Logger::getInstance()->log(utils::Logger::LogType::CRITICAL, e.what());
            exit(ExitState::CRITICAL_ERROR);
        }
    }

    void Mmu::writeRam(const uint16_t &address, const uint8_t &value)
    {
        // Writing to ROM space is handled by the cartridge MBC.
        if (address < 0x8000)
        {
            if (_cartridge._cartridgeType == Cartridge::CartridgeType::ROM_ONLY)
                return;
            _cartridge.mbcRomWrite(address, value);
            return;
        }
        else if (address < 0xA000)
        {
            // VRAM region.
            if (address <= 0x97FF)
            {
                int MSB = ((address & 0xF000) == 0x8000) ? 0 : 1;
                int multiplier = ((address - 0x8000) & 0xF00) >> 8;
                int index = ((address - 0x8000) & 0xF0) >> 4;
                _currModifiedTile = index + multiplier * 16 + 256 * MSB;
            }
            _vRam[address - 0x8000] = value;
            if (address < 0x9800)
            {
                updateTile(address);
                return;
            }
        }
        else if (address < 0xC000)
        {
            // External RAM.
            _cartridge.mbcRamWrite(address, value);
        }
        else if (address <= 0xFFFF)
        {
            // Working RAM and High RAM.
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
            else if (address >= 0xFEA0 && address <= 0xFEFF)
            {
                return; // Unusable region.
            }
            else
            {
                // I/O and High RAM.
                switch (address)
                {
                case 0xFF00:
                    _gamepad.setState(value);
                    break;
                case 0xFF02:
                    if (value == 0x81)
                    {
                        std::string serialCharOutput(1, static_cast<char>(_workingRam[0xFF01 - 0xC000]));
                        utils::Logger::getInstance()->log(utils::Logger::LogType::DEBUG, serialCharOutput);
                    }
                    break;
                case 0xFF04:
                    Timer::resetDIV();
                    break;
                case 0xFF05:
                    return; // TIMA is write-protected.
                case 0xFF06:
                    Timer::updateTMA(value);
                    break;
                case 0xFF07:
                    Timer::updateTAC(value);
                    break;
                case 0xFF44:
                    _workingRam[address - 0xC000] = 0; // LY is read-only.
                    break;
                case 0xFF45:
                    _workingRam[address - 0xC000] = value;
                    break;
                case 0xFF46:
                    for (uint16_t i = 0; i < 160; i++)
                        writeRam(0xFE00 + i, readRam((value << 8) + i));
                    return;
                case 0xFF47:
                    updatePalette(palette_BGP, value);
                    return;
                case 0xFF48:
                    updatePalette(palette_OBP0, value);
                    return;
                case 0xFF49:
                    updatePalette(palette_OBP1, value);
                    return;
                case 0xFF50:
                    if (value != 0)
                        _executeBios = false;
                    return;
                default:
                    _workingRam[address - 0xC000] = value;
                    break;
                }
            }
        }
        else
        {
            exit(78);
        }
    }

    uint8_t Mmu::getPaletteColor(const uint8_t &index)
    {
        uint8_t palette[4];
        uint8_t raw = readRam(0xFF47);
        palette[0] = raw & 0x03;
        palette[1] = (raw >> 2) & 0x03; // Corrected mask (0x0C >> 2 is equivalent to (raw >> 2) & 0x03)
        palette[2] = (raw >> 4) & 0x03;
        palette[3] = (raw >> 6) & 0x03;
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

    uint16_t Mmu::getNext2Bytes(const uint16_t &address)
    {
        return (readRam(address + 2) << 8) | readRam(address + 1);
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
        return "TODO: get title"; // TODO: extract title from cartridge header.
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
        uint16_t address = laddress & 0xFFFE;   // Align to even address.
        uint16_t tile = (address >> 4) & 0x1FF; // 9-bit tile index (0-511)
        uint16_t y = (address >> 1) & 0x07;     // Row within tile (0-7)

        uint8_t byte1 = readRam(address);
        uint8_t byte2 = readRam(address + 1);

        for (uint8_t x = 0; x < 8; x++)
        {
            uint8_t mask = 1 << (7 - x);
            tiles[tile].pixels[y][x] =
                ((byte1 & mask) ? 1 : 0) + ((byte2 & mask) ? 2 : 0);
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
        palette[0] = palette_colours[value & 0x03];
        palette[1] = palette_colours[(value >> 2) & 0x03];
        palette[2] = palette_colours[(value >> 4) & 0x03];
        palette[3] = palette_colours[(value >> 6) & 0x03];
    }

} // namespace gasyboy
