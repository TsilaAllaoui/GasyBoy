#include "interruptManagerProvider.h"
#include "utilitiesProvider.h"
#include "registersProvider.h"
#include "mmuProvider.h"
#include "gbException.h"
#include "timer.h"
#include "cpu.h"

namespace gasyboy
{

	Cpu::State Cpu::state = Cpu::State::RUNNING;

	Cpu::Cpu()
		: _mmu(provider::MmuProvider::getInstance()),
		  _registers(provider::RegistersProvider::getInstance()),
		  _interruptManager(provider::InterruptManagerProvider::getInstance()),
		  _currentOpcode(0),
		  _cycle(0)
	{
		// If not booting bios, set registers directly to program
		auto bootBios = provider::UtilitiesProvider::getInstance()->executeBios;
		if (!bootBios)
		{
			_mmu->disableBios();
			_registers->AF.set(0x01B0);
			_registers->BC.set(0x0013);
			_registers->DE.set(0x00D8);
			_registers->HL.set(0x014D);
			_registers->PC = 0x100;
			_registers->SP = 0xFFFE;
		}
	}

	Cpu &Cpu::operator=(const Cpu &other)
	{
		_mmu = other._mmu;
		_registers = other._registers;
		_interruptManager = other._interruptManager;
		_currentOpcode = other._currentOpcode;
		_cycle = other._cycle;
		return *this;
	}

	uint16_t Cpu::getRegister(const Register::RegisterPairName &reg)
	{
		if (reg == Register::RegisterPairName::PC)
		{
			return _registers->PC;
		}
		else if (reg == Register::RegisterPairName::SP)
		{
			return _registers->SP;
		}
		else
		{
			return _registers->getRegister(reg).get();
		}
	}

	uint8_t Cpu::getRegister(const Register::RegisterName &reg)
	{
		return _registers->getRegister(reg);
	}

	bool Cpu::checkAddHalfCarry(const uint8_t &a, const uint8_t &b)
	{
		return ((((a & 0xF) + (b & 0xF)) & 0x10) == 0x10);
	}

	bool Cpu::checkAddHalfCarry(const uint16_t &a, const uint16_t &b)
	{
		return ((a & 0xFF) + (b & 0xFF) >= 0x100);
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

	bool Cpu::checkAddCarry(const uint8_t &a, const uint8_t &b)
	{
		return ((uint16_t)a + (uint16_t)b) >= 0x100;
	}

	bool Cpu::checkAddCarry(const uint16_t &a, const uint16_t &b)
	{
		bool firstBit = a & 0x8000, secondtBit = b & 0x8000;
		return (firstBit & secondtBit) ? true : false;
	}

	uint16_t Cpu::next2bytes(const uint16_t &adress)
	{
		uint8_t leftValue = _mmu->readRam(adress + 1);
		uint16_t value = ((leftValue << 8) | (_mmu->readRam(adress)));
		return value;
	}

	void Cpu::reset()
	{
		_currentOpcode = 0;

		state = State::STOPPED;

		_registers->AF.set(0);
		_registers->BC.set(0);
		_registers->DE.set(0);
		_registers->HL.set(0);
		_registers->PC = 0;
		_registers->SP = 0xFFFE;

		auto bootBios = provider::UtilitiesProvider::getInstance()->executeBios;

		if (!bootBios)
		{
			_mmu->disableBios();
			_registers->AF.set(0x01B0);
			_registers->BC.set(0x0013);
			_registers->DE.set(0x00D8);
			_registers->HL.set(0x014D);
			_registers->PC = 0x100;
		}
	}

	long Cpu::step()
	{
		if (_registers->getStopMode())
		{
			return 4; // While in STOP mode, the CPU does nothing
		}

		if (state == State::RUNNING || state == State::STEPPING)
		{
			if (_haltBug)
			{
				_haltBug = false; // Clear the bug flag
				fetch();		  // Skip the usual fetch and directly execute next instruction
				execute();
				return _cycle;
			}

			if (!_registers->getHalted())
			{
				fetch();
				execute();
				return _cycle;
			}
			else
			{
				// Check if an interrupt can wake the CPU
				if ((_mmu->readRam(0xFF0F) & _mmu->readRam(0xFFFF) & 0x1F) > 0)
				{
					_registers->setHalted(false);
					_registers->PC++; // Wake up from HALT
				}
				return 4; // Halt state takes 4 cycles per iteration
			}
		}
		throw exception::GbException("Invalid cpu flow");
	}

	void Cpu::fetch()
	{
		_currentOpcode = _mmu->readRam(_registers->PC);
	}

	void Cpu::execute()
	{
		uint16_t prevPC = _registers->PC;
		_cycle = instructionTicks[_currentOpcode];
		switch (_currentOpcode)
		{
		case 0x0:
			NOP();
			_registers->PC++;
			break;
		case 0x01:
			LD_rr_16(_registers->PC + 1, Register::RegisterPairName::BC);
			_registers->PC += 3;
			break;
		case 0x02:
			LD_16_r(_registers->BC.get(), Register::RegisterName::A);
			_registers->PC++;
			break;
		case 0x03:
			INC_rr(Register::RegisterPairName::BC);
			_registers->PC++;
			break;
		case 0x04:
			INC_r(Register::RegisterName::B);
			_registers->PC++;
			break;
		case 0x05:
			DEC_r(Register::RegisterName::B);
			_registers->PC++;
			break;
		case 0x06:
			LD_r_n(_mmu->readRam(_registers->PC + 1), Register::RegisterName::B);
			_registers->PC += 2;
			break;
		case 0x07:
			RLCA();
			_registers->PC++;
			break;
		case 0x08:
			LD_16_rr(next2bytes(_registers->PC + 1), Register::RegisterPairName::SP);
			_registers->PC += 3;
			break;
		case 0x09:
			ADD_HL_rr(Register::RegisterPairName::BC);
			_registers->PC++;
			break;
		case 0x0A:
			LD_r_16(_registers->BC.get(), Register::RegisterName::A);
			_registers->PC++;
			break;
		case 0x0B:
			DEC_rr(Register::RegisterPairName::BC);
			_registers->PC++;
			break;
		case 0x0C:
			INC_r(Register::RegisterName::C);
			_registers->PC++;
			break;
		case 0x0D:
			DEC_r(Register::RegisterName::C);
			_registers->PC++;
			break;
		case 0x0E:
			LD_r_n(_mmu->readRam(_registers->PC + 1), Register::RegisterName::C);
			_registers->PC += 2;
			break;
		case 0x0F:
			RRCA();
			_registers->PC++;
			break;
		case 0x10:
			_registers->setStopMode(true); // Mark CPU as stopped
			_registers->PC++;
			Timer::resetDIV();
			break;
		case 0x11:
			LD_rr_16(_registers->PC + 1, Register::RegisterPairName::DE);
			_registers->PC += 3;
			break;
		case 0x12:
			LD_16_r(_registers->DE.get(), Register::RegisterName::A);
			_registers->PC++;
			break;
		case 0x13:
			INC_rr(Register::RegisterPairName::DE);
			_registers->PC++;
			break;
		case 0x14:
			INC_r(Register::RegisterName::D);
			_registers->PC++;
			break;
		case 0x15:
			DEC_r(Register::RegisterName::D);
			_registers->PC++;
			break;
		case 0x16:
			LD_r_n(_mmu->readRam(_registers->PC + 1), Register::RegisterName::D);
			_registers->PC += 2;
			break;
		case 0x17:
			RLA();
			_registers->PC++;
			break;
		case 0x18:
			JR_e(_mmu->readRam(_registers->PC + 1));
			break;
		case 0x19:
			ADD_HL_rr(Register::RegisterPairName::DE);
			_registers->PC++;
			break;
		case 0x1A:
			LD_r_16(_registers->DE.get(), Register::RegisterName::A);
			_registers->PC++;
			break;
		case 0x1B:
			DEC_rr(Register::RegisterPairName::DE);
			_registers->PC++;
			break;
		case 0x1C:
			INC_r(Register::RegisterName::E);
			_registers->PC++;
			break;
		case 0x1D:
			DEC_r(Register::RegisterName::E);
			_registers->PC++;
			break;
		case 0x1E:
			LD_r_n(_mmu->readRam(_registers->PC + 1), Register::RegisterName::E);
			_registers->PC += 2;
			break;
		case 0x1F:
			RRA();
			_registers->PC++;
			break;
		case 0x20:
			JR_NZ_e(_mmu->readRam(_registers->PC + 1));
			_registers->AF.getFlag(Register::FlagName::Z);
			break;
		case 0x21:
			LD_rr_nn(next2bytes(_registers->PC + 1), Register::RegisterPairName::HL);
			_registers->PC += 3;
			break;
		case 0x22:
			LD_16_r(_registers->HL.get(), Register::RegisterName::A);
			INC_rr(Register::RegisterPairName::HL);
			_registers->PC++;
			break;
		case 0x23:
			INC_rr(Register::RegisterPairName::HL);
			_registers->PC++;
			break;
		case 0x24:
			INC_r(Register::RegisterName::H);
			_registers->PC++;
			break;
		case 0x25:
			DEC_r(Register::RegisterName::H);
			_registers->PC++;
			break;
		case 0x26:
			LD_r_n(_mmu->readRam(_registers->PC + 1), Register::RegisterName::H);
			_registers->PC += 2;
			break;
		case 0x27:
			DAA();
			_registers->PC++;
			break;
		case 0x28:
			JR_Z_e(_mmu->readRam(_registers->PC + 1));
			break;
		case 0x29:
			ADD_HL_rr(Register::RegisterPairName::HL);
			_registers->PC++;
			break;
		case 0x2A:
			LD_r_n(_mmu->readRam(_registers->HL.get()), Register::RegisterName::A);
			INC_rr(Register::RegisterPairName::HL);
			_registers->PC++;
			break;
		case 0x2B:
			DEC_rr(Register::RegisterPairName::HL);
			_registers->PC++;
			break;
		case 0x2C:
			INC_r(Register::RegisterName::L);
			_registers->PC++;
			break;
		case 0x2D:
			DEC_r(Register::RegisterName::L);
			_registers->PC++;
			break;
		case 0x2E:
			LD_r_n(_mmu->readRam(_registers->PC + 1), Register::RegisterName::L);
			_registers->PC += 2;
			break;
		case 0x2F:
			CPL();
			_registers->PC++;
			break;
		case 0x30:
			JR_NC_e(_mmu->readRam(_registers->PC + 1));
			break;
		case 0x31:
			LD_rr_nn(next2bytes(_registers->PC + 1), Register::RegisterPairName::SP);
			_registers->PC += 3;
			break;
		case 0x32:
			LD_16_r(_registers->HL.get(), Register::RegisterName::A);
			DEC_rr(Register::RegisterPairName::HL);
			_registers->PC++;
			break;
		case 0x33:
			INC_rr(Register::RegisterPairName::SP);
			_registers->PC++;
			break;
		case 0x34:
			INC_16();
			_registers->PC++;
			break;
		case 0x35:
			DEC_16();
			_registers->PC++;
			break;
		case 0x36: // TODO may be innacurate
			LD_16_n(_registers->HL.get(), _mmu->readRam(_registers->PC + 1));
			_registers->PC += 2;
			break;
		case 0x37:
			SCF();
			_registers->PC++;
			break;
		case 0x38:
			JR_C_e(_mmu->readRam(_registers->PC + 1));
			break;
		case 0x39:
			ADD_HL_rr(Register::RegisterPairName::SP);
			_registers->PC++;
			break;
		case 0x3A:
			LD_r_16(_registers->HL.get(), Register::RegisterName::A);
			DEC_rr(Register::RegisterPairName::HL);
			_registers->PC++;
			break;
		case 0x3B:
			DEC_rr(Register::RegisterPairName::SP);
			_registers->PC++;
			break;
		case 0x3C:
			INC_r(Register::RegisterName::A);
			_registers->PC++;
			break;
		case 0x3D:
			DEC_r(Register::RegisterName::A);
			_registers->PC++;
			break;
		case 0x3E:
			LD_r_n(_mmu->readRam(_registers->PC + 1), Register::RegisterName::A);
			_registers->PC += 2;
			break;
		case 0x3F:
			CCF();
			_registers->PC++;
			break;
		case 0x40:
			LD_r_r(Register::RegisterName::B, Register::RegisterName::B);
			_registers->PC++;
			break;
		case 0x41:
			LD_r_r(Register::RegisterName::C, Register::RegisterName::B);
			_registers->PC++;
			break;
		case 0x42:
			LD_r_r(Register::RegisterName::D, Register::RegisterName::B);
			_registers->PC++;
			break;
		case 0x43:
			LD_r_r(Register::RegisterName::E, Register::RegisterName::B);
			_registers->PC++;
			break;
		case 0x44:
			LD_r_r(Register::RegisterName::H, Register::RegisterName::B);
			_registers->PC++;
			break;
		case 0x45:
			LD_r_r(Register::RegisterName::L, Register::RegisterName::B);
			_registers->PC++;
			break;
		case 0x46:
			LD_r_16(_registers->HL.get(), Register::RegisterName::B);
			_registers->PC++;
			break;
		case 0x47:
			LD_r_r(Register::RegisterName::A, Register::RegisterName::B);
			_registers->PC++;
			break;
		case 0x48:
			LD_r_r(Register::RegisterName::B, Register::RegisterName::C);
			_registers->PC++;
			break;
		case 0x49:
			LD_r_r(Register::RegisterName::C, Register::RegisterName::C);
			_registers->PC++;
			break;
		case 0x4A:
			LD_r_r(Register::RegisterName::D, Register::RegisterName::C);
			_registers->PC++;
			break;
		case 0x4B:
			LD_r_r(Register::RegisterName::E, Register::RegisterName::C);
			_registers->PC++;
			break;
		case 0x4C:
			LD_r_r(Register::RegisterName::H, Register::RegisterName::C);
			_registers->PC++;
			break;
		case 0x4D:
			LD_r_r(Register::RegisterName::L, Register::RegisterName::C);
			_registers->PC++;
			break;
		case 0x4E:
			LD_r_16(_registers->HL.get(), Register::RegisterName::C);
			_registers->PC++;
			break;
		case 0x4F:
			LD_r_r(Register::RegisterName::A, Register::RegisterName::C);
			_registers->PC++;
			break;
		case 0x50:
			LD_r_r(Register::RegisterName::B, Register::RegisterName::D);
			_registers->PC++;
			break;
		case 0x51:
			LD_r_r(Register::RegisterName::C, Register::RegisterName::D);
			_registers->PC++;
			break;
		case 0x52:
			LD_r_r(Register::RegisterName::D, Register::RegisterName::D);
			_registers->PC++;
			break;
		case 0x53:
			LD_r_r(Register::RegisterName::E, Register::RegisterName::D);
			_registers->PC++;
			break;
		case 0x54:
			LD_r_r(Register::RegisterName::H, Register::RegisterName::D);
			_registers->PC++;
			break;
		case 0x55:
			LD_r_r(Register::RegisterName::L, Register::RegisterName::D);
			_registers->PC++;
			break;
		case 0x56:
			LD_r_16(_registers->HL.get(), Register::RegisterName::D);
			_registers->PC++;
			break;
		case 0x57:
			LD_r_r(Register::RegisterName::A, Register::RegisterName::D);
			_registers->PC++;
			break;
		case 0x58:
			LD_r_r(Register::RegisterName::B, Register::RegisterName::E);
			_registers->PC++;
			break;
		case 0x59:
			LD_r_r(Register::RegisterName::C, Register::RegisterName::E);
			_registers->PC++;
			break;
		case 0x5A:
			LD_r_r(Register::RegisterName::D, Register::RegisterName::E);
			_registers->PC++;
			break;
		case 0x5B:
			LD_r_r(Register::RegisterName::E, Register::RegisterName::E);
			_registers->PC++;
			break;
		case 0x5C:
			LD_r_r(Register::RegisterName::H, Register::RegisterName::E);
			_registers->PC++;
			break;
		case 0x5D:
			LD_r_r(Register::RegisterName::L, Register::RegisterName::E);
			_registers->PC++;
			break;
		case 0x5E:
			LD_r_16(_registers->HL.get(), Register::RegisterName::E);
			_registers->PC++;
			break;
		case 0x5F:
			LD_r_r(Register::RegisterName::A, Register::RegisterName::E);
			_registers->PC++;
			break;
		case 0x60:
			LD_r_r(Register::RegisterName::B, Register::RegisterName::H);
			_registers->PC++;
			break;
		case 0x61:
			LD_r_r(Register::RegisterName::C, Register::RegisterName::H);
			_registers->PC++;
			break;
		case 0x62:
			LD_r_r(Register::RegisterName::D, Register::RegisterName::H);
			_registers->PC++;
			break;
		case 0x63:
			LD_r_r(Register::RegisterName::E, Register::RegisterName::H);
			_registers->PC++;
			break;
		case 0x64:
			LD_r_r(Register::RegisterName::H, Register::RegisterName::H);
			_registers->PC++;
			break;
		case 0x65:
			LD_r_r(Register::RegisterName::L, Register::RegisterName::H);
			_registers->PC++;
			break;
		case 0x66:
			LD_r_16(_registers->HL.get(), Register::RegisterName::H);
			_registers->PC++;
			break;
		case 0x67:
			LD_r_r(Register::RegisterName::A, Register::RegisterName::H);
			_registers->PC++;
			break;
		case 0x68:
			LD_r_r(Register::RegisterName::B, Register::RegisterName::L);
			_registers->PC++;
			break;
		case 0x69:
			LD_r_r(Register::RegisterName::C, Register::RegisterName::L);
			_registers->PC++;
			break;
		case 0x6A:
			LD_r_r(Register::RegisterName::D, Register::RegisterName::L);
			_registers->PC++;
			break;
		case 0x6B:
			LD_r_r(Register::RegisterName::E, Register::RegisterName::L);
			_registers->PC++;
			break;
		case 0x6C:
			LD_r_r(Register::RegisterName::H, Register::RegisterName::L);
			_registers->PC++;
			break;
		case 0x6D:
			LD_r_r(Register::RegisterName::L, Register::RegisterName::L);
			_registers->PC++;
			break;
		case 0x6E:
			LD_r_16(_registers->HL.get(), Register::RegisterName::L);
			_registers->PC++;
			break;
		case 0x6F:
			LD_r_r(Register::RegisterName::A, Register::RegisterName::L);
			_registers->PC++;
			break;
		case 0x70:
			LD_16_r(_registers->HL.get(), Register::RegisterName::B);
			_registers->PC++;
			break;
		case 0x71:
			LD_16_r(_registers->HL.get(), Register::RegisterName::C);
			_registers->PC++;
			break;
		case 0x72:
			LD_16_r(_registers->HL.get(), Register::RegisterName::D);
			_registers->PC++;
			break;
		case 0x73:
			LD_16_r(_registers->HL.get(), Register::RegisterName::E);
			_registers->PC++;
			break;
		case 0x74:
			LD_16_r(_registers->HL.get(), Register::RegisterName::H);
			_registers->PC++;
			break;
		case 0x75:
			LD_16_r(_registers->HL.get(), Register::RegisterName::L);
			_registers->PC++;
			break;
		case 0x76:
			HALT();
			_registers->PC++;
			break;
		case 0x77:
			LD_16_r(_registers->HL.get(), Register::RegisterName::A);
			_registers->PC++;
			break;
		case 0x78:
			LD_r_r(Register::RegisterName::B, Register::RegisterName::A);
			_registers->PC++;
			break;
		case 0x79:
			LD_r_r(Register::RegisterName::C, Register::RegisterName::A);
			_registers->PC++;
			break;
		case 0x7A:
			LD_r_r(Register::RegisterName::D, Register::RegisterName::A);
			_registers->PC++;
			break;
		case 0x7B:
			LD_r_r(Register::RegisterName::E, Register::RegisterName::A);
			_registers->PC++;
			break;
		case 0x7C:
			LD_r_r(Register::RegisterName::H, Register::RegisterName::A);
			_registers->PC++;
			break;
		case 0x7D:
			LD_r_r(Register::RegisterName::L, Register::RegisterName::A);
			_registers->PC++;
			break;
		case 0x7E:
			LD_r_16(_registers->HL.get(), Register::RegisterName::A);
			_registers->PC++;
			break;
		case 0x7F:
			LD_r_r(Register::RegisterName::A, Register::RegisterName::A);
			_registers->PC++;
			break;
		case 0x80:
			ADD_A_r(Register::RegisterName::B);
			_registers->PC++;
			break;
		case 0x81:
			ADD_A_r(Register::RegisterName::C);
			_registers->PC++;
			break;
		case 0x82:
			ADD_A_r(Register::RegisterName::D);
			_registers->PC++;
			break;
		case 0x83:
			ADD_A_r(Register::RegisterName::E);
			_registers->PC++;
			break;
		case 0x84:
			ADD_A_r(Register::RegisterName::H);
			_registers->PC++;
			break;
		case 0x85:
			ADD_A_r(Register::RegisterName::L);
			_registers->PC++;
			break;
		case 0x86:
			ADD_A_16();
			_registers->PC++;
			break;
		case 0x87:
			ADD_A_r(Register::RegisterName::A);
			_registers->PC++;
			break;
		case 0x88:
			ADC_A_r(Register::RegisterName::B);
			_registers->PC++;
			break;
		case 0x89:
			ADC_A_r(Register::RegisterName::C);
			_registers->PC++;
			break;
		case 0x8A:
			ADC_A_r(Register::RegisterName::D);
			_registers->PC++;
			break;
		case 0x8B:
			ADC_A_r(Register::RegisterName::E);
			_registers->PC++;
			break;
		case 0x8C:
			ADC_A_r(Register::RegisterName::H);
			_registers->PC++;
			break;
		case 0x8D:
			ADC_A_r(Register::RegisterName::L);
			_registers->PC++;
			break;
		case 0x8E:
			ADC_A_16();
			_registers->PC++;
			break;
		case 0x8F:
			ADC_A_r(Register::RegisterName::A);
			_registers->PC++;
			break;
		case 0x90:
			SUB_r(Register::RegisterName::B);
			_registers->PC++;
			break;
		case 0x91:
			SUB_r(Register::RegisterName::C);
			_registers->PC++;
			break;
		case 0x92:
			SUB_r(Register::RegisterName::D);
			_registers->PC++;
			break;
		case 0x93:
			SUB_r(Register::RegisterName::E);
			_registers->PC++;
			break;
		case 0x94:
			SUB_r(Register::RegisterName::H);
			_registers->PC++;
			break;
		case 0x95:
			SUB_r(Register::RegisterName::L);
			_registers->PC++;
			break;
		case 0x96:
			SUB_16();
			_registers->PC++;
			break;
		case 0x97:
			SUB_r(Register::RegisterName::A);
			_registers->PC++;
			break;
		case 0x98:
			SBC_r(Register::RegisterName::B);
			_registers->PC++;
			break;
		case 0x99:
			SBC_r(Register::RegisterName::C);
			_registers->PC++;
			break;
		case 0x9A:
			SBC_r(Register::RegisterName::D);
			_registers->PC++;
			break;
		case 0x9B:
			SBC_r(Register::RegisterName::E);
			_registers->PC++;
			break;
		case 0x9C:
			SBC_r(Register::RegisterName::H);
			_registers->PC++;
			break;
		case 0x9D:
			SBC_r(Register::RegisterName::L);
			_registers->PC++;
			break;
		case 0x9E:
			SBC_16();
			_registers->PC++;
			break;
		case 0x9F:
			SBC_r(Register::RegisterName::A);
			_registers->PC++;
			break;
		case 0xA0:
			AND_r(Register::RegisterName::B);
			_registers->PC++;
			break;
		case 0xA1:
			AND_r(Register::RegisterName::C);
			_registers->PC++;
			break;
		case 0xA2:
			AND_r(Register::RegisterName::D);
			_registers->PC++;
			break;
		case 0xA3:
			AND_r(Register::RegisterName::E);
			_registers->PC++;
			break;
		case 0xA4:
			AND_r(Register::RegisterName::H);
			_registers->PC++;
			break;
		case 0xA5:
			AND_r(Register::RegisterName::L);
			_registers->PC++;
			break;
		case 0xA6:
			AND_16();
			_registers->PC++;
			break;
		case 0xA7:
			AND_r(Register::RegisterName::A);
			_registers->PC++;
			break;
		case 0xA8:
			XOR_r(Register::RegisterName::B);
			_registers->PC++;
			break;
		case 0xA9:
			XOR_r(Register::RegisterName::C);
			_registers->PC++;
			break;
		case 0xAA:
			XOR_r(Register::RegisterName::D);
			_registers->PC++;
			break;
		case 0xAB:
			XOR_r(Register::RegisterName::E);
			_registers->PC++;
			break;
		case 0xAC:
			XOR_r(Register::RegisterName::H);
			_registers->PC++;
			break;
		case 0xAD:
			XOR_r(Register::RegisterName::L);
			_registers->PC++;
			break;
		case 0xAE:
			XOR_16();
			_registers->PC++;
			break;
		case 0xAF:
			XOR_r(Register::RegisterName::A);
			_registers->PC++;
			break;
		case 0xB0:
			OR_r(Register::RegisterName::B);
			_registers->PC++;
			break;
		case 0xB1:
			OR_r(Register::RegisterName::C);
			_registers->PC++;
			break;
		case 0xB2:
			OR_r(Register::RegisterName::D);
			_registers->PC++;
			break;
		case 0xB3:
			OR_r(Register::RegisterName::E);
			_registers->PC++;
			break;
		case 0xB4:
			OR_r(Register::RegisterName::H);
			_registers->PC++;
			break;
		case 0xB5:
			OR_r(Register::RegisterName::L);
			_registers->PC++;
			break;
		case 0xB6:
			OR_16();
			_registers->PC++;
			break;
		case 0xB7:
			OR_r(Register::RegisterName::A);
			_registers->PC++;
			break;
		case 0xB8:
			CP_r(Register::RegisterName::B);
			_registers->PC++;
			break;
		case 0xB9:
			CP_r(Register::RegisterName::C);
			_registers->PC++;
			break;
		case 0xBA:
			CP_r(Register::RegisterName::D);
			_registers->PC++;
			break;
		case 0xBB:
			CP_r(Register::RegisterName::E);
			_registers->PC++;
			break;
		case 0xBC:
			CP_r(Register::RegisterName::H);
			_registers->PC++;
			break;
		case 0xBD:
			CP_r(Register::RegisterName::L);
			_registers->PC++;
			break;
		case 0xBE:
			CP_16();
			_registers->PC++;
			break;
		case 0xBF:
			CP_r(Register::RegisterName::A);
			_registers->PC++;
			break;
		case 0xC0:
			RET_c(Register::FlagName::NZ);
			break;
		case 0xC1:
			POP(Register::RegisterPairName::BC);
			_registers->PC++;
			break;
		case 0xC2:
			JP_c_16(Register::FlagName::NZ, next2bytes(_registers->PC + 1));
			break;
		case 0xC3:
			JP_16(_registers->PC + 1);
			break;
		case 0xC4:
			CALL_c(Register::FlagName::NZ);
			break;
		case 0xC5:
			PUSH(Register::RegisterPairName::BC);
			_registers->PC++;
			break;
		case 0xC6:
			ADD_A_n(_mmu->readRam(_registers->PC + 1));
			_registers->PC += 2;
			break;
		case 0xC7:
			RST_p(0x00);
			break;
		case 0xC8:
			RET_c(Register::FlagName::Z);
			break;
		case 0xC9:
			RET();
			break;
		case 0xCA:
			JP_c_16(Register::FlagName::Z, next2bytes(_registers->PC + 1));
			break;
			// REFER TO CB PREFIX FOR SPECIAL INSTRUCTIONS FURTHER BELOW
		case 0xCC:
			CALL_c(Register::FlagName::Z);
			break;
		case 0xCD:
			CALL();
			break;
		case 0xCE:
			ADC_A_n(_mmu->readRam(_registers->PC + 1));
			_registers->PC += 2;
			break;
		case 0xCF:
			RST_p(0x08);
			break;
		case 0xD0:
			RET_c(Register::FlagName::NC);
			break;
		case 0xD1:
			POP(Register::RegisterPairName::DE);
			_registers->PC++;
			break;
		case 0xD2:
			JP_c_16(Register::FlagName::NC, next2bytes(_registers->PC + 1));
			break;
		case 0xD4:
			CALL_c(Register::FlagName::NC);
			break;
		case 0xD5:
			PUSH(Register::RegisterPairName::DE);
			_registers->PC++;
			break;
		case 0xD6:
			SUB_n(_mmu->readRam(_registers->PC + 1));
			_registers->PC += 2;
			break;
		case 0xD7:
			RST_p(0x10);
			break;
		case 0xD8:
			RET_c(Register::FlagName::C);
			break;
		case 0xD9:
			RETI();
			break;
		case 0xDA:
			JP_c_16(Register::FlagName::C, next2bytes(_registers->PC + 1));
			break;
		case 0xDC:
			CALL_c(Register::FlagName::C);
			break;
		case 0xDE:
			SBC_n(_mmu->readRam(_registers->PC + 1));
			_registers->PC += 2;
			break;
		case 0xDF:
			RST_p(0x18);
			break;
		case 0xE0:
			_mmu->writeRam(_mmu->readRam(_registers->PC + 1) + 0xFF00, _registers->AF.getLeftRegister());
			_registers->PC += 2;
			break;
		case 0xE1:
			POP(Register::RegisterPairName::HL);
			_registers->PC++;
			break;
		case 0xE2:
			_mmu->writeRam(0xFF00 + _registers->BC.getRightRegister(), _registers->AF.getLeftRegister());
			_registers->PC++;
			break;
		case 0xE5:
			PUSH(Register::RegisterPairName::HL);
			_registers->PC++;
			break;
		case 0xE6:
			AND_n(_mmu->readRam(_registers->PC + 1));
			_registers->PC += 2;
			break;
		case 0xE7:
			RST_p(0x20);
			break;
		case 0xE8:
			ADD_SP_n();
			_registers->PC += 2;
			break;
		case 0xE9:
			JP_16();
			break;
		case 0xEA:
			LD_16_r(next2bytes(_registers->PC + 1), Register::RegisterName::A);
			_registers->PC += 3;
			break;
		case 0xEE:
			XOR_n(_mmu->readRam(_registers->PC + 1));
			_registers->PC += 2;
			break;
		case 0xEF:
			RST_p(0x28);
			break;
		case 0xF0:
			_registers->AF.setLeftRegister(_mmu->readRam(_mmu->readRam(_registers->PC + 1) + 0xFF00));
			_registers->PC += 2;
			break;
		case 0xF1:
			POP(Register::RegisterPairName::AF);
			_registers->PC++;
			break;
		case 0xF2:
			_registers->AF.setLeftRegister(_mmu->readRam(_registers->BC.getRightRegister() + 0xFF00));
			_registers->PC++;
			break;
		case 0xF3:
			DI();
			_registers->PC++;
			break;
		case 0xF5:
			PUSH(Register::RegisterPairName::AF);
			_registers->PC++;
			break;
		case 0xF6:
			OR_n(_mmu->readRam(_registers->PC + 1));
			_registers->PC += 2;
			break;
		case 0xF7:
			RST_p(0x30);
			break;
		case 0xF8:
			LD_HL_SP_n();
			_registers->PC += 2;
			break;
		case 0xF9: // TODO may be innacurate
			_registers->SP = _registers->HL.get();
			_registers->PC++;
			break;
		case 0xFA:
			LD_r_16(next2bytes(_registers->PC + 1), Register::RegisterName::A);
			_registers->PC += 3;
			break;
		case 0xFB:
			EI();
			_registers->PC++;
			break;
		case 0xFE:
			CP_n(_mmu->readRam(_registers->PC + 1));
			_registers->PC += 2;
			break;
		case 0xFF:
			RST_p(0x38);
			break;

			/*-----SPECIAL OPCODES-----*/

		case 0xCB:
		{
			prevPC++;
			_registers->PC++;
			_cycle = extendedInstructionTicks[_mmu->readRam(_registers->PC)];
			switch (_mmu->readRam(_registers->PC))
			{
			case 0x00:
				RLC_r(Register::RegisterName::B);
				_registers->PC++;
				break;
			case 0x01:
				RLC_r(Register::RegisterName::C);
				_registers->PC++;
				break;
			case 0x02:
				RLC_r(Register::RegisterName::D);
				_registers->PC++;
				break;
			case 0x03:
				RLC_r(Register::RegisterName::E);
				_registers->PC++;
				break;
			case 0x04:
				RLC_r(Register::RegisterName::H);
				_registers->PC++;
				break;
			case 0x05:
				RLC_r(Register::RegisterName::L);
				_registers->PC++;
				break;
			case 0x06:
				RLC_16();
				_registers->PC++;
				break;
			case 0x07:
				RLC_r(Register::RegisterName::A);
				_registers->PC++;
				break;
			case 0x08:
				RRC_r(Register::RegisterName::B);
				_registers->PC++;
				break;
			case 0x09:
				RRC_r(Register::RegisterName::C);
				_registers->PC++;
				break;
			case 0x0A:
				RRC_r(Register::RegisterName::D);
				_registers->PC++;
				break;
			case 0x0B:
				RRC_r(Register::RegisterName::E);
				_registers->PC++;
				break;
			case 0x0C:
				RRC_r(Register::RegisterName::H);
				_registers->PC++;
				break;
			case 0x0D:
				RRC_r(Register::RegisterName::L);
				_registers->PC++;
				break;
			case 0x0E:
				RRC_16();
				_registers->PC++;
				break;
			case 0x0F:
				RRC_r(Register::RegisterName::A);
				_registers->PC++;
				break;
			case 0x10:
				RL_r(Register::RegisterName::B);
				_registers->PC++;
				break;
			case 0x11:
				RL_r(Register::RegisterName::C);
				_registers->PC++;
				break;
			case 0x12:
				RL_r(Register::RegisterName::D);
				_registers->PC++;
				break;
			case 0x13:
				RL_r(Register::RegisterName::E);
				_registers->PC++;
				break;
			case 0x14:
				RL_r(Register::RegisterName::H);
				_registers->PC++;
				break;
			case 0x15:
				RL_r(Register::RegisterName::L);
				_registers->PC++;
				break;
			case 0x16:
				RL_16();
				_registers->PC++;
				break;
			case 0x17:
				RL_r(Register::RegisterName::A);
				_registers->PC++;
				break;
			case 0x18:
				RR_r(Register::RegisterName::B);
				_registers->PC++;
				break;
			case 0x19:
				RR_r(Register::RegisterName::C);
				_registers->PC++;
				break;
			case 0x1A:
				RR_r(Register::RegisterName::D);
				_registers->PC++;
				break;
			case 0x1B:
				RR_r(Register::RegisterName::E);
				_registers->PC++;
				break;
			case 0x1C:
				RR_r(Register::RegisterName::H);
				_registers->PC++;
				break;
			case 0x1D:
				RR_r(Register::RegisterName::L);
				_registers->PC++;
				break;
			case 0x1E:
				RR_16();
				_registers->PC++;
				break;
			case 0x1F:
				RR_r(Register::RegisterName::A);
				_registers->PC++;
				break;
			case 0x20:
				SLA_r(Register::RegisterName::B);
				_registers->PC++;
				break;
			case 0x21:
				SLA_r(Register::RegisterName::C);
				_registers->PC++;
				break;
			case 0x22:
				SLA_r(Register::RegisterName::D);
				_registers->PC++;
				break;
			case 0x23:
				SLA_r(Register::RegisterName::E);
				_registers->PC++;
				break;
			case 0x24:
				SLA_r(Register::RegisterName::H);
				_registers->PC++;
				break;
			case 0x25:
				SLA_r(Register::RegisterName::L);
				_registers->PC++;
				break;
			case 0x26:
				SLA_16();
				_registers->PC++;
				break;
			case 0x27:
				SLA_r(Register::RegisterName::A);
				_registers->PC++;
				break;
			case 0x28:
				SRA_r(Register::RegisterName::B);
				_registers->PC++;
				break;
			case 0x29:
				SRA_r(Register::RegisterName::C);
				_registers->PC++;
				break;
			case 0x2A:
				SRA_r(Register::RegisterName::D);
				_registers->PC++;
				break;
			case 0x2B:
				SRA_r(Register::RegisterName::E);
				_registers->PC++;
				break;
			case 0x2C:
				SRA_r(Register::RegisterName::H);
				_registers->PC++;
				break;
			case 0x2D:
				SRA_r(Register::RegisterName::L);
				_registers->PC++;
				break;
			case 0x2E:
				SRA_16();
				_registers->PC++;
				break;
			case 0x2F:
				SRA_r(Register::RegisterName::A);
				_registers->PC++;
				break;
			case 0x30:
				SWAP_r(Register::RegisterName::B);
				_registers->PC++;
				break;
			case 0x31:
				SWAP_r(Register::RegisterName::C);
				_registers->PC++;
				break;
			case 0x32:
				SWAP_r(Register::RegisterName::D);
				_registers->PC++;
				break;
			case 0x33:
				SWAP_r(Register::RegisterName::E);
				_registers->PC++;
				break;
			case 0x34:
				SWAP_r(Register::RegisterName::H);
				_registers->PC++;
				break;
			case 0x35:
				SWAP_r(Register::RegisterName::L);
				_registers->PC++;
				break;
			case 0x36:
				SWAP_16();
				_registers->PC++;
				break;
			case 0x37:
				SWAP_r(Register::RegisterName::A);
				_registers->PC++;
				break;
			case 0x38:
				SRL_r(Register::RegisterName::B);
				_registers->PC++;
				break;
			case 0x39:
				SRL_r(Register::RegisterName::C);
				_registers->PC++;
				break;
			case 0x3A:
				SRL_r(Register::RegisterName::D);
				_registers->PC++;
				break;
			case 0x3B:
				SRL_r(Register::RegisterName::E);
				_registers->PC++;
				break;
			case 0x3C:
				SRL_r(Register::RegisterName::H);
				_registers->PC++;
				break;
			case 0x3D:
				SRL_r(Register::RegisterName::L);
				_registers->PC++;
				break;
			case 0x3E:
				SRL_16();
				_registers->PC++;
				break;
			case 0x3F:
				SRL_r(Register::RegisterName::A);
				_registers->PC++;
				break;
			case 0x40:
				BIT_b_r(0, Register::RegisterName::B);
				_registers->PC++;
				break;
			case 0x41:
				BIT_b_r(0, Register::RegisterName::C);
				_registers->PC++;
				break;
			case 0x42:
				BIT_b_r(0, Register::RegisterName::D);
				_registers->PC++;
				break;
			case 0x43:
				BIT_b_r(0, Register::RegisterName::E);
				_registers->PC++;
				break;
			case 0x44:
				BIT_b_r(0, Register::RegisterName::H);
				_registers->PC++;
				break;
			case 0x45:
				BIT_b_r(0, Register::RegisterName::L);
				_registers->PC++;
				break;
			case 0x46:
				BIT_b_16(0);
				_registers->PC++;
				break;
			case 0x47:
				BIT_b_r(0, Register::RegisterName::A);
				_registers->PC++;
				break;
			case 0x48:
				BIT_b_r(1, Register::RegisterName::B);
				_registers->PC++;
				break;
			case 0x49:
				BIT_b_r(1, Register::RegisterName::C);
				_registers->PC++;
				break;
			case 0x4A:
				BIT_b_r(1, Register::RegisterName::D);
				_registers->PC++;
				break;
			case 0x4B:
				BIT_b_r(1, Register::RegisterName::E);
				_registers->PC++;
				break;
			case 0x4C:
				BIT_b_r(1, Register::RegisterName::H);
				_registers->PC++;
				break;
			case 0x4D:
				BIT_b_r(1, Register::RegisterName::L);
				_registers->PC++;
				break;
			case 0x4E:
				BIT_b_16(1);
				_registers->PC++;
				break;
			case 0x4F:
				BIT_b_r(1, Register::RegisterName::A);
				_registers->PC++;
				break;
			case 0x50:
				BIT_b_r(2, Register::RegisterName::B);
				_registers->PC++;
				break;
			case 0x51:
				BIT_b_r(2, Register::RegisterName::C);
				_registers->PC++;
				break;
			case 0x52:
				BIT_b_r(2, Register::RegisterName::D);
				_registers->PC++;
				break;
			case 0x53:
				BIT_b_r(2, Register::RegisterName::E);
				_registers->PC++;
				break;
			case 0x54:
				BIT_b_r(2, Register::RegisterName::H);
				_registers->PC++;
				break;
			case 0x55:
				BIT_b_r(2, Register::RegisterName::L);
				_registers->PC++;
				break;
			case 0x56:
				BIT_b_16(2);
				_registers->PC++;
				break;
			case 0x57:
				BIT_b_r(2, Register::RegisterName::A);
				_registers->PC++;
				break;
			case 0x58:
				BIT_b_r(3, Register::RegisterName::B);
				_registers->PC++;
				break;
			case 0x59:
				BIT_b_r(3, Register::RegisterName::C);
				_registers->PC++;
				break;
			case 0x5A:
				BIT_b_r(3, Register::RegisterName::D);
				_registers->PC++;
				break;
			case 0x5B:
				BIT_b_r(3, Register::RegisterName::E);
				_registers->PC++;
				break;
			case 0x5C:
				BIT_b_r(3, Register::RegisterName::H);
				_registers->PC++;
				break;
			case 0x5D:
				BIT_b_r(3, Register::RegisterName::L);
				_registers->PC++;
				break;
			case 0x5E:
				BIT_b_16(3);
				_registers->PC++;
				break;
			case 0x5F:
				BIT_b_r(3, Register::RegisterName::A);
				_registers->PC++;
				break;
			case 0x60:
				BIT_b_r(4, Register::RegisterName::B);
				_registers->PC++;
				break;
			case 0x61:
				BIT_b_r(4, Register::RegisterName::C);
				_registers->PC++;
				break;
			case 0x62:
				BIT_b_r(4, Register::RegisterName::D);
				_registers->PC++;
				break;
			case 0x63:
				BIT_b_r(4, Register::RegisterName::E);
				_registers->PC++;
				break;
			case 0x64:
				BIT_b_r(4, Register::RegisterName::H);
				_registers->PC++;
				break;
			case 0x65:
				BIT_b_r(4, Register::RegisterName::L);
				_registers->PC++;
				break;
			case 0x66:
				BIT_b_16(4);
				_registers->PC++;
				break;
			case 0x67:
				BIT_b_r(4, Register::RegisterName::A);
				_registers->PC++;
				break;
			case 0x68:
				BIT_b_r(5, Register::RegisterName::B);
				_registers->PC++;
				break;
			case 0x69:
				BIT_b_r(5, Register::RegisterName::C);
				_registers->PC++;
				break;
			case 0x6A:
				BIT_b_r(5, Register::RegisterName::D);
				_registers->PC++;
				break;
			case 0x6B:
				BIT_b_r(5, Register::RegisterName::E);
				_registers->PC++;
				break;
			case 0x6C:
				BIT_b_r(5, Register::RegisterName::H);
				_registers->PC++;
				break;
			case 0x6D:
				BIT_b_r(5, Register::RegisterName::L);
				_registers->PC++;
				break;
			case 0x6E:
				BIT_b_16(5);
				_registers->PC++;
				break;
			case 0x6F:
				BIT_b_r(5, Register::RegisterName::A);
				_registers->PC++;
				break;
			case 0x70:
				BIT_b_r(6, Register::RegisterName::B);
				_registers->PC++;
				break;
			case 0x71:
				BIT_b_r(6, Register::RegisterName::C);
				_registers->PC++;
				break;
			case 0x72:
				BIT_b_r(6, Register::RegisterName::D);
				_registers->PC++;
				break;
			case 0x73:
				BIT_b_r(6, Register::RegisterName::E);
				_registers->PC++;
				break;
			case 0x74:
				BIT_b_r(6, Register::RegisterName::H);
				_registers->PC++;
				break;
			case 0x75:
				BIT_b_r(6, Register::RegisterName::L);
				_registers->PC++;
				break;
			case 0x76:
				BIT_b_16(6);
				_registers->PC++;
				break;
			case 0x77:
				BIT_b_r(6, Register::RegisterName::A);
				_registers->PC++;
				break;
			case 0x78:
				BIT_b_r(7, Register::RegisterName::B);
				_registers->PC++;
				break;
			case 0x79:
				BIT_b_r(7, Register::RegisterName::C);
				_registers->PC++;
				break;
			case 0x7A:
				BIT_b_r(7, Register::RegisterName::D);
				_registers->PC++;
				break;
			case 0x7B:
				BIT_b_r(7, Register::RegisterName::E);
				_registers->PC++;
				break;
			case 0x7C:
				BIT_b_r(7, Register::RegisterName::H);
				_registers->PC++;
				break;
			case 0x7D:
				BIT_b_r(7, Register::RegisterName::L);
				_registers->PC++;
				break;
			case 0x7E:
				BIT_b_16(7);
				_registers->PC++;
				break;
			case 0x7F:
				BIT_b_r(7, Register::RegisterName::A);
				_registers->PC++;
				break;
			case 0x80:
				RES_b_r(0, Register::RegisterName::B);
				_registers->PC++;
				break;
			case 0x81:
				RES_b_r(0, Register::RegisterName::C);
				_registers->PC++;
				break;
			case 0x82:
				RES_b_r(0, Register::RegisterName::D);
				_registers->PC++;
				break;
			case 0x83:
				RES_b_r(0, Register::RegisterName::E);
				_registers->PC++;
				break;
			case 0x84:
				RES_b_r(0, Register::RegisterName::H);
				_registers->PC++;
				break;
			case 0x85:
				RES_b_r(0, Register::RegisterName::L);
				_registers->PC++;
				break;
			case 0x86:
				RES_b_16(0);
				_registers->PC++;
				break;
			case 0x87:
				RES_b_r(0, Register::RegisterName::A);
				_registers->PC++;
				break;
			case 0x88:
				RES_b_r(1, Register::RegisterName::B);
				_registers->PC++;
				break;
			case 0x89:
				RES_b_r(1, Register::RegisterName::C);
				_registers->PC++;
				break;
			case 0x8A:
				RES_b_r(1, Register::RegisterName::D);
				_registers->PC++;
				break;
			case 0x8B:
				RES_b_r(1, Register::RegisterName::E);
				_registers->PC++;
				break;
			case 0x8C:
				RES_b_r(1, Register::RegisterName::H);
				_registers->PC++;
				break;
			case 0x8D:
				RES_b_r(1, Register::RegisterName::L);
				_registers->PC++;
				break;
			case 0x8E:
				RES_b_16(1);
				_registers->PC++;
				break;
			case 0x8F:
				RES_b_r(1, Register::RegisterName::A);
				_registers->PC++;
				break;
			case 0x90:
				RES_b_r(2, Register::RegisterName::B);
				_registers->PC++;
				break;
			case 0x91:
				RES_b_r(2, Register::RegisterName::C);
				;
				_registers->PC++;
				break;
			case 0x92:
				RES_b_r(2, Register::RegisterName::D);
				_registers->PC++;
				break;
			case 0x93:
				RES_b_r(2, Register::RegisterName::E);
				_registers->PC++;
				break;
			case 0x94:
				RES_b_r(2, Register::RegisterName::H);
				_registers->PC++;
				break;
			case 0x95:
				RES_b_r(2, Register::RegisterName::L);
				_registers->PC++;
				break;
			case 0x96:
				RES_b_16(2);
				_registers->PC++;
				break;
			case 0x97:
				RES_b_r(2, Register::RegisterName::A);
				_registers->PC++;
				break;
			case 0x98:
				RES_b_r(3, Register::RegisterName::B);
				_registers->PC++;
				break;
			case 0x99:
				RES_b_r(3, Register::RegisterName::C);
				_registers->PC++;
				break;
			case 0x9A:
				RES_b_r(3, Register::RegisterName::D);
				_registers->PC++;
				break;
			case 0x9B:
				RES_b_r(3, Register::RegisterName::E);
				_registers->PC++;
				break;
			case 0x9C:
				RES_b_r(3, Register::RegisterName::H);
				_registers->PC++;
				break;
			case 0x9D:
				RES_b_r(3, Register::RegisterName::L);
				_registers->PC++;
				break;
			case 0x9E:
				RES_b_16(3);
				_registers->PC++;
				break;
			case 0x9F:
				RES_b_r(3, Register::RegisterName::A);
				_registers->PC++;
				break;
			case 0xA0:
				RES_b_r(4, Register::RegisterName::B);
				_registers->PC++;
				break;
			case 0xA1:
				RES_b_r(4, Register::RegisterName::C);
				_registers->PC++;
				break;
			case 0xA2:
				RES_b_r(4, Register::RegisterName::D);
				_registers->PC++;
				break;
			case 0xA3:
				RES_b_r(4, Register::RegisterName::E);
				_registers->PC++;
				break;
			case 0xA4:
				RES_b_r(4, Register::RegisterName::H);
				_registers->PC++;
				break;
			case 0xA5:
				RES_b_r(4, Register::RegisterName::L);
				_registers->PC++;
				break;
			case 0xA6:
				RES_b_16(4);
				_registers->PC++;
				break;
			case 0xA7:
				RES_b_r(4, Register::RegisterName::A);
				_registers->PC++;
				break;
			case 0xA8:
				RES_b_r(5, Register::RegisterName::B);
				_registers->PC++;
				break;
			case 0xA9:
				RES_b_r(5, Register::RegisterName::C);
				_registers->PC++;
				break;
			case 0xAA:
				RES_b_r(5, Register::RegisterName::D);
				_registers->PC++;
				break;
			case 0xAB:
				RES_b_r(5, Register::RegisterName::E);
				_registers->PC++;
				break;
			case 0xAC:
				RES_b_r(5, Register::RegisterName::H);
				_registers->PC++;
				break;
			case 0xAD:
				RES_b_r(5, Register::RegisterName::L);
				_registers->PC++;
				break;
			case 0xAE:
				RES_b_16(5);
				_registers->PC++;
				break;
			case 0xAF:
				RES_b_r(5, Register::RegisterName::A);
				_registers->PC++;
				break;
			case 0xB0:
				RES_b_r(6, Register::RegisterName::B);
				_registers->PC++;
				break;
			case 0xB1:
				RES_b_r(6, Register::RegisterName::C);
				_registers->PC++;
				break;
			case 0xB2:
				RES_b_r(6, Register::RegisterName::D);
				_registers->PC++;
				break;
			case 0xB3:
				RES_b_r(6, Register::RegisterName::E);
				_registers->PC++;
				break;
			case 0xB4:
				RES_b_r(6, Register::RegisterName::H);
				_registers->PC++;
				break;
			case 0xB5:
				RES_b_r(6, Register::RegisterName::L);
				_registers->PC++;
				break;
			case 0xB6:
				RES_b_16(6);
				_registers->PC++;
				break;
			case 0xB7:
				RES_b_r(6, Register::RegisterName::A);
				_registers->PC++;
				break;
			case 0xB8:
				RES_b_r(7, Register::RegisterName::B);
				_registers->PC++;
				break;
			case 0xB9:
				RES_b_r(7, Register::RegisterName::C);
				_registers->PC++;
				break;
			case 0xBA:
				RES_b_r(7, Register::RegisterName::D);
				_registers->PC++;
				break;
			case 0xBB:
				RES_b_r(7, Register::RegisterName::E);
				_registers->PC++;
				break;
			case 0xBC:
				RES_b_r(7, Register::RegisterName::H);
				_registers->PC++;
				break;
			case 0xBD:
				RES_b_r(7, Register::RegisterName::L);
				_registers->PC++;
				break;
			case 0xBE:
				RES_b_16(7);
				_registers->PC++;
				break;
			case 0xBF:
				RES_b_r(7, Register::RegisterName::A);
				_registers->PC++;
				break;
			case 0xC0:
				SET_b_r(0, Register::RegisterName::B);
				_registers->PC++;
				break;
			case 0xC1:
				SET_b_r(0, Register::RegisterName::C);
				_registers->PC++;
				break;
			case 0xC2:
				SET_b_r(0, Register::RegisterName::D);
				_registers->PC++;
				break;
			case 0xC3:
				SET_b_r(0, Register::RegisterName::E);
				_registers->PC++;
				break;
			case 0xC4:
				SET_b_r(0, Register::RegisterName::H);
				_registers->PC++;
				break;
			case 0xC5:
				SET_b_r(0, Register::RegisterName::L);
				_registers->PC++;
				break;
			case 0xC6:
				SET_b_16(0);
				_registers->PC++;
				break;
			case 0xC7:
				SET_b_r(0, Register::RegisterName::A);
				_registers->PC++;
				break;
			case 0xC8:
				SET_b_r(1, Register::RegisterName::B);
				_registers->PC++;
				break;
			case 0xC9:
				SET_b_r(1, Register::RegisterName::C);
				_registers->PC++;
				break;
			case 0xCA:
				SET_b_r(1, Register::RegisterName::D);
				_registers->PC++;
				break;
			case 0xCB:
				SET_b_r(1, Register::RegisterName::E);
				_registers->PC++;
				break;
			case 0xCC:
				SET_b_r(1, Register::RegisterName::H);
				_registers->PC++;
				break;
			case 0xCD:
				SET_b_r(1, Register::RegisterName::L);
				_registers->PC++;
				break;
			case 0xCE:
				SET_b_16(1);
				_registers->PC++;
				break;
			case 0xCF:
				SET_b_r(1, Register::RegisterName::A);
				_registers->PC++;
				break;
			case 0xD0:
				SET_b_r(2, Register::RegisterName::B);
				_registers->PC++;
				break;
			case 0xD1:
				SET_b_r(2, Register::RegisterName::C);
				_registers->PC++;
				break;
			case 0xD2:
				SET_b_r(2, Register::RegisterName::D);
				_registers->PC++;
				break;
			case 0xD3:
				SET_b_r(2, Register::RegisterName::E);
				_registers->PC++;
				break;
			case 0xD4:
				SET_b_r(2, Register::RegisterName::H);
				_registers->PC++;
				break;
			case 0xD5:
				SET_b_r(2, Register::RegisterName::L);
				_registers->PC++;
				break;
			case 0xD6:
				SET_b_16(2);
				_registers->PC++;
				break;
			case 0xD7:
				SET_b_r(2, Register::RegisterName::A);
				_registers->PC++;
				break;
			case 0xD8:
				SET_b_r(3, Register::RegisterName::B);
				_registers->PC++;
				break;
			case 0xD9:
				SET_b_r(3, Register::RegisterName::C);
				_registers->PC++;
				break;
			case 0xDA:
				SET_b_r(3, Register::RegisterName::D);
				_registers->PC++;
				break;
			case 0xDB:
				SET_b_r(3, Register::RegisterName::E);
				_registers->PC++;
				break;
			case 0xDC:
				SET_b_r(3, Register::RegisterName::H);
				_registers->PC++;
				break;
			case 0xDD:
				SET_b_r(3, Register::RegisterName::L);
				_registers->PC++;
				break;
			case 0xDE:
				SET_b_16(3);
				_registers->PC++;
				break;
			case 0xDF:
				SET_b_r(3, Register::RegisterName::A);
				_registers->PC++;
				break;
			case 0xE0:
				SET_b_r(4, Register::RegisterName::B);
				_registers->PC++;
				break;
			case 0xE1:
				SET_b_r(4, Register::RegisterName::C);
				_registers->PC++;
				break;
			case 0xE2:
				SET_b_r(4, Register::RegisterName::D);
				_registers->PC++;
				break;
			case 0xE3:
				SET_b_r(4, Register::RegisterName::E);
				_registers->PC++;
				break;
			case 0xE4:
				SET_b_r(4, Register::RegisterName::H);
				_registers->PC++;
				break;
			case 0xE5:
				SET_b_r(4, Register::RegisterName::L);
				_registers->PC++;
				break;
			case 0xE6:
				SET_b_16(4);
				_registers->PC++;
				break;
			case 0xE7:
				SET_b_r(4, Register::RegisterName::A);
				_registers->PC++;
				break;
			case 0xE8:
				SET_b_r(5, Register::RegisterName::B);
				_registers->PC++;
				break;
			case 0xE9:
				SET_b_r(5, Register::RegisterName::C);
				_registers->PC++;
				break;
			case 0xEA:
				SET_b_r(5, Register::RegisterName::D);
				_registers->PC++;
				break;
			case 0xEB:
				SET_b_r(5, Register::RegisterName::E);
				_registers->PC++;
				break;
			case 0xEC:
				SET_b_r(5, Register::RegisterName::H);
				_registers->PC++;
				break;
			case 0xED:
				SET_b_r(5, Register::RegisterName::L);
				_registers->PC++;
				break;
			case 0xEE:
				SET_b_16(5);
				_registers->PC++;
				break;
			case 0xEF:
				SET_b_r(5, Register::RegisterName::A);
				_registers->PC++;
				break;
			case 0xF0:
				SET_b_r(6, Register::RegisterName::B);
				_registers->PC++;
				break;
			case 0xF1:
				SET_b_r(6, Register::RegisterName::C);
				_registers->PC++;
				break;
			case 0xF2:
				SET_b_r(6, Register::RegisterName::D);
				_registers->PC++;
				break;
			case 0xF3:
				SET_b_r(6, Register::RegisterName::E);
				_registers->PC++;
				break;
			case 0xF4:
				SET_b_r(6, Register::RegisterName::H);
				_registers->PC++;
				break;
			case 0xF5:
				SET_b_r(6, Register::RegisterName::L);
				_registers->PC++;
				break;
			case 0xF6:
				SET_b_16(6);
				_registers->PC++;
				break;
			case 0xF7:
				SET_b_r(6, Register::RegisterName::A);
				_registers->PC++;
				break;
			case 0xF8:
				SET_b_r(7, Register::RegisterName::B);
				_registers->PC++;
				break;
			case 0xF9:
				SET_b_r(7, Register::RegisterName::C);
				_registers->PC++;
				break;
			case 0xFA:
				SET_b_r(7, Register::RegisterName::D);
				_registers->PC++;
				break;
			case 0xFB:
				SET_b_r(7, Register::RegisterName::E);
				_registers->PC++;
				break;
			case 0xFC:
				SET_b_r(7, Register::RegisterName::H);
				_registers->PC++;
				break;
			case 0xFD:
				SET_b_r(7, Register::RegisterName::L);
				_registers->PC++;
				break;
			case 0xFE:
				SET_b_16(7);
				_registers->PC++;
				break;
			case 0xFF:
				SET_b_r(7, Register::RegisterName::A);
				_registers->PC++;
				break;

			default:
			{
				cout << "Unsupported 0xCB Instruction : " << hex << (int)_mmu->readRam(_registers->PC);
				exit(0xCB);
				break;
			}
			}
			break;
		}
		default:
			cout << "Unsupported Instruction : " << hex << (int)_mmu->readRam(_registers->PC);
			exit(0);
			break;
		}
		_prevOpcode = _mmu->readRam(prevPC);
	}
}
