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
	{
		// Setting current ROM Bank && RAM Bank (ROM usually start at 1)
		_currentRomBank = 1;
		_currentRamBank = 0;

		// Mode for MBC1
		_mbc1Mode = false;

		// Flag to check if ram write is enabled
		_isRamWriteEnabled = false;

		// The cartridge type
		_cartridgeType = CartridgeType::ROM_ONLY;

		// Setting current used RTC register
		_currentRtcReg = 0;
	}

	void Cartridge::loadRom(const std::string &filePath)
	{
		std::ifstream rom(filePath.c_str(), std::ios::in | std::ios::binary | std::ios::ate);
		if (rom.is_open())
		{
			// Reading rom file
			rom.seekg(0x147, std::ios::beg);
			char buffer[3];
			rom.read(buffer, 3);

			// Setting cartridge type
			setMBCType(buffer[0]);

			// Setting rom bank number
			setRomBankNumber(buffer[1]);

			// Setting ram bank number
			setRamBankNumber(buffer[2]);

			// Loading ext ram
			_ramBanks = std::vector<std::vector<uint8_t>>(_ramBanksCount, std::vector<uint8_t>(0x2000, 0));

			// Loading rom
			_romBanks = std::vector<std::vector<uint8_t>>(_romBanksCount, std::vector<uint8_t>(0x4000, 0));

			// Repositionning file cursor to read from start
			rom.seekg(0, std::ios::end);
			std::streampos romSize = rom.tellg();
			rom.seekg(0, std::ios::beg);

			int bank = 0, j = 0;
			for (int i = 0; i < static_cast<int>(romSize); i++)
			{
				if (i % 0x4000 == 0 && j > 0)
				{
					bank++;
					j = 0;
				}
				rom.seekg(i, std::ios::beg);
				char c;
				rom.read(&c, 1);
				_romBanks[bank][j] = (uint8_t)c;
				j++;
			}

			// Log cartridge informations
			std::stringstream cartridgeHeaderInfo;

			cartridgeHeaderInfo << "ROM Name:  ";
			for (int i = 0x134; i < 0x143; i++)
			{
				cartridgeHeaderInfo << static_cast<char>(_romBanks[0][i]);
			}

			cartridgeHeaderInfo << std::endl
								<< "Manufacturer:  ";

			for (int i = 0x13F; i < 0x142; i++)
			{
				cartridgeHeaderInfo << static_cast<char>(_romBanks[0][i]);
			}

			cartridgeHeaderInfo << std::endl
								<< "CGB Support:  ";

			if (_romBanks[0][0x143] == 0x80)
			{
				cartridgeHeaderInfo << "Yes (DMG support)" << std::endl;
			}
			else if (_romBanks[0][0x143] == 0xC0)
			{
				cartridgeHeaderInfo << "Yes (No DMG support)" << std::endl;
			}
			else
			{
				cartridgeHeaderInfo << "No" << std::endl;
			}

			cartridgeHeaderInfo << "License Code:  " << _romBanks[0][0x144] << _romBanks[0][0x145] << std::endl;

			cartridgeHeaderInfo << "SGB Support:  ";

			if (_romBanks[0][0x143] == 0x03)
			{
				cartridgeHeaderInfo << "Yes" << std::endl;
			}
			else if (_romBanks[0][0x143] == 0)
			{
				cartridgeHeaderInfo << "No" << std::endl;
			}

			cartridgeHeaderInfo << "Cartridge Type:  " << static_cast<uint8_t>(_romBanks[0][0x147]) << std::endl;
			cartridgeHeaderInfo << "Rom Size:  " << 32 * static_cast<uint8_t>(_romBanks[0][0x148]) << "KiB" << std::endl;
			cartridgeHeaderInfo << "RAM Size:  " << static_cast<uint8_t>(_romBanks[0][0x149]) << std::endl;
			cartridgeHeaderInfo << "Japanese Cartridge:  " << ((_romBanks[0][0x14A] & 0x1) ? "No" : "Yes") << std::endl;
			cartridgeHeaderInfo << "Old License Code:  " << static_cast<uint8_t>(_romBanks[0][0x14B]) << std::endl;
			cartridgeHeaderInfo << "Mask Rom Version:  " << static_cast<uint8_t>(_romBanks[0][0x14C]) << std::endl;

			utils::Logger::getInstance()->log(utils::Logger::LogType::DEBUG, "\n" + cartridgeHeaderInfo.str());
		}
		else
		{
			utils::Logger::getInstance()->log(utils::Logger::LogType::CRITICAL,
											  "Rom: \"" + filePath + "\" not found!");
			exit(ExitState::CRITICAL_ERROR);
		}
	}

	void Cartridge::loadRomFromByteArray(const std::vector<uint8_t> &byteArray)
	{
		if (byteArray.size() < 0x150) // Ensure the array is large enough to contain the header information
		{
			utils::Logger::getInstance()->log(utils::Logger::LogType::CRITICAL, "Byte array is too small!");
			exit(ExitState::CRITICAL_ERROR);
		}

		// Setting cartridge type
		setMBCType(byteArray[0x147]);

		// Setting bank number
		setRomBankNumber(byteArray[0x148]);

		// Setting ram bank number
		setRamBankNumber(byteArray[0x149]);

		// Loading ext ram
		_ramBanks = std::vector<std::vector<uint8_t>>(_ramBanksCount, std::vector<uint8_t>(0x2000, 0));

		// Loading rom
		_romBanks = std::vector<std::vector<uint8_t>>(_romBanksCount, std::vector<uint8_t>(0x4000, 0));

		int bank = 0, j = 0;
		for (size_t i = 0; i < byteArray.size(); i++)
		{
			if (i % 0x4000 == 0 && j > 0)
			{
				bank++;
				j = 0;
			}
			_romBanks[bank][j] = byteArray[i];
			j++;
		}

		// Log cartridge information
		std::stringstream cartridgeHeaderInfo;

		cartridgeHeaderInfo << "ROM Name:  ";
		for (int i = 0x134; i < 0x143; i++)
		{
			cartridgeHeaderInfo << static_cast<char>(_romBanks[0][i]);
		}

		cartridgeHeaderInfo << std::endl
							<< "Manufacturer:  ";

		for (int i = 0x13F; i < 0x142; i++)
		{
			cartridgeHeaderInfo << static_cast<char>(_romBanks[0][i]);
		}

		cartridgeHeaderInfo << std::endl
							<< "CGB Support:  ";

		if (_romBanks[0][0x143] == 0x80)
		{
			cartridgeHeaderInfo << "Yes (DMG support)" << std::endl;
		}
		else if (_romBanks[0][0x143] == 0xC0)
		{
			cartridgeHeaderInfo << "Yes (No DMG support)" << std::endl;
		}
		else
		{
			cartridgeHeaderInfo << "No" << std::endl;
		}

		cartridgeHeaderInfo << "License Code:  " << _romBanks[0][0x144] << _romBanks[0][0x145] << std::endl;

		cartridgeHeaderInfo << "SGB Support:  ";

		if (_romBanks[0][0x143] == 0x03)
		{
			cartridgeHeaderInfo << "Yes" << std::endl;
		}
		else if (_romBanks[0][0x143] == 0)
		{
			cartridgeHeaderInfo << "No" << std::endl;
		}

		cartridgeHeaderInfo << "Cartridge Type:  " << static_cast<uint8_t>(_romBanks[0][0x147]) << std::endl;
		cartridgeHeaderInfo << "Rom Size:  " << 32 * static_cast<uint8_t>(_romBanks[0][0x148]) << "KiB" << std::endl;
		cartridgeHeaderInfo << "RAM Size:  " << static_cast<uint8_t>(_romBanks[0][0x149]) << std::endl;
		cartridgeHeaderInfo << "Japanese Cartridge:  " << ((_romBanks[0][0x14A] & 0x1) ? "No" : "Yes") << std::endl;
		cartridgeHeaderInfo << "Old License Code:  " << static_cast<uint8_t>(_romBanks[0][0x14B]) << std::endl;
		cartridgeHeaderInfo << "Mask Rom Version:  " << static_cast<uint8_t>(_romBanks[0][0x14C]) << std::endl;

		utils::Logger::getInstance()->log(utils::Logger::LogType::DEBUG, "\n" + cartridgeHeaderInfo.str());
	}

	void Cartridge::loadRom(uint8_t size, uint8_t *mem)
	{
		for (int i = 0; i < size; i++)
		{
			_romBanks[0][i] = mem[i];
		}
	}

	void Cartridge::setMBCType(const uint8_t &value)
	{
		_cartridgeType = utils::uint8ToCartridgeType(value);
	}

	void Cartridge::setRomBankNumber(const uint8_t &value)
	{
		switch (value)
		{
		case 0:
			_romBanksCount = 2;
			break;
		case 0x1:
			_romBanksCount = 4;
			break;
		case 0x2:
			_romBanksCount = 8;
			break;
		case 0x3:
			_romBanksCount = 16;
			break;
		case 0x4:
			_romBanksCount = 32;
			break;
		case 0x5:
			_romBanksCount = 64;
			break;
		case 0x6:
			_romBanksCount = 128;
			break;
		case 0x7:
			_romBanksCount = 256;
			break;
		case 0x8:
			_romBanksCount = 512;
			break;
		case 0x52:
			_romBanksCount = 72;
			break;
		case 0x53:
			_romBanksCount = 80;
			break;
		case 0x54:
			_romBanksCount = 96;
			break;
		default:
			std::stringstream ssLog;
			ssLog << "Invalid Rom bank count at line: " << __LINE__ << ", in file: " << __FILE__;
			utils::Logger::getInstance()->log(utils::Logger::LogType::CRITICAL,
											  ssLog.str());
			exit(ExitState::CRITICAL_ERROR);
		}
	}

	void Cartridge::setRamBankNumber(const uint8_t &value)
	{
		switch (value)
		{
		case 0:
			_ramBanksCount = 0;
			break;
		case 0x1:
			// Unused
			break;
		case 0x2:
			_ramBanksCount = 1;
			break;
		case 0x3:
			_romBanksCount = 4;
			break;
		case 0x4:
			_romBanksCount = 16;
			break;
		case 0x5:
			_romBanksCount = 8;
			break;
		default:
			std::stringstream ssLog;
			ssLog << "Invalid Ram bank count at line: " << __LINE__ << ", in file: " << __FILE__;
			utils::Logger::getInstance()->log(utils::Logger::LogType::CRITICAL,
											  ssLog.str());
			exit(ExitState::CRITICAL_ERROR);
		}
	}

	uint8_t Cartridge::romBankRead(const uint16_t &adrr)
	{
		if (adrr >= 0 && adrr <= 0x3FFF)
		{
			return _romBanks[0][adrr];
		}
		else if (adrr >= 0x4000 && adrr < 0x8000)
		{
			if (_currentRomBank > _romBanksCount || _currentRomBank < 0)
			{
				std::stringstream ssLog;
				ssLog << "Invalid rom bank number access at line: " << __LINE__ << ", in file: " << __FILE__;
				utils::Logger::getInstance()->log(utils::Logger::LogType::CRITICAL,
												  ssLog.str());
				exit(ExitState::CRITICAL_ERROR);
			}

			return _romBanks[_currentRomBank][adrr - 0x4000];
		}
		else
		{
			std::stringstream ssLog;
			ssLog << "Invalid rom bank number access at line: " << __LINE__ << ", in file: " << __FILE__;
			utils::Logger::getInstance()->log(utils::Logger::LogType::CRITICAL,
											  ssLog.str());
			exit(ExitState::CRITICAL_ERROR);
		}
	}

	uint8_t Cartridge::ramBankRead(const uint16_t &adrr)
	{
		if (_cartridgeType == CartridgeType::MBC3_RAM_BATT)
		{
			return _currentRtcReg;
		}
		else
		{
			if (_currentRamBank > _romBanksCount || _currentRamBank < 0)
			{
				std::stringstream ssLog;
				ssLog << "Invalid ext ram bank number access: " << _currentRamBank << " ,at line: " << __LINE__ << ", in file: " << __FILE__;
				utils::Logger::getInstance()->log(utils::Logger::LogType::CRITICAL,
												  ssLog.str());
				exit(ExitState::CRITICAL_ERROR);
			}

			return _ramBanks[_currentRamBank][adrr - 0xA000];
		}
	}

	void Cartridge::mbcRomWrite(const uint16_t &adrr, const uint8_t &value)
	{
		// MBC1 External RAM Switch
		if (adrr < 0x2000)
		{
			if (_cartridgeType == CartridgeType::MBC1_RAM ||
				_cartridgeType == CartridgeType::MBC1_RAM_BATT)
			{
				uint8_t byte = value & 0xF;
				_isRamWriteEnabled = (byte == 0xA);
			}
		}

		// ROM Bank
		else if (adrr >= 0x2000 && adrr < 0x4000)
		{
			// MBC1
			if (_cartridgeType == CartridgeType::MBC1 ||
				_cartridgeType == CartridgeType::MBC1_RAM ||
				_cartridgeType == CartridgeType::MBC1_RAM_BATT)
			{
				_currentRomBank = (value & 0x1F);
				if (value == 0 || value == 0x20 || value == 0x40 || value == 0x60)
				{
					_currentRomBank += 1;
				}

				if (_currentRomBank == 64 || _currentRomBank > _romBanksCount)
				{
					int a = 0;
				}
			}
			// MBC3
			else if (_cartridgeType == CartridgeType::MBC3_RAM_BATT)
			{
				uint8_t lower7bits = value & 0x7F;
				_currentRomBank &= 0x80;
				_currentRomBank |= lower7bits;
				if (_currentRomBank == 0)
					_currentRomBank = 1;
			}
		}

		// MBC RAM Bank/RTC
		else if (adrr >= 0x4000 && adrr < 0x6000)
		{
			// MBC1
			if (_cartridgeType == CartridgeType::MBC1 ||
				_cartridgeType == CartridgeType::MBC1_RAM ||
				_cartridgeType == CartridgeType::MBC1_RAM_BATT)
			{
				if (!_mbc1Mode)
				{
					// ROM mode: Set high bits of bank
					// _currentRomBank &= 0x1F;
					_currentRomBank = ((value & 3) << 5);
					if (value == 0 || value == 0x20 || value == 0x40 || value == 0x60)
					{
						_currentRomBank += 1;
					}
				}
				// RAM mode: Set Bank
				else
				{
					_currentRamBank = value & 0x3;
				}
			}
			// MBC3
			else if (_cartridgeType == CartridgeType::MBC3_RAM_BATT)
			{
				// for RAM banking
				if (value >= 0 && value <= 3)
				{
					_currentRamBank = value & 0x3;
				}

				// for RTC register read/write
				else if (value >= 8 && value <= 0xC)
				{
					switch (value)
					{
					case 0x8:
						_currentRtcReg = RTCS;
						break;
					case 0x9:
						_currentRtcReg = RTCM;
						break;
					case 0xA:
						_currentRtcReg = RTCH;
						break;
					case 0xB:
						_currentRtcReg = RTCDL;
						break;
					case 0xC:
						_currentRtcReg = RTCDH;
						break;
					default:
						break;
					}
				}
			}
		}

		// MBC1: Mode switch
		else if (adrr >= 0x6000 && adrr < 0x8000)
		{
			// Only for MBC1
			if (_cartridgeType == CartridgeType::MBC1_RAM ||
				_cartridgeType == CartridgeType::MBC1_RAM_BATT)
			{
				_mbc1Mode = (value & 0x1);
			}
			// MBC3 RTC registers
			else if (_cartridgeType == CartridgeType::MBC3_RAM_BATT)
			{
				// latching RTC register
			}
		}
	}

	void Cartridge::mbcRamWrite(const uint16_t &adrr, const uint8_t &value)
	{
		if (_isRamWriteEnabled)
		{
			if (_currentRamBank > _romBanksCount || _currentRamBank < 0)
			{
				std::stringstream ssLog;
				ssLog << "Invalid ext ram bank number access: " << _currentRamBank << " ,at line: " << __LINE__ << ", in file: " << __FILE__;
				utils::Logger::getInstance()->log(utils::Logger::LogType::CRITICAL,
												  ssLog.str());
				exit(ExitState::CRITICAL_ERROR);
			}
			_ramBanks[_currentRamBank][adrr - 0xA000] = value;
		}
	}
}