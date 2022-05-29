#include "mmu.h"

Mmu::Mmu(string file)
{
	
	//allocating memory region
	VRAM = new uint8_t[0x2000];
	ExtRAM = new uint8_t[0x2000];
	WorkingRAM = new uint8_t[0x4000];

	//erasing memory at boot
	for (int i = 0; i < 0x2000; i++)
		VRAM[i] = 0;
	for (int i = 0; i < 0x2000; i++)
		ExtRAM[i] = 0x0;
	for (int i = 0; i < 0x4000; i++)
		WorkingRAM[i] = 0x0;
	in_bios = true;
	
	cartridge = new Cartridge();
	
		//loading rom file via file
	cartridge->loadRom(file);
	// ROM = cartridge->getROM();

	//gamepad
	gamepad = new Gamepad();

	currModifiedTile = -1;
}

Mmu::~Mmu()
{
	delete VRAM, ExtRAM, WorkingRAM;
	in_bios = true;
}

void Mmu::disableBios()
{
	in_bios = false;
}

bool Mmu::isInBios()
{
	return in_bios;
}

uint8_t Mmu::read_ram(uint16_t adrr)
{
	if (adrr < 0x100)
	{
		//if the gameboy is in internal BIOS, in should return BIOS content
		if (in_bios)
			return Bios[adrr];
		//return ROM content otherwise
		else return cartridge->RomBankRead(adrr);
	}
	else if (adrr >= 0x100)
	{
		if (adrr < 0x8000)
		    return cartridge->RomBankRead(adrr);
		else if (adrr >= 0x8000 && adrr < 0xA000)
            return VRAM[adrr - 0x8000];
		else if (adrr >= 0xA000 && adrr < 0xC000)
		{
			return cartridge->RamBankRead(adrr);
		}
		else if (adrr >= 0xC000 && adrr <= 0xFFFF)
		{
			if (adrr == 0xFF00)
				return gamepad->getState();
			else return WorkingRAM[adrr - 0xC000];
		}
	}
}

void Mmu::write_ram(uint16_t adrr, uint8_t value)
{
	if (adrr < 0x8000)
		cartridge->handleRomMemory(adrr, value);
	else if (adrr >= 0x8000 && adrr < 0xA000)
	{
		if (adrr <= 0x97FF)
		{
			if (adrr == 0x9000)
			{
				int a = 0;
			}
			VRAM[adrr - 0x8000] = value;
			int MSB;
			if (((adrr & 0xF000)) == 0x8000)
				MSB = 0;
			else MSB = 1;
			int multiplier = ((adrr - 0x8000) & 0xF00) >> 8;
			int index = ((adrr - 0x8000) & 0xF0) >> 4;
			currModifiedTile = (index + multiplier * 16 + 256 * MSB);
		}
		else VRAM[adrr - 0x8000] = value;
	}
	else if (adrr >= 0xA000 && adrr < 0xC000)
	     cartridge->handleRamMemory(adrr, value);
	else if (adrr >= 0xC000 && adrr <= 0xFFFF)
	{
		if (adrr >= 0xE000 && adrr <= 0xFE00)
		{
			WorkingRAM[adrr - 0xC000] = value;
			WorkingRAM[adrr - 0x2000 - 0xC000] = value;
		}
		else if ( adrr >= 0xFEA0 && adrr <= 0xFEFF)
		    return;
	    else if (adrr == 0xFF00)
        {
			gamepad->setState(value);
        }
        else if (adrr == 0xFF02)		//only for Blargg Test roms debugging, TODO: implement serial transfert protocol
        {
            if (value == 0x81)
			{
                cout << WorkingRAM[0xFF01 - 0xC000];
			}
        }
		else if (adrr == 0xFF04)
              WorkingRAM[adrr - 0xC000] = 0;
        else if (adrr == 0xFF46)
        {
            DoDMATransfert(value);
            return;
        }
        else if (adrr == 0xFF44)
              WorkingRAM[adrr - 0xC000] = 0;
        else WorkingRAM[adrr - 0xC000] = value;
	}
}

uint8_t **Mmu::get_bg_array()
{
	uint8_t **matrix = new uint8_t *[256];
	for (int i = 0; i < 256; i++)
		matrix[i] = new uint8_t[256];
	uint16_t baseAdress;
	if (get_LCDC_BGTileMap())
		baseAdress = 0x9C00;
	else
		baseAdress = 0x9800;

	for (int i = 0; i < 1024; i++)
	{
		uint8_t currentTile = read_ram(baseAdress + i);
		uint16_t tileAdress;
		if (get_LCDC_BGWindowTile())
			tileAdress = 0x8000 + (currentTile * 0x10);
		else
		{
			if (currentTile <= 127)
				tileAdress = 0x9000 + (currentTile * 0x10);
			else
				tileAdress = 0x8800 + (currentTile * 0x10);
		}
		for (int j = 0; j < 8; j++)
		{
			for (int k = 0; k < 8; k++)
			{
				int x = 0, y = 0;
				uint8_t pixel = (read_ram(tileAdress + (j * 2)) >> (7 - k)) & 1;
				pixel = pixel << 1;
				pixel = pixel | ((read_ram(tileAdress + 1 + (j * 2)) >> (7 - k)) & 1);
				matrix[j + ((i / 32) * 8)][k + ((i % 32) * 8)] = get_paletteColor(pixel);
			}
		}
	}
	return matrix;
}

bool Mmu::get_LCDC_BGTileMap()
{
	return (read_ram(0xFF40) & 0b00001000) == 0b00001000;
}

bool Mmu::get_LCDC_BGWindowTile()
{
	return (read_ram(0xFF40) & 0b00010000) == 0b00010000;
}

uint8_t Mmu::get_paletteColor(uint8_t index)
{
	uint8_t palette[4] = { 0 };
	palette[0] = read_ram(0xFF47) & 0b00000011;
	palette[1] = (read_ram(0xFF47) & 0b0000110) >> 2;
	palette[2] = (read_ram(0xFF47) & 0b00110000) >> 4;
	palette[3] = (read_ram(0xFF47) & 0b11000000) >> 6;
	return palette[index];
}

void Mmu::directSet(uint16_t adrr, uint8_t value)
{
	WorkingRAM[adrr - 0xC000] = value;
}


void Mmu::DoDMATransfert(uint8_t value)
{
	uint16_t data = (value << 8);
	for (int i=0; i<0xA0; i++)
	{
		uint8_t tmp = read_ram(data + i);
		directSet(0xFE00 + i, tmp);
	}
}

uint8_t Mmu::get_0xFF00()
{
	return WorkingRAM[0xFF00 - 0xC000];
}

void Mmu::setOut(uint8_t value)
{
	out = value;
}
uint8_t Mmu::getOut()
{
	return out;
}

uint16_t Mmu::getNext2Bytes(uint16_t adress)
{
	return (read_ram(adress + 2) << 8) | read_ram(adress + 1);
}

uint8_t *Mmu::getVRAM()
{
	return VRAM;
}

Gamepad *Mmu::getGamepad()
{
	return gamepad;
}

bool Mmu::isVramWritten()
{
	return vramWritten;
}

void Mmu::setVramWriteStatus(bool value)
{
	vramWritten = value;
}

vector<int> &Mmu::getModifiedTiles()
{
	return modifiedTiles;
}