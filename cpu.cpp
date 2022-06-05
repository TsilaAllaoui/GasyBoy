#include "cpu.h"

Cpu::Cpu(bool bootBios, Mmu *p_mmu)
{
	mmu = p_mmu;
	if (bootBios)
	{
		AF.set(0);
		BC.set(0);
		DE.set(0);
		HL.set(0);
		PC = 0;
		SP = 0;
	}
	else
	{
		AF.set(0x01B0);
		BC.set(0x0013);
		DE.set(0x00D8);
		HL.set(0x014D);
		PC = 0x100;
		SP = 0xFFFE;
		mmu->disableBios();
	}

	halted = false;
	interrupt_enabled = false;
	currentOpcode = 0;
	cycle = 0;
}

Cpu::~Cpu()
{
	mmu->~Mmu();
}

void Cpu::set_PC(uint16_t adress)
{
	PC = adress;
}

uint16_t Cpu::get_PC()
{
	return PC;
}

uint16_t Cpu::get_SP()
{
	return SP;
}

uint16_t Cpu::getRegister(string reg)
{
	uint16_t value = 0;
	if (reg == "AF")
		return AF.get();
	else if (reg == "BC")
		return BC.get();
	else if (reg == "DE")
		return DE.get();
	else if (reg == "HL")
		return HL.get();
	else if (reg == "SP")
		return SP;
	else
		exit(4);
}

void Cpu::setHaltedStatus(bool value)
{
	halted = value;
}

bool Cpu::getHaltedStatus()
{
	return halted;
}

void Cpu::push_SP(uint16_t adress)
{
	uint8_t firstByte = (uint8_t)(PC >> 8);
	uint8_t secondByte = (uint8_t)(PC & 0xFF);
	SP--;
	mmu->write_ram(SP, firstByte);
	SP--;
	mmu->write_ram(SP, secondByte);
}

void Cpu::LD_HL_SP_n()
{
	int8_t value = (int8_t)mmu->read_ram(PC + 1);
	uint16_t result = (SP + value);
	(((SP ^ value ^ (result & 0xFFFF)) & 0x10) == 0x10) ? AF.setFlag('H') : AF.clearFlag('H');
	(((SP ^ value ^ (result & 0xFFFF)) & 0x100) == 0x100) ? AF.setFlag('C') : AF.clearFlag('C');
	HL.set(result);
	AF.clearFlag('Z');
	AF.clearFlag('N');
}

void Cpu::LD_r_r(char from, char to)
{
	uint8_t regFrom = 0;
	switch (from)
	{
	case 'A':
		regFrom = AF.getLeftRegister();
		break;
	case 'B':
		regFrom = BC.getLeftRegister();
		break;
	case 'C':
		regFrom = BC.getRigthRegister();
		break;
	case 'D':
		regFrom = DE.getLeftRegister();
		break;
	case 'E':
		regFrom = DE.getRigthRegister();
		break;
	case 'H':
		regFrom = HL.getLeftRegister();
		break;
	case 'L':
		regFrom = HL.getRigthRegister();
		break;
	default:
		cout << "Flag error.";
		exit(0);
		break;
	}
	switch (to)
	{
	case 'A':
		AF.setLeftRegister(regFrom);
		break;
	case 'B':
		BC.setLeftRegister(regFrom);
		break;
	case 'C':
		BC.setRigthRegister(regFrom);
		break;
	case 'D':
		DE.setLeftRegister(regFrom);
		break;
	case 'E':
		DE.setRigthRegister(regFrom);
		break;
	case 'H':
		HL.setLeftRegister(regFrom);
		break;
	case 'L':
		HL.setRigthRegister(regFrom);
		break;
	default:
		cout << "Flag error.";
		exit(0);
		break;
	}
}

void Cpu::LD_r_n(uint8_t from, char to)
{
	switch (to)
	{
	case 'A':
		AF.setLeftRegister(from);
		break;
	case 'F':
		AF.setRigthRegister(from);
		break;
	case 'B':
		BC.setLeftRegister(from);
		break;
	case 'C':
		BC.setRigthRegister(from);
		break;
	case 'D':
		DE.setLeftRegister(from);
		break;
	case 'E':
		DE.setRigthRegister(from);
		break;
	case 'H':
		HL.setLeftRegister(from);
		break;
	case 'L':
		HL.setRigthRegister(from);
		break;
	default:
		cout << "Flag error.";
		exit(0);
		break;
	}
}

void Cpu::LD_r_16(uint16_t adress, char to)
{
	uint8_t from = mmu->read_ram(adress);
	switch (to)
	{
	case 'A':
		AF.setLeftRegister(from);
		break;
	case 'B':
		BC.setLeftRegister(from);
		break;
	case 'C':
		BC.setRigthRegister(from);
		break;
	case 'D':
		DE.setLeftRegister(from);
		break;
	case 'E':
		DE.setRigthRegister(from);
		break;
	case 'H':
		HL.setLeftRegister(from);
		break;
	case 'L':
		HL.setRigthRegister(from);
		break;
	default:
		cout << "Flag error.";
		exit(0);
		break;
	}
}

void Cpu::LD_16_r(uint16_t adress, char from)
{
	uint8_t value = 0;
	switch (from)
	{
	case 'A':
		value = AF.getLeftRegister();
		break;
	case 'B':
		value = BC.getLeftRegister();
		break;
	case 'C':
		value = BC.getRigthRegister();
		break;
	case 'D':
		value = DE.getLeftRegister();
		break;
	case 'E':
		value = DE.getRigthRegister();
		break;
	case 'H':
		value = HL.getLeftRegister();
		break;
	case 'L':
		value = HL.getRigthRegister();
		break;
	default:
		cout << "Flag error.";
		exit(0);
		break;
	}
	mmu->write_ram(adress, value);
}

void Cpu::LD_16_n(uint16_t adress, uint8_t value)
{
	mmu->write_ram(adress, value);
}

void Cpu::LD_rr_nn(uint16_t value, string reg)
{
	if (reg == "AF")
		AF.set(value);
	else if (reg == "BC")
		BC.set(value);
	else if (reg == "DE")
		DE.set(value);
	else if (reg == "HL")
		HL.set(value);
	else if (reg == "SP")
		SP = value;
	else
		exit(2);
}

void Cpu::LD_rr_16(uint16_t adress, string reg)
{
	uint8_t leftValue = mmu->read_ram(adress + 1);
	uint8_t rightValue = mmu->read_ram(adress);
	uint16_t value = ((uint16_t)(leftValue << 8) | rightValue);
	if (reg == "AF")
		AF.set(value);
	else if (reg == "BC")
		BC.set(value);
	else if (reg == "DE")
		DE.set(value);
	else if (reg == "HL")
		HL.set(value);
	else if (reg == "SP")
		SP = value;
	else
		exit(2);
}

void Cpu::LD_16_rr(uint16_t adress, string reg)
{
	uint16_t value;
	if (reg == "AF")
		value = AF.get();
	else if (reg == "BC")
		value = BC.get();
	else if (reg == "DE")
		value = DE.get();
	else if (reg == "HL")
		value = HL.get();
	else if (reg == "SP")
		value = SP;
	else
		exit(2);
	uint8_t firstByte = (uint8_t)(value & 0xFF);
	uint8_t secondByte = (uint8_t)((value & 0xFF00) >> 8);
	mmu->write_ram(adress, firstByte);
	mmu->write_ram(adress + 1, secondByte);
}

void Cpu::LD_SP_HL()
{
	SP = HL.get();
}

void Cpu::PUSH(string reg)
{
	uint16_t value = 0xFFFF;
	if (reg == "AF")
		value = AF.get();
	else if (reg == "BC")
		value = BC.get();
	else if (reg == "DE")
		value = DE.get();
	else if (reg == "HL")
		value = HL.get();
	else
		exit(2);
	uint8_t firstByte = (uint8_t)((value & 0xFF00) >> 8);
	uint8_t secondByte = (uint8_t)(value & 0xFF);
	SP--;
	mmu->write_ram(SP, firstByte);
	SP--;
	mmu->write_ram(SP, secondByte);
}

void Cpu::POP(string reg)
{
	uint8_t firstByte = mmu->read_ram(SP + 1);
	uint16_t secondByte = (mmu->read_ram(SP));
	SP += 2;
	uint16_t value = ((firstByte << 8) | secondByte);
	if (reg == "AF")
	{
		value &= 0xFFF0;
		AF.set(value);
	}
	//AF.set(value);
	else if (reg == "BC")
	{
		BC.set(value);
	}
	else if (reg == "DE")
	{
		DE.set(value);
	}
	else if (reg == "HL")
	{
		HL.set(value);
	}
	else
		exit(2);
}

void Cpu::ADD_A_r(char reg)
{
	uint8_t value = 0;
	switch (reg)
	{
	case 'A':
		value = AF.getLeftRegister();
		break;
	case 'F':
		value = AF.getRigthRegister();
		break;
	case 'B':
		value = BC.getLeftRegister();
		break;
	case 'C':
		value = BC.getRigthRegister();
		break;
	case 'D':
		value = DE.getLeftRegister();
		break;
	case 'E':
		value = DE.getRigthRegister();
		break;
	case 'H':
		value = HL.getLeftRegister();
		break;
	case 'L':
		value = HL.getRigthRegister();
		break;
	default:
		cout << "Flag error.";
		exit(0);
		break;
	}
	(checkAddHalfCarry(value, AF.getLeftRegister(), AF.getLeftRegister() + value)) ? AF.setFlag('H') : AF.clearFlag('H');
	(checkAddCarry(value, AF.getLeftRegister(), AF.getLeftRegister() + value)) ? AF.setFlag('C') : AF.clearFlag('C');
	uint8_t result = AF.getLeftRegister() + value;
	(result == 0) ? AF.setFlag('Z') : AF.clearFlag('Z');
	AF.clearFlag('N');
	AF.setLeftRegister(result);
}

void Cpu::ADD_A_n(uint8_t value)
{
	(checkAddHalfCarry(value, AF.getLeftRegister(), AF.getLeftRegister() + value)) ? AF.setFlag('H') : AF.clearFlag('H');
	(checkAddCarry(value, AF.getLeftRegister(), AF.getLeftRegister() + value)) ? AF.setFlag('C') : AF.clearFlag('C');
	uint8_t result = AF.getLeftRegister() + value;
	(result == 0) ? AF.setFlag('Z') : AF.clearFlag('Z');
	AF.clearFlag('N');
	AF.setLeftRegister(result);
}

void Cpu::ADD_A_16()
{
	uint8_t value = mmu->read_ram(HL.get());
	(checkAddHalfCarry(value, AF.getLeftRegister(), AF.getLeftRegister() + value)) ? AF.setFlag('H') : AF.clearFlag('H');
	// (checkAddCarry(value, AF.getLeftRegister(), AF.getLeftRegister() + value)) ? AF.setFlag('C') : AF.clearFlag('C');
	((uint16_t)value + (uint16_t)AF.getLeftRegister() >= 0x100) ? AF.setFlag('C') : AF.clearFlag('C');
	uint8_t result = AF.getLeftRegister() + value;
	(result == 0) ? AF.setFlag('Z') : AF.clearFlag('Z');
	AF.clearFlag('N');
	AF.setLeftRegister(result);
}

void Cpu::ADC_A_r(char reg)
{
	uint8_t value = 0;
	switch (reg)
	{
	case 'A':
		value = AF.getLeftRegister();
		break;
	case 'B':
		value = BC.getLeftRegister();
		break;
	case 'C':
		value = BC.getRigthRegister();
		break;
	case 'D':
		value = DE.getLeftRegister();
		break;
	case 'E':
		value = DE.getRigthRegister();
		break;
	case 'H':
		value = HL.getLeftRegister();
		break;
	case 'L':
		value = HL.getRigthRegister();
		break;
	default:
		cout << "Flag error.";
		exit(0);
		break;
	}
	uint8_t A = AF.getLeftRegister();
	uint8_t carry = AF.getFlag('C') ? 1 : 0;

	unsigned int result_full = A + value + carry;
	uint8_t result = static_cast<uint8_t>(result_full);

	(result == 0) ? AF.setFlag('Z') : AF.clearFlag('Z');
	AF.clearFlag('N');
	(((A & 0xF) + (value & 0xF) + carry) > 0xF) ? AF.setFlag('H') : AF.clearFlag('H');
	(result_full > 0xFF) ? AF.setFlag('C') : AF.clearFlag('C');

	AF.setLeftRegister(result);

}

void Cpu::ADC_A_n(uint8_t value)
{
	uint8_t reg = AF.getLeftRegister();
	uint8_t carry = AF.getFlag('C') ? 1 : 0;

	unsigned int result_full = reg + value + carry;
	uint8_t result = static_cast<uint8_t>(result_full);

	(result == 0) ? AF.setFlag('Z') : AF.clearFlag('Z');
	AF.clearFlag('N');
	(((reg & 0xF) + (value & 0xF) + carry) > 0xF) ? AF.setFlag('H') : AF.clearFlag('H');
	(result_full > 0xFF) ? AF.setFlag('C') : AF.clearFlag('C');

	AF.setLeftRegister(result);
}
void Cpu::ADC_A_16()
{
	uint8_t value = mmu->read_ram(HL.get());
	uint8_t reg = AF.getLeftRegister();
	uint8_t carry = AF.getFlag('C') ? 1 : 0;

	unsigned int result_full = reg + value + carry;
	uint8_t result = static_cast<uint8_t>(result_full);

	(result == 0) ? AF.setFlag('Z') : AF.clearFlag('Z');
	AF.clearFlag('N');
	(((reg & 0xF) + (value & 0xF) + carry) > 0xF) ? AF.setFlag('H') : AF.clearFlag('H');
	(result_full > 0xFF) ? AF.setFlag('C') : AF.clearFlag('C');

	AF.setLeftRegister(result);
}

void Cpu::SUB_r(char reg)
{
	uint8_t value = 0;
	switch (reg)
	{
	case 'A':
		value = AF.getLeftRegister();
		break;
	case 'F':
		value = AF.getRigthRegister();
		break;
	case 'B':
		value = BC.getLeftRegister();
		break;
	case 'C':
		value = BC.getRigthRegister();
		break;
	case 'D':
		value = DE.getLeftRegister();
		break;
	case 'E':
		value = DE.getRigthRegister();
		break;
	case 'H':
		value = HL.getLeftRegister();
		break;
	case 'L':
		value = HL.getRigthRegister();
		break;
	default:
		cout << "Flag error.";
		exit(0);
		break;
	}
	(AF.getLeftRegister() - value < 0) ? AF.setFlag('C') : AF.clearFlag('C');
	(checkSubHalfCarry(AF.getLeftRegister(), value)) ? AF.setFlag('H') : AF.clearFlag('H');
	AF.setLeftRegister(AF.getLeftRegister() - value);
	(AF.getLeftRegister() == 0) ? AF.setFlag('Z') : AF.clearFlag('Z');
	AF.setFlag('N');
}

void Cpu::SUB_n(uint8_t value)
{
	(AF.getLeftRegister() - value < 0) ? AF.setFlag('C') : AF.clearFlag('C');
	(checkSubHalfCarry(AF.getLeftRegister(), value)) ? AF.setFlag('H') : AF.clearFlag('H');
	AF.setLeftRegister(AF.getLeftRegister() - value);
	(AF.getLeftRegister() == 0) ? AF.setFlag('Z') : AF.clearFlag('Z');
	AF.setFlag('N');
}

void Cpu::SUB_16()
{
	uint8_t value = mmu->read_ram(HL.get());
	(AF.getLeftRegister() - value < 0) ? AF.setFlag('C') : AF.clearFlag('C');
	(checkSubHalfCarry(AF.getLeftRegister(), value)) ? AF.setFlag('H') : AF.clearFlag('H');
	AF.setLeftRegister(AF.getLeftRegister() - value);
	(AF.getLeftRegister() == 0) ? AF.setFlag('Z') : AF.clearFlag('Z');
	AF.setFlag('N');
}

void Cpu::SBC_r(char reg)
{
	uint8_t value = 0;
	switch (reg)
	{
	case 'A':
		value = AF.getLeftRegister();
		break;
	case 'B':
		value = BC.getLeftRegister();
		break;
	case 'C':
		value = BC.getRigthRegister();
		break;
	case 'D':
		value = DE.getLeftRegister();
		break;
	case 'E':
		value = DE.getRigthRegister();
		break;
	case 'H':
		value = HL.getLeftRegister();
		break;
	case 'L':
		value = HL.getRigthRegister();
		break;
	default:
		cout << "Flag error.";
		exit(0);
		break;
	}
	uint8_t carry = AF.getFlag('C') ? 1 : 0;
	uint8_t A = AF.getLeftRegister();

	int result_full = A - value - carry;
	uint8_t result = static_cast<uint8_t>(result_full);

	(result == 0) ? AF.setFlag('Z') : AF.clearFlag('Z');
	AF.setFlag('N');
	(result_full < 0) ? AF.setFlag('C') : AF.clearFlag('C');
	(((A & 0xF) - (value & 0xF) - carry) < 0) ? AF.setFlag('H') : AF.clearFlag('H');

	AF.setLeftRegister(result);
}

void Cpu::SBC_n(uint8_t value)
{
	uint8_t carry = AF.getFlag('C') ? 1 : 0;
	uint8_t A = AF.getLeftRegister();

	int result_full = A - value - carry;
	uint8_t result = static_cast<uint8_t>(result_full);

	(result == 0) ? AF.setFlag('Z') : AF.clearFlag('Z');
	AF.setFlag('N');
	(result_full < 0) ? AF.setFlag('C') : AF.clearFlag('C');
	(((A & 0xF) - (value & 0xF) - carry) < 0) ? AF.setFlag('H') : AF.clearFlag('H');

	AF.setLeftRegister(result);
}

void Cpu::SBC_16()
{
	uint8_t value = mmu->read_ram(HL.get());
	uint8_t carry = AF.getFlag('C') ? 1 : 0;
	uint8_t A = AF.getLeftRegister();

	int result_full = A - value - carry;
	uint8_t result = static_cast<uint8_t>(result_full);

	(result == 0) ? AF.setFlag('Z') : AF.clearFlag('Z');
	AF.setFlag('N');
	(result_full < 0) ? AF.setFlag('C') : AF.clearFlag('C');
	(((A & 0xF) - (value & 0xF) - carry) < 0) ? AF.setFlag('H') : AF.clearFlag('H');

	AF.setLeftRegister(result);
}

void Cpu::AND_r(char reg)
{
	uint8_t value = 0;
	switch (reg)
	{
	case 'A':
		value = AF.getLeftRegister();
		break;
	case 'F':
		value = AF.getRigthRegister();
		break;
	case 'B':
		value = BC.getLeftRegister();
		break;
	case 'C':
		value = BC.getRigthRegister();
		break;
	case 'D':
		value = DE.getLeftRegister();
		break;
	case 'E':
		value = DE.getRigthRegister();
		break;
	case 'H':
		value = HL.getLeftRegister();
		break;
	case 'L':
		value = HL.getRigthRegister();
		break;
	default:
		cout << "Flag error.";
		exit(0);
		break;
	}
	uint8_t b = value;
	uint8_t result = AF.getLeftRegister() & value;
	AF.setLeftRegister(result);
	(result == 0) ? AF.setFlag('Z') : AF.clearFlag('Z');
	AF.setFlag('H');
	AF.clearFlag('N');
	AF.clearFlag('C');
}

void Cpu::AND_n(uint8_t value)
{
	uint8_t result = AF.getLeftRegister() & value;
	AF.setLeftRegister(result);
	(result == 0) ? AF.setFlag('Z') : AF.clearFlag('Z');
	AF.setFlag('H');
	AF.clearFlag('N');
	AF.clearFlag('C');
}

void Cpu::AND_16()
{
	uint8_t value = mmu->read_ram(HL.get());
	uint8_t result = AF.getLeftRegister() & value;
	AF.setLeftRegister(result);
	(result == 0) ? AF.setFlag('Z') : AF.clearFlag('Z');
	AF.setFlag('H');
	AF.clearFlag('N');
	AF.clearFlag('C');
}

void Cpu::OR_r(char reg)
{
	uint8_t value = 0;
	switch (reg)
	{
	case 'A':
		value = AF.getLeftRegister();
		break;
	case 'B':
		value = BC.getLeftRegister();
		break;
	case 'C':
		value = BC.getRigthRegister();
		break;
	case 'D':
		value = DE.getLeftRegister();
		break;
	case 'E':
		value = DE.getRigthRegister();
		break;
	case 'H':
		value = HL.getLeftRegister();
		break;
	case 'L':
		value = HL.getRigthRegister();
		break;
	default:
		cout << "Flag error.";
		exit(0);
		break;
	}
	uint8_t b = value;
	uint8_t result = AF.getLeftRegister() | value;
	AF.setLeftRegister(result);
	(result == 0) ? AF.setFlag('Z') : AF.clearFlag('Z');
	AF.clearFlag('H');
	AF.clearFlag('N');
	AF.clearFlag('C');
}

void Cpu::OR_n(uint8_t value)
{
	uint8_t result = AF.getLeftRegister() | value;
	AF.setLeftRegister(result);
	(result == 0) ? AF.setFlag('Z') : AF.clearFlag('Z');
	AF.clearFlag('H');
	AF.clearFlag('N');
	AF.clearFlag('C');
}

void Cpu::OR_16()
{
	uint8_t value = mmu->read_ram(HL.get());
	uint8_t result = AF.getLeftRegister() | value;
	AF.setLeftRegister(result);
	(result == 0) ? AF.setFlag('Z') : AF.clearFlag('Z');
	AF.clearFlag('H');
	AF.clearFlag('N');
	AF.clearFlag('C');
}

void Cpu::XOR_r(char reg)
{
	uint8_t value = 0;
	switch (reg)
	{
	case 'A':
		value = AF.getLeftRegister();
		break;
	case 'B':
		value = BC.getLeftRegister();
		break;
	case 'C':
		value = BC.getRigthRegister();
		break;
	case 'D':
		value = DE.getLeftRegister();
		break;
	case 'E':
		value = DE.getRigthRegister();
		break;
	case 'H':
		value = HL.getLeftRegister();
		break;
	case 'L':
		value = HL.getRigthRegister();
		break;
	default:
		cout << "Flag error.";
		exit(0);
		break;
	}
	uint8_t b = value;
	uint8_t result = AF.getLeftRegister() ^ value;
	AF.setLeftRegister(result);
	(result == 0) ? AF.setFlag('Z') : AF.clearFlag('Z');
	AF.clearFlag('H');
	AF.clearFlag('N');
	AF.clearFlag('C');
}

void Cpu::XOR_n(uint8_t value)
{
	uint8_t result = AF.getLeftRegister() ^ value;
	AF.setLeftRegister(result);
	(result == 0) ? AF.setFlag('Z') : AF.clearFlag('Z');
	AF.clearFlag('H');
	AF.clearFlag('N');
	AF.clearFlag('C');
}

void Cpu::XOR_16()
{
	uint8_t value = mmu->read_ram(HL.get());
	uint8_t result = AF.getLeftRegister() ^ value;
	AF.setLeftRegister(result);
	(result == 0) ? AF.setFlag('Z') : AF.clearFlag('Z');
	AF.clearFlag('H');
	AF.clearFlag('N');
	AF.clearFlag('C');
}

void Cpu::CP_r(char reg)
{
	uint8_t value = 0;
	switch (reg)
	{
	case 'A':
		value = AF.getLeftRegister();
		break;
	case 'F':
		value = AF.getRigthRegister();
		break;
	case 'B':
		value = BC.getLeftRegister();
		break;
	case 'C':
		value = BC.getRigthRegister();
		break;
	case 'D':
		value = DE.getLeftRegister();
		break;
	case 'E':
		value = DE.getRigthRegister();
		break;
	case 'H':
		value = HL.getLeftRegister();
		break;
	case 'L':
		value = HL.getRigthRegister();
		break;
	default:
		cout << "Flag error.";
		exit(0);
		break;
	}
	((uint16_t)AF.getLeftRegister() - (uint16_t)value < 0) ? AF.setFlag('C') : AF.clearFlag('C');
	(checkSubHalfCarry(AF.getLeftRegister(), value)) ? AF.setFlag('H') : AF.clearFlag('H');
	(AF.getLeftRegister() - value == 0) ? AF.setFlag('Z') : AF.clearFlag('Z');
	AF.setFlag('N');
}

void Cpu::CP_n(uint8_t value)
{
   uint8_t reg = AF.getLeftRegister();
    uint8_t result = static_cast<uint8_t>(reg - value);

    (result == 0) ? AF.setFlag('Z') : AF.clearFlag('Z');
    AF.setFlag('N');
    (((reg & 0xf) - (value & 0xf)) < 0) ? AF.setFlag('H') : AF.clearFlag('H');
    (reg < value) ? AF.setFlag('C') : AF.clearFlag('C');
}

void Cpu::CP_16()
{
	uint8_t value = mmu->read_ram(HL.get());
	((uint16_t)AF.getLeftRegister() - (uint16_t)value < 0) ? AF.setFlag('C') : AF.clearFlag('C');
	(checkSubHalfCarry(AF.getLeftRegister(), value)) ? AF.setFlag('H') : AF.clearFlag('H');
	(AF.getLeftRegister() - value == 0) ? AF.setFlag('Z') : AF.clearFlag('Z');
	AF.setFlag('N');
}

void Cpu::INC_r(char reg)
{
	uint8_t value = 0, oldReg = 0;
	switch (reg)
	{
	case 'A':
		oldReg = AF.getLeftRegister();
		value = AF.getLeftRegister() + 1;
		AF.setLeftRegister(value);
		break;
	case 'F':
		oldReg = AF.getRigthRegister();
		value = AF.getRigthRegister() + 1;
		AF.setRigthRegister(value);
		break;
	case 'B':
		oldReg = BC.getLeftRegister();
		value = BC.getLeftRegister() + 1;
		BC.setLeftRegister(value);
		break;
	case 'C':
		oldReg = BC.getRigthRegister();
		value = BC.getRigthRegister() + 1;
		BC.setRigthRegister(value);
		break;
	case 'D':
		oldReg = DE.getLeftRegister();
		value = DE.getLeftRegister() + 1;
		DE.setLeftRegister(value);
		break;
	case 'E':
		oldReg = DE.getRigthRegister();
		value = DE.getRigthRegister() + 1;
		DE.setRigthRegister(value);
		break;
	case 'H':
		oldReg = HL.getLeftRegister();
		value = HL.getLeftRegister() + 1;
		HL.setLeftRegister(value);
		break;
	case 'L':
		oldReg = HL.getRigthRegister();
		value = HL.getRigthRegister() + 1;
		HL.setRigthRegister(value);
		break;
	default:
		cout << "Flag error.";
		exit(0);
		break;
	}
	(value == 0) ? AF.setFlag('Z') : AF.clearFlag('Z');
	(checkAddHalfCarry(oldReg, 1, oldReg + 1)) ? AF.setFlag('H') : AF.clearFlag('H');
	AF.clearFlag('N');
}

void Cpu::INC_16()
{
	uint8_t oldValue = mmu->read_ram(HL.get());
	uint8_t value = mmu->read_ram(HL.get()) + 1;
	mmu->write_ram(HL.get(), value);
	(value == 0) ? AF.setFlag('Z') : AF.clearFlag('Z');
	(checkAddHalfCarry(oldValue, 1, oldValue + 1)) ? AF.setFlag('H') : AF.clearFlag('H');
	AF.clearFlag('N');
}

void Cpu::DEC_r(char reg)
{
	uint8_t value = 0;
	switch (reg)
	{
	case 'A':
		value = AF.getLeftRegister();
		break;
	case 'F':
		value = AF.getRigthRegister();
		break;
	case 'B':
		value = BC.getLeftRegister();
		break;
	case 'C':
		value = BC.getRigthRegister();
		break;
	case 'D':
		value = DE.getLeftRegister();
		break;
	case 'E':
		value = DE.getRigthRegister();
		break;
	case 'H':
		value = HL.getLeftRegister();
		break;
	case 'L':
		value = HL.getRigthRegister();
		break;
	default:
		cout << "Flag error.";
		exit(0);
		break;
	}
	switch (reg)
	{
	case 'A':
		AF.setLeftRegister(value - 1);
		break;
	case 'F':
		AF.setRigthRegister(value - 1);
		break;
	case 'B':
		BC.setLeftRegister(value - 1);
		break;
	case 'C':
		BC.setRigthRegister(value - 1);
		break;
	case 'D':
		DE.setLeftRegister(value - 1);
		break;
	case 'E':
		DE.setRigthRegister(value - 1);
		break;
	case 'H':
		HL.setLeftRegister(value - 1);
		break;
	case 'L':
		HL.setRigthRegister(value - 1);
		break;
	default:
		cout << "Flag error.";
		exit(0);
		break;
	}
	((value - 1) == 0) ? AF.setFlag('Z') : AF.clearFlag('Z');
	(checkSubHalfCarry(value, 1)) ? AF.setFlag('H') : AF.clearFlag('H');
	AF.setFlag('N');
}

void Cpu::DEC_16()
{
	uint8_t value = mmu->read_ram(HL.get());
	mmu->write_ram(HL.get(), value - 1);
	((value - 1) == 0) ? AF.setFlag('Z') : AF.clearFlag('Z');
	(checkSubHalfCarry(value, 1)) ? AF.setFlag('H') : AF.clearFlag('H');
	AF.setFlag('N');
}

void Cpu::DAA()
{
	if (!AF.getFlag('N'))
	{
		if (AF.getFlag('C') || (AF.getLeftRegister() > 0x99))
		{
			uint8_t value = AF.getLeftRegister();
			AF.setLeftRegister(value + 0x60);
			AF.setFlag('C');
		}
		if (AF.getFlag('H') || ((AF.getLeftRegister() & 0x0F) > 0x09))
		{
			uint8_t value = AF.getLeftRegister();
			AF.setLeftRegister(value + 0x06);
		}
	}
	else
	{
		if (AF.getFlag('C'))
		{
			uint8_t value = AF.getLeftRegister();
			AF.setLeftRegister(value - 0x60);
		}
		if (AF.getFlag('H'))
		{
			uint8_t value = AF.getLeftRegister();
			AF.setLeftRegister(value - 0x06);
		}
	}
	(AF.getLeftRegister() == 0) ? AF.setFlag('Z') : AF.clearFlag('Z');
	AF.clearFlag('H');
}

void Cpu::CPL()
{
	uint8_t value = AF.getLeftRegister();
	AF.setLeftRegister(~value);
	AF.setFlag('H');
	AF.setFlag('N');
}

void Cpu::CCF()
{
	uint8_t value = AF.getRigthRegister();
	value ^= 0x10;
	AF.setRigthRegister(value);
	AF.clearFlag('N');
	AF.clearFlag('H');
}

void Cpu::SCF()
{
	uint8_t value = AF.getRigthRegister();
	value |= 0x10;
	AF.setRigthRegister(value);
	AF.clearFlag('N');
	AF.clearFlag('H');
}

void Cpu::NOP()
{
}

void Cpu::HALT()
{
	halted = true;
}

void Cpu::DI()
{
	interrupt_enabled = false;
}

void Cpu::EI()
{
	interrupt_enabled = true;
}

void Cpu::ADD_HL_rr(string reg)
{
	uint16_t value;
	if (reg == "AF")
		value = AF.get();
	else if (reg == "BC")
		value = BC.get();
	else if (reg == "DE")
		value = DE.get();
	else if (reg == "HL")
		value = HL.get();
	else if (reg == "SP")
		value = SP;
	else
		exit(2);
	uint16_t operand = HL.get();
	HL.set(operand + value);
	AF.clearFlag('N');
	((operand + value) >= 0x10000 ) ? AF.setFlag('C') : AF.clearFlag('C');
	((operand & 0xFFF) + (value & 0xFFF) >= 0x1000 ) ? AF.setFlag('H') : AF.clearFlag('H');
}

void Cpu::ADD_SP_n()
{
	int8_t value = (int8_t)mmu->read_ram(PC + 1);
	uint16_t result = (SP + value);
	(((SP ^ value ^ (result & 0xFFFF)) & 0x10) == 0x10) ? AF.setFlag('H') : AF.clearFlag('H');
	(((SP ^ value ^ (result & 0xFFFF)) & 0x100) == 0x100) ? AF.setFlag('C') : AF.clearFlag('C');
	SP = (result);
	AF.clearFlag('Z');
	AF.clearFlag('N');
}

void Cpu::ADC_HL_rr(string reg)
{
	uint16_t value = 0;
	if (reg == "AF")
		value = AF.get();
	else if (reg == "BC")
		value = BC.get();
	else if (reg == "DE")
		value = DE.get();
	else if (reg == "HL")
		value = HL.get();
	else if (reg == "SP")
		value = SP;
	else
		exit(2);
	value += AF.getFlag('C') ? 1 : 0;
	uint16_t operand = HL.get();
	HL.set(operand + value);
	AF.clearFlag('N');
	(checkAddCarry(operand, value, value + operand)) ? AF.setFlag('C') : AF.clearFlag('C');
	(checkAddHalfCarry(operand, value, value + operand)) ? AF.setFlag('H') : AF.clearFlag('H');
}

void Cpu::SBC_HL_rr(string reg)
{
	uint16_t value = 0;
	if (reg == "AF")
		value = AF.get();
	else if (reg == "BC")
		value = BC.get();
	else if (reg == "DE")
		value = DE.get();
	else if (reg == "HL")
		value = HL.get();
	else if (reg == "SP")
		value = SP;
	else
		exit(2);
	value += AF.getFlag('C') ? 1 : 0;
	uint16_t operand = HL.get();
	HL.set(operand - value);
	AF.setFlag('N');
	(checkAddCarry(operand, value, value + operand)) ? AF.setFlag('C') : AF.clearFlag('C');
	(checkSubHalfCarry(operand, value)) ? AF.setFlag('H') : AF.clearFlag('H');
}

void Cpu::INC_rr(string reg)
{
	uint16_t value = 0;
	if (reg == "AF")
	{
		value = AF.get() + 1;
		AF.set(value);
	}
	else if (reg == "BC")
	{
		value = BC.get() + 1;
		BC.set(value);
	}
	else if (reg == "DE")
	{
		value = DE.get() + 1;
		DE.set(value);
	}
	else if (reg == "HL")
	{
		value = HL.get() + 1;
		HL.set(value);
	}
	else if (reg == "SP")
	{
		value = SP + 1;
		SP++;
	}
	else
		exit(2);
}

void Cpu::DEC_rr(string reg)
{
	uint16_t value = 0;
	if (reg == "AF")
	{
		value = AF.get() - 1;
		AF.set(value);
	}
	else if (reg == "BC")
	{
		value = BC.get() - 1;
		BC.set(value);
	}
	else if (reg == "DE")
	{
		value = DE.get() - 1;
		DE.set(value);
	}
	else if (reg == "HL")
	{
		value = HL.get() - 1;
		HL.set(value);
	}
	else if (reg == "SP")
	{
		value = SP - 1;
		SP--;
	}
	else
		exit(2);
}

void Cpu::RLCA()
{
	uint8_t reg = AF.getLeftRegister();
	((reg & 0x80) == 0x80)? AF.setFlag('C') : AF.clearFlag('C');
	int old7bit = ((reg & 0x80) == 0x80) ? 1 : 0;
	uint8_t value = ((reg << 1) | (old7bit));
	AF.setLeftRegister(value);
	(AF.getLeftRegister() == 0) ? AF.setFlag('Z') : AF.clearFlag('Z');
	// AF.clearFlag('Z');
	AF.clearFlag('H');
	AF.clearFlag('N');
}

void Cpu::RLA()
{
	int oldCarry = AF.getFlag('C');
	(AF.getLeftRegister() & 0x80) ? AF.setFlag('C') : AF.clearFlag('C');
	uint8_t value = ((AF.getLeftRegister() << 1) | (oldCarry << 0));
	AF.setLeftRegister(value);
	(AF.getLeftRegister() == 0) ? AF.setFlag('Z') : AF.clearFlag('Z');
	// AF.clearFlag('Z');
	AF.clearFlag('H');
	AF.clearFlag('N');
}

void Cpu::RRCA()
{
	(AF.getLeftRegister() & 0x1) ? AF.setFlag('C') : AF.clearFlag('C');
	AF.setLeftRegister((AF.getLeftRegister() >> 1) | (AF.getFlag('C') << 7));
	(AF.getLeftRegister() == 0) ? AF.setFlag('Z') : AF.clearFlag('Z');
	// AF.clearFlag('H');
	AF.clearFlag('N');
	AF.clearFlag('Z');
}

void Cpu::RRA()
{
	int oldCarry = AF.getFlag('C');
	(AF.getLeftRegister() & 0x1) ? AF.setFlag('C') : AF.clearFlag('C');
	uint8_t value = ((AF.getLeftRegister() >> 1) | (oldCarry << 7));
	AF.setLeftRegister(value);
	(AF.getLeftRegister() == 0) ? AF.setFlag('Z') : AF.clearFlag('Z');
	// AF.clearFlag('Z');
	AF.clearFlag('H');
	AF.clearFlag('N');
}

void Cpu::RLC_r(char reg)
{
	uint8_t value = 0;
	switch (reg)
	{
	case 'A':
		(AF.getLeftRegister() & 0x80) ? AF.setFlag('C') : AF.clearFlag('C');
		AF.setLeftRegister((AF.getLeftRegister() << 1) | AF.getFlag('C'));
		AF.clearFlag('H');
		AF.clearFlag('N');
		value = AF.getLeftRegister();
		break;
	case 'B':
		(BC.getLeftRegister() & 0x80) ? AF.setFlag('C') : AF.clearFlag('C');
		BC.setLeftRegister((BC.getLeftRegister() << 1) | AF.getFlag('C'));
		AF.clearFlag('H');
		AF.clearFlag('N');
		value = BC.getLeftRegister();
		break;
	case 'C':
		(BC.getRigthRegister() & 0x80) ? AF.setFlag('C') : AF.clearFlag('C');
		BC.setRigthRegister((BC.getRigthRegister() << 1) | AF.getFlag('C'));
		AF.clearFlag('H');
		AF.clearFlag('N');
		value = BC.getRigthRegister();
		break;
	case 'D':
		(DE.getLeftRegister() & 0x80) ? AF.setFlag('C') : AF.clearFlag('C');
		DE.setLeftRegister((DE.getLeftRegister() << 1) | AF.getFlag('C'));
		AF.clearFlag('H');
		AF.clearFlag('N');
		value = DE.getLeftRegister();
		break;
	case 'E':
		(DE.getRigthRegister() & 0x80) ? AF.setFlag('C') : AF.clearFlag('C');
		DE.setRigthRegister((DE.getRigthRegister() << 1) | AF.getFlag('C'));
		AF.clearFlag('H');
		AF.clearFlag('N');
		value = DE.getRigthRegister();
		break;
	case 'H':
		(HL.getLeftRegister() & 0x80) ? AF.setFlag('C') : AF.clearFlag('C');
		HL.setLeftRegister((HL.getLeftRegister() << 1) | AF.getFlag('C'));
		AF.clearFlag('H');
		AF.clearFlag('N');
		value = HL.getLeftRegister();
		break;
	case 'L':
		(HL.getRigthRegister() & 0x80) ? AF.setFlag('C') : AF.clearFlag('C');
		HL.setRigthRegister((HL.getRigthRegister() << 1) | AF.getFlag('C'));
		AF.clearFlag('H');
		AF.clearFlag('N');
		value = HL.getRigthRegister();
		break;
	default:
		cout << "Flag error.";
		exit(0);
		break;
	}
	(value == 0) ? AF.setFlag('Z') : AF.clearFlag('Z');
}

void Cpu::RLC_16()
{
	uint8_t value = mmu->read_ram(HL.get());
	(value & 0x80) ? AF.setFlag('C') : AF.clearFlag('C');
	mmu->write_ram(HL.get(), (value << 1) | AF.getFlag('C'));
	AF.clearFlag('H');
	AF.clearFlag('N');
	(((value << 1) | AF.getFlag('C')) == 0) ? AF.setFlag('Z') : AF.clearFlag('Z');
}

void Cpu::RL_16()
{
	uint8_t value = mmu->read_ram(HL.get());
	int oldCarry = AF.getFlag('C');
	(value & 0x80) ? AF.setFlag('C') : AF.clearFlag('C');
	value = (value << 1) | (oldCarry);
	(value == 0) ? AF.setFlag('Z') : AF.clearFlag('Z');
	AF.clearFlag('H');
	AF.clearFlag('N');
	mmu->write_ram(HL.get(), value);
}

void Cpu::RL_r(char reg)
{
	uint8_t value = 0, oldCarry = AF.getFlag('C');
	switch (reg)
	{
	case 'A':
		(AF.getLeftRegister() & 0x80) ? AF.setFlag('C') : AF.clearFlag('C');
		AF.setLeftRegister((AF.getLeftRegister() << 1) | oldCarry);
		AF.clearFlag('H');
		AF.clearFlag('N');
		value = AF.getLeftRegister();
		break;
	case 'B':
		(BC.getLeftRegister() & 0x80) ? AF.setFlag('C') : AF.clearFlag('C');
		BC.setLeftRegister((BC.getLeftRegister() << 1) | oldCarry);
		AF.clearFlag('H');
		AF.clearFlag('N');
		value = BC.getLeftRegister();
		break;
	case 'C':
		(BC.getRigthRegister() & 0x80) ? AF.setFlag('C') : AF.clearFlag('C');
		BC.setRigthRegister((BC.getRigthRegister() << 1) | oldCarry);
		AF.clearFlag('H');
		AF.clearFlag('N');
		value = BC.getRigthRegister();
		break;
	case 'D':
		(DE.getLeftRegister() & 0x80) ? AF.setFlag('C') : AF.clearFlag('C');
		DE.setLeftRegister((DE.getLeftRegister() << 1) | oldCarry);
		AF.clearFlag('H');
		AF.clearFlag('N');
		value = DE.getLeftRegister();
		break;
	case 'E':
		(DE.getRigthRegister() & 0x80) ? AF.setFlag('C') : AF.clearFlag('C');
		DE.setRigthRegister((DE.getRigthRegister() << 1) | oldCarry);
		AF.clearFlag('H');
		AF.clearFlag('N');
		value = DE.getRigthRegister();
		break;
	case 'H':
		(HL.getLeftRegister() & 0x80) ? AF.setFlag('C') : AF.clearFlag('C');
		HL.setLeftRegister((HL.getLeftRegister() << 1) | oldCarry);
		AF.clearFlag('H');
		AF.clearFlag('N');
		value = HL.getLeftRegister();
		break;
	case 'L':
		(HL.getRigthRegister() & 0x80) ? AF.setFlag('C') : AF.clearFlag('C');
		HL.setRigthRegister((HL.getRigthRegister() << 1) | oldCarry);
		AF.clearFlag('H');
		AF.clearFlag('N');
		value = HL.getRigthRegister();
		break;
	default:
		cout << "Flag error.";
		exit(0);
		break;
	}
	(value == 0) ? AF.setFlag('Z') : AF.clearFlag('Z');
}

void Cpu::RR_r(char reg)
{
	uint8_t value = 0;
	int oldCarry = AF.getFlag('C');
	switch (reg)
	{
	case 'A':
		(AF.getLeftRegister() & 0x1) ? AF.setFlag('C') : AF.clearFlag('C');
		AF.setLeftRegister((AF.getLeftRegister() >> 1) | (oldCarry << 7));
		AF.clearFlag('H');
		AF.clearFlag('N');
		value = AF.getLeftRegister();
		break;
	case 'B':
		(BC.getLeftRegister() & 0x1) ? AF.setFlag('C') : AF.clearFlag('C');
		BC.setLeftRegister((BC.getLeftRegister() >> 1) | (oldCarry << 7));
		AF.clearFlag('H');
		AF.clearFlag('N');
		value = BC.getLeftRegister();
		break;
	case 'C':
		(BC.getRigthRegister() & 0x1) ? AF.setFlag('C') : AF.clearFlag('C');
		BC.setRigthRegister((BC.getRigthRegister() >> 1) | (oldCarry << 7));
		AF.clearFlag('H');
		AF.clearFlag('N');
		value = BC.getRigthRegister();
		break;
	case 'D':
		(DE.getLeftRegister() & 0x1) ? AF.setFlag('C') : AF.clearFlag('C');
		DE.setLeftRegister((DE.getLeftRegister() >> 1) | (oldCarry << 7));
		AF.clearFlag('H');
		AF.clearFlag('N');
		value = DE.getLeftRegister();
		break;
	case 'E':
		(DE.getRigthRegister() & 0x1) ? AF.setFlag('C') : AF.clearFlag('C');
		DE.setRigthRegister((DE.getRigthRegister() >> 1) | (oldCarry << 7));
		AF.clearFlag('H');
		AF.clearFlag('N');
		value = DE.getRigthRegister();
		break;
	case 'H':
		(HL.getLeftRegister() & 0x1) ? AF.setFlag('C') : AF.clearFlag('C');
		HL.setLeftRegister((HL.getLeftRegister() >> 1) | (oldCarry << 7));
		AF.clearFlag('H');
		AF.clearFlag('N');
		value = HL.getLeftRegister();
		break;
	case 'L':
		(HL.getRigthRegister() & 0x1) ? AF.setFlag('C') : AF.clearFlag('C');
		HL.setRigthRegister((HL.getRigthRegister() >> 1) | (oldCarry << 7));
		AF.clearFlag('H');
		AF.clearFlag('N');
		value = HL.getRigthRegister();
		break;
	default:
		cout << "Flag error.";
		exit(0);
		break;
	}
	(value == 0) ? AF.setFlag('Z') : AF.clearFlag('Z');
}

void Cpu::RR_16() //TODO may be innacurate
{
	int oldCarry = AF.getFlag('C');
	uint8_t value = mmu->read_ram(HL.get());
	(value & 0x1) ? AF.setFlag('C') : AF.clearFlag('C');
	value = ((value >> 1) | (oldCarry << 7));
	mmu->write_ram(HL.get(), value);
	(value == 0) ? AF.setFlag('Z') : AF.clearFlag('Z');
	AF.clearFlag('H');
	AF.clearFlag('N');
}

void Cpu::RRC_r(char reg)
{
	uint8_t value = 0;
	switch (reg)
	{
	case 'A':
		(AF.getLeftRegister() & 0x1) ? AF.setFlag('C') : AF.clearFlag('C');
		AF.setLeftRegister((AF.getLeftRegister() >> 1) | (AF.getFlag('C') << 7));
		AF.clearFlag('H');
		AF.clearFlag('N');
		value = AF.getLeftRegister();
		break;
	case 'B':
		(BC.getLeftRegister() & 0x1) ? AF.setFlag('C') : AF.clearFlag('C');
		BC.setLeftRegister((BC.getLeftRegister() >> 1) | (AF.getFlag('C') << 7));
		AF.clearFlag('H');
		AF.clearFlag('N');
		value = BC.getLeftRegister();
		break;
	case 'C':
		(BC.getRigthRegister() & 0x1) ? AF.setFlag('C') : AF.clearFlag('C');
		BC.setRigthRegister((BC.getRigthRegister() >> 1) | (AF.getFlag('C') << 7));
		AF.clearFlag('H');
		AF.clearFlag('N');
		value = BC.getRigthRegister();
		break;
	case 'D':
		(DE.getLeftRegister() & 0x1) ? AF.setFlag('C') : AF.clearFlag('C');
		DE.setLeftRegister((DE.getLeftRegister() >> 1) | (AF.getFlag('C') << 7));
		AF.clearFlag('H');
		AF.clearFlag('N');
		value = DE.getLeftRegister();
		break;
	case 'E':
		(DE.getRigthRegister() & 0x1) ? AF.setFlag('C') : AF.clearFlag('C');
		DE.setRigthRegister((DE.getRigthRegister() >> 1) | (AF.getFlag('C') << 7));
		AF.clearFlag('H');
		AF.clearFlag('N');
		value = DE.getRigthRegister();
		break;
	case 'H':
		(HL.getLeftRegister() & 0x1) ? AF.setFlag('C') : AF.clearFlag('C');
		HL.setLeftRegister((HL.getLeftRegister() >> 1) | (AF.getFlag('C') << 7));
		AF.clearFlag('H');
		AF.clearFlag('N');
		value = HL.getLeftRegister();
		break;
	case 'L':
		(HL.getRigthRegister() & 0x1) ? AF.setFlag('C') : AF.clearFlag('C');
		HL.setRigthRegister((HL.getRigthRegister() >> 1) | (AF.getFlag('C') << 7));
		AF.clearFlag('H');
		AF.clearFlag('N');
		value = HL.getRigthRegister();
		break;
	default:
		cout << "Flag error.";
		exit(0);
		break;
	}
	(value == 0) ? AF.setFlag('Z') : AF.clearFlag('Z');
}

void Cpu::RRC_16() 
{
	uint8_t value = mmu->read_ram(HL.get());
	(value & 0x1) ? AF.setFlag('C') : AF.clearFlag('C');
	value = ((value >> 1) | (AF.getFlag('C') << 7));
	(value == 0) ? AF.setFlag('Z') : AF.clearFlag('Z');
	AF.clearFlag('H');
	AF.clearFlag('N');
	mmu->write_ram(HL.get(), value);
}

void Cpu::SLA_r(char reg)
{
	uint8_t value = 0, oldCarry = AF.getFlag('C');
	switch (reg)
	{
	case 'A':
		(AF.getLeftRegister() & 0x80) ? AF.setFlag('C') : AF.clearFlag('C');
		AF.setLeftRegister((AF.getLeftRegister() << 1));
		AF.clearFlag('H');
		AF.clearFlag('N');
		value = AF.getLeftRegister();
		break;
	case 'B':
		(BC.getLeftRegister() & 0x80) ? AF.setFlag('C') : AF.clearFlag('C');
		BC.setLeftRegister((BC.getLeftRegister() << 1));
		AF.clearFlag('H');
		AF.clearFlag('N');
		value = BC.getLeftRegister();
		break;
	case 'C':
		(BC.getRigthRegister() & 0x80) ? AF.setFlag('C') : AF.clearFlag('C');
		BC.setRigthRegister((BC.getRigthRegister() << 1));
		AF.clearFlag('H');
		AF.clearFlag('N');
		value = BC.getRigthRegister();
		break;
	case 'D':
		(DE.getLeftRegister() & 0x80) ? AF.setFlag('C') : AF.clearFlag('C');
		DE.setLeftRegister((DE.getLeftRegister() << 1));
		AF.clearFlag('H');
		AF.clearFlag('N');
		value = DE.getLeftRegister();
		break;
	case 'E':
		(DE.getRigthRegister() & 0x80) ? AF.setFlag('C') : AF.clearFlag('C');
		DE.setRigthRegister((DE.getRigthRegister() << 1));
		AF.clearFlag('H');
		AF.clearFlag('N');
		value = DE.getRigthRegister();
		break;
	case 'H':
		(HL.getLeftRegister() & 0x80) ? AF.setFlag('C') : AF.clearFlag('C');
		HL.setLeftRegister((HL.getLeftRegister() << 1));
		AF.clearFlag('H');
		AF.clearFlag('N');
		value = HL.getLeftRegister();
		break;
	case 'L':
		(HL.getRigthRegister() & 0x80) ? AF.setFlag('C') : AF.clearFlag('C');
		HL.setRigthRegister((HL.getRigthRegister() << 1));
		AF.clearFlag('H');
		AF.clearFlag('N');
		value = HL.getRigthRegister();
		break;
	default:
		cout << "Flag error.";
		exit(0);
		break;
	}
	(value == 0) ? AF.setFlag('Z') : AF.clearFlag('Z');
}

void Cpu::SLA_16()
{
	uint8_t value = mmu->read_ram(HL.get()), oldCarry = AF.getFlag('C');
	(value & 0x80) ? AF.setFlag('C') : AF.clearFlag('C');
	mmu->write_ram(HL.get(), (value << 1));
	AF.clearFlag('H');
	AF.clearFlag('N');
	value = mmu->read_ram(HL.get());
	(value == 0) ? AF.setFlag('Z') : AF.clearFlag('Z');
}

void Cpu::SRA_r(char reg)
{
	uint8_t value = 0;
	uint8_t old7thbit = 0;
	switch (reg)
	{
	case 'A':
		(AF.getLeftRegister() & 0x1) ? AF.setFlag('C') : AF.clearFlag('C');
		old7thbit = (AF.getLeftRegister() & 0x80);
		AF.setLeftRegister((AF.getLeftRegister() >> 1) | (old7thbit ));
		AF.clearFlag('H');
		AF.clearFlag('N');
		value = AF.getLeftRegister();
		break;
	case 'B':
		(BC.getLeftRegister() & 0x1) ? AF.setFlag('C') : AF.clearFlag('C');
		old7thbit = (BC.getLeftRegister() & 0x80);
		BC.setLeftRegister((BC.getLeftRegister() >> 1) | (old7thbit ));
		AF.clearFlag('H');
		AF.clearFlag('N');
		value = BC.getLeftRegister();
		break;
	case 'C':
		(BC.getRigthRegister() & 0x1) ? AF.setFlag('C') : AF.clearFlag('C');
		old7thbit = (BC.getRigthRegister() & 0x80);
		BC.setRigthRegister((BC.getRigthRegister() >> 1) | (old7thbit));
		AF.clearFlag('H');
		AF.clearFlag('N');
		value = BC.getRigthRegister();
		break;
	case 'D':
		(DE.getLeftRegister() & 0x1) ? AF.setFlag('C') : AF.clearFlag('C');
		old7thbit = (DE.getLeftRegister() & 0x80);
		DE.setLeftRegister((DE.getLeftRegister() >> 1) | (old7thbit ));
		AF.clearFlag('H');
		AF.clearFlag('N');
		value = DE.getLeftRegister();
		break;
	case 'E':
		(DE.getRigthRegister() & 0x1) ? AF.setFlag('C') : AF.clearFlag('C');
		old7thbit = (DE.getRigthRegister() & 0x80);
		DE.setRigthRegister((DE.getRigthRegister() >> 1) | (old7thbit ));
		AF.clearFlag('H');
		AF.clearFlag('N');
		value = DE.getRigthRegister();
		break;
	case 'H':
		(HL.getLeftRegister() & 0x1) ? AF.setFlag('C') : AF.clearFlag('C');
		old7thbit = (HL.getLeftRegister() & 0x80);
		HL.setLeftRegister((HL.getLeftRegister() >> 1) | (old7thbit));
		AF.clearFlag('H');
		AF.clearFlag('N');
		value = HL.getLeftRegister();
		break;
	case 'L':
		(HL.getRigthRegister() & 0x1) ? AF.setFlag('C') : AF.clearFlag('C');
		old7thbit = (HL.getRigthRegister() & 0x80);
		HL.setRigthRegister((HL.getRigthRegister() >> 1) | (old7thbit));
		AF.clearFlag('H');
		AF.clearFlag('N');
		value = HL.getRigthRegister();
		break;
	default:
		cout << "Flag error.";
		exit(0);
		break;
	}
	(value == 0) ? AF.setFlag('Z') : AF.clearFlag('Z');
}

void Cpu::SRA_16()
{
	uint8_t value = mmu->read_ram(HL.get());
	int old7bit = (value & 0x80);
	(value & 0x1) ? AF.setFlag('C') : AF.clearFlag('C');
	value = ((value >> 1) | (old7bit));
	mmu->write_ram(HL.get(), value);
	(value == 0) ? AF.setFlag('Z') : AF.clearFlag('Z');
	AF.clearFlag('H');
	AF.clearFlag('N');
}

void Cpu::SRL_r(char reg)
{
	uint8_t value = 0;
	bool old7thbit;
	switch (reg)
	{
	case 'A':
		(AF.getLeftRegister() & 0x1) ? AF.setFlag('C') : AF.clearFlag('C');
		AF.setLeftRegister((AF.getLeftRegister() >> 1));
		(AF.getLeftRegister() == 0) ? AF.setFlag('Z') : AF.clearFlag('Z');
		AF.clearFlag('H');
		AF.clearFlag('N');
		break;
	case 'B':
		(BC.getLeftRegister() & 0x1) ? AF.setFlag('C') : AF.clearFlag('C');
		BC.setLeftRegister((BC.getLeftRegister() >> 1));
		AF.clearFlag('H');
		AF.clearFlag('N');
			(BC.getLeftRegister() == 0) ? AF.setFlag('Z') : AF.clearFlag('Z');
		break;
	case 'C':
		(BC.getRigthRegister() & 0x1) ? AF.setFlag('C') : AF.clearFlag('C');
		BC.setRigthRegister((BC.getRigthRegister() >> 1));
		AF.clearFlag('H');
		AF.clearFlag('N');
			(BC.getRigthRegister() == 0) ? AF.setFlag('Z') : AF.clearFlag('Z');
		break;
	case 'D':
		(DE.getLeftRegister() & 0x1) ? AF.setFlag('C') : AF.clearFlag('C');
		DE.setLeftRegister((DE.getLeftRegister() >> 1));
		AF.clearFlag('H');
		AF.clearFlag('N');
			(DE.getLeftRegister() == 0) ? AF.setFlag('Z') : AF.clearFlag('Z');
		break;
	case 'E':
		(DE.getRigthRegister() & 0x1) ? AF.setFlag('C') : AF.clearFlag('C');
		DE.setRigthRegister((DE.getRigthRegister() >> 1));
		AF.clearFlag('H');
		AF.clearFlag('N');
	    (DE.getRigthRegister() == 0) ? AF.setFlag('Z') : AF.clearFlag('Z');
		break;
	case 'H':
		(HL.getLeftRegister() & 0x1) ? AF.setFlag('C') : AF.clearFlag('C');
		HL.setLeftRegister((HL.getLeftRegister() >> 1));
		AF.clearFlag('H');
		AF.clearFlag('N');
		(HL.getLeftRegister() == 0) ? AF.setFlag('Z') : AF.clearFlag('Z');
		break;
	case 'L':
		(HL.getRigthRegister() & 0x1) ? AF.setFlag('C') : AF.clearFlag('C');
		HL.setRigthRegister((HL.getRigthRegister() >> 1));
		AF.clearFlag('H');
		AF.clearFlag('N');
		(HL.getRigthRegister() == 0) ? AF.setFlag('Z') : AF.clearFlag('Z');
		break;
	default:
		cout << "Flag error.";
		exit(0);
		break;
	}
}

void Cpu::SRL_16()
{
	uint8_t value = 0;
	(mmu->read_ram(HL.get()) & 0x1) ? AF.setFlag('C') : AF.clearFlag('C');
	mmu->write_ram(HL.get(), (mmu->read_ram(HL.get()) >> 1));
	AF.clearFlag('H');
	AF.clearFlag('N');
	value = mmu->read_ram(HL.get());
	(value == 0) ? AF.setFlag('Z') : AF.clearFlag('Z');
}

void Cpu::SWAP_r(char reg)
{
	uint8_t left = 0, rigth = 0, value = 0;
	switch (reg)
	{
	case 'A':
		left = AF.getLeftRegister() & 0xF0;
		rigth = AF.getLeftRegister() & 0xF;
		value = ((left >> 4) | (rigth << 4));
		AF.setLeftRegister(value);
		break;
	case 'B':
		left = BC.getLeftRegister() & 0xF0;
		rigth = BC.getLeftRegister() & 0xF;
		value = ((left >> 4) | (rigth << 4));
		BC.setLeftRegister(value);
		break;
	case 'C':
		left = BC.getRigthRegister() & 0xF0;
		rigth = BC.getRigthRegister() & 0xF;
		value = ((left >> 4) | (rigth << 4));
		BC.setRigthRegister(value);
		break;
	case 'D':
		left = DE.getLeftRegister() & 0xF0;
		rigth = DE.getLeftRegister() & 0xF;
		value = ((left >> 4) | (rigth << 4));
		DE.setLeftRegister(value);
		break;
	case 'E':
		left = DE.getRigthRegister() & 0xF0;
		rigth = DE.getRigthRegister() & 0xF;
		value = ((left >> 4) | (rigth << 4));
		DE.setRigthRegister(value);
		break;
	case 'H':
		left = HL.getLeftRegister() & 0xF0;
		rigth = HL.getLeftRegister() & 0xF;
		value = ((left >> 4) | (rigth << 4));
		HL.setLeftRegister(value);
		break;
	case 'L':
		left = HL.getRigthRegister() & 0xF0;
		rigth = HL.getRigthRegister() & 0xF;
		value = ((left >> 4) | (rigth << 4));
		HL.setRigthRegister(value);
		break;
	}
	AF.clearFlag('N');
	AF.clearFlag('C');
	AF.clearFlag('H');
	(value == 0) ? AF.setFlag('Z') : AF.clearFlag('Z');
}

void Cpu::SWAP_16()
{
	uint8_t left = 0, rigth = 0, value = mmu->read_ram(HL.get());
	left = (value & 0xF0);
	rigth = (value & 0xF);
	value = ((left >> 4) | (rigth << 4));
	mmu->write_ram(HL.get(), value);
	(value == 0) ? AF.setFlag('Z') : AF.clearFlag('Z');
	AF.clearFlag('N');
	AF.clearFlag('H');
	AF.clearFlag('C');
}

void Cpu::BIT_b_r(int bit, char reg)
{
	if (bit > 7 || bit < 0)
	{
		cout << "Bit to check out of bound" << endl;
		exit(3);
	}
	uint8_t value = 0;
	switch (reg)
	{
	case 'A':
		value = AF.getLeftRegister();
		break;
	case 'F':
		value = AF.getRigthRegister();
		break;
	case 'B':
		value = BC.getLeftRegister();
		break;
	case 'C':
		value = BC.getRigthRegister();
		break;
	case 'D':
		value = DE.getLeftRegister();
		break;
	case 'E':
		value = DE.getRigthRegister();
		break;
	case 'H':
		value = HL.getLeftRegister();
		break;
	case 'L':
		value = HL.getRigthRegister();
		break;
	default:
		cout << "Flag error.";
		exit(0);
		break;
	}
	(value & (1 << bit)) ? AF.clearFlag('Z') : AF.setFlag('Z');
	AF.clearFlag('N');
	AF.setFlag('H');
}

void Cpu::BIT_b_16(int bit)
{
	if (bit > 7 || bit < 0)
	{
		cout << "Bit to check out of bound" << endl;
		exit(3);
	}
	uint8_t value = mmu->read_ram(HL.get());
	(value & (1 << bit)) ? AF.clearFlag('Z') : AF.setFlag('Z');
	AF.clearFlag('N');
	AF.setFlag('H');
}

void Cpu::SET_b_r(int bit, char reg)
{
	if (bit > 7 || bit < 0)
	{
		cout << "Bit to check out of bound" << endl;
		exit(3);
	}
	uint8_t value = (1 << bit);
	switch (reg)
	{
	case 'A':
		AF.setLeftRegister(AF.getLeftRegister() | value);
		break;
	case 'F':
		AF.setRigthRegister(AF.getRigthRegister() | value);
		break;
	case 'B':
		BC.setLeftRegister(BC.getLeftRegister() | value);
		break;
	case 'C':
		BC.setRigthRegister(BC.getRigthRegister() | value);
		break;
	case 'D':
		DE.setLeftRegister(DE.getLeftRegister() | value);
		break;
	case 'E':
		DE.setRigthRegister(DE.getRigthRegister() | value);
		break;
	case 'H':
		HL.setLeftRegister(HL.getLeftRegister() | value);
		break;
	case 'L':
		HL.setRigthRegister(HL.getRigthRegister() | value);
		break;
	default:
		cout << "Flag error.";
		exit(0);
		break;
	}
}

void Cpu::SET_b_16(int bit)
{
	if (bit > 7 || bit < 0)
	{
		cout << "Bit to check out of bound" << endl;
		exit(3);
	}
	uint8_t value = mmu->read_ram(HL.get());
	mmu->write_ram(HL.get(), (value | (1 << bit)));
}

void Cpu::RES_b_r(int bit, char reg)
{
	if (bit > 7 || bit < 0)
	{
		cout << "Bit to check out of bound" << endl;
		exit(3);
	}
	uint8_t value = (1 << bit);
	switch (reg)
	{
	case 'A':
		AF.setLeftRegister(AF.getLeftRegister() & ~value);
		break;
	case 'B':
		BC.setLeftRegister(BC.getLeftRegister() & ~value);
		break;
	case 'C':
		BC.setRigthRegister(BC.getRigthRegister() & ~value);
		break;
	case 'D':
		DE.setLeftRegister(DE.getLeftRegister() & ~value);
		break;
	case 'E':
		DE.setRigthRegister(DE.getRigthRegister() & ~value);
		break;
	case 'H':
		HL.setLeftRegister(HL.getLeftRegister() & ~value);
		break;
	case 'L':
		HL.setRigthRegister(HL.getRigthRegister() & ~value);
		break;
	default:
		cout << "Flag error.";
		exit(0);
		break;
	}
}

void Cpu::RES_b_16(int bit)
{
	if (bit > 7 || bit < 0)
	{
		cout << "Bit to check out of bound" << endl;
		exit(3);
	}
	uint8_t value = ~(1 << bit);
	mmu->write_ram(HL.get(), mmu->read_ram(HL.get()) & value);
}

void Cpu::JP_16(uint16_t adress)
{
	uint16_t leftValue = mmu->read_ram(adress + 1);
	uint8_t rigthValue = mmu->read_ram(adress);
	PC = ((leftValue << 8) | rigthValue);
}

void Cpu::JP_c_16(string condition, uint16_t adress)
{
	if (condition == "Z")
		(AF.getFlag('Z')) ? PC = adress : PC += 3;
	else if (condition == "NZ")
		(!AF.getFlag('Z')) ? PC = adress : PC += 3;
	else if (condition == "C")
		(AF.getFlag('C')) ? PC = adress : PC += 3;
	else if (condition == "NC")
		(!AF.getFlag('C')) ? PC = adress : PC += 3;
}

void Cpu::JR_e(uint8_t value)
{
	PC += 2;
	PC += (int8_t)value;
}

void Cpu::JR_C_e(uint8_t value)
{
	PC += 2;
	if (AF.getFlag('C'))
		(PC += (int8_t)value);
}

void Cpu::JR_NC_e(uint8_t value)
{
	PC += 2;
	if (!AF.getFlag('C'))
		(PC += (int8_t)value);
}

void Cpu::JR_Z_e(uint8_t value)
{
	PC += 2;
	if (AF.getFlag('Z'))
		(PC += (int8_t)value);
}

void Cpu::JR_NZ_e(uint8_t value)
{
	PC += 2;
	if (!AF.getFlag('Z'))
		(PC += (int8_t)value);
}

void Cpu::JP_16()
{
	PC = HL.get();
}

void Cpu::CALL()
{
	uint8_t leftValue = mmu->read_ram(PC + 2);
	uint8_t rightValue = mmu->read_ram(PC + 1);
	SP--;
	mmu->write_ram(SP, (((PC + 3) & 0xFF00) >> 8));
	SP--;
	mmu->write_ram(SP, (uint8_t)((PC + 3) & 0xFF));
	PC = ((leftValue << 8) | rightValue);
}

void Cpu::CALL_c(string condition)
{
	uint8_t leftValue = mmu->read_ram(PC + 2);
	uint8_t rightValue = mmu->read_ram(PC + 1);
	if (condition == "Z")
	{
		if (AF.getFlag('Z'))
		{

			SP--;
			mmu->write_ram(SP, (((PC + 3) & 0xFF00) >> 8));
			SP--;
			mmu->write_ram(SP, (uint8_t)((PC + 3) & 0xFF));
			PC = ((leftValue << 8) | rightValue);
			return;
		}
		else
		{
			PC += 3;
			return;
		}
	}

	else if (condition == "NZ")
	{
		if (!AF.getFlag('Z'))
		{

			SP--;
			mmu->write_ram(SP, (((PC + 3) & 0xFF00) >> 8));
			SP--;
			mmu->write_ram(SP, (uint8_t)((PC + 3) & 0xFF));
			PC = ((leftValue << 8) | rightValue);
			return;
		}
		else
		{
			PC += 3;
			return;
		}
	}
	else if (condition == "C")
	{
		if (AF.getFlag('C'))
		{

			SP--;
			mmu->write_ram(SP, (((PC + 3) & 0xFF00) >> 8));
			SP--;
			mmu->write_ram(SP, (uint8_t)((PC + 3) & 0xFF));
			PC = ((leftValue << 8) | rightValue);
			return;
		}
		else
		{
			PC += 3;
			return;
		}
	}
	else if (condition == "NC")
	{
		if (!AF.getFlag('C'))
		{

			SP--;
			mmu->write_ram(SP, (((PC + 3) & 0xFF00) >> 8));
			SP--;
			mmu->write_ram(SP, (uint8_t)((PC + 3) & 0xFF));
			PC = ((leftValue << 8) | rightValue);
			return;
		}
		else
		{
			PC += 3;
			return;
		}
	}
}

void Cpu::RET()
{
	uint8_t firstByte = mmu->read_ram(SP + 1);
	uint16_t secondByte = (mmu->read_ram(SP));
	SP += 2;
	PC = ((firstByte << 8) | secondByte);
}

void Cpu::RET_c(string condition)
{
	if (condition == "Z")
	{
		if (AF.getFlag('Z'))
		{
			uint16_t leftNibble = (mmu->read_ram(SP + 1) << 8);
			uint8_t rightNibble = mmu->read_ram(SP);
			SP += 2;
			PC = (leftNibble | rightNibble);
			return;
		}
		else
		{
			PC++;
			return;
		}
	}
	
	if (condition == "NZ")
	{
		if (!AF.getFlag('Z'))
		{
			uint16_t leftNibble = (mmu->read_ram(SP + 1) << 8);
			uint8_t rightNibble = mmu->read_ram(SP);
			SP += 2;
			PC = (leftNibble | rightNibble);
			return;
		}
		else
		{
			PC++;
			return;
		}
	}
	if (condition == "C")
	{
		if (AF.getFlag('C'))
		{
			uint16_t leftNibble = (mmu->read_ram(SP + 1) << 8);
			uint8_t rightNibble = mmu->read_ram(SP);
			SP += 2;
			PC = (leftNibble | rightNibble);
			return;
		}
		else
		{
			PC++;
			return;
		}
	}
	if (condition == "NC")
	{
		if (!AF.getFlag('C'))
		{
			uint16_t leftNibble = (mmu->read_ram(SP + 1) << 8);
			uint8_t rightNibble = mmu->read_ram(SP);
			SP += 2;
			PC = (leftNibble | rightNibble);
			return;
		}
		else
		{
			PC++;
			return;
		}
	}
}

void Cpu::RETI() //TODO Unkown behaviour (may produce bugs)
{
	interrupt_enabled = true;
	uint16_t leftNibble = (mmu->read_ram(SP + 1) << 8);
	uint8_t rightNibble = mmu->read_ram(SP);
	SP += 2;
	PC = (leftNibble | rightNibble);
}

void Cpu::RST_p(uint16_t p)
{
	SP--;
	mmu->write_ram(SP, (((PC+1) & 0xFF00) >> 8));
	SP--;
	mmu->write_ram(SP, ((PC+1) & 0x00FF));
	PC = p;
}

bool Cpu::checkAddHalfCarry(uint8_t a, uint8_t b, uint8_t c)
{
	return ((((a & 0xF) + (b & 0xF)) & 0x10) == 0x10);
}

bool Cpu::checkAddHalfCarry(uint16_t a, uint16_t b)
{
	return ((a & 0xFF) + (b & 0xFF) >= 0x100 );
}


bool Cpu::checkAddHalfCarry(uint16_t a, uint16_t b, uint16_t c)
{
	return ((((a & 0xFFF) + (b & 0xFFF)) & 0x800) == 0x800);
}

bool Cpu::checkSubHalfCarry(uint8_t a, uint8_t b)
{
	return (((a & 0xF) - (b & 0xF)) < 0);
}

bool Cpu::checkSubHalfCarry(uint16_t a, uint16_t b)
{
	return (((a & 0xF000) - (b & 0xF000)) < 0);
}

bool Cpu::checkSubCarry(uint16_t a, uint16_t b)
{
	return (((a & 0x8000) - (b & 0x8000)) < 0);
}

bool Cpu::checkAddCarry(uint8_t a, uint8_t b, uint8_t c)
{
	return ((uint16_t)a + (uint16_t)b >= 0x100);
}

bool Cpu::checkAddCarry(uint16_t a, uint16_t b, uint16_t c)
{
	bool firstBit = a & 0x8000, secondtBit = b & 0x8000;
	return (firstBit & secondtBit) ? true : false;
}

uint16_t Cpu::next2bytes(uint16_t adress)
{
	uint8_t leftValue = mmu->read_ram(adress + 1);
	uint16_t value = ((leftValue << 8) | (mmu->read_ram(adress)));
	return value;
}

bool Cpu::getInterruptStatus()
{
	return interrupt_enabled;
}

long Cpu::step()
{
	if ((mmu->read_ram(0xFF50) == 0x1 && mmu->isInBios()))
		mmu->disableBios();

	if (!halted)
	{
     	fetch();
     	execute();
     	return cycle;
	}
	else
	{
		if ((mmu->read_ram(0xFF0F) & 0xF) > 0)
		{
			halted = false;
			PC++;
		}
		return 4;
	}
}

void Cpu::fetch()
{
	currentOpcode = mmu->read_ram(PC);
}

void Cpu::execute()
{
	uint16_t prevPC = PC;
	switch (currentOpcode)
	{
	case 0x0:
		PC++;
		cycle = 4;
		break;
	case 0x01:
		LD_rr_16(PC + 1, "BC");
		cycle = 12;
		PC += 3;
		break;
	case 0x02:
		LD_16_r(BC.get(), 'A');
		cycle = 8;
		PC++;
		break;
	case 0x03:
		INC_rr("BC");
		cycle = 8;
		PC++;
		break;
	case 0x04:
		INC_r('B');
		cycle = 4;
		PC++;
		break;
	case 0x05:
		DEC_r('B');
		cycle = 4;
		PC++;
		break;
	case 0x06:
		LD_r_n(mmu->read_ram(PC + 1), 'B');
		cycle = 8;
		PC += 2;
		break;
	case 0x07:
		RLCA();
		PC++;
		cycle = 4;
		break;
	case 0x08:
		LD_16_rr(next2bytes(PC+1), "SP");
		PC += 3;
		cycle = 20;
		break;
	case 0x09:
		ADD_HL_rr("BC");
		cycle = 8;
		PC++;
		break;
	case 0x0A:
		LD_r_16(BC.get(), 'A');
		cycle = 8;
		PC++;
		break;
	case 0x0B:
		DEC_rr("BC");
		cycle = 8;
		PC++;
		break;
	case 0x0C:
		INC_r('C');
		cycle = 4;
		PC++;
		break;
	case 0x0D:
		DEC_r('C');
		cycle = 4;
		PC++;
		break;
	case 0x0E:
		LD_r_n(mmu->read_ram(PC + 1), 'C');
		cycle = 8;
		PC += 2;
		break;
	case 0x0F:
		RRCA();
		cycle = 4;
		PC++;
		break;
	case 0x10:
		cycle = 4;
		break;
	case 0x11:
		LD_rr_16(PC + 1, "DE");
		cycle = 12;
		PC += 3;
		break;
	case 0x12:
		LD_16_r(DE.get(), 'A');
		cycle = 8;
		PC++;
		break;
	case 0x13:
		INC_rr("DE");
		cycle = 8;
		PC++;
		break;
	case 0x14:
		INC_r('D');
		cycle = 4;
		PC++;
		break;
	case 0x15:
		DEC_r('D');
		cycle = 4;
		PC++;
		break;
	case 0x16:
		LD_r_n(mmu->read_ram(PC + 1), 'D');
		PC += 2;
		cycle = 8;
		break;
	case 0x17:
		RLA();
		PC++;
		cycle = 4;
		break;
	case 0x18:
		JR_e(mmu->read_ram(PC + 1));
		cycle = 12;
		break;
	case 0x19:
		ADD_HL_rr("DE");
		cycle = 8;
		PC++;
		break;
	case 0x1A:
		LD_r_16(DE.get(), 'A');
		cycle = 8;
		PC++;
		break;
	case 0x1B:
		DEC_rr("DE");
		cycle = 8;
		PC++;
		break;
	case 0x1C:
		INC_r('E');
		cycle = 4;
		PC++;
		break;
	case 0x1D:
		DEC_r('E');
		cycle = 4;
		PC++;
		break;
	case 0x1E:
		LD_r_n(mmu->read_ram(PC + 1), 'E');
		PC += 2;
		cycle = 8;
		break;
	case 0x1F:
		RRA();
		cycle = 4;
		PC++;
		break;
	case 0x20:
		JR_NZ_e(mmu->read_ram(PC + 1));
		(AF.getFlag('Z')) ? cycle = 12 : cycle = 16;
		break;
	case 0x21:
		LD_rr_nn(next2bytes(PC + 1), "HL");
		PC += 3;
		cycle = 12;
		break;
	case 0x22:
		LD_16_r(HL.get(), 'A');
		INC_rr("HL");
		cycle = 8;
		PC++;
		break;
	case 0x23:
		INC_rr("HL");
		cycle = 8;
		PC++;
		break;
	case 0x24:
		INC_r('H');
		cycle = 4;
		PC++;
		break;
	case 0x25:
		DEC_r('H');
		cycle = 4;
		PC++;
		break;
	case 0x26:
		LD_r_n(mmu->read_ram(PC + 1), 'H');
		cycle = 8;
		PC += 2;
		break;
	case 0x27:
		DAA();
		PC++;
		cycle = 4;
		break;
	case 0x28:
		JR_Z_e(mmu->read_ram(PC + 1));
		cycle = 12;
		break;
	case 0x29:
		ADD_HL_rr("HL");
		PC++;
		cycle = 8;
		break;
	case 0x2A:
		LD_r_n(mmu->read_ram(HL.get()), 'A');
		INC_rr("HL");
		cycle = 8;
		PC++;
		break;
	case 0x2B:
		DEC_rr("HL");
		cycle = 8;
		PC++;
		break;
	case 0x2C:
		INC_r('L');
		cycle = 4;
		PC++;
		break;
	case 0x2D:
		DEC_r('L');
		cycle = 4;
		PC++;
		break;
	case 0x2E:
		LD_r_n(mmu->read_ram(PC + 1), 'L');
		PC += 2;
		cycle = 8;
		break;
	case 0x2F:
		CPL();
		PC++;
		cycle = 4;
		break;
	case 0x30:
		JR_NC_e(mmu->read_ram(PC + 1));
		cycle = 12;
		break;
	case 0x31:
		LD_rr_nn(next2bytes(PC + 1), "SP");
		PC += 3;
		cycle = 12;
		break;
	case 0x32:
		LD_16_r(HL.get(), 'A');
		DEC_rr("HL");
		cycle = 8;
		PC++;
		break;
	case 0x33:
		INC_rr("SP");
		cycle = 8;
		PC++;
		break;
	case 0x34:
		INC_16();
		cycle = 12;
		PC++;
		break;
	case 0x35:
		DEC_16();
		cycle = 12;
		PC++;
		break;
	case 0x36: //TODO may be innacurate
		LD_16_n(HL.get(), mmu->read_ram(PC + 1));
		PC += 2;
		cycle = 12;
		break;
	case 0x37:
		SCF();
		cycle = 4;
		PC++;
		break;
	case 0x38:
		JR_C_e(mmu->read_ram(PC + 1));
		cycle = 12;
		break;
	case 0x39:
		ADD_HL_rr("SP");
		cycle = 8;
		PC++;
		break;
	case 0x3A:
		LD_r_16(HL.get(), 'A');
		DEC_rr("HL");
		cycle = 8;
		PC++;
		break;
	case 0x3B:
		DEC_rr("SP");
		cycle = 8;
		PC++;
		break;
	case 0x3C:
		INC_r('A');
		cycle = 4;
		PC++;
		break;
	case 0x3D:
		DEC_r('A');
		cycle = 4;
		PC++;
		break;
	case 0x3E:
		LD_r_n(mmu->read_ram(PC + 1), 'A');
		PC += 2;
		cycle = 8;
		break;
	case 0x3F:
		CCF();
		cycle = 4;
		PC++;
		break;
	case 0x40:
		LD_r_r('B', 'B');
		cycle = 4;
		PC++;
		break;
	case 0x41:
		LD_r_r('C', 'B');
		cycle = 4;
		PC++;
		break;
	case 0x42:
		LD_r_r('D', 'B');
		cycle = 4;
		PC++;
		break;
	case 0x43:
		LD_r_r('E', 'B');
		cycle = 4;
		PC++;
		break;
	case 0x44:
		LD_r_r('H', 'B');
		cycle = 4;
		PC++;
		break;
	case 0x45:
		LD_r_r('L', 'B');
		cycle = 4;
		PC++;
		break;
	case 0x46:
		LD_r_16(HL.get(), 'B');
		cycle = 8;
		PC++;
		break;
	case 0x47:
		LD_r_r('A', 'B');
		cycle = 4;
		PC++;
		break;
	case 0x48:
		LD_r_r('B', 'C');
		cycle = 4;
		PC++;
		break;
	case 0x49:
		LD_r_r('C', 'C');
		cycle = 4;
		PC++;
		break;
	case 0x4A:
		LD_r_r('D', 'C');
		cycle = 4;
		PC++;
		break;
	case 0x4B:
		LD_r_r('E', 'C');
		cycle = 4;
		PC++;
		break;
	case 0x4C:
		LD_r_r('H', 'C');
		cycle = 4;
		PC++;
		break;
	case 0x4D:
		LD_r_r('L', 'C');
		cycle = 4;
		PC++;
		break;
	case 0x4E:
		LD_r_16(HL.get(), 'C');
		cycle = 8;
		PC++;
		break;
	case 0x4F:
		LD_r_r('A', 'C');
		PC++;
		cycle = 4;
		break;
	case 0x50:
		LD_r_r('B', 'D');
		cycle = 4;
		PC++;
		break;
	case 0x51:
		LD_r_r('C', 'D');
		cycle = 4;
		PC++;
		break;
	case 0x52:
		LD_r_r('D', 'D');
		cycle = 4;
		PC++;
		break;
	case 0x53:
		LD_r_r('E', 'D');
		cycle = 4;
		PC++;
		break;
	case 0x54:
		LD_r_r('H', 'D');
		cycle = 4;
		PC++;
		break;
	case 0x55:
		LD_r_r('L', 'D');
		cycle = 4;
		PC++;
		break;
	case 0x56:
		LD_r_16(HL.get(), 'D');
		cycle = 8;
		PC++;
		break;
	case 0x57:
		LD_r_r('A', 'D');
		PC++;
		cycle = 4;
		break;
	case 0x58:
		LD_r_r('B', 'E');
		cycle = 4;
		PC++;
		break;
	case 0x59:
		LD_r_r('C', 'E');
		cycle = 4;
		PC++;
		break;
	case 0x5A:
		LD_r_r('D', 'E');
		cycle = 4;
		PC++;
		break;
	case 0x5B:
		LD_r_r('E', 'E');
		cycle = 4;
		PC++;
		break;
	case 0x5C:
		LD_r_r('H', 'E');
		cycle = 4;
		PC++;
		break;
	case 0x5D:
		LD_r_r('L', 'E');
		cycle = 4;
		PC++;
		break;
	case 0x5E:
		LD_r_16(HL.get(), 'E');
		cycle = 8;
		PC++;
		break;
	case 0x5F:
		LD_r_r('A', 'E');
		cycle = 4;
		PC++;
		break;
	case 0x60:
		LD_r_r('B', 'H');
		cycle = 4;
		PC++;
		break;
	case 0x61:
		LD_r_r('C', 'H');
		cycle = 4;
		PC++;
		break;
	case 0x62:
		LD_r_r('D', 'H');
		cycle = 4;
		PC++;
		break;
	case 0x63:
		LD_r_r('E', 'H');
		cycle = 4;
		PC++;
		break;
	case 0x64:
		LD_r_r('H', 'H');
		cycle = 4;
		PC++;
		break;
	case 0x65:
		LD_r_r('L', 'H');
		cycle = 4;
		PC++;
		break;
	case 0x66:
		LD_r_16(HL.get(), 'H');
		cycle = 8;
		PC++;
		break;
	case 0x67:
		LD_r_r('A', 'H');
		cycle = 4;
		PC++;
		break;
	case 0x68:
		LD_r_r('B', 'L');
		cycle = 4;
		PC++;
		break;
	case 0x69:
		LD_r_r('C', 'L');
		cycle = 4;
		PC++;
		break;
	case 0x6A:
		LD_r_r('D', 'L');
		cycle = 4;
		PC++;
		break;
	case 0x6B:
		LD_r_r('E', 'L');
		cycle = 4;
		PC++;
		break;
	case 0x6C:
		LD_r_r('H', 'L');
		cycle = 4;
		PC++;
		break;
	case 0x6D:
		LD_r_r('L', 'L');
		cycle = 4;
		PC++;
		break;
	case 0x6E:
		LD_r_16(HL.get(), 'L');
		cycle = 8;
		PC++;
		break;
	case 0x6F:
		LD_r_r('A', 'L');
		cycle = 4;
		PC++;
		break;
	case 0x70:
		LD_16_r(HL.get(), 'B');
		cycle = 4;
		PC++;
		break;
	case 0x71:
		LD_16_r(HL.get(), 'C');
		cycle = 4;
		PC++;
		break;
	case 0x72:
		LD_16_r(HL.get(), 'D');
		cycle = 8;
		PC++;
		break;
	case 0x73:
		LD_16_r(HL.get(), 'E');
		cycle = 8;
		PC++;
		break;
	case 0x74:
		LD_16_r(HL.get(), 'H');
		cycle = 8;
		PC++;
		break;
	case 0x75:
		LD_16_r(HL.get(), 'L');
		cycle = 8;
		PC++;
		break;
	case 0x76:
		HALT();
		PC++;
		cycle = 4;
		break;
	case 0x77:
		LD_16_r(HL.get(), 'A');
		cycle = 8;
		PC++;
		break;
	case 0x78:
		LD_r_r('B', 'A');
		cycle = 4;
		PC++;
		break;
	case 0x79:
		LD_r_r('C', 'A');
		cycle = 4;
		PC++;
		break;
	case 0x7A:
		LD_r_r('D', 'A');
		cycle = 4;
		PC++;
		break;
	case 0x7B:
		LD_r_r('E', 'A');
		cycle = 4;
		PC++;
		break;
	case 0x7C:
		LD_r_r('H', 'A');
		cycle = 4;
		PC++;
		break;
	case 0x7D:
		LD_r_r('L', 'A');
		cycle = 4;
		PC++;
		break;
	case 0x7E:
		LD_r_16(HL.get(), 'A');
		cycle = 8;
		PC++;
		break;
	case 0x7F:
		LD_r_r('A', 'A');
		cycle = 4;
		PC++;
		break;
	case 0x80:
		ADD_A_r('B');
		cycle = 4;
		PC++;
		break;
	case 0x81:
		ADD_A_r('C');
		cycle = 4;
		PC++;
		break;
	case 0x82:
		ADD_A_r('D');
		cycle = 4;
		PC++;
		break;
	case 0x83:
		ADD_A_r('E');
		cycle = 4;
		PC++;
		break;
	case 0x84:
		ADD_A_r('H');
		cycle = 4;
		PC++;
		break;
	case 0x85:
		ADD_A_r('L');
		cycle = 4;
		PC++;
		break;
	case 0x86:
		ADD_A_16();
		cycle = 8;
		PC++;
		break;
	case 0x87:
		ADD_A_r('A');
		cycle = 4;
		PC++;
		break;
	case 0x88:
		ADC_A_r('B');
		cycle = 4;
		PC++;
		break;
	case 0x89:
		ADC_A_r('C');
		cycle = 4;
		PC++;
		break;
	case 0x8A:
		ADC_A_r('D');
		cycle = 4;
		PC++;
		break;
	case 0x8B:
		ADC_A_r('E');
		cycle = 4;
		PC++;
		break;
	case 0x8C:
		ADC_A_r('H');
		cycle = 4;
		PC++;
		break;
	case 0x8D:
		ADC_A_r('L');
		cycle = 4;
		PC++;
		break;
	case 0x8E:
		ADC_A_16();
		cycle = 8;
		PC++;
		break;
	case 0x8F:
		ADC_A_r('A');
		cycle = 4;
		PC++;
		break;
	case 0x90:
		SUB_r('B');
		cycle = 4;
		PC++;
		break;
	case 0x91:
		SUB_r('C');
		cycle = 4;
		PC++;
		break;
	case 0x92:
		SUB_r('D');
		cycle = 4;
		PC++;
		break;
	case 0x93:
		SUB_r('E');
		cycle = 4;
		PC++;
		break;
	case 0x94:
		SUB_r('H');
		cycle = 4;
		PC++;
		break;
	case 0x95:
		SUB_r('L');
		cycle = 4;
		PC++;
		break;
	case 0x96:
		SUB_16();
		cycle = 8;
		PC++;
		break;
	case 0x97:
		SUB_r('A');
		cycle = 4;
		PC++;
		break;
	case 0x98:
		SBC_r('B');
		cycle = 4;
		PC++;
		break;
	case 0x99:
		SBC_r('C');
		cycle = 4;
		PC++;
		break;
	case 0x9A:
		SBC_r('D');
		cycle = 4;
		PC++;
		break;
	case 0x9B:
		SBC_r('E');
		cycle = 4;
		PC++;
		break;
	case 0x9C:
		SBC_r('H');
		cycle = 4;
		PC++;
		break;
	case 0x9D:
		SBC_r('L');
		cycle = 4;
		PC++;
		break;
	case 0x9E:
		SBC_16();
		cycle = 8;
		PC++;
		break;
	case 0x9F:
		SBC_r('A');
		cycle = 4;
		PC++;
		break;
	case 0xA0:
		AND_r('B');
		cycle = 4;
		PC++;
		break;
	case 0xA1:
		AND_r('C');
		cycle = 4;
		PC++;
		break;
	case 0xA2:
		AND_r('D');
		cycle = 4;
		PC++;
		break;
	case 0xA3:
		AND_r('E');
		cycle = 4;
		PC++;
		break;
	case 0xA4:
		AND_r('H');
		cycle = 4;
		PC++;
		break;
	case 0xA5:
		AND_r('L');
		cycle = 4;
		PC++;
		break;
	case 0xA6:
		AND_16();
		cycle = 8;
		PC++;
		break;
	case 0xA7:
		AND_r('A');
		cycle = 4;
		PC++;
		break;
	case 0xA8:
		XOR_r('B');
		cycle = 4;
		PC++;
		break;
	case 0xA9:
		XOR_r('C');
		cycle = 4;
		PC++;
		break;
	case 0xAA:
		XOR_r('D');
		cycle = 4;
		PC++;
		break;
	case 0xAB:
		XOR_r('E');
		cycle = 4;
		PC++;
		break;
	case 0xAC:
		XOR_r('H');
		cycle = 4;
		PC++;
		break;
	case 0xAD:
		XOR_r('L');
		cycle = 4;
		PC++;
		break;
	case 0xAE:
		XOR_16();
		cycle = 8;
		PC++;
		break;
	case 0xAF:
		XOR_r('A');
		cycle = 4;
		PC++;
		break;
	case 0xB0:
		OR_r('B');
		cycle = 4;
		PC++;
		break;
	case 0xB1:
		OR_r('C');
		cycle = 4;
		PC++;
		break;
	case 0xB2:
		OR_r('D');
		cycle = 4;
		PC++;
		break;
	case 0xB3:
		OR_r('E');
		cycle = 4;
		PC++;
		break;
	case 0xB4:
		OR_r('H');
		cycle = 4;
		PC++;
		break;
	case 0xB5:
		OR_r('L');
		cycle = 4;
		PC++;
		break;
	case 0xB6:
		OR_16();
		cycle = 8;
		PC++;
		break;
	case 0xB7:
		OR_r('A');
		cycle = 4;
		PC++;
		break;
	case 0xB8:
		CP_r('B');
		cycle = 4;
		PC++;
		break;
	case 0xB9:
		CP_r('C');
		cycle = 4;
		PC++;
		break;
	case 0xBA:
		CP_r('D');
		cycle = 4;
		PC++;
		break;
	case 0xBB:
		CP_r('E');
		cycle = 4;
		PC++;
		break;
	case 0xBC:
		CP_r('H');
		cycle = 4;
		PC++;
		break;
	case 0xBD:
		CP_r('L');
		cycle = 4;
		PC++;
		break;
	case 0xBE:
		CP_16();
		cycle = 8;
		PC++;
		break;
	case 0xBF:
		CP_r('A');
		cycle = 4;
		PC++;
		break;
	case 0xC0:
		RET_c("NZ");
		cycle = 20;
		break;
	case 0xC1:
		POP("BC");
		PC++;
		cycle = 12;
		break;
	case 0xC2:
		JP_c_16("NZ", next2bytes(PC + 1));
		cycle = 16;
		break;
	case 0xC3:
		JP_16(PC + 1);
		cycle = 16;
		break;
	case 0xC4:
		CALL_c("NZ");
		cycle = 24;
		break;
	case 0xC5:
		PUSH("BC");
		PC++;
		cycle = 16;
		break;
	case 0xC6:
		ADD_A_n(mmu->read_ram(PC + 1));
		cycle = 8;
		PC += 2;
		break;
	case 0xC7:
		RST_p(0x00);
		cycle = 16;
		break;
	case 0xC8:
		RET_c("Z");
		cycle = 20;
		break;
	case 0xC9:
		RET();
		cycle = 16;
		break;
	case 0xCA:
		JP_c_16("Z", next2bytes(PC + 1));
		cycle = 16;
		break;
		// REFER TO CB PREFIX FOR SPECIAL INSTRUCTIONS FURTHER BELOW
	case 0xCC:
		CALL_c("Z");
		cycle = 24;
		break;
	case 0xCD:
		CALL();
		cycle = 24;
		break;
	case 0xCE:
		ADC_A_n(mmu->read_ram(PC + 1));
		cycle = 8;
		PC+=2;
		break;
	case 0xCF:
		RST_p(0x08);
		cycle = 16;
		break;
	case 0xD0:
		RET_c("NC");
		cycle = 20;
		break;
	case 0xD1:
		POP("DE");
		cycle = 12;
		PC++;
		break;
	case 0xD2:
		JP_c_16("NC", next2bytes(PC + 1));
		cycle = 16;
		break;
	case 0xD4:
		CALL_c("NC");
		cycle = 24;
		break;
	case 0xD5:
		PUSH("DE");
		cycle = 16;
		PC++;
		break;
	case 0xD6:
		SUB_n(mmu->read_ram(PC + 1));
		cycle = 8;
		PC += 2;
		break;
	case 0xD7:
		RST_p(0x10);
		cycle = 16;
		break;
	case 0xD8:
		RET_c("C");
		cycle = 20;
		break;
	case 0xD9:
		RETI();
		cycle = 16;
		break;
	case 0xDA:
		JP_c_16("C", next2bytes(PC + 1));
		cycle = 16;
		break;
	case 0xDC:
		CALL_c("C");
		cycle = 24;
		break;
	case 0xDE:
		SBC_n(mmu->read_ram(PC+1));
		cycle = 8;
		PC += 2;
		break;
	case 0xDF:
		RST_p(0x18);
		cycle = 16;
		break;
	case 0xE0:
		mmu->write_ram(mmu->read_ram(PC + 1) + 0xFF00, AF.getLeftRegister());
		cycle = 12;
		PC += 2;
		break;
	case 0xE1:
		POP("HL");
		cycle = 12;
		PC++;
		break;
	case 0xE2:
		mmu->write_ram(0xFF00 + BC.getRigthRegister(), AF.getLeftRegister());
		cycle = 8;
		PC++;
		break;
	case 0xE5:
		PUSH("HL");
		cycle = 16;
		PC++;
		break;
	case 0xE6:
		AND_n(mmu->read_ram(PC + 1));
		PC += 2;
		cycle = 8;
		break;
	case 0xE7:
		RST_p(0x20);
		cycle = 16;
		break;
	case 0xE8:
		ADD_SP_n();
		PC += 2;
		cycle = 16;
		break;
	case 0xE9:
		JP_16();
		cycle = 4;
		break;
	case 0xEA:
		LD_16_r(next2bytes(PC + 1), 'A');
		cycle = 16;
		PC += 3;
		break;
	case 0xEE:
		XOR_n(mmu->read_ram(PC + 1));
		PC += 2;
		cycle = 8;
		break;
	case 0xEF:
		RST_p(0x28);
		cycle = 16;
		break;
	case 0xF0:
		AF.setLeftRegister(mmu->read_ram(mmu->read_ram(PC + 1) + 0xFF00));
		cycle = 12;
		PC += 2;
		break;
	case 0xF1:
		POP("AF");
		cycle = 12;
		PC++;
		break;
	case 0xF2:
		AF.setLeftRegister(mmu->read_ram(BC.getRigthRegister() + 0xFF00));
		cycle = 8;
		PC++;
		break;
	case 0xF3:
		interrupt_enabled = false;
		cycle = 4;
		PC++;
		break;
	case 0xF5:
		PUSH("AF");
		cycle = 16;
		PC++;
		break;
	case 0xF6:
		OR_n(mmu->read_ram(PC + 1));
		PC += 2;
		cycle = 8;
		break;
	case 0xF7:
		RST_p(0x30);
		cycle = 16;
		break;
	case 0xF8: 
		LD_HL_SP_n();
		cycle = 12;
		PC += 2;
		break;
	case 0xF9: //TODO may be innacurate
		SP = HL.get();
		cycle = 8;
		PC++;
		break;
	case 0xFA:
		LD_r_16(next2bytes(PC + 1), 'A');
		cycle = 16;
		PC += 3;
		break;
	case 0xFB: //TODO may be innacurate
		interrupt_enabled = true;
		cycle = 4;
		PC++;
		break;
	case 0xFE:
		CP_n(mmu->read_ram(PC + 1));
		cycle = 8;
		PC += 2;
		break;
	case 0xFF:
		RST_p(0x38);
		cycle = 16;
		break;

		/*-----SPECIAL OPCODES-----*/

	case 0xCB:
	{
		prevPC++;
		PC++;
		switch (mmu->read_ram(PC))
		{
		case 0x00:
			RLC_r('B');
			cycle = 8;
			PC++;
			break;
		case 0x01:
			RLC_r('C');
			cycle = 8;
			PC++;
			break;
		case 0x02:
			RLC_r('D');
			cycle = 8;
			PC++;
			break;
		case 0x03:
			RLC_r('E');
			cycle = 8;
			PC++;
			break;
		case 0x04:
			RLC_r('H');
			cycle = 8;
			PC++;
			break;
		case 0x05:
			RLC_r('L');
			cycle = 8;
			PC++;
			break;
		case 0x06:
			RLC_16();
			cycle = 16;
			PC++;
			break;
		case 0x07:
			RLC_r('A');
			cycle = 8;
			PC++;
			break;
		case 0x08:
			RRC_r('B');
			cycle = 8;
			PC++;
			break;
		case 0x09:
			RRC_r('C');
			cycle = 8;
			PC++;
			break;
		case 0x0A:
			RRC_r('D');
			cycle = 8;
			PC++;
			break;
		case 0x0B:
			RRC_r('E');
			cycle = 8;
			PC++;
			break;
		case 0x0C:
			RRC_r('H');
			cycle = 8;
			PC++;
			break;
		case 0x0D:
			RRC_r('L');
			cycle = 8;
			PC++;
			break;
		case 0x0E:
			RRC_16();
			cycle = 16;
			PC++;
			break;
		case 0x0F:
			RRC_r('A');
			cycle = 8;
			PC++;
			break;
		case 0x10:
			RL_r('B');
			cycle = 8;
			PC++;
			break;
		case 0x11:
			RL_r('C');
			cycle = 8;
			PC++;
			break;
		case 0x12:
			RL_r('D');
			cycle = 8;
			PC++;
			break;
		case 0x13:
			RL_r('E');
			cycle = 8;
			PC++;
			break;
		case 0x14:
			RL_r('H');
			cycle = 8;
			PC++;
			break;
		case 0x15:
			RL_r('L');
			cycle = 8;
			PC++;
			break;
		case 0x16:
			RL_16();
			cycle = 16;
			PC++;
			break;
		case 0x17:
			RL_r('A');
			cycle = 8;
			PC++;
			break;
		case 0x18:
			RR_r('B');
			cycle = 8;
			PC++;
			break;
		case 0x19:
			RR_r('C');
			cycle = 8;
			PC++;
			break;
		case 0x1A:
			RR_r('D');
			cycle = 8;
			PC++;
			break;
		case 0x1B:
			RR_r('E');
			cycle = 8;
			PC++;
			break;
		case 0x1C:
			RR_r('H');
			cycle = 8;
			PC++;
			break;
		case 0x1D:
			RR_r('L');
			cycle = 8;
			PC++;
			break;
		case 0x1E:
			RR_16();
			cycle = 16;
			PC++;
			break;
		case 0x1F:
			RR_r('A');
			cycle = 8;
			PC++;
			break;
		case 0x20:
			SLA_r('B');
			cycle = 8;
			PC++;
			break;
		case 0x21:
			SLA_r('C');
			cycle = 8;
			PC++;
			break;
		case 0x22:
			SLA_r('D');
			cycle = 8;
			PC++;
			break;
		case 0x23:
			SLA_r('E');
			cycle = 8;
			PC++;
			break;
		case 0x24:
			SLA_r('H');
			cycle = 8;
			PC++;
			break;
		case 0x25:
			SLA_r('L');
			cycle = 8;
			PC++;
			break;
		case 0x26:
			SLA_16();
			cycle = 16;
			PC++;
			break;
		case 0x27:
			SLA_r('A');
			cycle = 8;
			PC++;
			break;
		case 0x28:
			SRA_r('B');
			cycle = 8;
			PC++;
			break;
		case 0x29:
			SRA_r('C');
			cycle = 8;
			PC++;
			break;
		case 0x2A:
			SRA_r('D');
			cycle = 8;
			PC++;
			break;
		case 0x2B:
			SRA_r('E');
			cycle = 8;
			PC++;
			break;
		case 0x2C:
			SRA_r('H');
			cycle = 8;
			PC++;
			break;
		case 0x2D:
			SRA_r('L');
			cycle = 8;
			PC++;
			break;
		case 0x2E:
			SRA_16();
			cycle = 16;
			PC++;
			break;
		case 0x2F:
			SRA_r('A');
			cycle = 8;
			PC++;
			break;
		case 0x30:
			SWAP_r('B');
			cycle = 8;
			PC++;
			break;
		case 0x31:
			SWAP_r('C');
			cycle = 8;
			PC++;
			break;
		case 0x32:
			SWAP_r('D');
			cycle = 8;
			PC++;
			break;
		case 0x33:
			SWAP_r('E');
			cycle = 8;
			PC++;
			break;
		case 0x34:
			SWAP_r('H');
			cycle = 8;
			PC++;
			break;
		case 0x35:
			SWAP_r('L');
			cycle = 8;
			PC++;
			break;
		case 0x36:
			SWAP_16();
			cycle = 16;
			PC++;
			break;
		case 0x37:
			SWAP_r('A');
			cycle = 8;
			PC++;
			break;
		case 0x38:
			SRL_r('B');
			cycle = 8;
			PC++;
			break;
		case 0x39:
			SRL_r('C');
			cycle = 8;
			PC++;
			break;
		case 0x3A:
			SRL_r('D');
			cycle = 8;
			PC++;
			break;
		case 0x3B:
			SRL_r('E');
			cycle = 8;
			PC++;
			break;
		case 0x3C:
			SRL_r('H');
			cycle = 8;
			PC++;
			break;
		case 0x3D:
			SRL_r('L');
			cycle = 8;
			PC++;
			break;
		case 0x3E:
			SRL_16();
			cycle = 16;
			PC++;
			break;
		case 0x3F:
			SRL_r('A');
			cycle = 8;
			PC++;
			break;
		case 0x40:
			BIT_b_r(0, 'B');
			PC++;
			break;
		case 0x41:
			BIT_b_r(0, 'C');
			cycle = 8;
			PC++;
			break;
		case 0x42:
			BIT_b_r(0, 'D');
			cycle = 8;
			PC++;
			break;
		case 0x43:
			BIT_b_r(0, 'E');
			cycle = 8;
			PC++;
			break;
		case 0x44:
			BIT_b_r(0, 'H');
			cycle = 8;
			PC++;
			break;
		case 0x45:
			BIT_b_r(0, 'L');
			cycle = 8;
			PC++;
			break;
		case 0x46:
			BIT_b_16(0);
			cycle = 16;
			PC++;
			break;
		case 0x47:
			BIT_b_r(0, 'A');
			cycle = 8;
			PC++;
			break;
		case 0x48:
			BIT_b_r(1, 'B');
			cycle = 8;
			PC++;
			break;
		case 0x49:
			BIT_b_r(1, 'C');
			cycle = 8;
			PC++;
			break;
		case 0x4A:
			BIT_b_r(1, 'D');
			cycle = 8;
			PC++;
			break;
		case 0x4B:
			BIT_b_r(1, 'E');
			cycle = 8;
			PC++;
			break;
		case 0x4C:
			BIT_b_r(1, 'H');
			cycle = 8;
			PC++;
			break;
		case 0x4D:
			BIT_b_r(1, 'L');
			cycle = 8;
			PC++;
			break;
		case 0x4E:
			BIT_b_16(1);
			cycle = 8;
			PC++;
			break;
		case 0x4F:
			BIT_b_r(1, 'A');
			cycle = 16;
			PC++;
			break;
		case 0x50:
			BIT_b_r(2, 'B');
			cycle = 8;
			PC++;
			break;
		case 0x51:
			BIT_b_r(2, 'C');
			cycle = 8;
			PC++;
			break;
		case 0x52:
			BIT_b_r(2, 'D');
			cycle = 8;
			PC++;
			break;
		case 0x53:
			BIT_b_r(2, 'E');
			cycle = 8;
			PC++;
			break;
		case 0x54:
			BIT_b_r(2, 'H');
			cycle = 8;
			PC++;
			break;
		case 0x55:
			BIT_b_r(2, 'L');
			cycle = 8;
			PC++;
			break;
		case 0x56:
			BIT_b_16(2);
			cycle = 16;
			PC++;
			break;
		case 0x57:
			BIT_b_r(2, 'A');
			cycle = 8;
			PC++;
			break;
		case 0x58:
			BIT_b_r(3, 'B');
			cycle = 8;
			PC++;
			break;
		case 0x59:
			BIT_b_r(3, 'C');
			cycle = 8;
			PC++;
			break;
		case 0x5A:
			BIT_b_r(3, 'D');
			cycle = 8;
			PC++;
			break;
		case 0x5B:
			BIT_b_r(3, 'E');
			cycle = 8;
			PC++;
			break;
		case 0x5C:
			BIT_b_r(3, 'H');
			cycle = 8;
			PC++;
			break;
		case 0x5D:
			BIT_b_r(3, 'L');
			cycle = 8;
			PC++;
			break;
		case 0x5E:
			BIT_b_16(3);
			cycle = 16;
			PC++;
			break;
		case 0x5F:
			BIT_b_r(3, 'A');
			cycle = 8;
			PC++;
			break;
		case 0x60:
			BIT_b_r(4, 'B');
			cycle = 8;
			PC++;
			break;
		case 0x61:
			BIT_b_r(4, 'C');
			cycle = 8;
			PC++;
			break;
		case 0x62:
			BIT_b_r(4, 'D');
			cycle = 8;
			PC++;
			break;
		case 0x63:
			BIT_b_r(4, 'E');
			cycle = 8;
			PC++;
			break;
		case 0x64:
			BIT_b_r(4, 'H');
			cycle = 8;
			PC++;
			break;
		case 0x65:
			BIT_b_r(4, 'L');
			cycle = 8;
			PC++;
			break;
		case 0x66:
			BIT_b_16(4);
			cycle = 16;
			PC++;
			break;
		case 0x67:
			BIT_b_r(4, 'A');
			cycle = 8;
			PC++;
			break;
		case 0x68:
			BIT_b_r(5, 'B');
			cycle = 8;
			PC++;
			break;
		case 0x69:
			BIT_b_r(5, 'C');
			cycle = 8;
			PC++;
			break;
		case 0x6A:
			BIT_b_r(5, 'D');
			cycle = 8;
			PC++;
			break;
		case 0x6B:
			BIT_b_r(5, 'E');
			cycle = 8;
			PC++;
			break;
		case 0x6C:
			BIT_b_r(5, 'H');
			cycle = 8;
			PC++;
			break;
		case 0x6D:
			BIT_b_r(5, 'L');
			cycle = 8;
			PC++;
			break;
		case 0x6E:
			BIT_b_16(5);
			cycle = 16;
			PC++;
			break;
		case 0x6F:
			BIT_b_r(5, 'A');
			cycle = 8;
			PC++;
			break;
		case 0x70:
			BIT_b_r(6, 'B');
			cycle = 8;
			PC++;
			break;
		case 0x71:
			BIT_b_r(6, 'C');
			cycle = 8;
			PC++;
			break;
		case 0x72:
			BIT_b_r(6, 'D');
			cycle = 8;
			PC++;
			break;
		case 0x73:
			BIT_b_r(6, 'E');
			cycle = 8;
			PC++;
			break;
		case 0x74:
			BIT_b_r(6, 'H');
			cycle = 8;
			PC++;
			break;
		case 0x75:
			BIT_b_r(6, 'L');
			cycle = 8;
			PC++;
			break;
		case 0x76:
			BIT_b_16(6);
			cycle = 16;
			PC++;
			break;
		case 0x77:
			BIT_b_r(6, 'A');
			cycle = 8;
			PC++;
			break;
		case 0x78:
			BIT_b_r(7, 'B');
			cycle = 8;
			PC++;
			break;
		case 0x79:
			BIT_b_r(7, 'C');
			cycle = 8;
			PC++;
			break;
		case 0x7A:
			BIT_b_r(7, 'D');
			cycle = 8;
			PC++;
			break;
		case 0x7B:
			BIT_b_r(7, 'E');
			cycle = 8;
			PC++;
			break;
		case 0x7C:
			BIT_b_r(7, 'H');
			cycle = 8;
			PC++;
			break;
		case 0x7D:
			BIT_b_r(7, 'L');
			cycle = 8;
			PC++;
			break;
		case 0x7E:
			BIT_b_16(7);
			cycle = 16;
			PC++;
			break;
		case 0x7F:
			BIT_b_r(7, 'A');
			cycle = 8;
			PC++;
			break;
		case 0x80:
			RES_b_r(0, 'B');
			cycle = 8;
			PC++;
			break;
		case 0x81:
			RES_b_r(0, 'C');
			cycle = 8;
			PC++;
			break;
		case 0x82:
			RES_b_r(0, 'D');
			cycle = 8;
			PC++;
			break;
		case 0x83:
			RES_b_r(0, 'E');
			cycle = 8;
			PC++;
			break;
		case 0x84:
			RES_b_r(0, 'H');
			cycle = 8;
			PC++;
			break;
		case 0x85:
			RES_b_r(0, 'L');
			cycle = 8;
			PC++;
			break;
		case 0x86:
			RES_b_16(0);
			cycle = 16;
			PC++;
			break;
		case 0x87:
			RES_b_r(0, 'A');
			cycle = 8;
			PC++;
			break;
		case 0x88:
			RES_b_r(1, 'B');
			cycle = 8;
			PC++;
			break;
		case 0x89:
			RES_b_r(1, 'C');
			cycle = 8;
			PC++;
			break;
		case 0x8A:
			RES_b_r(1, 'D');
			cycle = 8;
			PC++;
			break;
		case 0x8B:
			RES_b_r(1, 'E');
			cycle = 8;
			PC++;
			break;
		case 0x8C:
			RES_b_r(1, 'H');
			cycle = 8;
			PC++;
			break;
		case 0x8D:
			RES_b_r(1, 'L');
			cycle = 8;
			PC++;
			break;
		case 0x8E:
			RES_b_16(1);
			cycle = 16;
			PC++;
			break;
		case 0x8F:
			RES_b_r(1, 'A');
			cycle = 8;
			PC++;
			break;
		case 0x90:
			RES_b_r(2, 'B');
			cycle = 8;
			PC++;
			break;
		case 0x91:
			RES_b_r(2, 'C');
			;
			cycle = 8;
			PC++;
			break;
		case 0x92:
			RES_b_r(2, 'D');
			cycle = 8;
			PC++;
			break;
		case 0x93:
			RES_b_r(2, 'E');
			cycle = 8;
			PC++;
			break;
		case 0x94:
			RES_b_r(2, 'H');
			cycle = 8;
			PC++;
			break;
		case 0x95:
			RES_b_r(2, 'L');
			cycle = 8;
			PC++;
			break;
		case 0x96:
			RES_b_16(2);
			cycle = 16;
			PC++;
			break;
		case 0x97:
			RES_b_r(2, 'A');
			cycle = 8;
			PC++;
			break;
		case 0x98:
			RES_b_r(3, 'B');
			cycle = 8;
			PC++;
			break;
		case 0x99:
			RES_b_r(3, 'C');
			cycle = 8;
			PC++;
			break;
		case 0x9A:
			RES_b_r(3, 'D');
			cycle = 8;
			PC++;
			break;
		case 0x9B:
			RES_b_r(3, 'E');
			cycle = 8;
			PC++;
			break;
		case 0x9C:
			RES_b_r(3, 'H');
			cycle = 8;
			PC++;
			break;
		case 0x9D:
			RES_b_r(3, 'L');
			cycle = 8;
			PC++;
			break;
		case 0x9E:
			RES_b_16(3);
			cycle = 16;
			PC++;
			break;
		case 0x9F:
			RES_b_r(3, 'A');
			cycle = 8;
			PC++;
			break;
		case 0xA0:
			RES_b_r(4, 'B');
			cycle = 8;
			PC++;
			break;
		case 0xA1:
			RES_b_r(4, 'C');
			cycle = 8;
			PC++;
			break;
		case 0xA2:
			RES_b_r(4, 'D');
			cycle = 8;
			PC++;
			break;
		case 0xA3:
			RES_b_r(4, 'E');
			cycle = 8;
			PC++;
			break;
		case 0xA4:
			RES_b_r(4, 'H');
			cycle = 8;
			PC++;
			break;
		case 0xA5:
			RES_b_r(4, 'L');
			cycle = 8;
			PC++;
			break;
		case 0xA6:
			RES_b_16(4);
			cycle = 16;
			PC++;
			break;
		case 0xA7:
			RES_b_r(4, 'A');
			cycle = 8;
			PC++;
			break;
		case 0xA8:
			RES_b_r(5, 'B');
			cycle = 8;
			PC++;
			break;
		case 0xA9:
			RES_b_r(5, 'C');
			cycle = 8;
			PC++;
			break;
		case 0xAA:
			RES_b_r(5, 'D');
			cycle = 8;
			PC++;
			break;
		case 0xAB:
			RES_b_r(5, 'E');
			cycle = 8;
			PC++;
			break;
		case 0xAC:
			RES_b_r(5, 'H');
			cycle = 8;
			PC++;
			break;
		case 0xAD:
			RES_b_r(5, 'L');
			cycle = 8;
			PC++;
			break;
		case 0xAE:
			RES_b_16(5);
			cycle = 16;
			PC++;
			break;
		case 0xAF:
			RES_b_r(5, 'A');
			cycle = 8;
			PC++;
			break;
		case 0xB0:
			RES_b_r(6, 'B');
			cycle = 8;
			PC++;
			break;
		case 0xB1:
			RES_b_r(6, 'C');
			cycle = 8;
			PC++;
			break;
		case 0xB2:
			RES_b_r(6, 'D');
			cycle = 8;
			PC++;
			break;
		case 0xB3:
			RES_b_r(6, 'E');
			cycle = 8;
			PC++;
			break;
		case 0xB4:
			RES_b_r(6, 'H');
			cycle = 8;
			PC++;
			break;
		case 0xB5:
			RES_b_r(6, 'L');
			cycle = 8;
			PC++;
			break;
		case 0xB6:
			RES_b_16(6);
			cycle = 16;
			PC++;
			break;
		case 0xB7:
			RES_b_r(6, 'A');
			cycle = 8;
			PC++;
			break;
		case 0xB8:
			RES_b_r(7, 'B');
			cycle = 8;
			PC++;
			break;
		case 0xB9:
			RES_b_r(7, 'C');
			cycle = 8;
			PC++;
			break;
		case 0xBA:
			RES_b_r(7, 'D');
			cycle = 8;
			PC++;
			break;
		case 0xBB:
			RES_b_r(7, 'E');
			cycle = 8;
			PC++;
			break;
		case 0xBC:
			RES_b_r(7, 'H');
			cycle = 8;
			PC++;
			break;
		case 0xBD:
			RES_b_r(7, 'L');
			cycle = 8;
			PC++;
			break;
		case 0xBE:
			RES_b_16(7);
			cycle = 16;
			PC++;
			break;
		case 0xBF:
			RES_b_r(7, 'A');
			cycle = 8;
			PC++;
			break;
		case 0xC0:
			SET_b_r(0, 'B');
			cycle = 8;
			PC++;
			break;
		case 0xC1:
			SET_b_r(0, 'C');
			cycle = 8;
			PC++;
			break;
		case 0xC2:
			SET_b_r(0, 'D');
			cycle = 8;
			PC++;
			break;
		case 0xC3:
			SET_b_r(0, 'E');
			cycle = 8;
			PC++;
			break;
		case 0xC4:
			SET_b_r(0, 'H');
			cycle = 8;
			PC++;
			break;
		case 0xC5:
			SET_b_r(0, 'L');
			cycle = 8;
			PC++;
			break;
		case 0xC6:
			SET_b_16(0);
			cycle = 16;
			PC++;
			break;
		case 0xC7:
			SET_b_r(0, 'A');
			cycle = 8;
			PC++;
			break;
		case 0xC8:
			SET_b_r(1, 'B');
			cycle = 8;
			PC++;
			break;
		case 0xC9:
			SET_b_r(1, 'C');
			cycle = 8;
			PC++;
			break;
		case 0xCA:
			SET_b_r(1, 'D');
			cycle = 8;
			PC++;
			break;
		case 0xCB:
			SET_b_r(1, 'E');
			cycle = 8;
			PC++;
			break;
		case 0xCC:
			SET_b_r(1, 'H');
			cycle = 8;
			PC++;
			break;
		case 0xCD:
			SET_b_r(1, 'L');
			cycle = 8;
			PC++;
			break;
		case 0xCE:
			SET_b_16(1);
			cycle = 16;
			PC++;
			break;
		case 0xCF:
			SET_b_r(1, 'A');
			cycle = 8;
			PC++;
			break;
		case 0xD0:
			SET_b_r(2, 'B');
			cycle = 8;
			PC++;
			break;
		case 0xD1:
			SET_b_r(2, 'C');
			cycle = 8;
			PC++;
			break;
		case 0xD2:
			SET_b_r(2, 'D');
			cycle = 8;
			PC++;
			break;
		case 0xD3:
			SET_b_r(2, 'E');
			cycle = 8;
			PC++;
			break;
		case 0xD4:
			SET_b_r(2, 'H');
			cycle = 8;
			PC++;
			break;
		case 0xD5:
			SET_b_r(2, 'L');
			cycle = 8;
			PC++;
			break;
		case 0xD6:
			SET_b_16(2);
			cycle = 16;
			PC++;
			break;
		case 0xD7:
			SET_b_r(2, 'A');
			cycle = 8;
			PC++;
			break;
		case 0xD8:
			SET_b_r(3, 'B');
			cycle = 8;
			PC++;
			break;
		case 0xD9:
			SET_b_r(3, 'C');
			cycle = 8;
			PC++;
			break;
		case 0xDA:
			SET_b_r(3, 'D');
			cycle = 8;
			PC++;
			break;
		case 0xDB:
			SET_b_r(3, 'E');
			cycle = 8;
			PC++;
			break;
		case 0xDC:
			SET_b_r(3, 'H');
			cycle = 8;
			PC++;
			break;
		case 0xDD:
			SET_b_r(3, 'L');
			cycle = 8;
			PC++;
			break;
		case 0xDE:
			SET_b_16(3);
			cycle = 16;
			PC++;
			break;
		case 0xDF:
			SET_b_r(3, 'A');
			cycle = 8;
			PC++;
			break;
		case 0xE0:
			SET_b_r(4, 'B');
			cycle = 8;
			PC++;
			break;
		case 0xE1:
			SET_b_r(4, 'C');
			cycle = 8;
			PC++;
			break;
		case 0xE2:
			SET_b_r(4, 'D');
			cycle = 8;
			PC++;
			break;
		case 0xE3:
			SET_b_r(4, 'E');
			cycle = 8;
			PC++;
			break;
		case 0xE4:
			SET_b_r(4, 'H');
			cycle = 8;
			PC++;
			break;
		case 0xE5:
			SET_b_r(4, 'L');
			cycle = 8;
			PC++;
			break;
		case 0xE6:
			SET_b_16(4);
			cycle = 16;
			PC++;
			break;
		case 0xE7:
			SET_b_r(4, 'A');
			cycle = 8;
			PC++;
			break;
		case 0xE8:
			SET_b_r(5, 'B');
			cycle = 8;
			PC++;
			break;
		case 0xE9:
			SET_b_r(5, 'C');
			cycle = 8;
			PC++;
			break;
		case 0xEA:
			SET_b_r(5, 'D');
			cycle = 8;
			PC++;
			break;
		case 0xEB:
			SET_b_r(5, 'E');
			PC++;
			break;
		case 0xEC:
			SET_b_r(5, 'H');
			cycle = 8;
			PC++;
			break;
		case 0xED:
			SET_b_r(5, 'L');
			cycle = 8;
			PC++;
			break;
		case 0xEE:
			SET_b_16(5);
			cycle = 16;
			PC++;
			break;
		case 0xEF:
			SET_b_r(5, 'A');
			cycle = 8;
			PC++;
			break;
		case 0xF0:
			SET_b_r(6, 'B');
			cycle = 8;
			PC++;
			break;
		case 0xF1:
			SET_b_r(6, 'C');
			cycle = 8;
			PC++;
			break;
		case 0xF2:
			SET_b_r(6, 'D');
			cycle = 8;
			PC++;
			break;
		case 0xF3:
			SET_b_r(6, 'E');
			cycle = 8;
			PC++;
			break;
		case 0xF4:
			SET_b_r(6, 'H');
			cycle = 8;
			PC++;
			break;
		case 0xF5:
			SET_b_r(6, 'L');
			cycle = 8;
			PC++;
			break;
		case 0xF6:
			SET_b_16(6);
			cycle = 16;
			PC++;
			break;
		case 0xF7:
			SET_b_r(6, 'A');
			cycle = 8;
			PC++;
			break;
		case 0xF8:
			SET_b_r(7, 'B');
			cycle = 8;
			PC++;
			break;
		case 0xF9:
			SET_b_r(7, 'C');
			cycle = 8;
			PC++;
			break;
		case 0xFA:
			SET_b_r(7, 'D');
			cycle = 8;
			PC++;
			break;
		case 0xFB:
			SET_b_r(7, 'E');
			cycle = 8;
			PC++;
			break;
		case 0xFC:
			SET_b_r(7, 'H');
			cycle = 8;
			PC++;
			break;
		case 0xFD:
			SET_b_r(7, 'L');
			cycle = 8;
			PC++;
			break;
		case 0xFE:
			SET_b_16(7);
			cycle = 16;
			PC++;
			break;
		case 0xFF:
			SET_b_r(7, 'A');
			cycle = 8;
			PC++;
			break;

		default:
		{
			cout << "Unsupported 0xCB Instruction : " << hex << (int)mmu->read_ram(PC);
			exit(0xCB);
			break;
		}
		}
		break;
	}
	default:
		cout << "Unsupported Instruction : " << hex << (int)mmu->read_ram(PC);
		break;
		exit(0);
		break;
	}
	prevOpcode = mmu->read_ram(prevPC);
}
