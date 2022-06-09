#include "cartridge.h"


Cartridge::Cartridge()
{
	RAMBanks = new uint8_t[0x8000];
	std::memset(RAMBanks, 0, 0x8000);

	MBC1 = MBC2 = false;

	currRomBank = 1;
	currRamBank = 0;
	
	romBanking = true;
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
		ROM = new uint8_t*[(int)banksNumber];
		for (int i=0; i<(int)banksNumber; i++)
			ROM[i] = new uint8_t[0x4000];
		for (int i=0; i<(int)banksNumber; i++)
			for (int j=0; j<0x4000; j++)
				ROM[i][j] = 0;
	
		//reading file and put into ROM
		Rom.seekg(0, std::ios::end);
		long romSize = Rom.tellg();
 		uint8_t *buff = new uint8_t[romSize];
		Rom.seekg(0, std::ios::beg);
		// Rom.read((char*)buff, romSize);
		int bank  = 0, j = 0;
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
		std::ofstream out("C:/Users/Allaoui/Desktop/GasyBoy/Roms/NoBanks/CartHeader.txt", std::ios::out);
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
	}
	else
	{
		cout << "Rom not found!";
		exit(1);
	}
}

void Cartridge::setMBCType(uint8_t value)
{
	switch (value)
	{
	case 0:
	case 1:
	case 2:
		MBC1 = true;
		break;
	case 3:
	case 4:
		MBC2 = true;
		break;
	default:
		break;
	}
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
	return RAMBanks[adrr - 0xA000 + currRamBank * 0x2000];
}

bool Cartridge::isRamWriteEnabled()
{
	return enabledRAM;
}

void Cartridge::handleRomMemory(uint16_t adrr, uint8_t value)
{
	if (adrr < 0x2000)
	{
		if (MBC1 || MBC2)
		{
			if (MBC2)
			{
				if ((adrr & 0x10) == 0x10)
					return;
			}
			value &= 0xF;
			if (value == 0xA)
				enabledRAM = true;
			else if (value == 0)
				enabledRAM = false;
		}
	}
	else if (adrr >= 0x2000 && adrr < 0x4000)
	{
		if (MBC1 || MBC2)
		{
			if (MBC2)
			{
				currRomBank = value & 0xF;
				if (currRomBank == 0)
					currRomBank++;
			}
			uint8_t lower5bits = value & 0x1F;
			currRomBank &= 0xE0;
			currRomBank |= lower5bits;
			if (currRomBank == 0)
				currRomBank++;
		}  
	}
	else if (adrr >= 0x4000 && adrr < 0x6000)
	{
		if (MBC1)
		{
			if (romBanking)
			{
				currRomBank &= 0x1F;
				currRomBank &= 0xE0;
				currRomBank |= value;
				if (currRomBank == 0)
					currRomBank++;
			}
			else currRamBank = value & 0x3;
		}
	}
	else if (adrr >= 0x6000 && adrr < 0x8000)
	{
		if (MBC1)
		{
			romBanking = ((value & 0x1) == 0) ? true : false;
			if (romBanking)	currRamBank = 0;
		}
	}
}

void Cartridge::handleRamMemory(uint16_t adrr, uint8_t value)
{
	if (enabledRAM)
	{
		RAMBanks[adrr - 0xA000 + currRamBank * 0x2000] = value;
	}
}
