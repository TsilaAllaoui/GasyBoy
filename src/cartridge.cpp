#include <iostream>
#include <bitset>
#include <cmath>
#include "gbException.h"
#include "cartridge.h"
#include "logger.h"
#include "utils.h"
#include "defs.h"
#include "mbc.h"

namespace gasyboy
{

	Cartridge::Cartridge()
		: _cartridgeType(CartridgeType::ROM_ONLY)
	{
	}

	Cartridge &Cartridge::operator=(const Cartridge &other)
	{
		_rom = other._rom;
		_ram = other._ram;
		_cartridgeType = other._cartridgeType;
		_cartridgeHeader = other._cartridgeHeader;
		if (other._mbc)
		{
			_mbc.reset(other._mbc.get());
		}
		else
		{
			_mbc.reset();
		}
		return *this;
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

		uint8_t *buffer = new uint8_t[size];

		if (!file.read(reinterpret_cast<char *>(buffer), size))
		{
			throw std::runtime_error("Failed to read ROM file.");
		}

		// Setting up ROM
		_rom = std::vector<uint8_t>(buffer, buffer + size);

		// Setting up RAM
		_ram = std::vector<uint8_t>(getRamBanksCount() * 0x2000, 0);

		// Setting up MBC
		setMBC(_rom[0x147]);

		// Get cartridge header infos
		getCartridgeHeaderInfos();

		// Log cartridge informations
		logCartridgeHeaderInfos();
	}

	void Cartridge::loadRomFromByteArray(const size_t &size, uint8_t *mem)
	{
		// Setting up ROM
		_rom = std::vector<uint8_t>(mem, mem + size);

		// Setting up RAM
		_ram = std::vector<uint8_t>(getRamBanksCount() * 0x2000, 0);

		// Setting up MBC
		setMBC(_rom[0x147]);

		// Get cartridge header infos
		getCartridgeHeaderInfos();

		// Log cartridge informations
		logCartridgeHeaderInfos();
	}

	const std::vector<uint8_t> &Cartridge::getRom()
	{
		return _rom;
	}

	const std::vector<uint8_t> &Cartridge::getRam()
	{
		return _ram;
	}

	void Cartridge::setMBC(const uint8_t &value)
	{
		_cartridgeType = utils::uint8ToCartridgeType(value);

		int ramBanksCount = getRamBanksCount();

		int romBanksCount = _rom.size() / 0x4000;

		switch (value)
		{
		case 0x00:
		case 0x08:
		case 0x09:
			_mbc = std::make_unique<MBC0>(_rom);
			break;
		case 0x01:
		case 0x02:
		case 0x03:
			_mbc = std::make_unique<MBC1>(_rom, _ram, romBanksCount, ramBanksCount);
			break;
		case 0x05:
		case 0x06:
			_mbc = std::make_unique<MBC2>(_rom, _ram, romBanksCount, ramBanksCount);
			break;
		case 0x0F:
		case 0x10:
		case 0x11:
		case 0x12:
		case 0x13:
			_mbc = std::make_unique<MBC3>(_rom, _ram, romBanksCount, ramBanksCount);
			break;
		case 0x19:
		case 0x1A:
		case 0x1B:
		case 0x1C:
		case 0x1D:
		case 0x1E:
			_mbc = std::make_unique<MBC5>(_rom, _ram, romBanksCount, ramBanksCount);
			break;
		default:
			std::stringstream ss;
			ss << "\nIncorrect MBC type: " << std::hex << (int)value << "\n";
			throw exception::GbException(ss.str());
		}
	}

	uint8_t Cartridge::mbcRomRead(const uint16_t &addr)
	{
		return _mbc->readByte(addr);
	}

	void Cartridge::mbcRomWrite(const uint16_t &addr, const uint8_t &value)
	{
		_mbc->writeByte(addr, value);
	}

	uint8_t Cartridge::mbcRamRead(const uint16_t &addr)
	{
		return _mbc->readByte(addr);
	}

	void Cartridge::mbcRamWrite(const uint16_t &addr, const uint8_t &value)
	{
		_mbc->writeByte(addr, value);
	}

	void Cartridge::setRom(const std::vector<uint8_t> &rom)
	{
		_rom = rom;
	}

	void Cartridge::setRam(const std::vector<uint8_t> &ram)
	{
		_ram = ram;
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
		_rom.clear();
		_ram.clear();
		_cartridgeType = CartridgeType::ROM_ONLY;
		_cartridgeHeader = CartridgeHeader();
	}

	void Cartridge::getCartridgeHeaderInfos()
	{
		for (int i = 0x134; i < 0x143; i++)
		{
			uint8_t byte = static_cast<char>(_rom[i]);
			if (byte != 0)
			{
				_cartridgeHeader.name += byte;
			}

			if (i >= 0x13F && i < 0x143 && byte != 0)
			{
				_cartridgeHeader.manufacturer += byte;
			}
		}

		uint8_t cgbSupportByte = _rom[0x143];
		_cartridgeHeader.cgbSupport =
			cgbSupportByte == 0xC0 ? "Yes (No DMG support)" : (cgbSupportByte == 0x80 ? "Yes (DMG support)" : "No");

		_cartridgeHeader.sgbSupport = _rom[0x143] == 0x03 ? "Yes" : "No";

		uint8_t oldLicenseeCodeByte = _rom[0x14B];
		if (oldLicenseeCodeByte == 0x33)
		{
			std::string newLicenseeStr;
			newLicenseeStr.push_back(static_cast<char>(_rom[0x144]));
			newLicenseeStr.push_back(static_cast<char>(_rom[0x145]));

			auto code = newLicenseeCodes.find(newLicenseeStr);
			_cartridgeHeader.licenseeCode = code == newLicenseeCodes.end() ? "Unknown" : code->second;
		}
		else
		{
			auto code = oldLicenseeCodes.find(oldLicenseeCodeByte);
			_cartridgeHeader.licenseeCode = code == oldLicenseeCodes.end() ? "Unknown" : code->second;
		}

		_cartridgeHeader.cartridgeType = cartridgeTypeStr(_rom[0x147]);
		_cartridgeHeader.romSize = romSizeStr(_rom[0x148]);
		_cartridgeHeader.ramSize = ramSizeStr(_rom[0x149]);
		_cartridgeHeader.isJapaneseCartridge = (_rom[0x14A] & 0x1);
		_cartridgeHeader.maskRomVersion = _rom[0x14C];
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

	int Cartridge::getRamBanksCount()
	{
		uint8_t type = _rom[0x149];

		switch (type)
		{
		case 0x00:
			return 0;
			break;
		case 0x01:
			return 0;
			break;
		case 0x02:
			return 1;
			break;
		case 0x03:
			return 4;
			break;
		case 0x04:
			return 16;
			break;
		case 0x05:
			return 8;
			break;
		default:
			std::stringstream ss;
			ss << "\nIncorrect RAM type: " << std::hex << (int)type << "\n";
			throw exception::GbException(ss.str());
		}
	}
}