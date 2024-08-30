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

			// Get cartridge header infos
			getCartridgeHeaderInfos();

			// Log cartridge informations
			logCartridgeHeaderInfos();
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
		setBankNumber(byteArray[0x148]);

		// Loading rom
		_rom = std::vector<std::vector<uint8_t>>(_banksNumber, std::vector<uint8_t>(0x4000, 0));

		int bank = 0, j = 0;
		for (size_t i = 0; i < byteArray.size(); i++)
		{
			if (i % 0x4000 == 0 && j > 0)
			{
				bank++;
				j = 0;
			}
			_rom[bank][j] = byteArray[i];
			j++;
		}

		// Get cartridge header infos
		getCartridgeHeaderInfos();

		// Log cartridge information
		logCartridgeHeaderInfos();
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

	void Cartridge::getCartridgeHeaderInfos()
	{
		for (int i = 0x134; i < 0x143; i++)
		{
			uint8_t byte = static_cast<char>(_rom[0][i]);
			if (byte != 0)
			{
				_cartridgeHeader.name += byte;
			}

			if (i >= 0x13F && i < 0x143 && byte != 0)
			{
				_cartridgeHeader.manufacturer += byte;
			}
		}

		uint8_t cgbSupportByte = _rom[0][0x143];
		_cartridgeHeader.cgbSupport =
			cgbSupportByte == 0xC0 ? "Yes (No DMG support)" : (cgbSupportByte == 0x80 ? "Yes (DMG support)" : "No");

		_cartridgeHeader.sgbSupport = _rom[0][0x143] == 0x03 ? "Yes" : "No";

		uint8_t oldLicenseeCodeByte = _rom[0][0x14B];
		if (oldLicenseeCodeByte == 0x33)
		{
			std::string newLicenseeStr;
			newLicenseeStr.push_back(static_cast<char>(_rom[0][0x144]));
			newLicenseeStr.push_back(static_cast<char>(_rom[0][0x145]));

			auto code = newLicenseeCodes.find(newLicenseeStr);
			_cartridgeHeader.licenseeCode = code == newLicenseeCodes.end() ? "Unknown" : code->second;
		}
		else
		{
			auto code = oldLicenseeCodes.find(oldLicenseeCodeByte);
			_cartridgeHeader.licenseeCode = code == oldLicenseeCodes.end() ? "Unknown" : code->second;
		}

		_cartridgeHeader.cartridgeType = cartridgeTypeStr(_rom[0][0x147]);
		_cartridgeHeader.romSize = romSizeStr(_rom[0][0x148]);
		_cartridgeHeader.ramSize = ramSizeStr(_rom[0][0x149]);
		_cartridgeHeader.isJapaneseCartridge = (_rom[0][0x14A] & 0x1);
		_cartridgeHeader.maskRomVersion = _rom[0][0x14C];
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