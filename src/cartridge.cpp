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
		  _isRamEnabled(false),
		  _currentRtcReg(0),
		  _bankingMode(BankingMode::MODE_0),
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

		_ramBanks.resize(_ramBanksCount, std::vector<uint8_t>(0x2000, 0));
	}

	uint8_t Cartridge::romBankRead(const uint16_t &addr)
	{
		if (addr < 0x4000)
		{
			return _romBanks[0][addr];
		}
		else if (addr >= 0x4000 && addr < 0x8000)
		{
			if (_currentRomBank < _romBanksCount)
			{
				return _romBanks[_currentRomBank][addr - 0x4000];
			}
			else
			{
				std::stringstream ss;
				ss << "Read Warning: Invalid Rom bank acces: " << std::hex << (int)_currentRomBank << ", max Rom banks number is: " << std::hex << (int)_romBanksCount;
				utils::Logger::getInstance()->log(utils::Logger::LogType::DEBUG, ss.str());
				return _romBanks[_currentRomBank % _romBanksCount][addr - 0x4000]; // Wrap around rom banks if overflow
			}
		}

		std::stringstream ss;
		ss << "Read Warning: Invalid Rom read at address: 0x" << std::hex << (int)addr;
		utils::Logger::getInstance()->log(utils::Logger::LogType::DEBUG, ss.str());
		return 0xFF;
	}

	void Cartridge::mbcRomWrite(const uint16_t &addr, const uint8_t &value)
	{
		if (_cartridgeType == CartridgeType::MBC1 ||
			_cartridgeType == CartridgeType::MBC1_RAM ||
			_cartridgeType == CartridgeType::MBC1_RAM_BATT)
		{
			if (addr >= 0 && addr < 0x2000)
			{
				// Enable/disable RAM writing
				_isRamEnabled = (value & 0x0F) == 0x0A;
			}
			else if (addr >= 0x2000 && addr < 0x4000)
			{
				// Set ROM bank number (lower 5 bits)
				_currentRomBank = value & 0x1F;
				if (_currentRomBank == 0)
				{
					_currentRomBank = 1;
				}
				else if (_currentRomBank == 0x20)
				{
					_currentRomBank = 0x21;
				}
				else if (_currentRomBank == 0x40)
				{
					_currentRomBank = 0x41;
				}
				else if (_currentRomBank == 0x60)
				{
					_currentRomBank = 0x61;
				}
			}
			else if (addr >= 0x4000 && addr < 0x6000)
			{
				if (_bankingMode == BankingMode::MODE_1)
				{
					_currentRamBank = value & 0x03;
				}
				else if (_bankingMode == BankingMode::MODE_0)
				{
					_currentRamBank = 0;
					_currentRomBank = (_currentRomBank & 0x1F) | ((value & 0x03) << 5);
				}
			}
			else if (addr >= 0x6000 && addr < 0x8000)
			{
				// Set MBC1 mode
				_bankingMode = (value & 0x01) == 0 ? BankingMode::MODE_0 : BankingMode::MODE_1;
			}
		}
	}

	uint8_t Cartridge::ramBankRead(const uint16_t &addr)
	{
		if (_isRamEnabled && addr >= 0xA000 && addr < 0xC000)
		{
			if (_currentRamBank < _ramBanksCount)
			{
				return _ramBanks[_currentRamBank][addr - 0xA000];
			}
			else
			{
				std::stringstream ss;
				ss << "Read Warning: Invalid Ram bank acces: " << std::hex << (int)_currentRamBank << ", max Ram banks number is: " << std::hex << (int)_ramBanksCount;
				utils::Logger::getInstance()->log(utils::Logger::LogType::DEBUG, ss.str());
				return _ramBanks[_currentRamBank % _ramBanksCount][addr - 0xA000]; // Wrap around ram banks if overflow
			}
		}
		return 0xFF;
	}

	void Cartridge::mbcRamWrite(const uint16_t &addr, const uint8_t &value)
	{
		if (_isRamEnabled && addr >= 0xA000 && addr < 0xC000)
		{
			if (_currentRamBank < _ramBanksCount)
			{
				_ramBanks[_currentRamBank][addr - 0xA000] = value;
			}
			else
			{
				std::stringstream ss;
				ss << "Write Warning: Invalid Ram bank acces: " << std::hex << (int)_currentRamBank << ", max Ram banks number is: " << std::hex << (int)_ramBanksCount;
				utils::Logger::getInstance()->log(utils::Logger::LogType::DEBUG, ss.str());
				_ramBanks[_currentRamBank % _ramBanksCount][addr - 0xA000] = value; // Wrap around ram banks if overflow
			}
		}
	}
}