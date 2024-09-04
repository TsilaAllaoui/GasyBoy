#include <iostream>
#include <bitset>
#include <cmath>
#include "cartridge.h"
#include "logger.h"
#include "utils.h"
#include "defs.h"

namespace gasyboy
{

	Cartridge::Cartridge()
		: _currentRomBank(1), // Default ROM bank starts at 1 (0 is always mapped)
		  _currentRamBank(0),
		  _isRamWriteEnabled(false),
		  _currentRtcReg(0),
		  _mbc1Mode(false),
		  _romBanksCount(0),
		  _ramBanksCount(0),
		  _cartridgeType(CartridgeType::ROM_ONLY)
	{
	}

	void Cartridge::loadRom(const std::string &filename)
	{
		std::ifstream file(filename, std::ios::binary | std::ios::ate);
		if (!file.is_open())
		{
			throw std::runtime_error("Unable to open ROM file.");
		}

		std::streamsize size = file.tellg();
		file.seekg(0, std::ios::beg);

		std::vector<uint8_t> buffer(size);
		if (!file.read(reinterpret_cast<char *>(buffer.data()), size))
		{
			throw std::runtime_error("Failed to read ROM file.");
		}

		loadRomFromByteArray(buffer);
	}

	void Cartridge::loadRom(uint8_t size, uint8_t *mem)
	{
		std::vector<uint8_t> buffer(mem, mem + size);
		loadRomFromByteArray(buffer);
	}

	void Cartridge::loadRomFromByteArray(const std::vector<uint8_t> &byteArray)
	{
		// Initialize ROM banks
		_romBanksCount = std::max(2, int(byteArray.size() / 0x4000));
		_romBanks.resize(_romBanksCount, std::vector<uint8_t>(0x4000));

		for (int i = 0; i < _romBanksCount; ++i)
		{
			std::copy(byteArray.begin() + i * 0x4000, byteArray.begin() + (i + 1) * 0x4000, _romBanks[i].begin());
		}

		// Set cartridge type from the ROM header
		setMBCType(_romBanks[0][0x0147]);

		// Set the number of ROM and RAM banks from the ROM header
		setRomBankNumber(_romBanks[0][0x0148]);
		setRamBankNumber(_romBanks[0][0x0149]);
	}

	void Cartridge::setMBCType(const uint8_t &value)
	{
		_cartridgeType = utils::uint8ToCartridgeType(value);
	}

	void Cartridge::setRomBankNumber(const uint8_t &value)
	{
		switch (value)
		{
		case 0x00:
			_romBanksCount = 2;
			break;
		case 0x01:
			_romBanksCount = 4;
			break;
		case 0x02:
			_romBanksCount = 8;
			break;
		case 0x03:
			_romBanksCount = 16;
			break;
		case 0x04:
			_romBanksCount = 32;
			break;
		case 0x05:
			_romBanksCount = 64;
			break;
		case 0x06:
			_romBanksCount = 128;
			break;
		case 0x07:
			_romBanksCount = 256;
			break;
		case 0x08:
			_romBanksCount = 512;
			break;
		default:
			_romBanksCount = 2;
			break;
		}
	}

	void Cartridge::setRamBankNumber(const uint8_t &value)
	{
		switch (value)
		{
		case 0x00:
			_ramBanksCount = 0;
			break;
		case 0x01:
		case 0x02:
			_ramBanksCount = 1;
			break;
		case 0x03:
			_ramBanksCount = 4;
			break;
		case 0x04:
			_ramBanksCount = 16;
			break;
		default:
			_ramBanksCount = 1;
			break;
		}

		_ramBanks.resize(_ramBanksCount, std::vector<uint8_t>(0x2000, 0)); // Each RAM bank is 8KB
	}

	uint8_t Cartridge::romBankRead(const uint16_t &addr)
	{
		if (addr < 0x4000)
		{
			return _romBanks[0][addr]; // Return data from the first ROM bank (0x0000 - 0x3FFF)
		}
		else if (addr >= 0x4000 && addr < 0x8000)
		{
			return _romBanks[_currentRomBank][addr - 0x4000]; // Return data from the current ROM bank (0x4000 - 0x7FFF)
		}
		return 0xFF; // Default value if address is out of range
	}

	uint8_t Cartridge::ramBankRead(const uint16_t &addr)
	{
		if (_isRamWriteEnabled && addr >= 0xA000 && addr < 0xC000)
		{
			return _ramBanks[_currentRamBank][addr - 0xA000]; // Return data from the current RAM bank (0xA000 - 0xBFFF)
		}
		return 0xFF; // Default value if RAM is not accessible or address is out of range
	}

	void Cartridge::mbcRomWrite(const uint16_t &addr, const uint8_t &value)
	{
		if (_cartridgeType == CartridgeType::MBC1 ||
			_cartridgeType == CartridgeType::MBC1_RAM ||
			_cartridgeType == CartridgeType::MBC1_RAM_BATT)
		{
			if (addr < 0x2000)
			{
				// Enable/disable RAM writing
				_isRamWriteEnabled = (value & 0x0F) == 0x0A;
			}
			else if (addr >= 0x2000 && addr < 0x4000)
			{
				// Set ROM bank number
				_currentRomBank = value & 0x1F;
				if (_currentRomBank == 0)
				{
					_currentRomBank = 1;
				}
			}
			else if (addr >= 0x4000 && addr < 0x6000)
			{
				// Set RAM bank number or upper bits of ROM bank number
				if (_mbc1Mode)
				{
					_currentRamBank = value & 0x03;
				}
				else
				{
					_currentRomBank |= (value & 0x03) << 5;
					_currentRomBank &= 0x1F;
					if (_currentRomBank == 0)
					{
						_currentRomBank = 1;
					}
				}
			}
			else if (addr >= 0x6000 && addr < 0x8000)
			{
				// Set MBC1 mode
				_mbc1Mode = (value & 0x01);
			}
		}
	}

	void Cartridge::mbcRamWrite(const uint16_t &addr, const uint8_t &value)
	{
		if (_isRamWriteEnabled && addr >= 0xA000 && addr < 0xC000)
		{
			_ramBanks[_currentRamBank][addr - 0xA000] = value;
		}
	}
}