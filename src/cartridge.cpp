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
		// Allocating banks of External RAM
		_ramBanks = std::vector<uint8_t>(0x8000, 0);

		// Setting current ROM Bank && RAM Bank (ROM usually start at 1)
		_currRomBank = 1;
		_currRamBank = 0;

		// Mode for MBC1
		_mode = true;

		// The cartridge type
		_cartridgeType = CartridgeType::ROM_ONLY;

		// Setting current used RTC register
		_currRTCReg = 0;
	}

	void Cartridge::loadRom(const std::string &filePath)
	{
		std::ifstream rom(filePath.c_str(), std::ios::in | std::ios::binary | std::ios::ate);
		if (rom.is_open())
		{
			// Reading rom file
			rom.seekg(0x147, std::ios::beg);
			char buffer[2];
			rom.read(buffer, 2);

			// Setting cartridge type
			setMBCType(buffer[0]);

			// Setting bank number
			setBankNumber(buffer[1]);

			// Loading rom
			_rom = std::vector<std::vector<uint8_t>>(_banksNumber, std::vector<uint8_t>(0x4000, 0));

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
				_rom[bank][j] = (uint8_t)c;
				j++;
			}

			// Log cartridge informations
			std::stringstream cartridgeHeaderInfo;

			cartridgeHeaderInfo << "ROM Name:  ";
			for (int i = 0x134; i < 0x143; i++)
			{
				cartridgeHeaderInfo << static_cast<char>(_rom[0][i]);
			}

			cartridgeHeaderInfo << std::endl
								<< "Manufacturer:  ";

			for (int i = 0x13F; i < 0x142; i++)
			{
				cartridgeHeaderInfo << static_cast<char>(_rom[0][i]);
			}

			cartridgeHeaderInfo << std::endl
								<< "CGB Support:  ";

			if (_rom[0][0x143] == 0x80)
			{
				cartridgeHeaderInfo << "Yes (DMG support)" << std::endl;
			}
			else if (_rom[0][0x143] == 0xC0)
			{
				cartridgeHeaderInfo << "Yes (No DMG support)" << std::endl;
			}
			else
			{
				cartridgeHeaderInfo << "No" << std::endl;
			}

			cartridgeHeaderInfo << "License Code:  " << _rom[0][0x144] << _rom[0][0x145] << std::endl;

			cartridgeHeaderInfo << "SGB Support:  ";

			if (_rom[0][0x143] == 0x03)
			{
				cartridgeHeaderInfo << "Yes" << std::endl;
			}
			else if (_rom[0][0x143] == 0)
			{
				cartridgeHeaderInfo << "No" << std::endl;
			}

			cartridgeHeaderInfo << "Cartridge Type:  " << static_cast<uint8_t>(_rom[0][0x147]) << std::endl;
			cartridgeHeaderInfo << "Rom Size:  " << 32 * static_cast<uint8_t>(_rom[0][0x148]) << "KiB" << std::endl;
			cartridgeHeaderInfo << "RAM Size:  " << static_cast<uint8_t>(_rom[0][0x149]) << std::endl;
			cartridgeHeaderInfo << "Japanese Cartridge:  " << ((_rom[0][0x14A] & 0x1) ? "No" : "Yes") << std::endl;
			cartridgeHeaderInfo << "Old License Code:  " << static_cast<uint8_t>(_rom[0][0x14B]) << std::endl;
			cartridgeHeaderInfo << "Mask Rom Version:  " << static_cast<uint8_t>(_rom[0][0x14C]) << std::endl;

			utils::Logger::getInstance()->log(utils::Logger::LogType::DEBUG, "\n" + cartridgeHeaderInfo.str());
		}
		else
		{
			utils::Logger::getInstance()->log(utils::Logger::LogType::CRITICAL,
											  "Rom: \"" + filePath + "\" not found!");
			exit(ExitState::CRITICAL_ERROR);
		}
	}

	void Cartridge::loadRom(uint8_t size, uint8_t *mem)
	{
		for (int i = 0; i < size; i++)
		{
			_rom[0][i] = mem[i];
		}
	}

	void Cartridge::setMBCType(const uint8_t &value)
	{
		_cartridgeType = utils::uint8ToCartridgeType(value);
	}

	void Cartridge::setBankNumber(const uint8_t &value)
	{
		switch (value)
		{
		case 0:
			_banksNumber = 2;
			break;
		case 0x1:
			_banksNumber = 4;
			break;
		case 0x2:
			_banksNumber = 8;
			break;
		case 0x3:
			_banksNumber = 16;
			break;
		case 0x4:
			_banksNumber = 32;
			break;
		case 0x5:
			_banksNumber = 64;
			break;
		case 0x6:
			_banksNumber = 128;
			break;
		case 0x7:
			_banksNumber = 256;
			break;
		case 0x8:
			_banksNumber = 512;
			break;
		case 0x52:
			_banksNumber = static_cast<int>(std::pow(72, 3));
			break;
		case 0x53:
			_banksNumber = static_cast<int>(std::pow(80, 3));
			break;
		case 0x54:
			_banksNumber = static_cast<int>(std::pow(96, 3));
			break;
		}
	}

	uint8_t Cartridge::getCurrRomBanks()
	{
		return _currRomBank;
	}

	uint8_t Cartridge::getCurrRamBanks()
	{
		return _currRamBank;
	}

	uint8_t Cartridge::romBankRead(const uint16_t &adrr)
	{
		if (adrr >= 0 && adrr <= 0x3FFF)
		{
			return _rom[0][adrr];
		}
		else if (adrr >= 0x4000 && adrr < 0x8000)
		{
			return _rom[(int)_currRomBank][adrr - 0x4000];
		}

		utils::Logger::getInstance()->log(utils::Logger::LogType::CRITICAL,
										  "Invalid rom bank read");
		exit(ExitState::CRITICAL_ERROR);
	}

	uint8_t Cartridge::ramBankRead(const uint16_t &adrr)
	{
		if (_cartridgeType == CartridgeType::MBC3_RAM_BATT)
		{
			return _currRTCReg;
		}
		else
		{
			return _ramBanks[adrr - 0xA000 + _currRamBank * 0x2000];
		}
	}

	bool Cartridge::isRamWriteEnabled()
	{
		return _enabledRAM;
	}

	void Cartridge::handleRomMemory(const uint16_t &adrr, const uint8_t &value)
	{
		// MBC1 External RAM Switch
		if (adrr < 0x2000)
		{
			if (_cartridgeType == CartridgeType::MBC1_RAM ||
				_cartridgeType == CartridgeType::MBC1_RAM_BATT)
			{
				uint8_t byte = value & 0xF;
				_enabledRAM = (byte == 0xA) ? true : false;
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
				uint8_t lower5bits = value & 0x1F;
				_currRomBank &= 0xE0;
				_currRomBank |= lower5bits;
				if (_currRomBank == 0)
					_currRomBank = 1;
			}
			// MBC3
			else if (_cartridgeType == CartridgeType::MBC3_RAM_BATT)
			{
				uint8_t lower7bits = value & 0x7F;
				_currRomBank &= 0x80;
				_currRomBank |= lower7bits;
				if (_currRomBank == 0)
					_currRomBank = 1;
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
				if (!_mode)
				{
					// ROM mode: Set high bits of bank
					_currRomBank &= 0x1F;
					_currRomBank |= value;
					if (_currRomBank == 0)
						_currRomBank = 1;
				}
				// RAM mode: Set Bank
				else
				{
					_currRamBank = value & 0x3;
				}
			}
			// MBC3
			else if (_cartridgeType == CartridgeType::MBC3_RAM_BATT)
			{
				// for RAM banking
				if (value >= 0 && value <= 3)
				{
					_currRamBank = value & 0x3;
				}

				// for RTC register read/write
				else if (value >= 8 && value <= 0xC)
				{
					switch (value)
					{
					case 0x8:
						_currRTCReg = RTCS;
						break;
					case 0x9:
						_currRTCReg = RTCM;
						break;
					case 0xA:
						_currRTCReg = RTCH;
						break;
					case 0xB:
						_currRTCReg = RTCDL;
						break;
					case 0xC:
						_currRTCReg = RTCDH;
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
				_mode = (value & 0x1);
			}
			// MBC3 RTC registers
			else if (_cartridgeType == CartridgeType::MBC3_RAM_BATT)
			{
				// latching RTC register
			}
		}
	}

	void Cartridge::handleRamMemory(const uint16_t &adrr, const uint8_t &value)
	{
		if (_enabledRAM)
		{
			_ramBanks[adrr - 0xA000 + _currRamBank * 0x2000] = value;
		}
	}

	std::string Cartridge::getGameName()
	{
		return _romName;
	}
}