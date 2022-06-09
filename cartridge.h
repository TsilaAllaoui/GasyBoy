#pragma once

#include <iostream>
#include <fstream>
#include <cstring>
#include <cmath>

using namespace std;

class Cartridge
{
  private:

	//cartridge type
	  int cartridgeType;
	
	//ROM memory
	uint8_t **ROM;
	
	//the current ROM bank
	uint8_t currRomBank;
	
	//RAM banks
	uint8_t *RAMBanks;
	
	//the current RAM Bank
	uint8_t currRamBank;
	
	//for checking if RAM is writable
	bool enabledRAM;
	
	//MBC mode
	bool mode;

	//number of banks of the ROM
	int banksNumber;

	//MBC type
	string cartTypes[4] = { "No MBC", "MBC1", "MBC1+ExternalRAM", "MBC1+Battery" };
	
  public:

	// constructor/destructor
	Cartridge();
	~Cartridge();

	// loading ROM 
	void loadRom(string filename);
	
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