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

		// Get cartridge header infos
		getCartridgeHeaderInfos();

		// Log cartridge informations
		logCartridgeHeaderInfos();
	}

	void Cartridge::loadRom(uint8_t size, uint8_t *mem)
	{
		std::vector<uint8_t> buffer(mem, mem + size);
		loadRomFromByteArray(buffer);
	}

	void Cartridge::loadRomFromByteArray(const std::vector<uint8_t> &byteArray)
	{
		// Initialize ROM banks
		_romBanksCount = static_cast<uint8_t>(std::max(2, static_cast<int>(byteArray.size() / 0x4000)));
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

		// Get cartridge header infos
		getCartridgeHeaderInfos();

		// Log cartridge informations
		logCartridgeHeaderInfos();
	}

	std::vector<std::vector<uint8_t>> Cartridge::getRomBanks()
	{
		return _romBanks;
	}

	std::vector<std::vector<uint8_t>> Cartridge::getRamBanks()
	{
		return _ramBanks;
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
			_romBanksCount = 2u;
			break;
		case 0x01:
			_romBanksCount = 4u;
			break;
		case 0x02:
			_romBanksCount = 8u;
			break;
		case 0x03:
			_romBanksCount = 16u;
			break;
		case 0x04:
			_romBanksCount = 32u;
			break;
		case 0x05:
			_romBanksCount = 64u;
			break;
		case 0x06:
			_romBanksCount = 128u;
			break;
		case 0x07:
			_romBanksCount = 256u;
			break;
		case 0x08:
			_romBanksCount = 512u;
			break;
		default:
			_romBanksCount = 2u;
			break;
		}
	}

	uint16_t Cartridge::getRomBanksNumber()
	{
		return _romBanksCount;
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

	uint8_t Cartridge::getRamBanksNumber()
	{
		return _ramBanksCount;
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

	std::string Cartridge::cartridgeTypeStr(const uint8_t &byte)
	{
		switch (byte)
		{
		case 0x00:
			return "ROM_ONLY";
		case 0x01:
			return "MBC1";
		case 0x02:
			return "MBC1_RAM";
		case 0x03:
			return "MBC1_RAM_BATT";
		case 0x05:
			return "MBC2";
		case 0x06:
			return "MBC2_BATT";
		case 0x08:
			return "RAM";
		case 0x09:
			return "RAM_BATT";
		case 0x0B:
			return "MMM01";
		case 0x0C:
			return "MMM01_RAM";
		case 0x0D:
			return "MMM01_RAM_BATT";
		case 0x0F:
			return "MBC3_TIMER_BATT";
		case 0x10:
			return "MBC3_TIMER_RAM_BATT";
		case 0x11:
			return "MBC3";
		case 0x12:
			return "MBC3_RAM";
		case 0x13:
			return "MBC3_RAM_BATT";
		case 0x19:
			return "MBC5";
		case 0x1A:
			return "MBC5_RAM";
		case 0x1B:
			return "MBC5_RAM_BATT";
		case 0x1C:
			return "MBC5_RUMBLE";
		case 0x1D:
			return "MBC5_RUMBLE_RAM";
		case 0x1E:
			return "MBC5_RUMBLE_RAM_BATT";
		case 0xFC:
			return "POCKET_CAMERA";
		case 0xFD:
			return "BANDAI_TAMA5";
		case 0xFE:
			return "HuC3";
		case 0xFF:
			return "HuC1_RAM_BATT";
		default:
			return "Unknown";
		}
	}

	std::string Cartridge::romSizeStr(const uint8_t &byte)
	{
		switch (byte)
		{
		case 0:
			return "32KiB";
		case 1:
			return "64KiB";
		case 2:
			return "128KiB";
		case 3:
			return "256KiB";
		case 4:
			return "512KiB";
		case 5:
			return "1MB";
		case 6:
			return "2MB";
		case 0x52:
			return "1.1MB";
		case 0x53:
			return "1.2MB";
		case 0x54:
			return "1.5MB";
		default:
			return "Unknown";
		}
	}

	std::string Cartridge::ramSizeStr(const uint8_t &byte)
	{
		switch (byte)
		{
		case 0:
			return "None";
		case 1:
			return "1KiB";
		case 2:
			return "8KiB";
		case 3:
			return "32KiB";
		case 4:
			return "128KiB";
		default:
			return "Unknown";
		}
	}

	void Cartridge::reset()
	{
		_currentRomBank = 1;
		_currentRamBank = 0;
		_isRamEnabled = false;
		_currentRtcReg = 0;
		_bankingMode = BankingMode::MODE_0;
		_romBanksCount = 0;
		_ramBanksCount = 0;
		_cartridgeType = CartridgeType::ROM_ONLY;
		_cartridgeHeader = CartridgeHeader();
	}

	void Cartridge::getCartridgeHeaderInfos()
	{
		for (int i = 0x134; i < 0x143; i++)
		{
			uint8_t byte = static_cast<char>(_romBanks[0][i]);
			if (byte != 0)
			{
				_cartridgeHeader.name += byte;
			}

			if (i >= 0x13F && i < 0x143 && byte != 0)
			{
				_cartridgeHeader.manufacturer += byte;
			}
		}

		uint8_t cgbSupportByte = _romBanks[0][0x143];
		_cartridgeHeader.cgbSupport =
			cgbSupportByte == 0xC0 ? "Yes (No DMG support)" : (cgbSupportByte == 0x80 ? "Yes (DMG support)" : "No");

		_cartridgeHeader.sgbSupport = _romBanks[0][0x143] == 0x03 ? "Yes" : "No";

		uint8_t oldLicenseeCodeByte = _romBanks[0][0x14B];
		if (oldLicenseeCodeByte == 0x33)
		{
			std::string newLicenseeStr;
			newLicenseeStr.push_back(static_cast<char>(_romBanks[0][0x144]));
			newLicenseeStr.push_back(static_cast<char>(_romBanks[0][0x145]));

			auto code = newLicenseeCodes.find(newLicenseeStr);
			_cartridgeHeader.licenseeCode = code == newLicenseeCodes.end() ? "Unknown" : code->second;
		}
		else
		{
			auto code = oldLicenseeCodes.find(oldLicenseeCodeByte);
			_cartridgeHeader.licenseeCode = code == oldLicenseeCodes.end() ? "Unknown" : code->second;
		}

		_cartridgeHeader.cartridgeType = cartridgeTypeStr(_romBanks[0][0x147]);
		_cartridgeHeader.romSize = romSizeStr(_romBanks[0][0x148]);
		_cartridgeHeader.ramSize = ramSizeStr(_romBanks[0][0x149]);
		_cartridgeHeader.isJapaneseCartridge = (_romBanks[0][0x14A] & 0x1);
		_cartridgeHeader.maskRomVersion = _romBanks[0][0x14C];
	}

	void Cartridge::logCartridgeHeaderInfos()
	{
		std::stringstream cartridgeHeaderInfo;

		cartridgeHeaderInfo << "ROM Name:        	 " << _cartridgeHeader.name << std::endl;
		cartridgeHeaderInfo << "Manufacturer:    	 " << (_cartridgeHeader.manufacturer.empty() ? "N/A" : _cartridgeHeader.manufacturer) << std::endl;
		cartridgeHeaderInfo << "CGB Support:      	 " << _cartridgeHeader.cgbSupport << std::endl;
		cartridgeHeaderInfo << "SGB Support:      	 " << (_cartridgeHeader.sgbSupport ? "Yes" : "No") << std::endl;
		cartridgeHeaderInfo << "Cartridge Type:  	 " << _cartridgeHeader.cartridgeType << std::endl;
		cartridgeHeaderInfo << "Rom Size:        	 " << _cartridgeHeader.romSize << std::endl;
		cartridgeHeaderInfo << "RAM Size:        	 " << _cartridgeHeader.ramSize << std::endl;
		cartridgeHeaderInfo << "Japanese Cartridge:  " << (_cartridgeHeader.isJapaneseCartridge ? "No" : "Yes") << std::endl;
		cartridgeHeaderInfo << "Mask Rom Version:    " << std::hex << static_cast<int>(_cartridgeHeader.maskRomVersion) << std::endl;

		utils::Logger::getInstance()->log(utils::Logger::LogType::DEBUG, "\n" + cartridgeHeaderInfo.str());
	}
}