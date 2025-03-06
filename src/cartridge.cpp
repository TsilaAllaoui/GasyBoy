#include <iostream>
#include <bitset>
#include <cmath>
#include <fstream>
#include <sstream>
#include <vector>
#include <stdexcept>
#include <algorithm>
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
		  _isRamRtcEnabled(false),
		  _currentRtcReg(-1),
		  _rtcLatchState(0),
		  _bankingMode(BankingMode::MODE_0),
		  _romBanksCount(0),
		  _ramBanksCount(0),
		  _cartridgeType(CartridgeType::ROM_ONLY)
	{
		// TODO: add correct rtc bank size
		_rtcBanks.resize(4, std::vector<uint8_t>(0x2000, 0));
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
		// Initialize ROM banks; each bank is 16KB (0x4000)
		_romBanksCount = static_cast<uint16_t>(std::max(2, static_cast<int>(byteArray.size() / 0x4000)));
		_romBanks.resize(_romBanksCount, std::vector<uint8_t>(0x4000));

		for (size_t i = 0; i < _romBanksCount; ++i)
		{
			// Ensure we don't read past byteArray end in case of an incomplete bank.
			size_t start = i * 0x4000;
			size_t end = std::min(start + 0x4000, byteArray.size());
			std::copy(byteArray.begin() + start, byteArray.begin() + end, _romBanks[i].begin());
		}

		// Set cartridge type from the ROM header (at 0x0147)
		setMBCType(_romBanks[0][0x0147]);

		// Set the number of ROM and RAM banks from the ROM header (0x0148 and 0x0149)
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
		// MBC2 typically uses 512 x 4 bits, but here we set 1 bank for consistency if accessed
		case 0x01:
			_ramBanksCount = 1;
			break;
		// 0x02 typically means 8KB RAM (1 bank)
		case 0x02:
			_ramBanksCount = 1;
			break;
		// 0x03 means 32KB RAM (4 banks of 8KB each)
		case 0x03:
			_ramBanksCount = 4;
			break;
		// 0x04 means 128KB RAM (16 banks of 8KB each)
		case 0x04:
			_ramBanksCount = 16;
			break;
		// 0x05 means 64KB RAM (8 banks of 8KB each)
		case 0x05:
			_ramBanksCount = 8;
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
				ss << "Read Warning: Invalid Rom bank access: " << std::hex << (int)_currentRomBank
				   << ", max Rom banks number is: " << std::hex << (int)_romBanksCount;
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
			if (addr >= 0x0000 && addr < 0x2000)
			{
				// Enable/disable external RAM writing.
				_isRamEnabled = ((value & 0x0F) == 0x0A);
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
				// Banking mode affects whether this register selects an additional ROM bank bit
				// or selects the RAM bank number.
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
				// Set banking mode (ROM/RAM banking)
				_bankingMode = (value & 0x01) == 0 ? BankingMode::MODE_0 : BankingMode::MODE_1;
			}
		}
		else if (_cartridgeType == CartridgeType::MBC3 ||
				 _cartridgeType == CartridgeType::MBC3_RAM ||
				 _cartridgeType == CartridgeType::MBC3_RAM_BATT ||
				 _cartridgeType == CartridgeType::MBC3_TIMER_BATT ||
				 _cartridgeType == CartridgeType::MBC3_TIMER_RAM_BATT)
		{
			if (addr >= 0x0000 && addr < 0x2000)
			{
				// Enable/disable external RAM and RTC registers.
				_isRamRtcEnabled = ((value & 0x0F) == 0x0A);
			}
			else if (addr >= 0x2000 && addr < 0x4000)
			{
				// Set ROM bank number (0x01 - 0x7F). Bank 0 is not allowed.
				_currentRomBank = value & 0x7F;
				if (_currentRomBank == 0)
				{
					_currentRomBank = 1;
				}
			}
			else if (addr >= 0x4000 && addr < 0x6000)
			{
				// Select RAM bank (0x00-0x03) or RTC register (0x08-0x0C)
				if (value <= 0x03)
				{
					_currentRamBank = value;
					_currentRtcReg = -1; // No RTC register selected
				}
				else if (value >= 0x08 && value <= 0x0C)
				{
					_currentRtcReg = value;
				}
			}
			else if (addr >= 0x6000 && addr < 0x8000)
			{
				// Latch clock data: writing 0x00 then 0x01 latches the current RTC values.
				if (value == 0x00)
				{
					_rtcLatchState = 0;
				}
				else if (value == 0x01 && _rtcLatchState == 0)
				{
					_rtcLatchState = 1;
					// TODO: Implement LatchRtc() to safely latch RTC counters.
					// LatchRtc();
				}
			}
		}
		else if (_cartridgeType == CartridgeType::MBC5 ||
				 _cartridgeType == CartridgeType::MBC5_RAM ||
				 _cartridgeType == CartridgeType::MBC5_RAM_BATT ||
				 _cartridgeType == CartridgeType::MBC5_RUMBLE ||
				 _cartridgeType == CartridgeType::MBC5_RUMBLE_RAM ||
				 _cartridgeType == CartridgeType::MBC5_RUMBLE_RAM_BATT)
		{
			if (addr >= 0x0000 && addr < 0x2000)
			{
				// Enable/disable external RAM.
				_isRamEnabled = ((value & 0x0F) == 0x0A);
			}
			else if (addr >= 0x2000 && addr < 0x3000)
			{
				// Set lower 8 bits of the ROM bank number.
				_currentRomBank = (_currentRomBank & 0x100) | value;
			}
			else if (addr >= 0x3000 && addr < 0x4000)
			{
				// Set the 9th bit of the ROM bank number.
				_currentRomBank = (_currentRomBank & 0xFF) | ((value & 0x01) << 8);
			}
			else if (addr >= 0x4000 && addr < 0x6000)
			{
				// Select external RAM bank.
				_currentRamBank = value & 0x0F;
			}
		}
	}

	uint8_t Cartridge::ramBankRead(const uint16_t &addr)
	{
		if (_isRamEnabled && addr >= 0xA000 && addr < 0xC000)
		{
			if (_currentRamBank < _ramBanksCount)
			{
				if (_cartridgeType == CartridgeType::MBC1 ||
					_cartridgeType == CartridgeType::MBC1_RAM ||
					_cartridgeType == CartridgeType::MBC1_RAM_BATT)
				{
					return _ramBanks[_currentRamBank][addr - 0xA000];
				}
				else if (_cartridgeType == CartridgeType::MBC3 ||
						 _cartridgeType == CartridgeType::MBC3_RAM ||
						 _cartridgeType == CartridgeType::MBC3_RAM_BATT ||
						 _cartridgeType == CartridgeType::MBC3_TIMER_BATT ||
						 _cartridgeType == CartridgeType::MBC3_TIMER_RAM_BATT)
				{
					return (_currentRtcReg < 0) ? _ramBanks[_currentRamBank][addr - 0xA000]
												: _rtcBanks[_currentRtcReg][addr - 0xA000];
				}
				else if (_cartridgeType == CartridgeType::MBC5 ||
						 _cartridgeType == CartridgeType::MBC5_RAM ||
						 _cartridgeType == CartridgeType::MBC5_RAM_BATT ||
						 _cartridgeType == CartridgeType::MBC5_RUMBLE ||
						 _cartridgeType == CartridgeType::MBC5_RUMBLE_RAM ||
						 _cartridgeType == CartridgeType::MBC5_RUMBLE_RAM_BATT)
				{
					return _ramBanks[_currentRamBank][addr - 0xA000];
				}
			}
			else
			{
				std::stringstream ss;
				ss << "Read Warning: Invalid Ram bank access: " << std::hex << (int)_currentRamBank
				   << ", max Ram banks number is: " << std::hex << (int)_ramBanksCount;
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
				ss << "Write Warning: Invalid Ram bank access: " << std::hex << (int)_currentRamBank
				   << ", max Ram banks number is: " << std::hex << (int)_ramBanksCount;
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
		_currentRtcReg = -1;
		_bankingMode = BankingMode::MODE_0;
		_romBanksCount = 0;
		_ramBanksCount = 0;
		_cartridgeType = CartridgeType::ROM_ONLY;
		_cartridgeHeader = CartridgeHeader();
	}

	void Cartridge::getCartridgeHeaderInfos()
	{
		_cartridgeHeader.name.clear();
		_cartridgeHeader.manufacturer.clear();

		for (int i = 0x134; i < 0x143; i++)
		{
			char byte = static_cast<char>(_romBanks[0][i]);
			if (byte != 0)
			{
				_cartridgeHeader.name.push_back(byte);
			}

			if (i >= 0x13F && i < 0x143 && byte != 0)
			{
				_cartridgeHeader.manufacturer.push_back(byte);
			}
		}

		uint8_t cgbSupportByte = _romBanks[0][0x143];
		_cartridgeHeader.cgbSupport =
			(cgbSupportByte == 0xC0) ? "Yes (No DMG support)" : (cgbSupportByte == 0x80) ? "Yes (DMG support)"
																						 : "No";

		_cartridgeHeader.sgbSupport = (_romBanks[0][0x143] == 0x03) ? "Yes" : "No";

		uint8_t oldLicenseeCodeByte = _romBanks[0][0x14B];
		if (oldLicenseeCodeByte == 0x33)
		{
			std::string newLicenseeStr;
			newLicenseeStr.push_back(static_cast<char>(_romBanks[0][0x144]));
			newLicenseeStr.push_back(static_cast<char>(_romBanks[0][0x145]));

			auto code = newLicenseeCodes.find(newLicenseeStr);
			_cartridgeHeader.licenseeCode = (code == newLicenseeCodes.end()) ? "Unknown" : code->second;
		}
		else
		{
			auto code = oldLicenseeCodes.find(oldLicenseeCodeByte);
			_cartridgeHeader.licenseeCode = (code == oldLicenseeCodes.end()) ? "Unknown" : code->second;
		}

		_cartridgeHeader.cartridgeType = cartridgeTypeStr(_romBanks[0][0x147]);
		_cartridgeHeader.romSize = romSizeStr(_romBanks[0][0x148]);
		_cartridgeHeader.ramSize = ramSizeStr(_romBanks[0][0x149]);
		// Correct interpretation: a destination code of 0 means Japanese.
		_cartridgeHeader.isJapaneseCartridge = (_romBanks[0][0x14A] == 0);
		_cartridgeHeader.maskRomVersion = _romBanks[0][0x14C];
	}

	void Cartridge::logCartridgeHeaderInfos()
	{
		std::stringstream cartridgeHeaderInfo;

		cartridgeHeaderInfo << "ROM Name:            " << _cartridgeHeader.name << std::endl;
		cartridgeHeaderInfo << "Manufacturer:        " << (_cartridgeHeader.manufacturer.empty() ? "N/A" : _cartridgeHeader.manufacturer) << std::endl;
		cartridgeHeaderInfo << "CGB Support:         " << _cartridgeHeader.cgbSupport << std::endl;
		cartridgeHeaderInfo << "SGB Support:         " << _cartridgeHeader.sgbSupport << std::endl;
		cartridgeHeaderInfo << "Cartridge Type:      " << _cartridgeHeader.cartridgeType << std::endl;
		cartridgeHeaderInfo << "Rom Size:            " << _cartridgeHeader.romSize << std::endl;
		cartridgeHeaderInfo << "RAM Size:            " << _cartridgeHeader.ramSize << std::endl;
		cartridgeHeaderInfo << "Japanese Cartridge:  " << (_cartridgeHeader.isJapaneseCartridge ? "Yes" : "No") << std::endl;
		cartridgeHeaderInfo << "Mask Rom Version:    " << std::hex << static_cast<int>(_cartridgeHeader.maskRomVersion) << std::endl;

		utils::Logger::getInstance()->log(utils::Logger::LogType::DEBUG, "\n" + cartridgeHeaderInfo.str());
	}
}
