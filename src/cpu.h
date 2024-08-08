#ifndef _CPU_H_
#define _CPU_H_

#include "registers.h"
#include "register.h"
#include "mmu.h"
#include <memory>

// class of the gameboy CPU (nearly the same as the z80)

namespace gasyboy
{
	class Cpu
	{
	private:
		// Registers
		Registers &_registers;

		// MMU
		Mmu &_mmu;

		// the current opcode pointed by the PC
		uint8_t _currentOpcode;
		uint8_t _prevOpcode;

		// the cycle counter of the cpu
		long _cycle;

	public:
		// contructor/destructor
		Cpu(const bool &bootBios, Mmu &mmu, Registers &registers);
		~Cpu() = default;

		// a step of the cpu
		long step();

		// execute the next opcode pointed by the pc
		void execute();

		// fetch the current opcode
		void fetch();

		// getters
		uint16_t getRegister(const std::string &reg);
		uint8_t getRegister(const char &reg);

		/***********useful function**********/
		uint16_t next2bytes(const uint16_t &adress);

		/***************Carry/HalCarry*********************/
		bool checkAddHalfCarry(const uint8_t &a, const uint8_t &b, const uint8_t &c);
		bool checkAddHalfCarry(const uint16_t &a, const uint16_t &b, const uint16_t &c);
		bool checkAddHalfCarry(const uint16_t &a, const uint16_t &b);
		bool checkSubHalfCarry(const uint8_t &a, const uint8_t &b);
		bool checkSubHalfCarry(const uint16_t &a, const uint16_t &b);
		bool checkSubCarry(const uint16_t &a, const uint16_t &b);
		bool checkAddCarry(const uint8_t &a, const uint8_t &b, const uint8_t &c);
		bool checkAddCarry(const uint16_t &a, const uint16_t &b, const uint16_t &c);

		/*******8bits load group instructions*******/

		// load a register value to another regsiter
		void LD_r_r(const char &from, const char &to);

		// load a 8bit integer in a register
		void LD_r_n(const uint8_t &from, const char &to);

		// load memory value at a 16bit adress to a register
		void LD_r_16(const uint16_t &adress, const char &to);

		// the content of the register is loaded at the memory location of a 16bit adress
		void LD_16_r(const uint16_t &adress, const char &from);

		// an integer is loaded at the memory location of a 16bit adress
		void LD_16_n(const uint16_t &adress, const uint8_t &value);

		/*******16bits load group instructions*******/

		// a 16bits integer is loaded in a 16bits register pair
		void LD_rr_nn(const uint16_t &value, const std::string &reg);

		// load the content of the nex two bytes in memory starting at an adress to the appropriate 16bits register
		void LD_rr_16(const uint16_t &adress, const std::string &reg);

		// load the content of a 16bit register at memory adress
		void LD_16_rr(const uint16_t &adress, const std::string &reg);

		// load in SP the content of HL
		void LD_SP_HL();

		// load HL with the sum of SP && the next immediate byte
		void LD_HL_SP_n();

		// push a 16bit register content to SP
		void PUSH(const std::string &reg);

		// pop 2bytes from the adress where the SP point to && load it in a 16bit register
		void POP(const std::string &reg);

		/********8bit Arithmetic group instructions*********/

		// add A && an 8bit register && store the result in A
		void ADD_A_r(const char &reg);

		// add A && an 8bit integer && store the result in A
		void ADD_A_n(const uint8_t &value);

		// add A && an 8bit integer at an adress memory && store the result in A
		void ADD_A_16();

		// add the content of the operand && the carry flag with A && store it in A
		void ADC_A_r(const char &reg);
		void ADC_A_n(const uint8_t &value);
		void ADC_A_16();

		// substract the content of A && the operand && store it in A
		void SUB_r(const char &reg);
		void SUB_n(const uint8_t &value);
		void SUB_16();

		// substract the content of A && the operand along with carry flag && store it in A
		void SBC_r(const char &reg);
		void SBC_n(const uint8_t &value);
		void SBC_16();

		// AND operation on A && the operand
		void AND_r(const char &reg);
		void AND_n(const uint8_t &value);
		void AND_16();

		// OR operation on A && the operand
		void OR_r(const char &reg);
		void OR_n(const uint8_t &value);
		void OR_16();

		// XOR operation on A && the operand
		void XOR_r(const char &reg);
		void XOR_n(const uint8_t &value);
		void XOR_16();

		// compare the Accumulator && the operand
		void CP_r(const char &reg);
		void CP_n(const uint8_t &reg);
		void CP_16();

		// increment a 8bit register
		void INC_r(const char &reg);

		// increment the value at memory pointed by HL
		void INC_16();

		// decrement a 8bit register
		void DEC_r(const char &reg);

		// decrement the value at memory pointed by HL
		void DEC_16();

		/********General Purpose Arithmetic && CPU Control group instructions*********/

		// Decimal Adjust
		void DAA();

		// the accumulator is inverted (complemented)
		void CPL();

		// the carry flag is inverted
		void CCF();

		// set carry flag
		void SCF();

		// no operation
		void NOP();

		// halting the cpu
		void HALT();

		// disable maskable interrupt of the cpu
		void DI();

		// enable maskable interrupt of the cpu
		void EI();

		/********16bit Arithmetic group instructions*********/

		// add HL && another 16bits register
		void ADD_HL_rr(const std::string &reg);

		void ADD_SP_n();

		// add HL && the carryflag with another 16bits register
		void ADC_HL_rr(const std::string &reg);

		// substract HL with the carryflag && another 16bits register
		void SBC_HL_rr(const std::string &reg);

		// increment a 16bit register
		void INC_rr(const std::string &reg);

		// decrement a 168bit register
		void DEC_rr(const std::string &reg);

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
		void RLC_r(const char &reg);

		// rotate left a 8bit register
		void RL_r(const char &reg);

		// rotate left value pointed by HL
		void RL_16();

		// Rotate left a 8bit value pointed by HL with carry
		void RLC_16();

		// rotate rigth a 8bit register with carry
		void RRC_r(const char &reg);

		// rotate rigth a 8bit register
		void RR_r(const char &reg);

		// rotate right value pointed by HL
		void RR_16();

		// Rotate rigth a 8bit value pointed by HL with carry
		void RRC_16();

		// shift left a 8bit register
		void SLA_r(const char &reg);

		// shift left a value at HL
		void SLA_16();

		// shift right a 8bit register without changing bit7
		void SRA_r(const char &reg);

		// shift right a value at HL without changing bit7
		void SRA_16();

		// shift right a 8bit register && reset bit7
		void SRL_r(const char &reg);

		// shift right a value pointed by HL && reset bit7
		void SRL_16();

		// swap upper with lower nibbles
		void SWAP_r(const char &reg);
		void SWAP_16();

		/***********Bit Set/Reset/Test group instructions********************/

		// check if bit n is set in a 8bit register && set flags accordingly
		void BIT_b_r(const int &bit, const char &reg);

		// check if bit n is set in a 8bit value pointed by HL register && set flags accordingly
		void BIT_b_16(const int &bit);

		// set bit n in 8bit register
		void SET_b_r(const int &b, const char &reg);

		// set bit n in 8bit value pointed by HL register
		void SET_b_16(const int &b);

		// reset bit n in a 8bit register
		void RES_b_r(const int &b, const char &reg);

		// reset bit n in memory pointed by HL
		void RES_b_16(const int &b);

		/**********************Jump group instructions**************/

		// jump to adress
		void JP_16(const uint16_t &adress);

		// jump with conditions
		void JP_c_16(const std::string &condition, const uint16_t &adress);

		// jump relative
		void JR_e(const uint8_t &value);

		// jump relative with set C flag condition
		void JR_C_e(const uint8_t &value);

		// jump relative with not set C flag condition
		void JR_NC_e(const uint8_t &value);

		// jump relative with set Z flag condition
		void JR_Z_e(const uint8_t &value);

		// jump relative with not set Z flag condition
		void JR_NZ_e(const uint8_t &value);

		// jump to HL
		void JP_16();

		/******************Call/Return Group intruction*********************/

		// subroutine jump
		void CALL();

		// subroutine jump with condition
		void CALL_c(const std::string &condition);

		// return from subroutine
		void RET();

		// return from subroutine with condition
		void RET_c(const std::string &condition);

		// return fomr interrupt
		void RETI();

		// RST Routines
		void RST_p(const uint16_t &p);
	};
}

#endif