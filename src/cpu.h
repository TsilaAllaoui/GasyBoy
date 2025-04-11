#ifndef _CPU_H_
#define _CPU_H_

#include "interruptManager.h"
#include "registers.h"
#include "register.h"
#include "mmu.h"
#include <memory>

// Class of the gameboy CPU (nearly the same as the z80)

namespace gasyboy
{
	class Cpu
	{
	private:
		// Registers
		std::shared_ptr<Registers> _registers;

		// MMU
		std::shared_ptr<Mmu> _mmu;

		// Interrupt Manager
		std::shared_ptr<InterruptManager> _interruptManager;

		// The current opcode pointed by the PC
		uint8_t _currentOpcode;
		uint8_t _prevOpcode;

		// The cycle counter of the cpu
		long _cycle;

		// If HALT bug is occuring
		bool _haltBug;

	public:
		// Contructor/destructor
		Cpu();
		Cpu &operator=(const Cpu &);
		~Cpu() = default;

		// Reset the cpu
		void reset();

		// A step of the cpu
		long step();

		// Execute the next opcode pointed by the pc
		void execute();

		// Fetch the current opcode
		void fetch();

		enum class State
		{
			PAUSED,
			RUNNING,
			STOPPED,
			STEPPING
		};
		static State state;

		// Getters
		uint16_t getRegister(const Register::RegisterPairName &reg);
		uint8_t getRegister(const Register::RegisterName &reg);

		/***********useful function**********/
		uint16_t next2bytes(const uint16_t &adress);

		/***************Carry/HalCarry*********************/
		bool checkAddHalfCarry(const uint8_t &a, const uint8_t &b);
		bool checkAddHalfCarry(const uint16_t &a, const uint16_t &b);
		bool checkSubHalfCarry(const uint8_t &a, const uint8_t &b);
		bool checkSubHalfCarry(const uint16_t &a, const uint16_t &b);
		bool checkAddCarry(const uint8_t &a, const uint8_t &b);
		bool checkAddCarry(const uint16_t &a, const uint16_t &b);
		bool checkSubCarry(const uint16_t &a, const uint16_t &b);

		/*******8bits load group instructions*******/

		// Load a register value to another regsiter
		void LD_r_r(const Register::RegisterName &from, const Register::RegisterName &to);

		// Load a 8bit integer in a register
		void LD_r_n(const uint8_t &from, const Register::RegisterName &to);

		// Load memory value at a 16bit adress to a register
		void LD_r_16(const uint16_t &adress, const Register::RegisterName &to);

		// The content of the register is loaded at the memory location of a 16bit adress
		void LD_16_r(const uint16_t &adress, const Register::RegisterName &from);

		// An integer is loaded at the memory location of a 16bit adress
		void LD_16_n(const uint16_t &adress, const uint8_t &value);

		/*******16bits load group instructions*******/

		// A 16bits integer is loaded in a 16bits register pair
		void LD_rr_nn(const uint16_t &value, const Register::RegisterPairName &reg);

		// Load the content of the nex two bytes in memory starting at an adress to the appropriate 16bits register
		void LD_rr_16(const uint16_t &adress, const Register::RegisterPairName &reg);

		// Load the content of a 16bit register at memory adress
		void LD_16_rr(const uint16_t &adress, const Register::RegisterPairName &reg);

		// Load in SP the content of HL
		void LD_SP_HL();

		// Load HL with the sum of SP && the next immediate byte
		void LD_HL_SP_n();

		// push a 16bit register content to SP
		void PUSH(const Register::RegisterPairName &reg);

		// pop 2bytes from the adress where the SP point to && load it in a 16bit register
		void POP(const Register::RegisterPairName &reg);

		/********8bit Arithmetic group instructions*********/

		// Add A && an 8bit register && store the result in A
		void ADD_A_r(const Register::RegisterName &reg);

		// Add A && an 8bit integer && store the result in A
		void ADD_A_n(const uint8_t &value);

		// Add A && an 8bit integer at an adress memory && store the result in A
		void ADD_A_16();

		// Add the content of the operand && the carry flag with A && store it in A
		void ADC_A_r(const Register::RegisterName &reg);
		void ADC_A_n(const uint8_t &value);
		void ADC_A_16();

		// Substract the content of A && the operand && store it in A
		void SUB_r(const Register::RegisterName &reg);
		void SUB_n(const uint8_t &value);
		void SUB_16();

		// Substract the content of A && the operand along with carry flag && store it in A
		void SBC_r(const Register::RegisterName &reg);
		void SBC_n(const uint8_t &value);
		void SBC_16();

		// AND operation on A && the operand
		void AND_r(const Register::RegisterName &reg);
		void AND_n(const uint8_t &value);
		void AND_16();

		// OR operation on A && the operand
		void OR_r(const Register::RegisterName &reg);
		void OR_n(const uint8_t &value);
		void OR_16();

		// XOR operation on A && the operand
		void XOR_r(const Register::RegisterName &reg);
		void XOR_n(const uint8_t &value);
		void XOR_16();

		// Compare the Accumulator && the operand
		void CP_r(const Register::RegisterName &reg);
		void CP_n(const uint8_t &reg);
		void CP_16();

		// Increment a 8bit register
		void INC_r(const Register::RegisterName &reg);

		// Increment the value at memory pointed by HL
		void INC_16();

		// Decrement a 8bit register
		void DEC_r(const Register::RegisterName &reg);

		// Decrement the value at memory pointed by HL
		void DEC_16();

		/********General Purpose Arithmetic && CPU Control group instructions*********/

		// Decimal Adjust
		void DAA();

		// The accumulator is inverted (complemented)
		void CPL();

		// The carry flag is inverted
		void CCF();

		// Set carry flag
		void SCF();

		// No operation
		void NOP();

		// halting the cpu
		void HALT();

		// Disable maskable interrupt of the cpu
		void DI();

		// Enable maskable interrupt of the cpu
		void EI();

		/********16bit Arithmetic group instructions*********/

		// Add HL && another 16bits register
		void ADD_HL_rr(const Register::RegisterPairName &reg);

		void ADD_SP_n();

		// Add HL && the carryflag with another 16bits register
		void ADC_HL_rr(const Register::RegisterPairName &reg);

		// Substract HL with the carryflag && another 16bits register
		void SBC_HL_rr(const Register::RegisterPairName &reg);

		// Increment a 16bit register
		void INC_rr(const Register::RegisterPairName &reg);

		// Decrement a 168bit register
		void DEC_rr(const Register::RegisterPairName &reg);

		/********Rotate && Shift group instructions*********/

		// Rotate left with carry A
		void RLCA();

		// Rotate left A
		void RLA();

		// Rotate rigth with carry A
		void RRCA();

		// Rotate right A
		void RRA();

		// Rotate left a 8bit register with carry
		void RLC_r(const Register::RegisterName &reg);

		// Rotate left a 8bit register
		void RL_r(const Register::RegisterName &reg);

		// Rotate left value pointed by HL
		void RL_16();

		// Rotate left a 8bit value pointed by HL with carry
		void RLC_16();

		// Rotate rigth a 8bit register with carry
		void RRC_r(const Register::RegisterName &reg);

		// Rotate rigth a 8bit register
		void RR_r(const Register::RegisterName &reg);

		// Rotate right value pointed by HL
		void RR_16();

		// Rotate rigth a 8bit value pointed by HL with carry
		void RRC_16();

		// Shift left a 8bit register
		void SLA_r(const Register::RegisterName &reg);

		// Shift left a value at HL
		void SLA_16();

		// Shift right a 8bit register without changing bit7
		void SRA_r(const Register::RegisterName &reg);

		// Shift right a value at HL without changing bit7
		void SRA_16();

		// Shift right a 8bit register && reset bit7
		void SRL_r(const Register::RegisterName &reg);

		// Shift right a value pointed by HL && reset bit7
		void SRL_16();

		// Swap upper with lower nibbles
		void SWAP_r(const Register::RegisterName &reg);
		void SWAP_16();

		/***********Bit Set/Reset/Test group instructions********************/

		// Check if bit n is set in a 8bit register && set flags accordingly
		void BIT_b_r(const int &bit, const Register::RegisterName &reg);

		// Check if bit n is set in a 8bit value pointed by HL register && set flags accordingly
		void BIT_b_16(const int &bit);

		// Set bit n in 8bit register
		void SET_b_r(const int &b, const Register::RegisterName &reg);

		// Set bit n in 8bit value pointed by HL register
		void SET_b_16(const int &b);

		// Reset bit n in a 8bit register
		void RES_b_r(const int &b, const Register::RegisterName &reg);

		// Reset bit n in memory pointed by HL
		void RES_b_16(const int &b);

		/**********************Jump group instructions**************/

		// Jump to adress
		void JP_16(const uint16_t &adress);

		// Jump with conditions
		void JP_c_16(const Register::FlagName &condition, const uint16_t &adress);

		// Jump relative
		void JR_e(const uint8_t &value);

		// Jump relative with set C flag condition
		void JR_C_e(const uint8_t &value);

		// Jump relative with not set C flag condition
		void JR_NC_e(const uint8_t &value);

		// Jump relative with set Z flag condition
		void JR_Z_e(const uint8_t &value);

		// Jump relative with not set Z flag condition
		void JR_NZ_e(const uint8_t &value);

		// Jump to HL
		void JP_16();

		/******************Call/Return Group intruction*********************/

		// Subroutine jump
		void CALL();

		// Subroutine jump with condition
		void CALL_c(const Register::FlagName &condition);

		// Return from subroutine
		void RET();

		// Return from subroutine with condition
		void RET_c(const Register::FlagName &condition);

		// Return fomr interrupt
		void RETI();

		// RST Routines
		void RST_p(const uint16_t &p);
	};
}

#endif