#include "cartridge.h"
#include "logger.h"
#include "utils.h"
#include <iostream>
#include <bitset>

namespace gasyboy
{

	Cartridge::Cartridge()
	{
		// allocating banks of External RAM
		_ramBanks = std::vector<uint8_t>(0x8000, 0);

		// setting current ROM Bank && RAM Bank (ROM usually start at 1)
		_currRomBank = 1;
		_currRamBank = 0;

		// mode for MBC1
		_mode = true;

		// the cartridge type
		_cartridgeType = CartridgeType::ROM_ONLY;

		// setting current used RTC register
		_currRTCReg = 0;
	}

	Cartridge::~Cartridge()
	{
	}

	void Cartridge::loadRom(const std::string &filePath)
	{
		std::ifstream rom(filePath.c_str(), std::ios::in | std::ios::binary | std::ios::ate);
		if (rom.is_open())
		{
			// reading rom file
			rom.seekg(0x147, std::ios::beg);
			char buffer[2];
			rom.read(buffer, 2);

			// setting cartridge type
			setMBCType(buffer[0]);

			// setting bank number
			setBankNumber(buffer[1]);

			// loading rom
			_rom = std::vector<std::vector<uint8_t>>(_banksNumber, std::vector<uint8_t>(0x4000, 0));

			rom.seekg(0, std::ios::end);
			long romSize = rom.tellg();
			uint8_t *buff = new uint8_t[romSize];
			rom.seekg(0, std::ios::beg);

			int bank = 0, j = 0;
			for (int i = 0; i < romSize; i++)
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

			cartridgeHeaderInfo << "Cartridge _rom Name:  ";
			for (int i = 0x134; i < 0x143; i++)
				cartridgeHeaderInfo << _rom[0][i];
			cartridgeHeaderInfo << std::endl
								<< "Manufacturer:  ";
			for (int i = 0x13F; i < 0x142; i++)
				cartridgeHeaderInfo << _rom[0][i];
			cartridgeHeaderInfo << std::endl
								<< "CGB Support:  ";
			if (_rom[0][0x143] == 0x80)
				cartridgeHeaderInfo << "Yes (DMG support)" << std::endl;
			else if (_rom[0][0x143] == 0xC0)
				cartridgeHeaderInfo << "Yes (No DMG support)" << std::endl;
			else
				cartridgeHeaderInfo << "No" << std::endl;
			cartridgeHeaderInfo << "License Code:  " << _rom[0][0x144] << _rom[0][0x145] << std::endl;
			cartridgeHeaderInfo << "SGB Support:  ";
			if (_rom[0][0x143] == 0x03)
				cartridgeHeaderInfo << "Yes" << std::endl;
			else if (_rom[0][0x143] == 0)
				cartridgeHeaderInfo << "No" << std::endl;
			cartridgeHeaderInfo << "Cartridge Type:  " << std::hex <<  (int)_rom[0][0x147]] << std::endl;
			cartridgeHeaderInfo << "_rom Size:  " << std::hex << (int)_rom[0][0x148] << std::endl;
			cartridgeHeaderInfo << "RAM Size:  " << std::hex << (int)_rom[0][0x149] << std::endl;
			cartridgeHeaderInfo << "Japanese:  " << ((_rom[0][0x14A] & 0x1) ? "No" : "Yes") << std::endl;
			cartridgeHeaderInfo << "Old License Code:  " << std::hex << (int)_rom[0][0x14B] << std::endl;
			cartridgeHeaderInfo << "Mask _rom Version:  " << std::hex << (int)_rom[0][0x14C] << std::endl;
		}
		else
		{
			utils::Logger::getInstance()->log("Rom: \"" + filePath + "\" not found!");
			exit(1);
		}
	}

	void Cartridge::setMBCType(const uint8_t &value)
	{
		_cartridgeType = uint8ToCartridgeType(value);
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
			_banksNumber = pow(72, 3);
			break;
		case 0x53:
			_banksNumber = pow(80, 3);
			break;
		case 0x54:
			_banksNumber = pow(96, 3);
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

	uint8_t Cartridge::romBankRead(uint16_t adrr)
	{
		if (adrr >= 0 && adrr <= 0x3FFF)
			return _rom[0][adrr];
		else if (adrr >= 0x4000 && adrr < 0x8000)
			return _rom[(int)_currRomBank][adrr - 0x4000];
		else
			exit(45);
	}

	uint8_t Cartridge::ramBankRead(uint16_t adrr)
	{
		// if there is RTC
		if (_cartridgeType == CartridgeType::MBC3_RAM_BATT)
			return _currRTCReg;
		// else
		else
			return _ramBanks[adrr - 0xA000 + _currRamBank * 0x2000];
	}

	bool Cartridge::isRamWriteEnabled()
	{
		return _enabledRAM;
	}

	void Cartridge::handleRomMemory(uint16_t adrr, uint8_t value)
	{
		// MBC1 External RAM Switch
		if (adrr < 0x2000)
		{
			if (_cartridgeType == 2 || _cartridgeType == 3)
			{
				uint8_t byte = value & 0xF;
				(byte == 0xA) ? enabledRAM = true : enabledRAM = false;
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
			else if (_cartridgeType == 0x13)
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
					uint8_t upperBits = value & 0xE0;
					_currRomBank |= value;
					if (_currRomBank == 0)
						_currRomBank = 1;
				}

				// RAM mode: Set Bank
				else
					_currRamBank = value & 0x3;
			}
			// MBC3
			else if (_cartridgeType == 0x13)
			{
				// for RAM banking
				if (value >= 0 && value <= 3)
					_currRamBank = value & 0x3;

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
			if (_cartridgeType == 2 || _cartridgeType == 3)
			{
				_mode = (value & 0x1);
			}
			// MBC3 RTC registers
			else if (_cartridgeType == 0x13)
			{
				// latching RTC register
			}
		}
	}

	void Cartridge::handleRamMemory(uint16_t adrr, uint8_t value)
	{
		if (_enabledRAM)
			_ramBanks[adrr - 0xA000 + _currRamBank * 0x2000] = value;
	}
}