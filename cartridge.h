#pragma once

#include <iostream>
#include <fstream>

using namespace std;

class Cartridge
{
  private:
  
	// MBC Chip type
	bool MBC1, MBC2;
	
	//ROM memory
	uint8_t *ROM;
	
	//the current ROM bank
	uint8_t currRomBank;
	
	//RAM banks
	uint8_t *RAMBanks;
	
	//the current RAM Bank
	uint8_t currRamBank;
	
	//for checking if RAM is writable
	bool enabledRAM;
	
	//for checking if a Rom banking change is occuring
	bool romBanking;

	//number of banks of the ROM
	int banksNumber;
	
  public:

	// constructor/destructor
	Cartridge();
	~Cartridge();

	// loading ROM 
	void loadRom(string filename);
	
	//getting the rom
	uint8_t *getROM();
	
	//set MBC type
	void setMBCType(uint8_t value);
	
	//set rom banks number
	void setBankNumber(uint8_t value);

	//getters and setters
	uint8_t getCurrRomBanks();
	uint8_t getCurrRamBanks();
	bool isRamWriteEnabled();
	
	//ROM Bank reading
	uint8_t RomBankRead(uint16_t adrr);
	
	//RAM Bank reading
	uint8_t RamBankRead(uint16_t adrr);
	
	//to handle all bank changes
	void handleRomMemory(uint16_t adrr, uint8_t value);
	void handleRamMemory(uint16_t adrr, uint8_t value);

};