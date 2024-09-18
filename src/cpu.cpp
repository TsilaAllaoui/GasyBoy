#include "cpu.h"
#include "gbException.h"
#include "timer.h"

namespace gasyboy
{

	Cpu::State Cpu::state = Cpu::State::RUNNING;

	Cpu::Cpu(const bool &bootBios, Mmu &mmu, Registers &registers, InterruptManager &interruptManager)
		: _mmu(mmu),
		  _registers(registers),
		  _interruptManager(interruptManager),
		  _currentOpcode(0),
		  _cycle(0)
	{
		// If not booting bios, set registers directly to program
		if (!bootBios)
		{
			_mmu.disableBios();
			_registers.AF.set(0x01B0);
			_registers.BC.set(0x0013);
			_registers.DE.set(0x00D8);
			_registers.HL.set(0x014D);
			_registers.PC = 0x100;
			_registers.SP = 0xFFFE;
		}
	}

	uint16_t Cpu::getRegister(const std::string &reg)
	{
		if (reg == "PC")
		{
			return _registers.PC;
		}
		else if (reg == "SP")
		{
			return _registers.SP;
		}
		else
		{
			return _registers.getRegister(reg).get();
		}
	}

	uint8_t Cpu::getRegister(const char &reg)
	{
		return _registers.getRegister(reg);
	}

	void Cpu::LD_HL_SP_n()
	{
		int8_t value = static_cast<int8_t>(_mmu.readRam(_registers.PC + 1));
		uint16_t result = (_registers.SP + value);
		(((_registers.SP ^ value ^ (result & 0xFFFF)) & 0x10) == 0x10) ? _registers.AF.setFlag('H') : _registers.AF.clearFlag('H');
		(((_registers.SP ^ value ^ (result & 0xFFFF)) & 0x100) == 0x100) ? _registers.AF.setFlag('C') : _registers.AF.clearFlag('C');
		_registers.HL.set(result);
		_registers.AF.clearFlag('Z');
		_registers.AF.clearFlag('N');
	}

	// TODO: use Registers method
	void Cpu::LD_r_r(const char &from, const char &to)
	{
		uint8_t regFrom = 0;
		switch (from)
		{
		case 'A':
			regFrom = _registers.AF.getLeftRegister();
			break;
		case 'B':
			regFrom = _registers.BC.getLeftRegister();
			break;
		case 'C':
			regFrom = _registers.BC.getRightRegister();
			break;
		case 'D':
			regFrom = _registers.DE.getLeftRegister();
			break;
		case 'E':
			regFrom = _registers.DE.getRightRegister();
			break;
		case 'H':
			regFrom = _registers.HL.getLeftRegister();
			break;
		case 'L':
			regFrom = _registers.HL.getRightRegister();
			break;
		default:
			throw exception::GbException("Invalid register");
			break;
		}
		switch (to)
		{
		case 'A':
			_registers.AF.setLeftRegister(regFrom);
			break;
		case 'B':
			_registers.BC.setLeftRegister(regFrom);
			break;
		case 'C':
			_registers.BC.setRightRegister(regFrom);
			break;
		case 'D':
			_registers.DE.setLeftRegister(regFrom);
			break;
		case 'E':
			_registers.DE.setRightRegister(regFrom);
			break;
		case 'H':
			_registers.HL.setLeftRegister(regFrom);
			break;
		case 'L':
			_registers.HL.setRightRegister(regFrom);
			break;
		default:
			cout << "Flag error.";
			exit(0);
			break;
		}
	}

	void Cpu::LD_r_n(const uint8_t &from, const char &to)
	{
		switch (to)
		{
		case 'A':
			_registers.AF.setLeftRegister(from);
			break;
		case 'F':
			_registers.AF.setRightRegister(from);
			break;
		case 'B':
			_registers.BC.setLeftRegister(from);
			break;
		case 'C':
			_registers.BC.setRightRegister(from);
			break;
		case 'D':
			_registers.DE.setLeftRegister(from);
			break;
		case 'E':
			_registers.DE.setRightRegister(from);
			break;
		case 'H':
			_registers.HL.setLeftRegister(from);
			break;
		case 'L':
			_registers.HL.setRightRegister(from);
			break;
		default:
			cout << "Flag error.";
			exit(0);
			break;
		}
	}

	void Cpu::LD_r_16(const uint16_t &adress, const char &to)
	{
		uint8_t from = _mmu.readRam(adress);
		switch (to)
		{
		case 'A':
			_registers.AF.setLeftRegister(from);
			break;
		case 'B':
			_registers.BC.setLeftRegister(from);
			break;
		case 'C':
			_registers.BC.setRightRegister(from);
			break;
		case 'D':
			_registers.DE.setLeftRegister(from);
			break;
		case 'E':
			_registers.DE.setRightRegister(from);
			break;
		case 'H':
			_registers.HL.setLeftRegister(from);
			break;
		case 'L':
			_registers.HL.setRightRegister(from);
			break;
		default:
			cout << "Flag error.";
			exit(0);
			break;
		}
	}

	void Cpu::LD_16_r(const uint16_t &adress, const char &from)
	{
		uint8_t value = 0;
		switch (from)
		{
		case 'A':
			value = _registers.AF.getLeftRegister();
			break;
		case 'B':
			value = _registers.BC.getLeftRegister();
			break;
		case 'C':
			value = _registers.BC.getRightRegister();
			break;
		case 'D':
			value = _registers.DE.getLeftRegister();
			break;
		case 'E':
			value = _registers.DE.getRightRegister();
			break;
		case 'H':
			value = _registers.HL.getLeftRegister();
			break;
		case 'L':
			value = _registers.HL.getRightRegister();
			break;
		default:
			cout << "Flag error."; // TODO: add gbException here
			exit(0);
			break;
		}
		_mmu.writeRam(adress, value);
	}

	void Cpu::LD_16_n(const uint16_t &adress, const uint8_t &value)
	{
		_mmu.writeRam(adress, value);
	}

	void Cpu::LD_rr_nn(const uint16_t &value, const std::string &reg)
	{
		if (reg == "AF")
			_registers.AF.set(value);
		else if (reg == "BC")
			_registers.BC.set(value);
		else if (reg == "DE")
			_registers.DE.set(value);
		else if (reg == "HL")
			_registers.HL.set(value);
		else if (reg == "SP")
			_registers.SP = value;
		else
			exit(2); // TODO: add gbException here
	}

	void Cpu::LD_rr_16(const uint16_t &adress, const std::string &reg)
	{
		uint8_t leftValue = _mmu.readRam(adress + 1);
		uint8_t rightValue = _mmu.readRam(adress);
		uint16_t value = ((uint16_t)(leftValue << 8) | rightValue);
		if (reg == "AF")
			_registers.AF.set(value);
		else if (reg == "BC")
			_registers.BC.set(value);
		else if (reg == "DE")
			_registers.DE.set(value);
		else if (reg == "HL")
			_registers.HL.set(value);
		else if (reg == "SP")
			_registers.SP = value;
		else
			exit(2);
	}

	void Cpu::LD_16_rr(const uint16_t &adress, const std::string &reg)
	{
		uint16_t value;
		if (reg == "AF")
			value = _registers.AF.get();
		else if (reg == "BC")
			value = _registers.BC.get();
		else if (reg == "DE")
			value = _registers.DE.get();
		else if (reg == "HL")
			value = _registers.HL.get();
		else if (reg == "SP")
			value = _registers.SP;
		else
			exit(2);
		uint8_t firstByte = static_cast<uint8_t>(value & 0xFF);
		uint8_t secondByte = static_cast<uint8_t>((value & 0xFF00) >> 8);
		_mmu.writeRam(adress, firstByte);
		_mmu.writeRam(adress + 1, secondByte);
	}

	void Cpu::LD_SP_HL()
	{
		_registers.SP = _registers.HL.get();
	}

	void Cpu::PUSH(const std::string &reg)
	{
		uint16_t value = 0xFFFF;
		if (reg == "AF")
			value = _registers.AF.get();
		else if (reg == "BC")
			value = _registers.BC.get();
		else if (reg == "DE")
			value = _registers.DE.get();
		else if (reg == "HL")
			value = _registers.HL.get();
		else
			exit(2);
		uint8_t firstByte = static_cast<uint8_t>((value & 0xFF00) >> 8);
		uint8_t secondByte = static_cast<uint8_t>(value & 0xFF);
		_registers.SP--;
		_mmu.writeRam(_registers.SP, firstByte);
		_registers.SP--;
		_mmu.writeRam(_registers.SP, secondByte);
	}

	void Cpu::POP(const std::string &reg)
	{
		uint8_t firstByte = _mmu.readRam(_registers.SP + 1);
		uint16_t secondByte = (_mmu.readRam(_registers.SP));
		_registers.SP += 2;
		uint16_t value = ((firstByte << 8) | secondByte);
		if (reg == "AF")
		{
			value &= 0xFFF0;
			_registers.AF.set(value);
		}
		// _registers.AF.set(value);
		else if (reg == "BC")
		{
			_registers.BC.set(value);
		}
		else if (reg == "DE")
		{
			_registers.DE.set(value);
		}
		else if (reg == "HL")
		{
			_registers.HL.set(value);
		}
		else
			exit(2);
	}

	void Cpu::ADD_A_r(const char &reg)
	{
		uint8_t value = 0;
		switch (reg)
		{
		case 'A':
			value = _registers.AF.getLeftRegister();
			break;
		case 'F':
			value = _registers.AF.getRightRegister();
			break;
		case 'B':
			value = _registers.BC.getLeftRegister();
			break;
		case 'C':
			value = _registers.BC.getRightRegister();
			break;
		case 'D':
			value = _registers.DE.getLeftRegister();
			break;
		case 'E':
			value = _registers.DE.getRightRegister();
			break;
		case 'H':
			value = _registers.HL.getLeftRegister();
			break;
		case 'L':
			value = _registers.HL.getRightRegister();
			break;
		default:
			cout << "Flag error.";
			exit(0);
			break;
		}
		(checkAddHalfCarry(value, _registers.AF.getLeftRegister(), _registers.AF.getLeftRegister() + value)) ? _registers.AF.setFlag('H') : _registers.AF.clearFlag('H');
		(checkAddCarry(value, _registers.AF.getLeftRegister(), _registers.AF.getLeftRegister() + value)) ? _registers.AF.setFlag('C') : _registers.AF.clearFlag('C');
		uint8_t result = _registers.AF.getLeftRegister() + value;
		(result == 0) ? _registers.AF.setFlag('Z') : _registers.AF.clearFlag('Z');
		_registers.AF.clearFlag('N');
		_registers.AF.setLeftRegister(result);
	}

	void Cpu::ADD_A_n(const uint8_t &value)
	{
		(checkAddHalfCarry(value, _registers.AF.getLeftRegister(), _registers.AF.getLeftRegister() + value)) ? _registers.AF.setFlag('H') : _registers.AF.clearFlag('H');
		(checkAddCarry(value, _registers.AF.getLeftRegister(), _registers.AF.getLeftRegister() + value)) ? _registers.AF.setFlag('C') : _registers.AF.clearFlag('C');
		uint8_t result = _registers.AF.getLeftRegister() + value;
		(result == 0) ? _registers.AF.setFlag('Z') : _registers.AF.clearFlag('Z');
		_registers.AF.clearFlag('N');
		_registers.AF.setLeftRegister(result);
	}

	void Cpu::ADD_A_16()
	{
		uint8_t value = _mmu.readRam(_registers.HL.get());
		(checkAddHalfCarry(value, _registers.AF.getLeftRegister(), _registers.AF.getLeftRegister() + value)) ? _registers.AF.setFlag('H') : _registers.AF.clearFlag('H');
		// (checkAddCarry(value, _registers.AF.getLeftRegister(), _registers.AF.getLeftRegister() + value)) ? _registers.AF.setFlag('C') : _registers.AF.clearFlag('C');
		((uint16_t)value + (uint16_t)_registers.AF.getLeftRegister() >= 0x100) ? _registers.AF.setFlag('C') : _registers.AF.clearFlag('C');
		uint8_t result = _registers.AF.getLeftRegister() + value;
		(result == 0) ? _registers.AF.setFlag('Z') : _registers.AF.clearFlag('Z');
		_registers.AF.clearFlag('N');
		_registers.AF.setLeftRegister(result);
	}

	void Cpu::ADC_A_r(const char &reg)
	{
		uint8_t value = 0;
		switch (reg)
		{
		case 'A':
			value = _registers.AF.getLeftRegister();
			break;
		case 'B':
			value = _registers.BC.getLeftRegister();
			break;
		case 'C':
			value = _registers.BC.getRightRegister();
			break;
		case 'D':
			value = _registers.DE.getLeftRegister();
			break;
		case 'E':
			value = _registers.DE.getRightRegister();
			break;
		case 'H':
			value = _registers.HL.getLeftRegister();
			break;
		case 'L':
			value = _registers.HL.getRightRegister();
			break;
		default:
			cout << "Flag error.";
			exit(0);
			break;
		}
		uint8_t A = _registers.AF.getLeftRegister();
		uint8_t carry = _registers.AF.getFlag('C') ? 1 : 0;

		unsigned int result_full = A + value + carry;
		uint8_t result = static_cast<uint8_t>(result_full);

		(result == 0) ? _registers.AF.setFlag('Z') : _registers.AF.clearFlag('Z');
		_registers.AF.clearFlag('N');
		(((A & 0xF) + (value & 0xF) + carry) > 0xF) ? _registers.AF.setFlag('H') : _registers.AF.clearFlag('H');
		(result_full > 0xFF) ? _registers.AF.setFlag('C') : _registers.AF.clearFlag('C');

		_registers.AF.setLeftRegister(result);
	}

	void Cpu::ADC_A_n(const uint8_t &value)
	{
		uint8_t reg = _registers.AF.getLeftRegister();
		uint8_t carry = _registers.AF.getFlag('C') ? 1 : 0;

		unsigned int result_full = reg + value + carry;
		uint8_t result = static_cast<uint8_t>(result_full);

		(result == 0) ? _registers.AF.setFlag('Z') : _registers.AF.clearFlag('Z');
		_registers.AF.clearFlag('N');
		(((reg & 0xF) + (value & 0xF) + carry) > 0xF) ? _registers.AF.setFlag('H') : _registers.AF.clearFlag('H');
		(result_full > 0xFF) ? _registers.AF.setFlag('C') : _registers.AF.clearFlag('C');

		_registers.AF.setLeftRegister(result);
	}
	void Cpu::ADC_A_16()
	{
		uint8_t value = _mmu.readRam(_registers.HL.get());
		uint8_t reg = _registers.AF.getLeftRegister();
		uint8_t carry = _registers.AF.getFlag('C') ? 1 : 0;

		unsigned int result_full = reg + value + carry;
		uint8_t result = static_cast<uint8_t>(result_full);

		(result == 0) ? _registers.AF.setFlag('Z') : _registers.AF.clearFlag('Z');
		_registers.AF.clearFlag('N');
		(((reg & 0xF) + (value & 0xF) + carry) > 0xF) ? _registers.AF.setFlag('H') : _registers.AF.clearFlag('H');
		(result_full > 0xFF) ? _registers.AF.setFlag('C') : _registers.AF.clearFlag('C');

		_registers.AF.setLeftRegister(result);
	}

	void Cpu::SUB_r(const char &reg)
	{
		uint8_t value = 0;
		switch (reg)
		{
		case 'A':
			value = _registers.AF.getLeftRegister();
			break;
		case 'F':
			value = _registers.AF.getRightRegister();
			break;
		case 'B':
			value = _registers.BC.getLeftRegister();
			break;
		case 'C':
			value = _registers.BC.getRightRegister();
			break;
		case 'D':
			value = _registers.DE.getLeftRegister();
			break;
		case 'E':
			value = _registers.DE.getRightRegister();
			break;
		case 'H':
			value = _registers.HL.getLeftRegister();
			break;
		case 'L':
			value = _registers.HL.getRightRegister();
			break;
		default:
			cout << "Flag error.";
			exit(0);
			break;
		}
		(_registers.AF.getLeftRegister() - value < 0) ? _registers.AF.setFlag('C') : _registers.AF.clearFlag('C');
		(checkSubHalfCarry(_registers.AF.getLeftRegister(), value)) ? _registers.AF.setFlag('H') : _registers.AF.clearFlag('H');
		_registers.AF.setLeftRegister(_registers.AF.getLeftRegister() - value);
		(_registers.AF.getLeftRegister() == 0) ? _registers.AF.setFlag('Z') : _registers.AF.clearFlag('Z');
		_registers.AF.setFlag('N');
	}

	void Cpu::SUB_n(const uint8_t &value)
	{
		(_registers.AF.getLeftRegister() - value < 0) ? _registers.AF.setFlag('C') : _registers.AF.clearFlag('C');
		(checkSubHalfCarry(_registers.AF.getLeftRegister(), value)) ? _registers.AF.setFlag('H') : _registers.AF.clearFlag('H');
		_registers.AF.setLeftRegister(_registers.AF.getLeftRegister() - value);
		(_registers.AF.getLeftRegister() == 0) ? _registers.AF.setFlag('Z') : _registers.AF.clearFlag('Z');
		_registers.AF.setFlag('N');
	}

	void Cpu::SUB_16()
	{
		uint8_t value = _mmu.readRam(_registers.HL.get());
		(_registers.AF.getLeftRegister() - value < 0) ? _registers.AF.setFlag('C') : _registers.AF.clearFlag('C');
		(checkSubHalfCarry(_registers.AF.getLeftRegister(), value)) ? _registers.AF.setFlag('H') : _registers.AF.clearFlag('H');
		_registers.AF.setLeftRegister(_registers.AF.getLeftRegister() - value);
		(_registers.AF.getLeftRegister() == 0) ? _registers.AF.setFlag('Z') : _registers.AF.clearFlag('Z');
		_registers.AF.setFlag('N');
	}

	void Cpu::SBC_r(const char &reg)
	{
		uint8_t value = 0;
		switch (reg)
		{
		case 'A':
			value = _registers.AF.getLeftRegister();
			break;
		case 'B':
			value = _registers.BC.getLeftRegister();
			break;
		case 'C':
			value = _registers.BC.getRightRegister();
			break;
		case 'D':
			value = _registers.DE.getLeftRegister();
			break;
		case 'E':
			value = _registers.DE.getRightRegister();
			break;
		case 'H':
			value = _registers.HL.getLeftRegister();
			break;
		case 'L':
			value = _registers.HL.getRightRegister();
			break;
		default:
			cout << "Flag error.";
			exit(0);
			break;
		}
		uint8_t carry = _registers.AF.getFlag('C') ? 1 : 0;
		uint8_t A = _registers.AF.getLeftRegister();

		int result_full = A - value - carry;
		uint8_t result = static_cast<uint8_t>(result_full);

		(result == 0) ? _registers.AF.setFlag('Z') : _registers.AF.clearFlag('Z');
		_registers.AF.setFlag('N');
		(result_full < 0) ? _registers.AF.setFlag('C') : _registers.AF.clearFlag('C');
		(((A & 0xF) - (value & 0xF) - carry) < 0) ? _registers.AF.setFlag('H') : _registers.AF.clearFlag('H');

		_registers.AF.setLeftRegister(result);
	}

	void Cpu::SBC_n(const uint8_t &value)
	{
		uint8_t carry = _registers.AF.getFlag('C') ? 1 : 0;
		uint8_t A = _registers.AF.getLeftRegister();

		int result_full = A - value - carry;
		uint8_t result = static_cast<uint8_t>(result_full);

		(result == 0) ? _registers.AF.setFlag('Z') : _registers.AF.clearFlag('Z');
		_registers.AF.setFlag('N');
		(result_full < 0) ? _registers.AF.setFlag('C') : _registers.AF.clearFlag('C');
		(((A & 0xF) - (value & 0xF) - carry) < 0) ? _registers.AF.setFlag('H') : _registers.AF.clearFlag('H');

		_registers.AF.setLeftRegister(result);
	}

	void Cpu::SBC_16()
	{
		uint8_t value = _mmu.readRam(_registers.HL.get());
		uint8_t carry = _registers.AF.getFlag('C') ? 1 : 0;
		uint8_t A = _registers.AF.getLeftRegister();

		int result_full = A - value - carry;
		uint8_t result = static_cast<uint8_t>(result_full);

		(result == 0) ? _registers.AF.setFlag('Z') : _registers.AF.clearFlag('Z');
		_registers.AF.setFlag('N');
		(result_full < 0) ? _registers.AF.setFlag('C') : _registers.AF.clearFlag('C');
		(((A & 0xF) - (value & 0xF) - carry) < 0) ? _registers.AF.setFlag('H') : _registers.AF.clearFlag('H');

		_registers.AF.setLeftRegister(result);
	}

	void Cpu::AND_r(const char &reg)
	{
		uint8_t value = 0;
		switch (reg)
		{
		case 'A':
			value = _registers.AF.getLeftRegister();
			break;
		case 'F':
			value = _registers.AF.getRightRegister();
			break;
		case 'B':
			value = _registers.BC.getLeftRegister();
			break;
		case 'C':
			value = _registers.BC.getRightRegister();
			break;
		case 'D':
			value = _registers.DE.getLeftRegister();
			break;
		case 'E':
			value = _registers.DE.getRightRegister();
			break;
		case 'H':
			value = _registers.HL.getLeftRegister();
			break;
		case 'L':
			value = _registers.HL.getRightRegister();
			break;
		default:
			cout << "Flag error.";
			exit(0);
			break;
		}
		uint8_t b = value;
		uint8_t result = _registers.AF.getLeftRegister() & value;
		_registers.AF.setLeftRegister(result);
		(result == 0) ? _registers.AF.setFlag('Z') : _registers.AF.clearFlag('Z');
		_registers.AF.setFlag('H');
		_registers.AF.clearFlag('N');
		_registers.AF.clearFlag('C');
	}

	void Cpu::AND_n(const uint8_t &value)
	{
		uint8_t result = _registers.AF.getLeftRegister() & value;
		_registers.AF.setLeftRegister(result);
		(result == 0) ? _registers.AF.setFlag('Z') : _registers.AF.clearFlag('Z');
		_registers.AF.setFlag('H');
		_registers.AF.clearFlag('N');
		_registers.AF.clearFlag('C');
	}

	void Cpu::AND_16()
	{
		uint8_t value = _mmu.readRam(_registers.HL.get());
		uint8_t result = _registers.AF.getLeftRegister() & value;
		_registers.AF.setLeftRegister(result);
		(result == 0) ? _registers.AF.setFlag('Z') : _registers.AF.clearFlag('Z');
		_registers.AF.setFlag('H');
		_registers.AF.clearFlag('N');
		_registers.AF.clearFlag('C');
	}

	void Cpu::OR_r(const char &reg)
	{
		uint8_t value = 0;
		switch (reg)
		{
		case 'A':
			value = _registers.AF.getLeftRegister();
			break;
		case 'B':
			value = _registers.BC.getLeftRegister();
			break;
		case 'C':
			value = _registers.BC.getRightRegister();
			break;
		case 'D':
			value = _registers.DE.getLeftRegister();
			break;
		case 'E':
			value = _registers.DE.getRightRegister();
			break;
		case 'H':
			value = _registers.HL.getLeftRegister();
			break;
		case 'L':
			value = _registers.HL.getRightRegister();
			break;
		default:
			cout << "Flag error.";
			exit(0);
			break;
		}
		uint8_t b = value;
		uint8_t result = _registers.AF.getLeftRegister() | value;
		_registers.AF.setLeftRegister(result);
		(result == 0) ? _registers.AF.setFlag('Z') : _registers.AF.clearFlag('Z');
		_registers.AF.clearFlag('H');
		_registers.AF.clearFlag('N');
		_registers.AF.clearFlag('C');
	}

	void Cpu::OR_n(const uint8_t &value)
	{
		uint8_t result = _registers.AF.getLeftRegister() | value;
		_registers.AF.setLeftRegister(result);
		(result == 0) ? _registers.AF.setFlag('Z') : _registers.AF.clearFlag('Z');
		_registers.AF.clearFlag('H');
		_registers.AF.clearFlag('N');
		_registers.AF.clearFlag('C');
	}

	void Cpu::OR_16()
	{
		uint8_t value = _mmu.readRam(_registers.HL.get());
		uint8_t result = _registers.AF.getLeftRegister() | value;
		_registers.AF.setLeftRegister(result);
		(result == 0) ? _registers.AF.setFlag('Z') : _registers.AF.clearFlag('Z');
		_registers.AF.clearFlag('H');
		_registers.AF.clearFlag('N');
		_registers.AF.clearFlag('C');
	}

	void Cpu::XOR_r(const char &reg)
	{
		uint8_t value = 0;
		switch (reg)
		{
		case 'A':
			value = _registers.AF.getLeftRegister();
			break;
		case 'B':
			value = _registers.BC.getLeftRegister();
			break;
		case 'C':
			value = _registers.BC.getRightRegister();
			break;
		case 'D':
			value = _registers.DE.getLeftRegister();
			break;
		case 'E':
			value = _registers.DE.getRightRegister();
			break;
		case 'H':
			value = _registers.HL.getLeftRegister();
			break;
		case 'L':
			value = _registers.HL.getRightRegister();
			break;
		default:
			cout << "Flag error.";
			exit(0);
			break;
		}
		uint8_t b = value;
		uint8_t result = _registers.AF.getLeftRegister() ^ value;
		_registers.AF.setLeftRegister(result);
		(result == 0) ? _registers.AF.setFlag('Z') : _registers.AF.clearFlag('Z');
		_registers.AF.clearFlag('H');
		_registers.AF.clearFlag('N');
		_registers.AF.clearFlag('C');
	}

	void Cpu::XOR_n(const uint8_t &value)
	{
		uint8_t result = _registers.AF.getLeftRegister() ^ value;
		_registers.AF.setLeftRegister(result);
		(result == 0) ? _registers.AF.setFlag('Z') : _registers.AF.clearFlag('Z');
		_registers.AF.clearFlag('H');
		_registers.AF.clearFlag('N');
		_registers.AF.clearFlag('C');
	}

	void Cpu::XOR_16()
	{
		uint8_t value = _mmu.readRam(_registers.HL.get());
		uint8_t result = _registers.AF.getLeftRegister() ^ value;
		_registers.AF.setLeftRegister(result);
		(result == 0) ? _registers.AF.setFlag('Z') : _registers.AF.clearFlag('Z');
		_registers.AF.clearFlag('H');
		_registers.AF.clearFlag('N');
		_registers.AF.clearFlag('C');
	}

	void Cpu::CP_r(const char &reg)
	{
		uint8_t value = 0;
		switch (reg)
		{
		case 'A':
			value = _registers.AF.getLeftRegister();
			break;
		case 'F':
			value = _registers.AF.getRightRegister();
			break;
		case 'B':
			value = _registers.BC.getLeftRegister();
			break;
		case 'C':
			value = _registers.BC.getRightRegister();
			break;
		case 'D':
			value = _registers.DE.getLeftRegister();
			break;
		case 'E':
			value = _registers.DE.getRightRegister();
			break;
		case 'H':
			value = _registers.HL.getLeftRegister();
			break;
		case 'L':
			value = _registers.HL.getRightRegister();
			break;
		default:
			cout << "Flag error.";
			exit(0);
			break;
		}
		((uint16_t)_registers.AF.getLeftRegister() - (uint16_t)value < 0) ? _registers.AF.setFlag('C') : _registers.AF.clearFlag('C');
		(checkSubHalfCarry(_registers.AF.getLeftRegister(), value)) ? _registers.AF.setFlag('H') : _registers.AF.clearFlag('H');
		(_registers.AF.getLeftRegister() - value == 0) ? _registers.AF.setFlag('Z') : _registers.AF.clearFlag('Z');
		_registers.AF.setFlag('N');
	}

	void Cpu::CP_n(const uint8_t &value)
	{
		uint8_t reg = _registers.AF.getLeftRegister();
		uint8_t result = static_cast<uint8_t>(reg - value);

		(result == 0) ? _registers.AF.setFlag('Z') : _registers.AF.clearFlag('Z');
		_registers.AF.setFlag('N');
		(((reg & 0xf) - (value & 0xf)) < 0) ? _registers.AF.setFlag('H') : _registers.AF.clearFlag('H');
		(reg < value) ? _registers.AF.setFlag('C') : _registers.AF.clearFlag('C');
	}

	void Cpu::CP_16()
	{
		uint8_t value = _mmu.readRam(_registers.HL.get());
		((uint16_t)_registers.AF.getLeftRegister() - (uint16_t)value < 0) ? _registers.AF.setFlag('C') : _registers.AF.clearFlag('C');
		(checkSubHalfCarry(_registers.AF.getLeftRegister(), value)) ? _registers.AF.setFlag('H') : _registers.AF.clearFlag('H');
		(_registers.AF.getLeftRegister() - value == 0) ? _registers.AF.setFlag('Z') : _registers.AF.clearFlag('Z');
		_registers.AF.setFlag('N');
	}

	void Cpu::INC_r(const char &reg)
	{
		uint8_t value = 0, oldReg = 0;
		switch (reg)
		{
		case 'A':
			oldReg = _registers.AF.getLeftRegister();
			value = _registers.AF.getLeftRegister() + 1;
			_registers.AF.setLeftRegister(value);
			break;
		case 'F':
			oldReg = _registers.AF.getRightRegister();
			value = _registers.AF.getRightRegister() + 1;
			_registers.AF.setRightRegister(value);
			break;
		case 'B':
			oldReg = _registers.BC.getLeftRegister();
			value = _registers.BC.getLeftRegister() + 1;
			_registers.BC.setLeftRegister(value);
			break;
		case 'C':
			oldReg = _registers.BC.getRightRegister();
			value = _registers.BC.getRightRegister() + 1;
			_registers.BC.setRightRegister(value);
			break;
		case 'D':
			oldReg = _registers.DE.getLeftRegister();
			value = _registers.DE.getLeftRegister() + 1;
			_registers.DE.setLeftRegister(value);
			break;
		case 'E':
			oldReg = _registers.DE.getRightRegister();
			value = _registers.DE.getRightRegister() + 1;
			_registers.DE.setRightRegister(value);
			break;
		case 'H':
			oldReg = _registers.HL.getLeftRegister();
			value = _registers.HL.getLeftRegister() + 1;
			_registers.HL.setLeftRegister(value);
			break;
		case 'L':
			oldReg = _registers.HL.getRightRegister();
			value = _registers.HL.getRightRegister() + 1;
			_registers.HL.setRightRegister(value);
			break;
		default:
			cout << "Flag error.";
			exit(0);
			break;
		}
		(value == 0) ? _registers.AF.setFlag('Z') : _registers.AF.clearFlag('Z');
		(checkAddHalfCarry(oldReg, 1, oldReg + 1)) ? _registers.AF.setFlag('H') : _registers.AF.clearFlag('H');
		_registers.AF.clearFlag('N');
	}

	void Cpu::INC_16()
	{
		uint8_t oldValue = _mmu.readRam(_registers.HL.get());
		uint8_t value = _mmu.readRam(_registers.HL.get()) + 1;
		_mmu.writeRam(_registers.HL.get(), value);
		(value == 0) ? _registers.AF.setFlag('Z') : _registers.AF.clearFlag('Z');
		(checkAddHalfCarry(oldValue, 1, oldValue + 1)) ? _registers.AF.setFlag('H') : _registers.AF.clearFlag('H');
		_registers.AF.clearFlag('N');
	}

	void Cpu::DEC_r(const char &reg)
	{
		uint8_t value = 0;
		switch (reg)
		{
		case 'A':
			value = _registers.AF.getLeftRegister();
			break;
		case 'F':
			value = _registers.AF.getRightRegister();
			break;
		case 'B':
			value = _registers.BC.getLeftRegister();
			break;
		case 'C':
			value = _registers.BC.getRightRegister();
			break;
		case 'D':
			value = _registers.DE.getLeftRegister();
			break;
		case 'E':
			value = _registers.DE.getRightRegister();
			break;
		case 'H':
			value = _registers.HL.getLeftRegister();
			break;
		case 'L':
			value = _registers.HL.getRightRegister();
			break;
		default:
			cout << "Flag error.";
			exit(0);
			break;
		}
		switch (reg)
		{
		case 'A':
			_registers.AF.setLeftRegister(value - 1);
			break;
		case 'F':
			_registers.AF.setRightRegister(value - 1);
			break;
		case 'B':
			_registers.BC.setLeftRegister(value - 1);
			break;
		case 'C':
			_registers.BC.setRightRegister(value - 1);
			break;
		case 'D':
			_registers.DE.setLeftRegister(value - 1);
			break;
		case 'E':
			_registers.DE.setRightRegister(value - 1);
			break;
		case 'H':
			_registers.HL.setLeftRegister(value - 1);
			break;
		case 'L':
			_registers.HL.setRightRegister(value - 1);
			break;
		default:
			cout << "Flag error.";
			exit(0);
			break;
		}
		((value - 1) == 0) ? _registers.AF.setFlag('Z') : _registers.AF.clearFlag('Z');
		(checkSubHalfCarry(value, 1)) ? _registers.AF.setFlag('H') : _registers.AF.clearFlag('H');
		_registers.AF.setFlag('N');
	}

	void Cpu::DEC_16()
	{
		uint8_t value = _mmu.readRam(_registers.HL.get());
		_mmu.writeRam(_registers.HL.get(), value - 1);
		((value - 1) == 0) ? _registers.AF.setFlag('Z') : _registers.AF.clearFlag('Z');
		(checkSubHalfCarry(value, 1)) ? _registers.AF.setFlag('H') : _registers.AF.clearFlag('H');
		_registers.AF.setFlag('N');
	}

	void Cpu::DAA()
	{
		if (!_registers.AF.getFlag('N'))
		{
			if (_registers.AF.getFlag('C') || (_registers.AF.getLeftRegister() > 0x99))
			{
				uint8_t value = _registers.AF.getLeftRegister();
				_registers.AF.setLeftRegister(value + 0x60);
				_registers.AF.setFlag('C');
			}
			if (_registers.AF.getFlag('H') || ((_registers.AF.getLeftRegister() & 0x0F) > 0x09))
			{
				uint8_t value = _registers.AF.getLeftRegister();
				_registers.AF.setLeftRegister(value + 0x06);
			}
		}
		else
		{
			if (_registers.AF.getFlag('C'))
			{
				uint8_t value = _registers.AF.getLeftRegister();
				_registers.AF.setLeftRegister(value - 0x60);
			}
			if (_registers.AF.getFlag('H'))
			{
				uint8_t value = _registers.AF.getLeftRegister();
				_registers.AF.setLeftRegister(value - 0x06);
			}
		}
		(_registers.AF.getLeftRegister() == 0) ? _registers.AF.setFlag('Z') : _registers.AF.clearFlag('Z');
		_registers.AF.clearFlag('H');
	}

	void Cpu::CPL()
	{
		uint8_t value = _registers.AF.getLeftRegister();
		_registers.AF.setLeftRegister(~value);
		_registers.AF.setFlag('H');
		_registers.AF.setFlag('N');
	}

	void Cpu::CCF()
	{
		uint8_t value = _registers.AF.getRightRegister();
		value ^= 0x10;
		_registers.AF.setRightRegister(value);
		_registers.AF.clearFlag('N');
		_registers.AF.clearFlag('H');
	}

	void Cpu::SCF()
	{
		uint8_t value = _registers.AF.getRightRegister();
		value |= 0x10;
		_registers.AF.setRightRegister(value);
		_registers.AF.clearFlag('N');
		_registers.AF.clearFlag('H');
	}

	void Cpu::NOP()
	{
	}

	void Cpu::HALT()
	{
		_registers.setHalted(true);
	}

	void Cpu::DI()
	{
		_registers.setInterruptEnabled(false);
		_interruptManager.setMasterInterrupt(false);
	}

	void Cpu::EI()
	{
		_registers.setInterruptEnabled(true);
		_interruptManager.setMasterInterrupt(true);
	}

	void Cpu::ADD_HL_rr(const std::string &reg)
	{
		uint16_t value;
		if (reg == "AF")
			value = _registers.AF.get();
		else if (reg == "BC")
			value = _registers.BC.get();
		else if (reg == "DE")
			value = _registers.DE.get();
		else if (reg == "HL")
			value = _registers.HL.get();
		else if (reg == "SP")
			value = _registers.SP;
		else
			exit(2);
		uint16_t operand = _registers.HL.get();
		_registers.HL.set(operand + value);
		_registers.AF.clearFlag('N');
		((operand + value) >= 0x10000) ? _registers.AF.setFlag('C') : _registers.AF.clearFlag('C');
		((operand & 0xFFF) + (value & 0xFFF) >= 0x1000) ? _registers.AF.setFlag('H') : _registers.AF.clearFlag('H');
	}

	void Cpu::ADD_SP_n()
	{
		int8_t value = static_cast<int8_t>(_mmu.readRam(_registers.PC + 1));
		uint16_t result = (_registers.SP + value);
		(((_registers.SP ^ value ^ (result & 0xFFFF)) & 0x10) == 0x10) ? _registers.AF.setFlag('H') : _registers.AF.clearFlag('H');
		(((_registers.SP ^ value ^ (result & 0xFFFF)) & 0x100) == 0x100) ? _registers.AF.setFlag('C') : _registers.AF.clearFlag('C');
		_registers.SP = (result);
		_registers.AF.clearFlag('Z');
		_registers.AF.clearFlag('N');
	}

	void Cpu::ADC_HL_rr(const std::string &reg)
	{
		uint16_t value = 0;
		if (reg == "AF")
			value = _registers.AF.get();
		else if (reg == "BC")
			value = _registers.BC.get();
		else if (reg == "DE")
			value = _registers.DE.get();
		else if (reg == "HL")
			value = _registers.HL.get();
		else if (reg == "SP")
			value = _registers.SP;
		else
			exit(2);
		value += _registers.AF.getFlag('C') ? 1 : 0;
		uint16_t operand = _registers.HL.get();
		_registers.HL.set(operand + value);
		_registers.AF.clearFlag('N');
		(checkAddCarry(operand, value, value + operand)) ? _registers.AF.setFlag('C') : _registers.AF.clearFlag('C');
		(checkAddHalfCarry(operand, value, value + operand)) ? _registers.AF.setFlag('H') : _registers.AF.clearFlag('H');
	}

	void Cpu::SBC_HL_rr(const std::string &reg)
	{
		uint16_t value = 0;
		if (reg == "AF")
			value = _registers.AF.get();
		else if (reg == "BC")
			value = _registers.BC.get();
		else if (reg == "DE")
			value = _registers.DE.get();
		else if (reg == "HL")
			value = _registers.HL.get();
		else if (reg == "SP")
			value = _registers.SP;
		else
			exit(2);
		value += _registers.AF.getFlag('C') ? 1 : 0;
		uint16_t operand = _registers.HL.get();
		_registers.HL.set(operand - value);
		_registers.AF.setFlag('N');
		(checkAddCarry(operand, value, value + operand)) ? _registers.AF.setFlag('C') : _registers.AF.clearFlag('C');
		(checkSubHalfCarry(operand, value)) ? _registers.AF.setFlag('H') : _registers.AF.clearFlag('H');
	}

	void Cpu::INC_rr(const std::string &reg)
	{
		uint16_t value = 0;
		if (reg == "AF")
		{
			value = _registers.AF.get() + 1;
			_registers.AF.set(value);
		}
		else if (reg == "BC")
		{
			value = _registers.BC.get() + 1;
			_registers.BC.set(value);
		}
		else if (reg == "DE")
		{
			value = _registers.DE.get() + 1;
			_registers.DE.set(value);
		}
		else if (reg == "HL")
		{
			value = _registers.HL.get() + 1;
			_registers.HL.set(value);
		}
		else if (reg == "SP")
		{
			value = _registers.SP + 1;
			_registers.SP++;
		}
		else
			exit(2);
	}

	void Cpu::DEC_rr(const std::string &reg)
	{
		uint16_t value = 0;
		if (reg == "AF")
		{
			value = _registers.AF.get() - 1;
			_registers.AF.set(value);
		}
		else if (reg == "BC")
		{
			value = _registers.BC.get() - 1;
			_registers.BC.set(value);
		}
		else if (reg == "DE")
		{
			value = _registers.DE.get() - 1;
			_registers.DE.set(value);
		}
		else if (reg == "HL")
		{
			value = _registers.HL.get() - 1;
			_registers.HL.set(value);
		}
		else if (reg == "SP")
		{
			value = _registers.SP - 1;
			_registers.SP--;
		}
		else
			exit(2);
	}

	void Cpu::RLCA()
	{
		uint8_t reg = _registers.AF.getLeftRegister();
		((reg & 0x80) == 0x80) ? _registers.AF.setFlag('C') : _registers.AF.clearFlag('C');
		int old7bit = ((reg & 0x80) == 0x80) ? 1 : 0;
		uint8_t value = ((reg << 1) | (old7bit));
		_registers.AF.setLeftRegister(value);
		//(_registers.AF.getLeftRegister() == 0) ? _registers.AF.setFlag('Z') : _registers.AF.clearFlag('Z');
		_registers.AF.clearFlag('Z');
		_registers.AF.clearFlag('H');
		_registers.AF.clearFlag('N');
	}

	void Cpu::RLA()
	{
		int oldCarry = _registers.AF.getFlag('C');
		(_registers.AF.getLeftRegister() & 0x80) ? _registers.AF.setFlag('C') : _registers.AF.clearFlag('C');
		uint8_t value = ((_registers.AF.getLeftRegister() << 1) | (oldCarry << 0));
		_registers.AF.setLeftRegister(value);
		//(_registers.AF.getLeftRegister() == 0) ? _registers.AF.setFlag('Z') : _registers.AF.clearFlag('Z');
		_registers.AF.clearFlag('Z');
		_registers.AF.clearFlag('H');
		_registers.AF.clearFlag('N');
	}

	void Cpu::RRCA()
	{
		(_registers.AF.getLeftRegister() & 0x1) ? _registers.AF.setFlag('C') : _registers.AF.clearFlag('C');
		_registers.AF.setLeftRegister((_registers.AF.getLeftRegister() >> 1) | (_registers.AF.getFlag('C') << 7));
		//(_registers.AF.getLeftRegister() == 0) ? _registers.AF.setFlag('Z') : _registers.AF.clearFlag('Z');
		_registers.AF.clearFlag('H');
		_registers.AF.clearFlag('N');
		_registers.AF.clearFlag('Z');
	}

	void Cpu::RRA()
	{
		int oldCarry = _registers.AF.getFlag('C');
		(_registers.AF.getLeftRegister() & 0x1) ? _registers.AF.setFlag('C') : _registers.AF.clearFlag('C');
		uint8_t value = ((_registers.AF.getLeftRegister() >> 1) | (oldCarry << 7));
		_registers.AF.setLeftRegister(value);
		//(_registers.AF.getLeftRegister() == 0) ? _registers.AF.setFlag('Z') : _registers.AF.clearFlag('Z');
		_registers.AF.clearFlag('Z');
		_registers.AF.clearFlag('H');
		_registers.AF.clearFlag('N');
	}

	void Cpu::RLC_r(const char &reg)
	{
		uint8_t value = 0;
		switch (reg)
		{
		case 'A':
			(_registers.AF.getLeftRegister() & 0x80) ? _registers.AF.setFlag('C') : _registers.AF.clearFlag('C');
			_registers.AF.setLeftRegister((_registers.AF.getLeftRegister() << 1) | static_cast<uint8_t>(_registers.AF.getFlag('C')));
			_registers.AF.clearFlag('H');
			_registers.AF.clearFlag('N');
			value = _registers.AF.getLeftRegister();
			break;
		case 'B':
			(_registers.BC.getLeftRegister() & 0x80) ? _registers.AF.setFlag('C') : _registers.AF.clearFlag('C');
			_registers.BC.setLeftRegister((_registers.BC.getLeftRegister() << 1) | static_cast<uint8_t>(_registers.AF.getFlag('C')));
			_registers.AF.clearFlag('H');
			_registers.AF.clearFlag('N');
			value = _registers.BC.getLeftRegister();
			break;
		case 'C':
			(_registers.BC.getRightRegister() & 0x80) ? _registers.AF.setFlag('C') : _registers.AF.clearFlag('C');
			_registers.BC.setRightRegister((_registers.BC.getRightRegister() << 1) | static_cast<uint8_t>(_registers.AF.getFlag('C')));
			_registers.AF.clearFlag('H');
			_registers.AF.clearFlag('N');
			value = _registers.BC.getRightRegister();
			break;
		case 'D':
			(_registers.DE.getLeftRegister() & 0x80) ? _registers.AF.setFlag('C') : _registers.AF.clearFlag('C');
			_registers.DE.setLeftRegister((_registers.DE.getLeftRegister() << 1) | static_cast<uint8_t>(_registers.AF.getFlag('C')));
			_registers.AF.clearFlag('H');
			_registers.AF.clearFlag('N');
			value = _registers.DE.getLeftRegister();
			break;
		case 'E':
			(_registers.DE.getRightRegister() & 0x80) ? _registers.AF.setFlag('C') : _registers.AF.clearFlag('C');
			_registers.DE.setRightRegister((_registers.DE.getRightRegister() << 1) | static_cast<uint8_t>(_registers.AF.getFlag('C')));
			_registers.AF.clearFlag('H');
			_registers.AF.clearFlag('N');
			value = _registers.DE.getRightRegister();
			break;
		case 'H':
			(_registers.HL.getLeftRegister() & 0x80) ? _registers.AF.setFlag('C') : _registers.AF.clearFlag('C');
			_registers.HL.setLeftRegister((_registers.HL.getLeftRegister() << 1) | static_cast<uint8_t>(_registers.AF.getFlag('C')));
			_registers.AF.clearFlag('H');
			_registers.AF.clearFlag('N');
			value = _registers.HL.getLeftRegister();
			break;
		case 'L':
			(_registers.HL.getRightRegister() & 0x80) ? _registers.AF.setFlag('C') : _registers.AF.clearFlag('C');
			_registers.HL.setRightRegister((_registers.HL.getRightRegister() << 1) | static_cast<uint8_t>(_registers.AF.getFlag('C')));
			_registers.AF.clearFlag('H');
			_registers.AF.clearFlag('N');
			value = _registers.HL.getRightRegister();
			break;
		default:
			cout << "Flag error.";
			exit(0);
			break;
		}
		(value == 0) ? _registers.AF.setFlag('Z') : _registers.AF.clearFlag('Z');
	}

	void Cpu::RLC_16()
	{
		uint8_t value = _mmu.readRam(_registers.HL.get());
		(value & 0x80) ? _registers.AF.setFlag('C') : _registers.AF.clearFlag('C');
		uint8_t newValue = (value << 1) | static_cast<uint8_t>(_registers.AF.getFlag('C'));
		_mmu.writeRam(_registers.HL.get(), newValue);
		_registers.AF.clearFlag('H');
		_registers.AF.clearFlag('N');
		(newValue == 0) ? _registers.AF.setFlag('Z') : _registers.AF.clearFlag('Z');
	}

	void Cpu::RL_16()
	{
		uint8_t value = _mmu.readRam(_registers.HL.get());
		int oldCarry = _registers.AF.getFlag('C');
		(value & 0x80) ? _registers.AF.setFlag('C') : _registers.AF.clearFlag('C');
		value = (value << 1) | (oldCarry);
		(value == 0) ? _registers.AF.setFlag('Z') : _registers.AF.clearFlag('Z');
		_registers.AF.clearFlag('H');
		_registers.AF.clearFlag('N');
		_mmu.writeRam(_registers.HL.get(), value);
	}

	void Cpu::RL_r(const char &reg)
	{
		uint8_t value = 0, oldCarry = _registers.AF.getFlag('C');
		switch (reg)
		{
		case 'A':
			(_registers.AF.getLeftRegister() & 0x80) ? _registers.AF.setFlag('C') : _registers.AF.clearFlag('C');
			_registers.AF.setLeftRegister((_registers.AF.getLeftRegister() << 1) | oldCarry);
			_registers.AF.clearFlag('H');
			_registers.AF.clearFlag('N');
			value = _registers.AF.getLeftRegister();
			break;
		case 'B':
			(_registers.BC.getLeftRegister() & 0x80) ? _registers.AF.setFlag('C') : _registers.AF.clearFlag('C');
			_registers.BC.setLeftRegister((_registers.BC.getLeftRegister() << 1) | oldCarry);
			_registers.AF.clearFlag('H');
			_registers.AF.clearFlag('N');
			value = _registers.BC.getLeftRegister();
			break;
		case 'C':
			(_registers.BC.getRightRegister() & 0x80) ? _registers.AF.setFlag('C') : _registers.AF.clearFlag('C');
			_registers.BC.setRightRegister((_registers.BC.getRightRegister() << 1) | oldCarry);
			_registers.AF.clearFlag('H');
			_registers.AF.clearFlag('N');
			value = _registers.BC.getRightRegister();
			break;
		case 'D':
			(_registers.DE.getLeftRegister() & 0x80) ? _registers.AF.setFlag('C') : _registers.AF.clearFlag('C');
			_registers.DE.setLeftRegister((_registers.DE.getLeftRegister() << 1) | oldCarry);
			_registers.AF.clearFlag('H');
			_registers.AF.clearFlag('N');
			value = _registers.DE.getLeftRegister();
			break;
		case 'E':
			(_registers.DE.getRightRegister() & 0x80) ? _registers.AF.setFlag('C') : _registers.AF.clearFlag('C');
			_registers.DE.setRightRegister((_registers.DE.getRightRegister() << 1) | oldCarry);
			_registers.AF.clearFlag('H');
			_registers.AF.clearFlag('N');
			value = _registers.DE.getRightRegister();
			break;
		case 'H':
			(_registers.HL.getLeftRegister() & 0x80) ? _registers.AF.setFlag('C') : _registers.AF.clearFlag('C');
			_registers.HL.setLeftRegister((_registers.HL.getLeftRegister() << 1) | oldCarry);
			_registers.AF.clearFlag('H');
			_registers.AF.clearFlag('N');
			value = _registers.HL.getLeftRegister();
			break;
		case 'L':
			(_registers.HL.getRightRegister() & 0x80) ? _registers.AF.setFlag('C') : _registers.AF.clearFlag('C');
			_registers.HL.setRightRegister((_registers.HL.getRightRegister() << 1) | oldCarry);
			_registers.AF.clearFlag('H');
			_registers.AF.clearFlag('N');
			value = _registers.HL.getRightRegister();
			break;
		default:
			cout << "Flag error.";
			exit(0);
			break;
		}
		(value == 0) ? _registers.AF.setFlag('Z') : _registers.AF.clearFlag('Z');
	}

	void Cpu::RR_r(const char &reg)
	{
		uint8_t value = 0;
		int oldCarry = _registers.AF.getFlag('C');
		switch (reg)
		{
		case 'A':
			(_registers.AF.getLeftRegister() & 0x1) ? _registers.AF.setFlag('C') : _registers.AF.clearFlag('C');
			_registers.AF.setLeftRegister((_registers.AF.getLeftRegister() >> 1) | (oldCarry << 7));
			_registers.AF.clearFlag('H');
			_registers.AF.clearFlag('N');
			value = _registers.AF.getLeftRegister();
			break;
		case 'B':
			(_registers.BC.getLeftRegister() & 0x1) ? _registers.AF.setFlag('C') : _registers.AF.clearFlag('C');
			_registers.BC.setLeftRegister((_registers.BC.getLeftRegister() >> 1) | (oldCarry << 7));
			_registers.AF.clearFlag('H');
			_registers.AF.clearFlag('N');
			value = _registers.BC.getLeftRegister();
			break;
		case 'C':
			(_registers.BC.getRightRegister() & 0x1) ? _registers.AF.setFlag('C') : _registers.AF.clearFlag('C');
			_registers.BC.setRightRegister((_registers.BC.getRightRegister() >> 1) | (oldCarry << 7));
			_registers.AF.clearFlag('H');
			_registers.AF.clearFlag('N');
			value = _registers.BC.getRightRegister();
			break;
		case 'D':
			(_registers.DE.getLeftRegister() & 0x1) ? _registers.AF.setFlag('C') : _registers.AF.clearFlag('C');
			_registers.DE.setLeftRegister((_registers.DE.getLeftRegister() >> 1) | (oldCarry << 7));
			_registers.AF.clearFlag('H');
			_registers.AF.clearFlag('N');
			value = _registers.DE.getLeftRegister();
			break;
		case 'E':
			(_registers.DE.getRightRegister() & 0x1) ? _registers.AF.setFlag('C') : _registers.AF.clearFlag('C');
			_registers.DE.setRightRegister((_registers.DE.getRightRegister() >> 1) | (oldCarry << 7));
			_registers.AF.clearFlag('H');
			_registers.AF.clearFlag('N');
			value = _registers.DE.getRightRegister();
			break;
		case 'H':
			(_registers.HL.getLeftRegister() & 0x1) ? _registers.AF.setFlag('C') : _registers.AF.clearFlag('C');
			_registers.HL.setLeftRegister((_registers.HL.getLeftRegister() >> 1) | (oldCarry << 7));
			_registers.AF.clearFlag('H');
			_registers.AF.clearFlag('N');
			value = _registers.HL.getLeftRegister();
			break;
		case 'L':
			(_registers.HL.getRightRegister() & 0x1) ? _registers.AF.setFlag('C') : _registers.AF.clearFlag('C');
			_registers.HL.setRightRegister((_registers.HL.getRightRegister() >> 1) | (oldCarry << 7));
			_registers.AF.clearFlag('H');
			_registers.AF.clearFlag('N');
			value = _registers.HL.getRightRegister();
			break;
		default:
			cout << "Flag error.";
			exit(0);
			break;
		}
		(value == 0) ? _registers.AF.setFlag('Z') : _registers.AF.clearFlag('Z');
	}

	void Cpu::RR_16() // TODO may be innacurate
	{
		int oldCarry = _registers.AF.getFlag('C');
		uint8_t value = _mmu.readRam(_registers.HL.get());
		(value & 0x1) ? _registers.AF.setFlag('C') : _registers.AF.clearFlag('C');
		value = ((value >> 1) | (oldCarry << 7));
		_mmu.writeRam(_registers.HL.get(), value);
		(value == 0) ? _registers.AF.setFlag('Z') : _registers.AF.clearFlag('Z');
		_registers.AF.clearFlag('H');
		_registers.AF.clearFlag('N');
	}

	void Cpu::RRC_r(const char &reg)
	{
		uint8_t value = 0;
		switch (reg)
		{
		case 'A':
			(_registers.AF.getLeftRegister() & 0x1) ? _registers.AF.setFlag('C') : _registers.AF.clearFlag('C');
			_registers.AF.setLeftRegister((_registers.AF.getLeftRegister() >> 1) | (_registers.AF.getFlag('C') << 7));
			_registers.AF.clearFlag('H');
			_registers.AF.clearFlag('N');
			value = _registers.AF.getLeftRegister();
			break;
		case 'B':
			(_registers.BC.getLeftRegister() & 0x1) ? _registers.AF.setFlag('C') : _registers.AF.clearFlag('C');
			_registers.BC.setLeftRegister((_registers.BC.getLeftRegister() >> 1) | (_registers.AF.getFlag('C') << 7));
			_registers.AF.clearFlag('H');
			_registers.AF.clearFlag('N');
			value = _registers.BC.getLeftRegister();
			break;
		case 'C':
			(_registers.BC.getRightRegister() & 0x1) ? _registers.AF.setFlag('C') : _registers.AF.clearFlag('C');
			_registers.BC.setRightRegister((_registers.BC.getRightRegister() >> 1) | (_registers.AF.getFlag('C') << 7));
			_registers.AF.clearFlag('H');
			_registers.AF.clearFlag('N');
			value = _registers.BC.getRightRegister();
			break;
		case 'D':
			(_registers.DE.getLeftRegister() & 0x1) ? _registers.AF.setFlag('C') : _registers.AF.clearFlag('C');
			_registers.DE.setLeftRegister((_registers.DE.getLeftRegister() >> 1) | (_registers.AF.getFlag('C') << 7));
			_registers.AF.clearFlag('H');
			_registers.AF.clearFlag('N');
			value = _registers.DE.getLeftRegister();
			break;
		case 'E':
			(_registers.DE.getRightRegister() & 0x1) ? _registers.AF.setFlag('C') : _registers.AF.clearFlag('C');
			_registers.DE.setRightRegister((_registers.DE.getRightRegister() >> 1) | (_registers.AF.getFlag('C') << 7));
			_registers.AF.clearFlag('H');
			_registers.AF.clearFlag('N');
			value = _registers.DE.getRightRegister();
			break;
		case 'H':
			(_registers.HL.getLeftRegister() & 0x1) ? _registers.AF.setFlag('C') : _registers.AF.clearFlag('C');
			_registers.HL.setLeftRegister((_registers.HL.getLeftRegister() >> 1) | (_registers.AF.getFlag('C') << 7));
			_registers.AF.clearFlag('H');
			_registers.AF.clearFlag('N');
			value = _registers.HL.getLeftRegister();
			break;
		case 'L':
			(_registers.HL.getRightRegister() & 0x1) ? _registers.AF.setFlag('C') : _registers.AF.clearFlag('C');
			_registers.HL.setRightRegister((_registers.HL.getRightRegister() >> 1) | (_registers.AF.getFlag('C') << 7));
			_registers.AF.clearFlag('H');
			_registers.AF.clearFlag('N');
			value = _registers.HL.getRightRegister();
			break;
		default:
			cout << "Flag error.";
			exit(0);
			break;
		}
		(value == 0) ? _registers.AF.setFlag('Z') : _registers.AF.clearFlag('Z');
	}

	void Cpu::RRC_16()
	{
		uint8_t value = _mmu.readRam(_registers.HL.get());
		(value & 0x1) ? _registers.AF.setFlag('C') : _registers.AF.clearFlag('C');
		value = ((value >> 1) | (_registers.AF.getFlag('C') << 7));
		(value == 0) ? _registers.AF.setFlag('Z') : _registers.AF.clearFlag('Z');
		_registers.AF.clearFlag('H');
		_registers.AF.clearFlag('N');
		_mmu.writeRam(_registers.HL.get(), value);
	}

	void Cpu::SLA_r(const char &reg)
	{
		uint8_t value = 0, oldCarry = _registers.AF.getFlag('C');
		switch (reg)
		{
		case 'A':
			(_registers.AF.getLeftRegister() & 0x80) ? _registers.AF.setFlag('C') : _registers.AF.clearFlag('C');
			_registers.AF.setLeftRegister((_registers.AF.getLeftRegister() << 1));
			_registers.AF.clearFlag('H');
			_registers.AF.clearFlag('N');
			value = _registers.AF.getLeftRegister();
			break;
		case 'B':
			(_registers.BC.getLeftRegister() & 0x80) ? _registers.AF.setFlag('C') : _registers.AF.clearFlag('C');
			_registers.BC.setLeftRegister((_registers.BC.getLeftRegister() << 1));
			_registers.AF.clearFlag('H');
			_registers.AF.clearFlag('N');
			value = _registers.BC.getLeftRegister();
			break;
		case 'C':
			(_registers.BC.getRightRegister() & 0x80) ? _registers.AF.setFlag('C') : _registers.AF.clearFlag('C');
			_registers.BC.setRightRegister((_registers.BC.getRightRegister() << 1));
			_registers.AF.clearFlag('H');
			_registers.AF.clearFlag('N');
			value = _registers.BC.getRightRegister();
			break;
		case 'D':
			(_registers.DE.getLeftRegister() & 0x80) ? _registers.AF.setFlag('C') : _registers.AF.clearFlag('C');
			_registers.DE.setLeftRegister((_registers.DE.getLeftRegister() << 1));
			_registers.AF.clearFlag('H');
			_registers.AF.clearFlag('N');
			value = _registers.DE.getLeftRegister();
			break;
		case 'E':
			(_registers.DE.getRightRegister() & 0x80) ? _registers.AF.setFlag('C') : _registers.AF.clearFlag('C');
			_registers.DE.setRightRegister((_registers.DE.getRightRegister() << 1));
			_registers.AF.clearFlag('H');
			_registers.AF.clearFlag('N');
			value = _registers.DE.getRightRegister();
			break;
		case 'H':
			(_registers.HL.getLeftRegister() & 0x80) ? _registers.AF.setFlag('C') : _registers.AF.clearFlag('C');
			_registers.HL.setLeftRegister((_registers.HL.getLeftRegister() << 1));
			_registers.AF.clearFlag('H');
			_registers.AF.clearFlag('N');
			value = _registers.HL.getLeftRegister();
			break;
		case 'L':
			(_registers.HL.getRightRegister() & 0x80) ? _registers.AF.setFlag('C') : _registers.AF.clearFlag('C');
			_registers.HL.setRightRegister((_registers.HL.getRightRegister() << 1));
			_registers.AF.clearFlag('H');
			_registers.AF.clearFlag('N');
			value = _registers.HL.getRightRegister();
			break;
		default:
			cout << "Flag error.";
			exit(0);
			break;
		}
		(value == 0) ? _registers.AF.setFlag('Z') : _registers.AF.clearFlag('Z');
	}

	void Cpu::SLA_16()
	{
		uint8_t value = _mmu.readRam(_registers.HL.get()), oldCarry = _registers.AF.getFlag('C');
		(value & 0x80) ? _registers.AF.setFlag('C') : _registers.AF.clearFlag('C');
		_mmu.writeRam(_registers.HL.get(), (value << 1));
		_registers.AF.clearFlag('H');
		_registers.AF.clearFlag('N');
		value = _mmu.readRam(_registers.HL.get());
		(value == 0) ? _registers.AF.setFlag('Z') : _registers.AF.clearFlag('Z');
	}

	void Cpu::SRA_r(const char &reg)
	{
		uint8_t value = 0;
		uint8_t old7thbit = 0;
		switch (reg)
		{
		case 'A':
			(_registers.AF.getLeftRegister() & 0x1) ? _registers.AF.setFlag('C') : _registers.AF.clearFlag('C');
			old7thbit = (_registers.AF.getLeftRegister() & 0x80);
			_registers.AF.setLeftRegister((_registers.AF.getLeftRegister() >> 1) | (old7thbit));
			_registers.AF.clearFlag('H');
			_registers.AF.clearFlag('N');
			value = _registers.AF.getLeftRegister();
			break;
		case 'B':
			(_registers.BC.getLeftRegister() & 0x1) ? _registers.AF.setFlag('C') : _registers.AF.clearFlag('C');
			old7thbit = (_registers.BC.getLeftRegister() & 0x80);
			_registers.BC.setLeftRegister((_registers.BC.getLeftRegister() >> 1) | (old7thbit));
			_registers.AF.clearFlag('H');
			_registers.AF.clearFlag('N');
			value = _registers.BC.getLeftRegister();
			break;
		case 'C':
			(_registers.BC.getRightRegister() & 0x1) ? _registers.AF.setFlag('C') : _registers.AF.clearFlag('C');
			old7thbit = (_registers.BC.getRightRegister() & 0x80);
			_registers.BC.setRightRegister((_registers.BC.getRightRegister() >> 1) | (old7thbit));
			_registers.AF.clearFlag('H');
			_registers.AF.clearFlag('N');
			value = _registers.BC.getRightRegister();
			break;
		case 'D':
			(_registers.DE.getLeftRegister() & 0x1) ? _registers.AF.setFlag('C') : _registers.AF.clearFlag('C');
			old7thbit = (_registers.DE.getLeftRegister() & 0x80);
			_registers.DE.setLeftRegister((_registers.DE.getLeftRegister() >> 1) | (old7thbit));
			_registers.AF.clearFlag('H');
			_registers.AF.clearFlag('N');
			value = _registers.DE.getLeftRegister();
			break;
		case 'E':
			(_registers.DE.getRightRegister() & 0x1) ? _registers.AF.setFlag('C') : _registers.AF.clearFlag('C');
			old7thbit = (_registers.DE.getRightRegister() & 0x80);
			_registers.DE.setRightRegister((_registers.DE.getRightRegister() >> 1) | (old7thbit));
			_registers.AF.clearFlag('H');
			_registers.AF.clearFlag('N');
			value = _registers.DE.getRightRegister();
			break;
		case 'H':
			(_registers.HL.getLeftRegister() & 0x1) ? _registers.AF.setFlag('C') : _registers.AF.clearFlag('C');
			old7thbit = (_registers.HL.getLeftRegister() & 0x80);
			_registers.HL.setLeftRegister((_registers.HL.getLeftRegister() >> 1) | (old7thbit));
			_registers.AF.clearFlag('H');
			_registers.AF.clearFlag('N');
			value = _registers.HL.getLeftRegister();
			break;
		case 'L':
			(_registers.HL.getRightRegister() & 0x1) ? _registers.AF.setFlag('C') : _registers.AF.clearFlag('C');
			old7thbit = (_registers.HL.getRightRegister() & 0x80);
			_registers.HL.setRightRegister((_registers.HL.getRightRegister() >> 1) | (old7thbit));
			_registers.AF.clearFlag('H');
			_registers.AF.clearFlag('N');
			value = _registers.HL.getRightRegister();
			break;
		default:
			cout << "Flag error.";
			exit(0);
			break;
		}
		(value == 0) ? _registers.AF.setFlag('Z') : _registers.AF.clearFlag('Z');
	}

	void Cpu::SRA_16()
	{
		uint8_t value = _mmu.readRam(_registers.HL.get());
		int old7bit = (value & 0x80);
		(value & 0x1) ? _registers.AF.setFlag('C') : _registers.AF.clearFlag('C');
		value = ((value >> 1) | (old7bit));
		_mmu.writeRam(_registers.HL.get(), value);
		(value == 0) ? _registers.AF.setFlag('Z') : _registers.AF.clearFlag('Z');
		_registers.AF.clearFlag('H');
		_registers.AF.clearFlag('N');
	}

	void Cpu::SRL_r(const char &reg)
	{
		uint8_t value = 0;
		switch (reg)
		{
		case 'A':
			(_registers.AF.getLeftRegister() & 0x1) ? _registers.AF.setFlag('C') : _registers.AF.clearFlag('C');
			_registers.AF.setLeftRegister((_registers.AF.getLeftRegister() >> 1));
			(_registers.AF.getLeftRegister() == 0) ? _registers.AF.setFlag('Z') : _registers.AF.clearFlag('Z');
			_registers.AF.clearFlag('H');
			_registers.AF.clearFlag('N');
			break;
		case 'B':
			(_registers.BC.getLeftRegister() & 0x1) ? _registers.AF.setFlag('C') : _registers.AF.clearFlag('C');
			_registers.BC.setLeftRegister((_registers.BC.getLeftRegister() >> 1));
			_registers.AF.clearFlag('H');
			_registers.AF.clearFlag('N');
			(_registers.BC.getLeftRegister() == 0) ? _registers.AF.setFlag('Z') : _registers.AF.clearFlag('Z');
			break;
		case 'C':
			(_registers.BC.getRightRegister() & 0x1) ? _registers.AF.setFlag('C') : _registers.AF.clearFlag('C');
			_registers.BC.setRightRegister((_registers.BC.getRightRegister() >> 1));
			_registers.AF.clearFlag('H');
			_registers.AF.clearFlag('N');
			(_registers.BC.getRightRegister() == 0) ? _registers.AF.setFlag('Z') : _registers.AF.clearFlag('Z');
			break;
		case 'D':
			(_registers.DE.getLeftRegister() & 0x1) ? _registers.AF.setFlag('C') : _registers.AF.clearFlag('C');
			_registers.DE.setLeftRegister((_registers.DE.getLeftRegister() >> 1));
			_registers.AF.clearFlag('H');
			_registers.AF.clearFlag('N');
			(_registers.DE.getLeftRegister() == 0) ? _registers.AF.setFlag('Z') : _registers.AF.clearFlag('Z');
			break;
		case 'E':
			(_registers.DE.getRightRegister() & 0x1) ? _registers.AF.setFlag('C') : _registers.AF.clearFlag('C');
			_registers.DE.setRightRegister((_registers.DE.getRightRegister() >> 1));
			_registers.AF.clearFlag('H');
			_registers.AF.clearFlag('N');
			(_registers.DE.getRightRegister() == 0) ? _registers.AF.setFlag('Z') : _registers.AF.clearFlag('Z');
			break;
		case 'H':
			(_registers.HL.getLeftRegister() & 0x1) ? _registers.AF.setFlag('C') : _registers.AF.clearFlag('C');
			_registers.HL.setLeftRegister((_registers.HL.getLeftRegister() >> 1));
			_registers.AF.clearFlag('H');
			_registers.AF.clearFlag('N');
			(_registers.HL.getLeftRegister() == 0) ? _registers.AF.setFlag('Z') : _registers.AF.clearFlag('Z');
			break;
		case 'L':
			(_registers.HL.getRightRegister() & 0x1) ? _registers.AF.setFlag('C') : _registers.AF.clearFlag('C');
			_registers.HL.setRightRegister((_registers.HL.getRightRegister() >> 1));
			_registers.AF.clearFlag('H');
			_registers.AF.clearFlag('N');
			(_registers.HL.getRightRegister() == 0) ? _registers.AF.setFlag('Z') : _registers.AF.clearFlag('Z');
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
		(_mmu.readRam(_registers.HL.get()) & 0x1) ? _registers.AF.setFlag('C') : _registers.AF.clearFlag('C');
		_mmu.writeRam(_registers.HL.get(), (_mmu.readRam(_registers.HL.get()) >> 1));
		_registers.AF.clearFlag('H');
		_registers.AF.clearFlag('N');
		value = _mmu.readRam(_registers.HL.get());
		(value == 0) ? _registers.AF.setFlag('Z') : _registers.AF.clearFlag('Z');
	}

	void Cpu::SWAP_r(const char &reg)
	{
		uint8_t left = 0, rigth = 0, value = 0;
		switch (reg)
		{
		case 'A':
			left = _registers.AF.getLeftRegister() & 0xF0;
			rigth = _registers.AF.getLeftRegister() & 0xF;
			value = ((left >> 4) | (rigth << 4));
			_registers.AF.setLeftRegister(value);
			break;
		case 'B':
			left = _registers.BC.getLeftRegister() & 0xF0;
			rigth = _registers.BC.getLeftRegister() & 0xF;
			value = ((left >> 4) | (rigth << 4));
			_registers.BC.setLeftRegister(value);
			break;
		case 'C':
			left = _registers.BC.getRightRegister() & 0xF0;
			rigth = _registers.BC.getRightRegister() & 0xF;
			value = ((left >> 4) | (rigth << 4));
			_registers.BC.setRightRegister(value);
			break;
		case 'D':
			left = _registers.DE.getLeftRegister() & 0xF0;
			rigth = _registers.DE.getLeftRegister() & 0xF;
			value = ((left >> 4) | (rigth << 4));
			_registers.DE.setLeftRegister(value);
			break;
		case 'E':
			left = _registers.DE.getRightRegister() & 0xF0;
			rigth = _registers.DE.getRightRegister() & 0xF;
			value = ((left >> 4) | (rigth << 4));
			_registers.DE.setRightRegister(value);
			break;
		case 'H':
			left = _registers.HL.getLeftRegister() & 0xF0;
			rigth = _registers.HL.getLeftRegister() & 0xF;
			value = ((left >> 4) | (rigth << 4));
			_registers.HL.setLeftRegister(value);
			break;
		case 'L':
			left = _registers.HL.getRightRegister() & 0xF0;
			rigth = _registers.HL.getRightRegister() & 0xF;
			value = ((left >> 4) | (rigth << 4));
			_registers.HL.setRightRegister(value);
			break;
		}
		_registers.AF.clearFlag('N');
		_registers.AF.clearFlag('C');
		_registers.AF.clearFlag('H');
		(value == 0) ? _registers.AF.setFlag('Z') : _registers.AF.clearFlag('Z');
	}

	void Cpu::SWAP_16()
	{
		uint8_t left = 0, rigth = 0, value = _mmu.readRam(_registers.HL.get());
		left = (value & 0xF0);
		rigth = (value & 0xF);
		value = ((left >> 4) | (rigth << 4));
		_mmu.writeRam(_registers.HL.get(), value);
		(value == 0) ? _registers.AF.setFlag('Z') : _registers.AF.clearFlag('Z');
		_registers.AF.clearFlag('N');
		_registers.AF.clearFlag('H');
		_registers.AF.clearFlag('C');
	}

	void Cpu::BIT_b_r(const int &bit, const char &reg)
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
			value = _registers.AF.getLeftRegister();
			break;
		case 'F':
			value = _registers.AF.getRightRegister();
			break;
		case 'B':
			value = _registers.BC.getLeftRegister();
			break;
		case 'C':
			value = _registers.BC.getRightRegister();
			break;
		case 'D':
			value = _registers.DE.getLeftRegister();
			break;
		case 'E':
			value = _registers.DE.getRightRegister();
			break;
		case 'H':
			value = _registers.HL.getLeftRegister();
			break;
		case 'L':
			value = _registers.HL.getRightRegister();
			break;
		default:
			cout << "Flag error.";
			exit(0);
			break;
		}
		(value & (1 << bit)) ? _registers.AF.clearFlag('Z') : _registers.AF.setFlag('Z');
		_registers.AF.clearFlag('N');
		_registers.AF.setFlag('H');
	}

	void Cpu::BIT_b_16(const int &bit)
	{
		if (bit > 7 || bit < 0)
		{
			cout << "Bit to check out of bound" << endl;
			exit(3);
		}
		uint8_t value = _mmu.readRam(_registers.HL.get());
		(value & (1 << bit)) ? _registers.AF.clearFlag('Z') : _registers.AF.setFlag('Z');
		_registers.AF.clearFlag('N');
		_registers.AF.setFlag('H');
	}

	void Cpu::SET_b_r(const int &bit, const char &reg)
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
			_registers.AF.setLeftRegister(_registers.AF.getLeftRegister() | value);
			break;
		case 'F':
			_registers.AF.setRightRegister(_registers.AF.getRightRegister() | value);
			break;
		case 'B':
			_registers.BC.setLeftRegister(_registers.BC.getLeftRegister() | value);
			break;
		case 'C':
			_registers.BC.setRightRegister(_registers.BC.getRightRegister() | value);
			break;
		case 'D':
			_registers.DE.setLeftRegister(_registers.DE.getLeftRegister() | value);
			break;
		case 'E':
			_registers.DE.setRightRegister(_registers.DE.getRightRegister() | value);
			break;
		case 'H':
			_registers.HL.setLeftRegister(_registers.HL.getLeftRegister() | value);
			break;
		case 'L':
			_registers.HL.setRightRegister(_registers.HL.getRightRegister() | value);
			break;
		default:
			cout << "Flag error.";
			exit(0);
			break;
		}
	}

	void Cpu::SET_b_16(const int &bit)
	{
		if (bit > 7 || bit < 0)
		{
			cout << "Bit to check out of bound" << endl;
			exit(3);
		}
		uint8_t value = _mmu.readRam(_registers.HL.get());
		_mmu.writeRam(_registers.HL.get(), (value | (1 << bit)));
	}

	void Cpu::RES_b_r(const int &bit, const char &reg)
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
			_registers.AF.setLeftRegister(_registers.AF.getLeftRegister() & ~value);
			break;
		case 'B':
			_registers.BC.setLeftRegister(_registers.BC.getLeftRegister() & ~value);
			break;
		case 'C':
			_registers.BC.setRightRegister(_registers.BC.getRightRegister() & ~value);
			break;
		case 'D':
			_registers.DE.setLeftRegister(_registers.DE.getLeftRegister() & ~value);
			break;
		case 'E':
			_registers.DE.setRightRegister(_registers.DE.getRightRegister() & ~value);
			break;
		case 'H':
			_registers.HL.setLeftRegister(_registers.HL.getLeftRegister() & ~value);
			break;
		case 'L':
			_registers.HL.setRightRegister(_registers.HL.getRightRegister() & ~value);
			break;
		default:
			cout << "Flag error.";
			exit(0);
			break;
		}
	}

	void Cpu::RES_b_16(const int &bit)
	{
		if (bit > 7 || bit < 0)
		{
			cout << "Bit to check out of bound" << endl;
			exit(3);
		}
		uint8_t value = ~(1 << bit);
		_mmu.writeRam(_registers.HL.get(), _mmu.readRam(_registers.HL.get()) & value);
	}

	void Cpu::JP_16(const uint16_t &adress)
	{
		uint16_t leftValue = _mmu.readRam(adress + 1);
		uint8_t rigthValue = _mmu.readRam(adress);
		_registers.PC = ((leftValue << 8) | rigthValue);
	}

	void Cpu::JP_c_16(const std::string &condition, const uint16_t &adress)
	{
		if (condition == "Z")
			(_registers.AF.getFlag('Z')) ? _registers.PC = adress : _registers.PC += 3;
		else if (condition == "NZ")
			(!_registers.AF.getFlag('Z')) ? _registers.PC = adress : _registers.PC += 3;
		else if (condition == "C")
			(_registers.AF.getFlag('C')) ? _registers.PC = adress : _registers.PC += 3;
		else if (condition == "NC")
			(!_registers.AF.getFlag('C')) ? _registers.PC = adress : _registers.PC += 3;
	}

	void Cpu::JR_e(const uint8_t &value)
	{
		_registers.PC += 2;
		_registers.PC += static_cast<int8_t>(value);
	}

	void Cpu::JR_C_e(const uint8_t &value)
	{
		_registers.PC += 2;
		if (_registers.AF.getFlag('C'))
			_registers.PC += static_cast<int8_t>(value);
	}

	void Cpu::JR_NC_e(const uint8_t &value)
	{
		_registers.PC += 2;
		if (!_registers.AF.getFlag('C'))
			_registers.PC += static_cast<int8_t>(value);
	}

	void Cpu::JR_Z_e(const uint8_t &value)
	{
		_registers.PC += 2;
		if (_registers.AF.getFlag('Z'))
			_registers.PC += static_cast<int8_t>(value);
	}

	void Cpu::JR_NZ_e(const uint8_t &value)
	{
		_registers.PC += 2;
		if (!_registers.AF.getFlag('Z'))
			_registers.PC += static_cast<int8_t>(value);
	}

	void Cpu::JP_16()
	{
		_registers.PC = _registers.HL.get();
	}

	void Cpu::CALL()
	{
		uint8_t leftValue = _mmu.readRam(_registers.PC + 2);
		uint8_t rightValue = _mmu.readRam(_registers.PC + 1);
		_registers.SP--;
		_mmu.writeRam(_registers.SP, (((_registers.PC + 3) & 0xFF00) >> 8));
		_registers.SP--;
		_mmu.writeRam(_registers.SP, static_cast<uint8_t>((_registers.PC + 3) & 0xFF));
		_registers.PC = ((leftValue << 8) | rightValue);
	}

	void Cpu::CALL_c(const std::string &condition)
	{
		uint8_t leftValue = _mmu.readRam(_registers.PC + 2);
		uint8_t rightValue = _mmu.readRam(_registers.PC + 1);
		if (condition == "Z")
		{
			if (_registers.AF.getFlag('Z'))
			{

				_registers.SP--;
				_mmu.writeRam(_registers.SP, (((_registers.PC + 3) & 0xFF00) >> 8));
				_registers.SP--;
				_mmu.writeRam(_registers.SP, static_cast<uint8_t>((_registers.PC + 3) & 0xFF));
				_registers.PC = ((leftValue << 8) | rightValue);
				return;
			}
			else
			{
				_registers.PC += 3;
				return;
			}
		}

		else if (condition == "NZ")
		{
			if (!_registers.AF.getFlag('Z'))
			{

				_registers.SP--;
				_mmu.writeRam(_registers.SP, (((_registers.PC + 3) & 0xFF00) >> 8));
				_registers.SP--;
				_mmu.writeRam(_registers.SP, static_cast<uint8_t>((_registers.PC + 3) & 0xFF));
				_registers.PC = ((leftValue << 8) | rightValue);
				return;
			}
			else
			{
				_registers.PC += 3;
				return;
			}
		}
		else if (condition == "C")
		{
			if (_registers.AF.getFlag('C'))
			{

				_registers.SP--;
				_mmu.writeRam(_registers.SP, (((_registers.PC + 3) & 0xFF00) >> 8));
				_registers.SP--;
				_mmu.writeRam(_registers.SP, static_cast<uint8_t>((_registers.PC + 3) & 0xFF));
				_registers.PC = ((leftValue << 8) | rightValue);
				return;
			}
			else
			{
				_registers.PC += 3;
				return;
			}
		}
		else if (condition == "NC")
		{
			if (!_registers.AF.getFlag('C'))
			{

				_registers.SP--;
				_mmu.writeRam(_registers.SP, (((_registers.PC + 3) & 0xFF00) >> 8));
				_registers.SP--;
				_mmu.writeRam(_registers.SP, static_cast<uint8_t>((_registers.PC + 3) & 0xFF));
				_registers.PC = ((leftValue << 8) | rightValue);
				return;
			}
			else
			{
				_registers.PC += 3;
				return;
			}
		}
	}

	void Cpu::RET()
	{
		uint8_t firstByte = _mmu.readRam(_registers.SP + 1);
		uint16_t secondByte = (_mmu.readRam(_registers.SP));
		_registers.SP += 2;
		_registers.PC = ((firstByte << 8) | secondByte);
	}

	void Cpu::RET_c(const std::string &condition)
	{
		if (condition == "Z")
		{
			if (_registers.AF.getFlag('Z'))
			{
				uint16_t leftNibble = (_mmu.readRam(_registers.SP + 1) << 8);
				uint8_t rightNibble = _mmu.readRam(_registers.SP);
				_registers.SP += 2;
				_registers.PC = (leftNibble | rightNibble);
				return;
			}
			else
			{
				_registers.PC++;
				return;
			}
		}

		if (condition == "NZ")
		{
			if (!_registers.AF.getFlag('Z'))
			{
				uint16_t leftNibble = (_mmu.readRam(_registers.SP + 1) << 8);
				uint8_t rightNibble = _mmu.readRam(_registers.SP);
				_registers.SP += 2;
				_registers.PC = (leftNibble | rightNibble);
				return;
			}
			else
			{
				_registers.PC++;
				return;
			}
		}
		if (condition == "C")
		{
			if (_registers.AF.getFlag('C'))
			{
				uint16_t leftNibble = (_mmu.readRam(_registers.SP + 1) << 8);
				uint8_t rightNibble = _mmu.readRam(_registers.SP);
				_registers.SP += 2;
				_registers.PC = (leftNibble | rightNibble);
				return;
			}
			else
			{
				_registers.PC++;
				return;
			}
		}
		if (condition == "NC")
		{
			if (!_registers.AF.getFlag('C'))
			{
				uint16_t leftNibble = (_mmu.readRam(_registers.SP + 1) << 8);
				uint8_t rightNibble = _mmu.readRam(_registers.SP);
				_registers.SP += 2;
				_registers.PC = (leftNibble | rightNibble);
				return;
			}
			else
			{
				_registers.PC++;
				return;
			}
		}
	}

	void Cpu::RETI() // TODO  Unknown if the IME is enabled after execution of this
	{
		_registers.setInterruptEnabled(true);
		uint16_t leftNibble = (_mmu.readRam(_registers.SP + 1) << 8);
		uint8_t rightNibble = _mmu.readRam(_registers.SP);
		_registers.SP += 2;
		_registers.PC = (leftNibble | rightNibble);
		_interruptManager.setMasterInterrupt(true);
	}

	void Cpu::RST_p(const uint16_t &p)
	{
		_registers.SP--;
		_mmu.writeRam(_registers.SP, (((_registers.PC + 1) & 0xFF00) >> 8));
		_registers.SP--;
		_mmu.writeRam(_registers.SP, ((_registers.PC + 1) & 0x00FF));
		_registers.PC = p;
	}

	bool Cpu::checkAddHalfCarry(const uint8_t &a, const uint8_t &b, const uint8_t &c)
	{
		return ((((a & 0xF) + (b & 0xF)) & 0x10) == 0x10);
	}

	bool Cpu::checkAddHalfCarry(const uint16_t &a, const uint16_t &b)
	{
		return ((a & 0xFF) + (b & 0xFF) >= 0x100);
	}

	bool Cpu::checkAddHalfCarry(const uint16_t &a, const uint16_t &b, const uint16_t &c)
	{
		return ((((a & 0xFFF) + (b & 0xFFF)) & 0x800) == 0x800);
	}

	bool Cpu::checkSubHalfCarry(const uint8_t &a, const uint8_t &b)
	{
		return (((a & 0xF) - (b & 0xF)) < 0);
	}

	bool Cpu::checkSubHalfCarry(const uint16_t &a, const uint16_t &b)
	{
		return (((a & 0xF000) - (b & 0xF000)) < 0);
	}

	bool Cpu::checkSubCarry(const uint16_t &a, const uint16_t &b)
	{
		return (((a & 0x8000) - (b & 0x8000)) < 0);
	}

	bool Cpu::checkAddCarry(const uint8_t &a, const uint8_t &b, const uint8_t &c)
	{
		return ((uint16_t)a + (uint16_t)b) >= 0x100;
	}

	bool Cpu::checkAddCarry(const uint16_t &a, const uint16_t &b, const uint16_t &c)
	{
		bool firstBit = a & 0x8000, secondtBit = b & 0x8000;
		return (firstBit & secondtBit) ? true : false;
	}

	uint16_t Cpu::next2bytes(const uint16_t &adress)
	{
		uint8_t leftValue = _mmu.readRam(adress + 1);
		uint16_t value = ((leftValue << 8) | (_mmu.readRam(adress)));
		return value;
	}

	long Cpu::step()
	{
		if (state == State::RUNNING || state == State::STEPPING)
		{
			if (_mmu.readRam(0xFF50) == 0x1 && _mmu.isInBios())
				_mmu.disableBios();

			if (!_registers.getHalted())
			{
				fetch();
				execute();
				return _cycle;
			}
			else
			{
				if ((_mmu.readRam(0xFF0F) & 0xF) > 0)
				{
					_registers.setHalted(false);
					_registers.PC++;
				}
				return 4;
			}
		}

		return 0;
	}

	void Cpu::fetch()
	{
		_currentOpcode = _mmu.readRam(_registers.PC);
	}

	void Cpu::execute()
	{
		uint16_t prevPC = _registers.PC;
		switch (_currentOpcode)
		{
		case 0x0:
			_registers.PC++;
			_cycle = 4;
			break;
		case 0x01:
			LD_rr_16(_registers.PC + 1, "BC");
			_cycle = 12;
			_registers.PC += 3;
			break;
		case 0x02:
			LD_16_r(_registers.BC.get(), 'A');
			_cycle = 8;
			_registers.PC++;
			break;
		case 0x03:
			INC_rr("BC");
			_cycle = 8;
			_registers.PC++;
			break;
		case 0x04:
			INC_r('B');
			_cycle = 4;
			_registers.PC++;
			break;
		case 0x05:
			DEC_r('B');
			_cycle = 4;
			_registers.PC++;
			break;
		case 0x06:
			LD_r_n(_mmu.readRam(_registers.PC + 1), 'B');
			_cycle = 8;
			_registers.PC += 2;
			break;
		case 0x07:
			RLCA();
			_registers.PC++;
			_cycle = 4;
			break;
		case 0x08:
			LD_16_rr(next2bytes(_registers.PC + 1), "SP");
			_registers.PC += 3;
			_cycle = 20;
			break;
		case 0x09:
			ADD_HL_rr("BC");
			_cycle = 8;
			_registers.PC++;
			break;
		case 0x0A:
			LD_r_16(_registers.BC.get(), 'A');
			_cycle = 8;
			_registers.PC++;
			break;
		case 0x0B:
			DEC_rr("BC");
			_cycle = 8;
			_registers.PC++;
			break;
		case 0x0C:
			INC_r('C');
			_cycle = 4;
			_registers.PC++;
			break;
		case 0x0D:
			DEC_r('C');
			_cycle = 4;
			_registers.PC++;
			break;
		case 0x0E:
			LD_r_n(_mmu.readRam(_registers.PC + 1), 'C');
			_cycle = 8;
			_registers.PC += 2;
			break;
		case 0x0F:
			RRCA();
			_cycle = 4;
			_registers.PC++;
			break;
		case 0x10:
			_cycle = 4;
			_registers.setHalted(true);
			_registers.PC++;
			Timer::resetDIV();
			break;
		case 0x11:
			LD_rr_16(_registers.PC + 1, "DE");
			_cycle = 12;
			_registers.PC += 3;
			break;
		case 0x12:
			LD_16_r(_registers.DE.get(), 'A');
			_cycle = 8;
			_registers.PC++;
			break;
		case 0x13:
			INC_rr("DE");
			_cycle = 8;
			_registers.PC++;
			break;
		case 0x14:
			INC_r('D');
			_cycle = 4;
			_registers.PC++;
			break;
		case 0x15:
			DEC_r('D');
			_cycle = 4;
			_registers.PC++;
			break;
		case 0x16:
			LD_r_n(_mmu.readRam(_registers.PC + 1), 'D');
			_registers.PC += 2;
			_cycle = 8;
			break;
		case 0x17:
			RLA();
			_registers.PC++;
			_cycle = 4;
			break;
		case 0x18:
			JR_e(_mmu.readRam(_registers.PC + 1));
			_cycle = 12;
			break;
		case 0x19:
			ADD_HL_rr("DE");
			_cycle = 8;
			_registers.PC++;
			break;
		case 0x1A:
			LD_r_16(_registers.DE.get(), 'A');
			_cycle = 8;
			_registers.PC++;
			break;
		case 0x1B:
			DEC_rr("DE");
			_cycle = 8;
			_registers.PC++;
			break;
		case 0x1C:
			INC_r('E');
			_cycle = 4;
			_registers.PC++;
			break;
		case 0x1D:
			DEC_r('E');
			_cycle = 4;
			_registers.PC++;
			break;
		case 0x1E:
			LD_r_n(_mmu.readRam(_registers.PC + 1), 'E');
			_registers.PC += 2;
			_cycle = 8;
			break;
		case 0x1F:
			RRA();
			_cycle = 4;
			_registers.PC++;
			break;
		case 0x20:
			JR_NZ_e(_mmu.readRam(_registers.PC + 1));
			(_registers.AF.getFlag('Z')) ? _cycle = 12 : _cycle = 16;
			break;
		case 0x21:
			LD_rr_nn(next2bytes(_registers.PC + 1), "HL");
			_registers.PC += 3;
			_cycle = 12;
			break;
		case 0x22:
			LD_16_r(_registers.HL.get(), 'A');
			INC_rr("HL");
			_cycle = 8;
			_registers.PC++;
			break;
		case 0x23:
			INC_rr("HL");
			_cycle = 8;
			_registers.PC++;
			break;
		case 0x24:
			INC_r('H');
			_cycle = 4;
			_registers.PC++;
			break;
		case 0x25:
			DEC_r('H');
			_cycle = 4;
			_registers.PC++;
			break;
		case 0x26:
			LD_r_n(_mmu.readRam(_registers.PC + 1), 'H');
			_cycle = 8;
			_registers.PC += 2;
			break;
		case 0x27:
			DAA();
			_registers.PC++;
			_cycle = 4;
			break;
		case 0x28:
			JR_Z_e(_mmu.readRam(_registers.PC + 1));
			_cycle = 12;
			break;
		case 0x29:
			ADD_HL_rr("HL");
			_registers.PC++;
			_cycle = 8;
			break;
		case 0x2A:
			LD_r_n(_mmu.readRam(_registers.HL.get()), 'A');
			INC_rr("HL");
			_cycle = 8;
			_registers.PC++;
			break;
		case 0x2B:
			DEC_rr("HL");
			_cycle = 8;
			_registers.PC++;
			break;
		case 0x2C:
			INC_r('L');
			_cycle = 4;
			_registers.PC++;
			break;
		case 0x2D:
			DEC_r('L');
			_cycle = 4;
			_registers.PC++;
			break;
		case 0x2E:
			LD_r_n(_mmu.readRam(_registers.PC + 1), 'L');
			_registers.PC += 2;
			_cycle = 8;
			break;
		case 0x2F:
			CPL();
			_registers.PC++;
			_cycle = 4;
			break;
		case 0x30:
			JR_NC_e(_mmu.readRam(_registers.PC + 1));
			_cycle = 12;
			break;
		case 0x31:
			LD_rr_nn(next2bytes(_registers.PC + 1), "SP");
			_registers.PC += 3;
			_cycle = 12;
			break;
		case 0x32:
			LD_16_r(_registers.HL.get(), 'A');
			DEC_rr("HL");
			_cycle = 8;
			_registers.PC++;
			break;
		case 0x33:
			INC_rr("SP");
			_cycle = 8;
			_registers.PC++;
			break;
		case 0x34:
			INC_16();
			_cycle = 12;
			_registers.PC++;
			break;
		case 0x35:
			DEC_16();
			_cycle = 12;
			_registers.PC++;
			break;
		case 0x36: // TODO may be innacurate
			LD_16_n(_registers.HL.get(), _mmu.readRam(_registers.PC + 1));
			_registers.PC += 2;
			_cycle = 12;
			break;
		case 0x37:
			SCF();
			_cycle = 4;
			_registers.PC++;
			break;
		case 0x38:
			JR_C_e(_mmu.readRam(_registers.PC + 1));
			_cycle = 12;
			break;
		case 0x39:
			ADD_HL_rr("SP");
			_cycle = 8;
			_registers.PC++;
			break;
		case 0x3A:
			LD_r_16(_registers.HL.get(), 'A');
			DEC_rr("HL");
			_cycle = 8;
			_registers.PC++;
			break;
		case 0x3B:
			DEC_rr("SP");
			_cycle = 8;
			_registers.PC++;
			break;
		case 0x3C:
			INC_r('A');
			_cycle = 4;
			_registers.PC++;
			break;
		case 0x3D:
			DEC_r('A');
			_cycle = 4;
			_registers.PC++;
			break;
		case 0x3E:
			LD_r_n(_mmu.readRam(_registers.PC + 1), 'A');
			_registers.PC += 2;
			_cycle = 8;
			break;
		case 0x3F:
			CCF();
			_cycle = 4;
			_registers.PC++;
			break;
		case 0x40:
			LD_r_r('B', 'B');
			_cycle = 4;
			_registers.PC++;
			break;
		case 0x41:
			LD_r_r('C', 'B');
			_cycle = 4;
			_registers.PC++;
			break;
		case 0x42:
			LD_r_r('D', 'B');
			_cycle = 4;
			_registers.PC++;
			break;
		case 0x43:
			LD_r_r('E', 'B');
			_cycle = 4;
			_registers.PC++;
			break;
		case 0x44:
			LD_r_r('H', 'B');
			_cycle = 4;
			_registers.PC++;
			break;
		case 0x45:
			LD_r_r('L', 'B');
			_cycle = 4;
			_registers.PC++;
			break;
		case 0x46:
			LD_r_16(_registers.HL.get(), 'B');
			_cycle = 8;
			_registers.PC++;
			break;
		case 0x47:
			LD_r_r('A', 'B');
			_cycle = 4;
			_registers.PC++;
			break;
		case 0x48:
			LD_r_r('B', 'C');
			_cycle = 4;
			_registers.PC++;
			break;
		case 0x49:
			LD_r_r('C', 'C');
			_cycle = 4;
			_registers.PC++;
			break;
		case 0x4A:
			LD_r_r('D', 'C');
			_cycle = 4;
			_registers.PC++;
			break;
		case 0x4B:
			LD_r_r('E', 'C');
			_cycle = 4;
			_registers.PC++;
			break;
		case 0x4C:
			LD_r_r('H', 'C');
			_cycle = 4;
			_registers.PC++;
			break;
		case 0x4D:
			LD_r_r('L', 'C');
			_cycle = 4;
			_registers.PC++;
			break;
		case 0x4E:
			LD_r_16(_registers.HL.get(), 'C');
			_cycle = 8;
			_registers.PC++;
			break;
		case 0x4F:
			LD_r_r('A', 'C');
			_registers.PC++;
			_cycle = 4;
			break;
		case 0x50:
			LD_r_r('B', 'D');
			_cycle = 4;
			_registers.PC++;
			break;
		case 0x51:
			LD_r_r('C', 'D');
			_cycle = 4;
			_registers.PC++;
			break;
		case 0x52:
			LD_r_r('D', 'D');
			_cycle = 4;
			_registers.PC++;
			break;
		case 0x53:
			LD_r_r('E', 'D');
			_cycle = 4;
			_registers.PC++;
			break;
		case 0x54:
			LD_r_r('H', 'D');
			_cycle = 4;
			_registers.PC++;
			break;
		case 0x55:
			LD_r_r('L', 'D');
			_cycle = 4;
			_registers.PC++;
			break;
		case 0x56:
			LD_r_16(_registers.HL.get(), 'D');
			_cycle = 8;
			_registers.PC++;
			break;
		case 0x57:
			LD_r_r('A', 'D');
			_registers.PC++;
			_cycle = 4;
			break;
		case 0x58:
			LD_r_r('B', 'E');
			_cycle = 4;
			_registers.PC++;
			break;
		case 0x59:
			LD_r_r('C', 'E');
			_cycle = 4;
			_registers.PC++;
			break;
		case 0x5A:
			LD_r_r('D', 'E');
			_cycle = 4;
			_registers.PC++;
			break;
		case 0x5B:
			LD_r_r('E', 'E');
			_cycle = 4;
			_registers.PC++;
			break;
		case 0x5C:
			LD_r_r('H', 'E');
			_cycle = 4;
			_registers.PC++;
			break;
		case 0x5D:
			LD_r_r('L', 'E');
			_cycle = 4;
			_registers.PC++;
			break;
		case 0x5E:
			LD_r_16(_registers.HL.get(), 'E');
			_cycle = 8;
			_registers.PC++;
			break;
		case 0x5F:
			LD_r_r('A', 'E');
			_cycle = 4;
			_registers.PC++;
			break;
		case 0x60:
			LD_r_r('B', 'H');
			_cycle = 4;
			_registers.PC++;
			break;
		case 0x61:
			LD_r_r('C', 'H');
			_cycle = 4;
			_registers.PC++;
			break;
		case 0x62:
			LD_r_r('D', 'H');
			_cycle = 4;
			_registers.PC++;
			break;
		case 0x63:
			LD_r_r('E', 'H');
			_cycle = 4;
			_registers.PC++;
			break;
		case 0x64:
			LD_r_r('H', 'H');
			_cycle = 4;
			_registers.PC++;
			break;
		case 0x65:
			LD_r_r('L', 'H');
			_cycle = 4;
			_registers.PC++;
			break;
		case 0x66:
			LD_r_16(_registers.HL.get(), 'H');
			_cycle = 8;
			_registers.PC++;
			break;
		case 0x67:
			LD_r_r('A', 'H');
			_cycle = 4;
			_registers.PC++;
			break;
		case 0x68:
			LD_r_r('B', 'L');
			_cycle = 4;
			_registers.PC++;
			break;
		case 0x69:
			LD_r_r('C', 'L');
			_cycle = 4;
			_registers.PC++;
			break;
		case 0x6A:
			LD_r_r('D', 'L');
			_cycle = 4;
			_registers.PC++;
			break;
		case 0x6B:
			LD_r_r('E', 'L');
			_cycle = 4;
			_registers.PC++;
			break;
		case 0x6C:
			LD_r_r('H', 'L');
			_cycle = 4;
			_registers.PC++;
			break;
		case 0x6D:
			LD_r_r('L', 'L');
			_cycle = 4;
			_registers.PC++;
			break;
		case 0x6E:
			LD_r_16(_registers.HL.get(), 'L');
			_cycle = 8;
			_registers.PC++;
			break;
		case 0x6F:
			LD_r_r('A', 'L');
			_cycle = 4;
			_registers.PC++;
			break;
		case 0x70:
			LD_16_r(_registers.HL.get(), 'B');
			_cycle = 4;
			_registers.PC++;
			break;
		case 0x71:
			LD_16_r(_registers.HL.get(), 'C');
			_cycle = 4;
			_registers.PC++;
			break;
		case 0x72:
			LD_16_r(_registers.HL.get(), 'D');
			_cycle = 8;
			_registers.PC++;
			break;
		case 0x73:
			LD_16_r(_registers.HL.get(), 'E');
			_cycle = 8;
			_registers.PC++;
			break;
		case 0x74:
			LD_16_r(_registers.HL.get(), 'H');
			_cycle = 8;
			_registers.PC++;
			break;
		case 0x75:
			LD_16_r(_registers.HL.get(), 'L');
			_cycle = 8;
			_registers.PC++;
			break;
		case 0x76:
			HALT();
			_registers.PC++;
			_cycle = 4;
			break;
		case 0x77:
			LD_16_r(_registers.HL.get(), 'A');
			_cycle = 8;
			_registers.PC++;
			break;
		case 0x78:
			LD_r_r('B', 'A');
			_cycle = 4;
			_registers.PC++;
			break;
		case 0x79:
			LD_r_r('C', 'A');
			_cycle = 4;
			_registers.PC++;
			break;
		case 0x7A:
			LD_r_r('D', 'A');
			_cycle = 4;
			_registers.PC++;
			break;
		case 0x7B:
			LD_r_r('E', 'A');
			_cycle = 4;
			_registers.PC++;
			break;
		case 0x7C:
			LD_r_r('H', 'A');
			_cycle = 4;
			_registers.PC++;
			break;
		case 0x7D:
			LD_r_r('L', 'A');
			_cycle = 4;
			_registers.PC++;
			break;
		case 0x7E:
			LD_r_16(_registers.HL.get(), 'A');
			_cycle = 8;
			_registers.PC++;
			break;
		case 0x7F:
			LD_r_r('A', 'A');
			_cycle = 4;
			_registers.PC++;
			break;
		case 0x80:
			ADD_A_r('B');
			_cycle = 4;
			_registers.PC++;
			break;
		case 0x81:
			ADD_A_r('C');
			_cycle = 4;
			_registers.PC++;
			break;
		case 0x82:
			ADD_A_r('D');
			_cycle = 4;
			_registers.PC++;
			break;
		case 0x83:
			ADD_A_r('E');
			_cycle = 4;
			_registers.PC++;
			break;
		case 0x84:
			ADD_A_r('H');
			_cycle = 4;
			_registers.PC++;
			break;
		case 0x85:
			ADD_A_r('L');
			_cycle = 4;
			_registers.PC++;
			break;
		case 0x86:
			ADD_A_16();
			_cycle = 8;
			_registers.PC++;
			break;
		case 0x87:
			ADD_A_r('A');
			_cycle = 4;
			_registers.PC++;
			break;
		case 0x88:
			ADC_A_r('B');
			_cycle = 4;
			_registers.PC++;
			break;
		case 0x89:
			ADC_A_r('C');
			_cycle = 4;
			_registers.PC++;
			break;
		case 0x8A:
			ADC_A_r('D');
			_cycle = 4;
			_registers.PC++;
			break;
		case 0x8B:
			ADC_A_r('E');
			_cycle = 4;
			_registers.PC++;
			break;
		case 0x8C:
			ADC_A_r('H');
			_cycle = 4;
			_registers.PC++;
			break;
		case 0x8D:
			ADC_A_r('L');
			_cycle = 4;
			_registers.PC++;
			break;
		case 0x8E:
			ADC_A_16();
			_cycle = 8;
			_registers.PC++;
			break;
		case 0x8F:
			ADC_A_r('A');
			_cycle = 4;
			_registers.PC++;
			break;
		case 0x90:
			SUB_r('B');
			_cycle = 4;
			_registers.PC++;
			break;
		case 0x91:
			SUB_r('C');
			_cycle = 4;
			_registers.PC++;
			break;
		case 0x92:
			SUB_r('D');
			_cycle = 4;
			_registers.PC++;
			break;
		case 0x93:
			SUB_r('E');
			_cycle = 4;
			_registers.PC++;
			break;
		case 0x94:
			SUB_r('H');
			_cycle = 4;
			_registers.PC++;
			break;
		case 0x95:
			SUB_r('L');
			_cycle = 4;
			_registers.PC++;
			break;
		case 0x96:
			SUB_16();
			_cycle = 8;
			_registers.PC++;
			break;
		case 0x97:
			SUB_r('A');
			_cycle = 4;
			_registers.PC++;
			break;
		case 0x98:
			SBC_r('B');
			_cycle = 4;
			_registers.PC++;
			break;
		case 0x99:
			SBC_r('C');
			_cycle = 4;
			_registers.PC++;
			break;
		case 0x9A:
			SBC_r('D');
			_cycle = 4;
			_registers.PC++;
			break;
		case 0x9B:
			SBC_r('E');
			_cycle = 4;
			_registers.PC++;
			break;
		case 0x9C:
			SBC_r('H');
			_cycle = 4;
			_registers.PC++;
			break;
		case 0x9D:
			SBC_r('L');
			_cycle = 4;
			_registers.PC++;
			break;
		case 0x9E:
			SBC_16();
			_cycle = 8;
			_registers.PC++;
			break;
		case 0x9F:
			SBC_r('A');
			_cycle = 4;
			_registers.PC++;
			break;
		case 0xA0:
			AND_r('B');
			_cycle = 4;
			_registers.PC++;
			break;
		case 0xA1:
			AND_r('C');
			_cycle = 4;
			_registers.PC++;
			break;
		case 0xA2:
			AND_r('D');
			_cycle = 4;
			_registers.PC++;
			break;
		case 0xA3:
			AND_r('E');
			_cycle = 4;
			_registers.PC++;
			break;
		case 0xA4:
			AND_r('H');
			_cycle = 4;
			_registers.PC++;
			break;
		case 0xA5:
			AND_r('L');
			_cycle = 4;
			_registers.PC++;
			break;
		case 0xA6:
			AND_16();
			_cycle = 8;
			_registers.PC++;
			break;
		case 0xA7:
			AND_r('A');
			_cycle = 4;
			_registers.PC++;
			break;
		case 0xA8:
			XOR_r('B');
			_cycle = 4;
			_registers.PC++;
			break;
		case 0xA9:
			XOR_r('C');
			_cycle = 4;
			_registers.PC++;
			break;
		case 0xAA:
			XOR_r('D');
			_cycle = 4;
			_registers.PC++;
			break;
		case 0xAB:
			XOR_r('E');
			_cycle = 4;
			_registers.PC++;
			break;
		case 0xAC:
			XOR_r('H');
			_cycle = 4;
			_registers.PC++;
			break;
		case 0xAD:
			XOR_r('L');
			_cycle = 4;
			_registers.PC++;
			break;
		case 0xAE:
			XOR_16();
			_cycle = 8;
			_registers.PC++;
			break;
		case 0xAF:
			XOR_r('A');
			_cycle = 4;
			_registers.PC++;
			break;
		case 0xB0:
			OR_r('B');
			_cycle = 4;
			_registers.PC++;
			break;
		case 0xB1:
			OR_r('C');
			_cycle = 4;
			_registers.PC++;
			break;
		case 0xB2:
			OR_r('D');
			_cycle = 4;
			_registers.PC++;
			break;
		case 0xB3:
			OR_r('E');
			_cycle = 4;
			_registers.PC++;
			break;
		case 0xB4:
			OR_r('H');
			_cycle = 4;
			_registers.PC++;
			break;
		case 0xB5:
			OR_r('L');
			_cycle = 4;
			_registers.PC++;
			break;
		case 0xB6:
			OR_16();
			_cycle = 8;
			_registers.PC++;
			break;
		case 0xB7:
			OR_r('A');
			_cycle = 4;
			_registers.PC++;
			break;
		case 0xB8:
			CP_r('B');
			_cycle = 4;
			_registers.PC++;
			break;
		case 0xB9:
			CP_r('C');
			_cycle = 4;
			_registers.PC++;
			break;
		case 0xBA:
			CP_r('D');
			_cycle = 4;
			_registers.PC++;
			break;
		case 0xBB:
			CP_r('E');
			_cycle = 4;
			_registers.PC++;
			break;
		case 0xBC:
			CP_r('H');
			_cycle = 4;
			_registers.PC++;
			break;
		case 0xBD:
			CP_r('L');
			_cycle = 4;
			_registers.PC++;
			break;
		case 0xBE:
			CP_16();
			_cycle = 8;
			_registers.PC++;
			break;
		case 0xBF:
			CP_r('A');
			_cycle = 4;
			_registers.PC++;
			break;
		case 0xC0:
			RET_c("NZ");
			_cycle = 20;
			break;
		case 0xC1:
			POP("BC");
			_registers.PC++;
			_cycle = 12;
			break;
		case 0xC2:
			JP_c_16("NZ", next2bytes(_registers.PC + 1));
			_cycle = 16;
			break;
		case 0xC3:
			JP_16(_registers.PC + 1);
			_cycle = 16;
			break;
		case 0xC4:
			CALL_c("NZ");
			_cycle = 24;
			break;
		case 0xC5:
			PUSH("BC");
			_registers.PC++;
			_cycle = 16;
			break;
		case 0xC6:
			ADD_A_n(_mmu.readRam(_registers.PC + 1));
			_cycle = 8;
			_registers.PC += 2;
			break;
		case 0xC7:
			RST_p(0x00);
			_cycle = 16;
			break;
		case 0xC8:
			RET_c("Z");
			_cycle = 20;
			break;
		case 0xC9:
			RET();
			_cycle = 16;
			break;
		case 0xCA:
			JP_c_16("Z", next2bytes(_registers.PC + 1));
			_cycle = 16;
			break;
			// REFER TO CB PREFIX FOR SPECIAL INSTRUCTIONS FURTHER BELOW
		case 0xCC:
			CALL_c("Z");
			_cycle = 24;
			break;
		case 0xCD:
			CALL();
			_cycle = 24;
			break;
		case 0xCE:
			ADC_A_n(_mmu.readRam(_registers.PC + 1));
			_cycle = 8;
			_registers.PC += 2;
			break;
		case 0xCF:
			RST_p(0x08);
			_cycle = 16;
			break;
		case 0xD0:
			RET_c("NC");
			_cycle = 20;
			break;
		case 0xD1:
			POP("DE");
			_cycle = 12;
			_registers.PC++;
			break;
		case 0xD2:
			JP_c_16("NC", next2bytes(_registers.PC + 1));
			_cycle = 16;
			break;
		case 0xD4:
			CALL_c("NC");
			_cycle = 24;
			break;
		case 0xD5:
			PUSH("DE");
			_cycle = 16;
			_registers.PC++;
			break;
		case 0xD6:
			SUB_n(_mmu.readRam(_registers.PC + 1));
			_cycle = 8;
			_registers.PC += 2;
			break;
		case 0xD7:
			RST_p(0x10);
			_cycle = 16;
			break;
		case 0xD8:
			RET_c("C");
			_cycle = 20;
			break;
		case 0xD9:
			RETI();
			_cycle = 16;
			break;
		case 0xDA:
			JP_c_16("C", next2bytes(_registers.PC + 1));
			_cycle = 16;
			break;
		case 0xDC:
			CALL_c("C");
			_cycle = 24;
			break;
		case 0xDE:
			SBC_n(_mmu.readRam(_registers.PC + 1));
			_cycle = 8;
			_registers.PC += 2;
			break;
		case 0xDF:
			RST_p(0x18);
			_cycle = 16;
			break;
		case 0xE0:
			_mmu.writeRam(_mmu.readRam(_registers.PC + 1) + 0xFF00, _registers.AF.getLeftRegister());
			_cycle = 12;
			_registers.PC += 2;
			break;
		case 0xE1:
			POP("HL");
			_cycle = 12;
			_registers.PC++;
			break;
		case 0xE2:
			_mmu.writeRam(0xFF00 + _registers.BC.getRightRegister(), _registers.AF.getLeftRegister());
			_cycle = 8;
			_registers.PC++;
			break;
		case 0xE5:
			PUSH("HL");
			_cycle = 16;
			_registers.PC++;
			break;
		case 0xE6:
			AND_n(_mmu.readRam(_registers.PC + 1));
			_registers.PC += 2;
			_cycle = 8;
			break;
		case 0xE7:
			RST_p(0x20);
			_cycle = 16;
			break;
		case 0xE8:
			ADD_SP_n();
			_registers.PC += 2;
			_cycle = 16;
			break;
		case 0xE9:
			JP_16();
			_cycle = 4;
			break;
		case 0xEA:
			LD_16_r(next2bytes(_registers.PC + 1), 'A');
			_cycle = 16;
			_registers.PC += 3;
			break;
		case 0xEE:
			XOR_n(_mmu.readRam(_registers.PC + 1));
			_registers.PC += 2;
			_cycle = 8;
			break;
		case 0xEF:
			RST_p(0x28);
			_cycle = 16;
			break;
		case 0xF0:
			_registers.AF.setLeftRegister(_mmu.readRam(_mmu.readRam(_registers.PC + 1) + 0xFF00));
			_cycle = 12;
			_registers.PC += 2;
			break;
		case 0xF1:
			POP("AF");
			_cycle = 12;
			_registers.PC++;
			break;
		case 0xF2:
			_registers.AF.setLeftRegister(_mmu.readRam(_registers.BC.getRightRegister() + 0xFF00));
			_cycle = 8;
			_registers.PC++;
			break;
		case 0xF3:
			DI();
			_cycle = 4;
			_registers.PC++;
			break;
		case 0xF5:
			PUSH("AF");
			_cycle = 16;
			_registers.PC++;
			break;
		case 0xF6:
			OR_n(_mmu.readRam(_registers.PC + 1));
			_registers.PC += 2;
			_cycle = 8;
			break;
		case 0xF7:
			RST_p(0x30);
			_cycle = 16;
			break;
		case 0xF8:
			LD_HL_SP_n();
			_cycle = 12;
			_registers.PC += 2;
			break;
		case 0xF9: // TODO may be innacurate
			_registers.SP = _registers.HL.get();
			_cycle = 8;
			_registers.PC++;
			break;
		case 0xFA:
			LD_r_16(next2bytes(_registers.PC + 1), 'A');
			_cycle = 16;
			_registers.PC += 3;
			break;
		case 0xFB:
			EI();
			_cycle = 4;
			_registers.PC++;
			break;
		case 0xFE:
			CP_n(_mmu.readRam(_registers.PC + 1));
			_cycle = 8;
			_registers.PC += 2;
			break;
		case 0xFF:
			RST_p(0x38);
			_cycle = 16;
			break;

			/*-----SPECIAL OPCODES-----*/

		case 0xCB:
		{
			prevPC++;
			_registers.PC++;
			switch (_mmu.readRam(_registers.PC))
			{
			case 0x00:
				RLC_r('B');
				_cycle = 8;
				_registers.PC++;
				break;
			case 0x01:
				RLC_r('C');
				_cycle = 8;
				_registers.PC++;
				break;
			case 0x02:
				RLC_r('D');
				_cycle = 8;
				_registers.PC++;
				break;
			case 0x03:
				RLC_r('E');
				_cycle = 8;
				_registers.PC++;
				break;
			case 0x04:
				RLC_r('H');
				_cycle = 8;
				_registers.PC++;
				break;
			case 0x05:
				RLC_r('L');
				_cycle = 8;
				_registers.PC++;
				break;
			case 0x06:
				RLC_16();
				_cycle = 16;
				_registers.PC++;
				break;
			case 0x07:
				RLC_r('A');
				_cycle = 8;
				_registers.PC++;
				break;
			case 0x08:
				RRC_r('B');
				_cycle = 8;
				_registers.PC++;
				break;
			case 0x09:
				RRC_r('C');
				_cycle = 8;
				_registers.PC++;
				break;
			case 0x0A:
				RRC_r('D');
				_cycle = 8;
				_registers.PC++;
				break;
			case 0x0B:
				RRC_r('E');
				_cycle = 8;
				_registers.PC++;
				break;
			case 0x0C:
				RRC_r('H');
				_cycle = 8;
				_registers.PC++;
				break;
			case 0x0D:
				RRC_r('L');
				_cycle = 8;
				_registers.PC++;
				break;
			case 0x0E:
				RRC_16();
				_cycle = 16;
				_registers.PC++;
				break;
			case 0x0F:
				RRC_r('A');
				_cycle = 8;
				_registers.PC++;
				break;
			case 0x10:
				RL_r('B');
				_cycle = 8;
				_registers.PC++;
				break;
			case 0x11:
				RL_r('C');
				_cycle = 8;
				_registers.PC++;
				break;
			case 0x12:
				RL_r('D');
				_cycle = 8;
				_registers.PC++;
				break;
			case 0x13:
				RL_r('E');
				_cycle = 8;
				_registers.PC++;
				break;
			case 0x14:
				RL_r('H');
				_cycle = 8;
				_registers.PC++;
				break;
			case 0x15:
				RL_r('L');
				_cycle = 8;
				_registers.PC++;
				break;
			case 0x16:
				RL_16();
				_cycle = 16;
				_registers.PC++;
				break;
			case 0x17:
				RL_r('A');
				_cycle = 8;
				_registers.PC++;
				break;
			case 0x18:
				RR_r('B');
				_cycle = 8;
				_registers.PC++;
				break;
			case 0x19:
				RR_r('C');
				_cycle = 8;
				_registers.PC++;
				break;
			case 0x1A:
				RR_r('D');
				_cycle = 8;
				_registers.PC++;
				break;
			case 0x1B:
				RR_r('E');
				_cycle = 8;
				_registers.PC++;
				break;
			case 0x1C:
				RR_r('H');
				_cycle = 8;
				_registers.PC++;
				break;
			case 0x1D:
				RR_r('L');
				_cycle = 8;
				_registers.PC++;
				break;
			case 0x1E:
				RR_16();
				_cycle = 16;
				_registers.PC++;
				break;
			case 0x1F:
				RR_r('A');
				_cycle = 8;
				_registers.PC++;
				break;
			case 0x20:
				SLA_r('B');
				_cycle = 8;
				_registers.PC++;
				break;
			case 0x21:
				SLA_r('C');
				_cycle = 8;
				_registers.PC++;
				break;
			case 0x22:
				SLA_r('D');
				_cycle = 8;
				_registers.PC++;
				break;
			case 0x23:
				SLA_r('E');
				_cycle = 8;
				_registers.PC++;
				break;
			case 0x24:
				SLA_r('H');
				_cycle = 8;
				_registers.PC++;
				break;
			case 0x25:
				SLA_r('L');
				_cycle = 8;
				_registers.PC++;
				break;
			case 0x26:
				SLA_16();
				_cycle = 16;
				_registers.PC++;
				break;
			case 0x27:
				SLA_r('A');
				_cycle = 8;
				_registers.PC++;
				break;
			case 0x28:
				SRA_r('B');
				_cycle = 8;
				_registers.PC++;
				break;
			case 0x29:
				SRA_r('C');
				_cycle = 8;
				_registers.PC++;
				break;
			case 0x2A:
				SRA_r('D');
				_cycle = 8;
				_registers.PC++;
				break;
			case 0x2B:
				SRA_r('E');
				_cycle = 8;
				_registers.PC++;
				break;
			case 0x2C:
				SRA_r('H');
				_cycle = 8;
				_registers.PC++;
				break;
			case 0x2D:
				SRA_r('L');
				_cycle = 8;
				_registers.PC++;
				break;
			case 0x2E:
				SRA_16();
				_cycle = 16;
				_registers.PC++;
				break;
			case 0x2F:
				SRA_r('A');
				_cycle = 8;
				_registers.PC++;
				break;
			case 0x30:
				SWAP_r('B');
				_cycle = 8;
				_registers.PC++;
				break;
			case 0x31:
				SWAP_r('C');
				_cycle = 8;
				_registers.PC++;
				break;
			case 0x32:
				SWAP_r('D');
				_cycle = 8;
				_registers.PC++;
				break;
			case 0x33:
				SWAP_r('E');
				_cycle = 8;
				_registers.PC++;
				break;
			case 0x34:
				SWAP_r('H');
				_cycle = 8;
				_registers.PC++;
				break;
			case 0x35:
				SWAP_r('L');
				_cycle = 8;
				_registers.PC++;
				break;
			case 0x36:
				SWAP_16();
				_cycle = 16;
				_registers.PC++;
				break;
			case 0x37:
				SWAP_r('A');
				_cycle = 8;
				_registers.PC++;
				break;
			case 0x38:
				SRL_r('B');
				_cycle = 8;
				_registers.PC++;
				break;
			case 0x39:
				SRL_r('C');
				_cycle = 8;
				_registers.PC++;
				break;
			case 0x3A:
				SRL_r('D');
				_cycle = 8;
				_registers.PC++;
				break;
			case 0x3B:
				SRL_r('E');
				_cycle = 8;
				_registers.PC++;
				break;
			case 0x3C:
				SRL_r('H');
				_cycle = 8;
				_registers.PC++;
				break;
			case 0x3D:
				SRL_r('L');
				_cycle = 8;
				_registers.PC++;
				break;
			case 0x3E:
				SRL_16();
				_cycle = 16;
				_registers.PC++;
				break;
			case 0x3F:
				SRL_r('A');
				_cycle = 8;
				_registers.PC++;
				break;
			case 0x40:
				BIT_b_r(0, 'B');
				_registers.PC++;
				break;
			case 0x41:
				BIT_b_r(0, 'C');
				_cycle = 8;
				_registers.PC++;
				break;
			case 0x42:
				BIT_b_r(0, 'D');
				_cycle = 8;
				_registers.PC++;
				break;
			case 0x43:
				BIT_b_r(0, 'E');
				_cycle = 8;
				_registers.PC++;
				break;
			case 0x44:
				BIT_b_r(0, 'H');
				_cycle = 8;
				_registers.PC++;
				break;
			case 0x45:
				BIT_b_r(0, 'L');
				_cycle = 8;
				_registers.PC++;
				break;
			case 0x46:
				BIT_b_16(0);
				_cycle = 16;
				_registers.PC++;
				break;
			case 0x47:
				BIT_b_r(0, 'A');
				_cycle = 8;
				_registers.PC++;
				break;
			case 0x48:
				BIT_b_r(1, 'B');
				_cycle = 8;
				_registers.PC++;
				break;
			case 0x49:
				BIT_b_r(1, 'C');
				_cycle = 8;
				_registers.PC++;
				break;
			case 0x4A:
				BIT_b_r(1, 'D');
				_cycle = 8;
				_registers.PC++;
				break;
			case 0x4B:
				BIT_b_r(1, 'E');
				_cycle = 8;
				_registers.PC++;
				break;
			case 0x4C:
				BIT_b_r(1, 'H');
				_cycle = 8;
				_registers.PC++;
				break;
			case 0x4D:
				BIT_b_r(1, 'L');
				_cycle = 8;
				_registers.PC++;
				break;
			case 0x4E:
				BIT_b_16(1);
				_cycle = 8;
				_registers.PC++;
				break;
			case 0x4F:
				BIT_b_r(1, 'A');
				_cycle = 16;
				_registers.PC++;
				break;
			case 0x50:
				BIT_b_r(2, 'B');
				_cycle = 8;
				_registers.PC++;
				break;
			case 0x51:
				BIT_b_r(2, 'C');
				_cycle = 8;
				_registers.PC++;
				break;
			case 0x52:
				BIT_b_r(2, 'D');
				_cycle = 8;
				_registers.PC++;
				break;
			case 0x53:
				BIT_b_r(2, 'E');
				_cycle = 8;
				_registers.PC++;
				break;
			case 0x54:
				BIT_b_r(2, 'H');
				_cycle = 8;
				_registers.PC++;
				break;
			case 0x55:
				BIT_b_r(2, 'L');
				_cycle = 8;
				_registers.PC++;
				break;
			case 0x56:
				BIT_b_16(2);
				_cycle = 16;
				_registers.PC++;
				break;
			case 0x57:
				BIT_b_r(2, 'A');
				_cycle = 8;
				_registers.PC++;
				break;
			case 0x58:
				BIT_b_r(3, 'B');
				_cycle = 8;
				_registers.PC++;
				break;
			case 0x59:
				BIT_b_r(3, 'C');
				_cycle = 8;
				_registers.PC++;
				break;
			case 0x5A:
				BIT_b_r(3, 'D');
				_cycle = 8;
				_registers.PC++;
				break;
			case 0x5B:
				BIT_b_r(3, 'E');
				_cycle = 8;
				_registers.PC++;
				break;
			case 0x5C:
				BIT_b_r(3, 'H');
				_cycle = 8;
				_registers.PC++;
				break;
			case 0x5D:
				BIT_b_r(3, 'L');
				_cycle = 8;
				_registers.PC++;
				break;
			case 0x5E:
				BIT_b_16(3);
				_cycle = 16;
				_registers.PC++;
				break;
			case 0x5F:
				BIT_b_r(3, 'A');
				_cycle = 8;
				_registers.PC++;
				break;
			case 0x60:
				BIT_b_r(4, 'B');
				_cycle = 8;
				_registers.PC++;
				break;
			case 0x61:
				BIT_b_r(4, 'C');
				_cycle = 8;
				_registers.PC++;
				break;
			case 0x62:
				BIT_b_r(4, 'D');
				_cycle = 8;
				_registers.PC++;
				break;
			case 0x63:
				BIT_b_r(4, 'E');
				_cycle = 8;
				_registers.PC++;
				break;
			case 0x64:
				BIT_b_r(4, 'H');
				_cycle = 8;
				_registers.PC++;
				break;
			case 0x65:
				BIT_b_r(4, 'L');
				_cycle = 8;
				_registers.PC++;
				break;
			case 0x66:
				BIT_b_16(4);
				_cycle = 16;
				_registers.PC++;
				break;
			case 0x67:
				BIT_b_r(4, 'A');
				_cycle = 8;
				_registers.PC++;
				break;
			case 0x68:
				BIT_b_r(5, 'B');
				_cycle = 8;
				_registers.PC++;
				break;
			case 0x69:
				BIT_b_r(5, 'C');
				_cycle = 8;
				_registers.PC++;
				break;
			case 0x6A:
				BIT_b_r(5, 'D');
				_cycle = 8;
				_registers.PC++;
				break;
			case 0x6B:
				BIT_b_r(5, 'E');
				_cycle = 8;
				_registers.PC++;
				break;
			case 0x6C:
				BIT_b_r(5, 'H');
				_cycle = 8;
				_registers.PC++;
				break;
			case 0x6D:
				BIT_b_r(5, 'L');
				_cycle = 8;
				_registers.PC++;
				break;
			case 0x6E:
				BIT_b_16(5);
				_cycle = 16;
				_registers.PC++;
				break;
			case 0x6F:
				BIT_b_r(5, 'A');
				_cycle = 8;
				_registers.PC++;
				break;
			case 0x70:
				BIT_b_r(6, 'B');
				_cycle = 8;
				_registers.PC++;
				break;
			case 0x71:
				BIT_b_r(6, 'C');
				_cycle = 8;
				_registers.PC++;
				break;
			case 0x72:
				BIT_b_r(6, 'D');
				_cycle = 8;
				_registers.PC++;
				break;
			case 0x73:
				BIT_b_r(6, 'E');
				_cycle = 8;
				_registers.PC++;
				break;
			case 0x74:
				BIT_b_r(6, 'H');
				_cycle = 8;
				_registers.PC++;
				break;
			case 0x75:
				BIT_b_r(6, 'L');
				_cycle = 8;
				_registers.PC++;
				break;
			case 0x76:
				BIT_b_16(6);
				_cycle = 16;
				_registers.PC++;
				break;
			case 0x77:
				BIT_b_r(6, 'A');
				_cycle = 8;
				_registers.PC++;
				break;
			case 0x78:
				BIT_b_r(7, 'B');
				_cycle = 8;
				_registers.PC++;
				break;
			case 0x79:
				BIT_b_r(7, 'C');
				_cycle = 8;
				_registers.PC++;
				break;
			case 0x7A:
				BIT_b_r(7, 'D');
				_cycle = 8;
				_registers.PC++;
				break;
			case 0x7B:
				BIT_b_r(7, 'E');
				_cycle = 8;
				_registers.PC++;
				break;
			case 0x7C:
				BIT_b_r(7, 'H');
				_cycle = 8;
				_registers.PC++;
				break;
			case 0x7D:
				BIT_b_r(7, 'L');
				_cycle = 8;
				_registers.PC++;
				break;
			case 0x7E:
				BIT_b_16(7);
				_cycle = 16;
				_registers.PC++;
				break;
			case 0x7F:
				BIT_b_r(7, 'A');
				_cycle = 8;
				_registers.PC++;
				break;
			case 0x80:
				RES_b_r(0, 'B');
				_cycle = 8;
				_registers.PC++;
				break;
			case 0x81:
				RES_b_r(0, 'C');
				_cycle = 8;
				_registers.PC++;
				break;
			case 0x82:
				RES_b_r(0, 'D');
				_cycle = 8;
				_registers.PC++;
				break;
			case 0x83:
				RES_b_r(0, 'E');
				_cycle = 8;
				_registers.PC++;
				break;
			case 0x84:
				RES_b_r(0, 'H');
				_cycle = 8;
				_registers.PC++;
				break;
			case 0x85:
				RES_b_r(0, 'L');
				_cycle = 8;
				_registers.PC++;
				break;
			case 0x86:
				RES_b_16(0);
				_cycle = 16;
				_registers.PC++;
				break;
			case 0x87:
				RES_b_r(0, 'A');
				_cycle = 8;
				_registers.PC++;
				break;
			case 0x88:
				RES_b_r(1, 'B');
				_cycle = 8;
				_registers.PC++;
				break;
			case 0x89:
				RES_b_r(1, 'C');
				_cycle = 8;
				_registers.PC++;
				break;
			case 0x8A:
				RES_b_r(1, 'D');
				_cycle = 8;
				_registers.PC++;
				break;
			case 0x8B:
				RES_b_r(1, 'E');
				_cycle = 8;
				_registers.PC++;
				break;
			case 0x8C:
				RES_b_r(1, 'H');
				_cycle = 8;
				_registers.PC++;
				break;
			case 0x8D:
				RES_b_r(1, 'L');
				_cycle = 8;
				_registers.PC++;
				break;
			case 0x8E:
				RES_b_16(1);
				_cycle = 16;
				_registers.PC++;
				break;
			case 0x8F:
				RES_b_r(1, 'A');
				_cycle = 8;
				_registers.PC++;
				break;
			case 0x90:
				RES_b_r(2, 'B');
				_cycle = 8;
				_registers.PC++;
				break;
			case 0x91:
				RES_b_r(2, 'C');
				;
				_cycle = 8;
				_registers.PC++;
				break;
			case 0x92:
				RES_b_r(2, 'D');
				_cycle = 8;
				_registers.PC++;
				break;
			case 0x93:
				RES_b_r(2, 'E');
				_cycle = 8;
				_registers.PC++;
				break;
			case 0x94:
				RES_b_r(2, 'H');
				_cycle = 8;
				_registers.PC++;
				break;
			case 0x95:
				RES_b_r(2, 'L');
				_cycle = 8;
				_registers.PC++;
				break;
			case 0x96:
				RES_b_16(2);
				_cycle = 16;
				_registers.PC++;
				break;
			case 0x97:
				RES_b_r(2, 'A');
				_cycle = 8;
				_registers.PC++;
				break;
			case 0x98:
				RES_b_r(3, 'B');
				_cycle = 8;
				_registers.PC++;
				break;
			case 0x99:
				RES_b_r(3, 'C');
				_cycle = 8;
				_registers.PC++;
				break;
			case 0x9A:
				RES_b_r(3, 'D');
				_cycle = 8;
				_registers.PC++;
				break;
			case 0x9B:
				RES_b_r(3, 'E');
				_cycle = 8;
				_registers.PC++;
				break;
			case 0x9C:
				RES_b_r(3, 'H');
				_cycle = 8;
				_registers.PC++;
				break;
			case 0x9D:
				RES_b_r(3, 'L');
				_cycle = 8;
				_registers.PC++;
				break;
			case 0x9E:
				RES_b_16(3);
				_cycle = 16;
				_registers.PC++;
				break;
			case 0x9F:
				RES_b_r(3, 'A');
				_cycle = 8;
				_registers.PC++;
				break;
			case 0xA0:
				RES_b_r(4, 'B');
				_cycle = 8;
				_registers.PC++;
				break;
			case 0xA1:
				RES_b_r(4, 'C');
				_cycle = 8;
				_registers.PC++;
				break;
			case 0xA2:
				RES_b_r(4, 'D');
				_cycle = 8;
				_registers.PC++;
				break;
			case 0xA3:
				RES_b_r(4, 'E');
				_cycle = 8;
				_registers.PC++;
				break;
			case 0xA4:
				RES_b_r(4, 'H');
				_cycle = 8;
				_registers.PC++;
				break;
			case 0xA5:
				RES_b_r(4, 'L');
				_cycle = 8;
				_registers.PC++;
				break;
			case 0xA6:
				RES_b_16(4);
				_cycle = 16;
				_registers.PC++;
				break;
			case 0xA7:
				RES_b_r(4, 'A');
				_cycle = 8;
				_registers.PC++;
				break;
			case 0xA8:
				RES_b_r(5, 'B');
				_cycle = 8;
				_registers.PC++;
				break;
			case 0xA9:
				RES_b_r(5, 'C');
				_cycle = 8;
				_registers.PC++;
				break;
			case 0xAA:
				RES_b_r(5, 'D');
				_cycle = 8;
				_registers.PC++;
				break;
			case 0xAB:
				RES_b_r(5, 'E');
				_cycle = 8;
				_registers.PC++;
				break;
			case 0xAC:
				RES_b_r(5, 'H');
				_cycle = 8;
				_registers.PC++;
				break;
			case 0xAD:
				RES_b_r(5, 'L');
				_cycle = 8;
				_registers.PC++;
				break;
			case 0xAE:
				RES_b_16(5);
				_cycle = 16;
				_registers.PC++;
				break;
			case 0xAF:
				RES_b_r(5, 'A');
				_cycle = 8;
				_registers.PC++;
				break;
			case 0xB0:
				RES_b_r(6, 'B');
				_cycle = 8;
				_registers.PC++;
				break;
			case 0xB1:
				RES_b_r(6, 'C');
				_cycle = 8;
				_registers.PC++;
				break;
			case 0xB2:
				RES_b_r(6, 'D');
				_cycle = 8;
				_registers.PC++;
				break;
			case 0xB3:
				RES_b_r(6, 'E');
				_cycle = 8;
				_registers.PC++;
				break;
			case 0xB4:
				RES_b_r(6, 'H');
				_cycle = 8;
				_registers.PC++;
				break;
			case 0xB5:
				RES_b_r(6, 'L');
				_cycle = 8;
				_registers.PC++;
				break;
			case 0xB6:
				RES_b_16(6);
				_cycle = 16;
				_registers.PC++;
				break;
			case 0xB7:
				RES_b_r(6, 'A');
				_cycle = 8;
				_registers.PC++;
				break;
			case 0xB8:
				RES_b_r(7, 'B');
				_cycle = 8;
				_registers.PC++;
				break;
			case 0xB9:
				RES_b_r(7, 'C');
				_cycle = 8;
				_registers.PC++;
				break;
			case 0xBA:
				RES_b_r(7, 'D');
				_cycle = 8;
				_registers.PC++;
				break;
			case 0xBB:
				RES_b_r(7, 'E');
				_cycle = 8;
				_registers.PC++;
				break;
			case 0xBC:
				RES_b_r(7, 'H');
				_cycle = 8;
				_registers.PC++;
				break;
			case 0xBD:
				RES_b_r(7, 'L');
				_cycle = 8;
				_registers.PC++;
				break;
			case 0xBE:
				RES_b_16(7);
				_cycle = 16;
				_registers.PC++;
				break;
			case 0xBF:
				RES_b_r(7, 'A');
				_cycle = 8;
				_registers.PC++;
				break;
			case 0xC0:
				SET_b_r(0, 'B');
				_cycle = 8;
				_registers.PC++;
				break;
			case 0xC1:
				SET_b_r(0, 'C');
				_cycle = 8;
				_registers.PC++;
				break;
			case 0xC2:
				SET_b_r(0, 'D');
				_cycle = 8;
				_registers.PC++;
				break;
			case 0xC3:
				SET_b_r(0, 'E');
				_cycle = 8;
				_registers.PC++;
				break;
			case 0xC4:
				SET_b_r(0, 'H');
				_cycle = 8;
				_registers.PC++;
				break;
			case 0xC5:
				SET_b_r(0, 'L');
				_cycle = 8;
				_registers.PC++;
				break;
			case 0xC6:
				SET_b_16(0);
				_cycle = 16;
				_registers.PC++;
				break;
			case 0xC7:
				SET_b_r(0, 'A');
				_cycle = 8;
				_registers.PC++;
				break;
			case 0xC8:
				SET_b_r(1, 'B');
				_cycle = 8;
				_registers.PC++;
				break;
			case 0xC9:
				SET_b_r(1, 'C');
				_cycle = 8;
				_registers.PC++;
				break;
			case 0xCA:
				SET_b_r(1, 'D');
				_cycle = 8;
				_registers.PC++;
				break;
			case 0xCB:
				SET_b_r(1, 'E');
				_cycle = 8;
				_registers.PC++;
				break;
			case 0xCC:
				SET_b_r(1, 'H');
				_cycle = 8;
				_registers.PC++;
				break;
			case 0xCD:
				SET_b_r(1, 'L');
				_cycle = 8;
				_registers.PC++;
				break;
			case 0xCE:
				SET_b_16(1);
				_cycle = 16;
				_registers.PC++;
				break;
			case 0xCF:
				SET_b_r(1, 'A');
				_cycle = 8;
				_registers.PC++;
				break;
			case 0xD0:
				SET_b_r(2, 'B');
				_cycle = 8;
				_registers.PC++;
				break;
			case 0xD1:
				SET_b_r(2, 'C');
				_cycle = 8;
				_registers.PC++;
				break;
			case 0xD2:
				SET_b_r(2, 'D');
				_cycle = 8;
				_registers.PC++;
				break;
			case 0xD3:
				SET_b_r(2, 'E');
				_cycle = 8;
				_registers.PC++;
				break;
			case 0xD4:
				SET_b_r(2, 'H');
				_cycle = 8;
				_registers.PC++;
				break;
			case 0xD5:
				SET_b_r(2, 'L');
				_cycle = 8;
				_registers.PC++;
				break;
			case 0xD6:
				SET_b_16(2);
				_cycle = 16;
				_registers.PC++;
				break;
			case 0xD7:
				SET_b_r(2, 'A');
				_cycle = 8;
				_registers.PC++;
				break;
			case 0xD8:
				SET_b_r(3, 'B');
				_cycle = 8;
				_registers.PC++;
				break;
			case 0xD9:
				SET_b_r(3, 'C');
				_cycle = 8;
				_registers.PC++;
				break;
			case 0xDA:
				SET_b_r(3, 'D');
				_cycle = 8;
				_registers.PC++;
				break;
			case 0xDB:
				SET_b_r(3, 'E');
				_cycle = 8;
				_registers.PC++;
				break;
			case 0xDC:
				SET_b_r(3, 'H');
				_cycle = 8;
				_registers.PC++;
				break;
			case 0xDD:
				SET_b_r(3, 'L');
				_cycle = 8;
				_registers.PC++;
				break;
			case 0xDE:
				SET_b_16(3);
				_cycle = 16;
				_registers.PC++;
				break;
			case 0xDF:
				SET_b_r(3, 'A');
				_cycle = 8;
				_registers.PC++;
				break;
			case 0xE0:
				SET_b_r(4, 'B');
				_cycle = 8;
				_registers.PC++;
				break;
			case 0xE1:
				SET_b_r(4, 'C');
				_cycle = 8;
				_registers.PC++;
				break;
			case 0xE2:
				SET_b_r(4, 'D');
				_cycle = 8;
				_registers.PC++;
				break;
			case 0xE3:
				SET_b_r(4, 'E');
				_cycle = 8;
				_registers.PC++;
				break;
			case 0xE4:
				SET_b_r(4, 'H');
				_cycle = 8;
				_registers.PC++;
				break;
			case 0xE5:
				SET_b_r(4, 'L');
				_cycle = 8;
				_registers.PC++;
				break;
			case 0xE6:
				SET_b_16(4);
				_cycle = 16;
				_registers.PC++;
				break;
			case 0xE7:
				SET_b_r(4, 'A');
				_cycle = 8;
				_registers.PC++;
				break;
			case 0xE8:
				SET_b_r(5, 'B');
				_cycle = 8;
				_registers.PC++;
				break;
			case 0xE9:
				SET_b_r(5, 'C');
				_cycle = 8;
				_registers.PC++;
				break;
			case 0xEA:
				SET_b_r(5, 'D');
				_cycle = 8;
				_registers.PC++;
				break;
			case 0xEB:
				SET_b_r(5, 'E');
				_registers.PC++;
				break;
			case 0xEC:
				SET_b_r(5, 'H');
				_cycle = 8;
				_registers.PC++;
				break;
			case 0xED:
				SET_b_r(5, 'L');
				_cycle = 8;
				_registers.PC++;
				break;
			case 0xEE:
				SET_b_16(5);
				_cycle = 16;
				_registers.PC++;
				break;
			case 0xEF:
				SET_b_r(5, 'A');
				_cycle = 8;
				_registers.PC++;
				break;
			case 0xF0:
				SET_b_r(6, 'B');
				_cycle = 8;
				_registers.PC++;
				break;
			case 0xF1:
				SET_b_r(6, 'C');
				_cycle = 8;
				_registers.PC++;
				break;
			case 0xF2:
				SET_b_r(6, 'D');
				_cycle = 8;
				_registers.PC++;
				break;
			case 0xF3:
				SET_b_r(6, 'E');
				_cycle = 8;
				_registers.PC++;
				break;
			case 0xF4:
				SET_b_r(6, 'H');
				_cycle = 8;
				_registers.PC++;
				break;
			case 0xF5:
				SET_b_r(6, 'L');
				_cycle = 8;
				_registers.PC++;
				break;
			case 0xF6:
				SET_b_16(6);
				_cycle = 16;
				_registers.PC++;
				break;
			case 0xF7:
				SET_b_r(6, 'A');
				_cycle = 8;
				_registers.PC++;
				break;
			case 0xF8:
				SET_b_r(7, 'B');
				_cycle = 8;
				_registers.PC++;
				break;
			case 0xF9:
				SET_b_r(7, 'C');
				_cycle = 8;
				_registers.PC++;
				break;
			case 0xFA:
				SET_b_r(7, 'D');
				_cycle = 8;
				_registers.PC++;
				break;
			case 0xFB:
				SET_b_r(7, 'E');
				_cycle = 8;
				_registers.PC++;
				break;
			case 0xFC:
				SET_b_r(7, 'H');
				_cycle = 8;
				_registers.PC++;
				break;
			case 0xFD:
				SET_b_r(7, 'L');
				_cycle = 8;
				_registers.PC++;
				break;
			case 0xFE:
				SET_b_16(7);
				_cycle = 16;
				_registers.PC++;
				break;
			case 0xFF:
				SET_b_r(7, 'A');
				_cycle = 8;
				_registers.PC++;
				break;

			default:
			{
				cout << "Unsupported 0xCB Instruction : " << hex << (int)_mmu.readRam(_registers.PC);
				exit(0xCB);
				break;
			}
			}
			break;
		}
		default:
			cout << "Unsupported Instruction : " << hex << (int)_mmu.readRam(_registers.PC);
			break;
			exit(0);
			break;
		}
		_prevOpcode = _mmu.readRam(prevPC);
	}
}
