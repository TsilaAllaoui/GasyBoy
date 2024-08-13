#include "gbException.h"
#include "logger.h"
#include "mmu.h"
#include <cstring>

namespace gasyboy
{
    Mmu::Mmu()
        : _vRam(std::vector<uint8_t>(0x2000, 0)),
          _extRam(std::vector<uint8_t>(0x2000, 0)),
          _workingRam(std::vector<uint8_t>(0x4000, 0)),
          _executeBios(true),
          _cartridge(),
          _gamepad(),
          _currModifiedTile(-1),
          _dmaRegionWritten(false),
          _romFilePath("")
    {
        // setting joypad to off
        _workingRam[0xFFFF - 0xC000] = 0xFF;

        // set debug mode to true
        _debugMode = true;
    }

    Mmu::Mmu(const std::string &romFilePath, Gamepad &gamepad)
        : _vRam(std::vector<uint8_t>(0x2000, 0)),
          _extRam(std::vector<uint8_t>(0x2000, 0)),
          _workingRam(std::vector<uint8_t>(0x4000, 0)),
          _executeBios(true),
          _cartridge(),
          _gamepad(gamepad),
          _currModifiedTile(-1),
          _dmaRegionWritten(false),
          _romFilePath(romFilePath)
    {
        // setting joypad to off
        _workingRam[0xFFFF - 0xC000] = 0xFF;

        // set debug mode to false
        _debugMode = false;

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

    Mmu::Mmu(uint8_t size,
             uint8_t *mem,
             int *num_mem_accesses,
             void *mem_accesses) : Mmu()
    {
        _mem = mem;
        _memSize = size;
        _num_mem_accesses = num_mem_accesses;
        _mem_accesses = static_cast<struct mem_access *>(mem_accesses);
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
        if (_debugMode)
        {
            if (address < _memSize)
                return _mem[address];
            else
                return 0xaa;
        }

        try
        {
            if (address < 0x100)
            {
                // if the gameboy is in internal BIOS, in should return BIOS content
                if (_executeBios)
                    return _bios[address];

                // return ROM content otherwise
                else
                    return _cartridge.romBankRead(address);
            }

            // for reading ROM
            else if (address >= 0x100 && address < 0x8000)
                return _cartridge.romBankRead(address);

            // fro reading vram
            else if (address >= 0x8000 && address < 0xA000)
                return _vRam[address - 0x8000];

            // for reading _extRam
            else if (address >= 0xA000 && address < 0xC000)
                return _cartridge.ramBankRead(address);

            // for reading high RAM
            else if (address >= 0xC000 && address < 0xFF00)
                return _workingRam[address - 0xC000];

            // for reading I/O region
            else if (address >= 0xFF00 && address <= 0xFFFF)
            {
                if (address == 0xFF00)
                    return _gamepad.getState();
                else
                    return _workingRam[address - 0xC000];
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
        if (_debugMode)
        {
            struct mem_access *access = &_mem_accesses[*_num_mem_accesses];
            (*_num_mem_accesses)++;
            access->type = MEM_ACCESS_WRITE;
            access->addr = address;
            access->val = value;
            return;
        }

        // if writing to ROM, manage memory banks
        if (address < 0x8000)
        {
            if (_cartridge._cartridgeType == Cartridge::CartridgeType::ROM_ONLY)
            {
                return;
            }

            _cartridge.handleRomMemory(address, value);
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
        }

        // writing to _extRam
        else if (address >= 0xA000 && address < 0xC000)
            _cartridge.handleRamMemory(address, value);

        // writing to _workingRam && HighRAM
        else if (address >= 0xC000 && address <= 0xFFFF)
        {
            // writing to _workingRam
            if (address >= 0xE000 && address <= 0xFE00)
            {
                _workingRam[address - 0xC000] = value;
                _workingRam[address - 0x2000 - 0xC000] = value;
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
                _workingRam[address - 0xC000] = 0;

            // writing to LY register reset it
            else if (address == 0xFF44)
                _workingRam[address - 0xC000] = 0;

            // OAM DMA Transfert
            else if (address == 0xFF46)
            {
                doDmaTransfert(value);
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

        for (int i = 0; i < 0xA0; i++)
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
}