#include "cartridge.h"


Cartridge::Cartridge()
{
	//allocating banks of External RAM
	RAMBanks = new uint8_t[0x8000];
	std::memset(RAMBanks, 0, 0x8000);

	//setting current ROM Bank and RAM Bank (ROM usually start at 1)
	currRomBank = 1;
	currRamBank = 0;
	
	//mode for MBC1
	mode = true;

	//the cartridge type
	cartridgeType = 0;

	//setting current used RTC register
	currRTCReg = 0;
}

Cartridge::~Cartridge()
{
	delete[] ROM;
}

void Cartridge::loadRom(string file)
{
	std::ifstream Rom(file.c_str(), std::ios::in | std::ios::binary | std::ios::ate);
	if (Rom.is_open())
	{
		// //allocating ROM
		Rom.seekg(0x147, std::ios::beg);
		char buffer[2];
		Rom.read(buffer, 2);
		setBankNumber(buffer[1]);
		setMBCType(buffer[0]);
		ROM = new uint8_t * [(int)banksNumber];
		for (int i = 0; i < (int)banksNumber; i++)
			ROM[i] = new uint8_t[0x4000];
		for (int i = 0; i < (int)banksNumber; i++)
			for (int j = 0; j < 0x4000; j++)
				ROM[i][j] = 0;

		//reading file and put into ROM
		Rom.seekg(0, std::ios::end);
		long romSize = Rom.tellg();
		uint8_t * buff = new uint8_t[romSize];
		Rom.seekg(0, std::ios::beg);
		// Rom.read((char*)buff, romSize);
		int bank = 0, j = 0;
		for (int i = 0; i < romSize; i++)
		{
			if (i % 0x4000 == 0 && j > 0)
			{
				bank++;
				j = 0;
			}
			Rom.seekg(i, std::ios::beg);
			char c;
			Rom.read(&c, 1);
			ROM[bank][j] = (uint8_t)c;
			j++;
		}
		Rom.close();
	}
		/*std::ofstream out("./CartHeader.txt", std::ios::out);
		if (out.is_open())
		{
			out << "Cartridge ROM Name:  ";
			for (int i = 0x134; i < 0x143; i++)
				out << ROM[0][i];
			out << endl << "Manufacturer:  ";
			for (int i = 0x13F; i < 0x142; i++)
				out << ROM[0][i];
			out << endl << "CGB Support:  ";
			if (ROM[0][0x143] == 0x80)
				out << "Yes (DMG support)" << endl;
			else if (ROM[0][0x143] == 0xC0)
				out << "Yes (No DMG support)" << endl;
			else
				out << "No" << endl;
			out << "License Code:  " << ROM[0][0x144] << ROM[0][0x145] << endl;
			out << "SGB Support:  ";
			if (ROM[0][0x143] == 0x03)
				out << "Yes" << endl;
			else if (ROM[0][0x143] == 0)
				out << "No" << endl;
			out << "Cartridge Type:  " << hex << cartTypes[(int)ROM[0][0x147]]<< endl;
			out << "ROM Size:  " << hex << (int)ROM[0][0x148] << endl;
			out << "RAM Size:  " << hex << (int)ROM[0][0x149] << endl;
			out << "Japanese:  " << ((ROM[0][0x14A] & 0x1) ? "No" : "Yes") << endl;
			out << "Old License Code:  " << hex << (int)ROM[0][0x14B] << endl;
			out << "Mask ROM Version:  " << hex << (int)ROM[0][0x14C] << endl;
			out.close();
		}
		else
			exit(80);
	}*/
	else
	{
		cout << "Rom not found!";
		exit(1);
	}
}

void Cartridge::setMBCType(uint8_t value)
{
	cartridgeType = (int)value;
}

void Cartridge::setBankNumber(uint8_t value)
{
	switch (value)
	{
	case 0: banksNumber = 2; break;
	case 0x1: banksNumber = 4; break;
	case 0x2: banksNumber = 8; break;
	case 0x3: banksNumber = 16; break;
	case 0x4: banksNumber = 32; break;
	case 0x5: banksNumber = 64; break;
	case 0x6: banksNumber = 128; break;
	case 0x7: banksNumber = 256; break;
	case 0x8: banksNumber = 512; break;
	case 0x52: banksNumber = pow(72, 3); break;
	case 0x53: banksNumber = pow(80, 3); break;
	case 0x54: banksNumber = pow(96, 3); break;
	}
}

uint8_t Cartridge::getCurrRomBanks()
{
	return currRomBank;
}

uint8_t Cartridge::getCurrRamBanks()
{
	return currRamBank;
}

uint8_t Cartridge::RomBankRead(uint16_t adrr)
{
	if (adrr >= 0 && adrr <= 0x3FFF)
		return ROM[0][adrr];
	else if (adrr >= 0x4000 && adrr < 0x8000)
		return ROM[(int)currRomBank][adrr - 0x4000];
	else exit(45);
}

uint8_t Cartridge::RamBankRead(uint16_t adrr)
{
	//if there is RTC
	if (cartridgeType == 0x13)
		return currRTCReg;
	//else
	else return RAMBanks[adrr - 0xA000 + currRamBank * 0x2000];
}

bool Cartridge::isRamWriteEnabled()
{
	return enabledRAM;
}

void Cartridge::handleRomMemory(uint16_t adrr, uint8_t value)
{
	//MBC1 External RAM Switch
	if (adrr < 0x2000)
	{
		if (cartridgeType == 2 or cartridgeType == 3)
		{
			uint8_t byte = value & 0xF;
			(byte == 0xA) ? enabledRAM = true : enabledRAM = false;
		}
	}

	//ROM Bank
	else if (adrr >= 0x2000 && adrr < 0x4000)
	{
		//MBC1
		if (cartridgeType == 1 or cartridgeType == 2 or cartridgeType == 3)
		{
			uint8_t lower5bits = value & 0x1F;
			currRomBank &= 0xE0;
			currRomBank |= lower5bits;
			if (currRomBank == 0)
				currRomBank = 1;
		}  
		//MBC3
		else if (cartridgeType == 0x13)
		{
			//MessageBox(0, "Rom/Ram Bank Change", "Bank Handler", MB_ICONINFORMATION | MB_OK);
			uint8_t lower7bits = value & 0x7F;
			currRomBank &= 0x80;
			currRomBank |= lower7bits;
			if (currRomBank == 0)
				currRomBank = 1;
		}
	}

	//MBC RAM Bank/RTC
	else if (adrr >= 0x4000 && adrr < 0x6000)
	{
		//MBC1
		if (cartridgeType == 1 or cartridgeType == 2 or cartridgeType == 3)
		{
			if (!mode)
			{
				//ROM mode: Set high bits of bank
				currRomBank &= 0x1F;
				uint8_t upperBits = value & 0xE0;
				currRomBank |= value;
				if (currRomBank == 0)
					currRomBank = 1;
			}

			//RAM mode: Set Bank
			else currRamBank = value & 0x3;
		}
		//MBC3
		else if (cartridgeType == 0x13)
		{
			//for RAM banking
			if (value >= 0 and value <= 3)
				currRamBank = value & 0x3;

			//for RTC register read/write
			else if (value >= 8 and value <= 0xC)
			{
				switch (value)
				{
				case 0x8: currRTCReg = RTCS; break;
				case 0x9: currRTCReg = RTCM; break;
				case 0xA: currRTCReg = RTCH; break;
				case 0xB: currRTCReg = RTCDL; break;
				case 0xC: currRTCReg = RTCDH; break;
				default:
					break;
				}
			}
		}
	}

	//MBC1: Mode switch
	else if (adrr >= 0x6000 && adrr < 0x8000)
	{
		//Only for MBC1
		if (cartridgeType == 2 or cartridgeType == 3)
		{
			mode = (value & 0x1);
		}
		//MBC3 RTC registers
		else if (cartridgeType == 0x13)
		{
			//latching RTC register
		}
	}
}

void Cartridge::handleRamMemory(uint16_t adrr, uint8_t value)
{
	if (enabledRAM)
		RAMBanks[adrr - 0xA000 + currRamBank * 0x2000] = value;
}
