#include "register.h"

Register::Register()
{
    registerValue = 0;
}

Register::~Register()
{

}

void Register::set(uint16_t value)
{
    registerValue = value;
}

uint16_t Register::get()
{
    return registerValue;
}

uint8_t Register::getRigthRegister()
{
    return ((uint8_t)(registerValue & 0xFF));
}

uint8_t Register::getLeftRegister()
{
    return ((uint8_t)(registerValue >> 8));
}

void Register::setLeftRegister(uint8_t value)
{
    registerValue = ((value << 8) | (registerValue & 0x00FF));
}

void Register::setRigthRegister(uint8_t value)
{
    registerValue = ((registerValue & 0xFF00) | value);
}

void Register::show()
{
    std::cout << std::hex << registerValue << endl;
}

SpecialRegister::SpecialRegister() : Register()
{

}

SpecialRegister::~SpecialRegister() 
{

}

bool SpecialRegister::getFlag(char Reg)
{
    bool value;
    switch (Reg)
    {
        case 'Z':  value = getRigthRegister() & (1 << 7); break;
        case 'N':  value = getRigthRegister() & (1 << 6); break;
        case 'H':  value = getRigthRegister() & (1 << 5); break;
        case 'C':  value = getRigthRegister() & (1 << 4); break;
		default: cout << "Flag error."; exit(0); break;
    }
	return value;
}

void SpecialRegister::setFlag(char Reg)
{
    uint8_t bit = 0;
    switch (Reg)
    {
        case 'Z':  bit = 0x80 ; break;
        case 'N':  bit = 0x40; break;
        case 'H':  bit = 0x20; break;
        case 'C':  bit = 0x10; break;
		default: cout << "Flag error."; exit(0); break;
    }
    bit |= getRigthRegister();
    setRigthRegister(bit);
}

void SpecialRegister::clearFlag(char Reg)
{
    int bit = 0;
    switch (Reg)
    {
        case 'Z':  bit = (~0x80) ; break;
        case 'N':  bit = (~0x40); break;
        case 'H':  bit = (~0x20); break;
        case 'C':  bit = (~0x10); break;
		default: cout << "Flag error."; exit(0); break;
    }
    bit = getRigthRegister() & bit;
    setRigthRegister(bit);
}

