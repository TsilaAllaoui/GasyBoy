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

		bool _pcManuallySet = false;

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

		/**************************Opcode functions*****************************/
		void Opcode_00();
		void Opcode_01();
		void Opcode_02();
		void Opcode_03();
		void Opcode_04();
		void Opcode_05();
		void Opcode_06();
		void Opcode_07();
		void Opcode_08();
		void Opcode_09();
		void Opcode_0A();
		void Opcode_0B();
		void Opcode_0C();
		void Opcode_0D();
		void Opcode_0E();
		void Opcode_0F();

		void Opcode_10();
		void Opcode_11();
		void Opcode_12();
		void Opcode_13();
		void Opcode_14();
		void Opcode_15();
		void Opcode_16();
		void Opcode_17();
		void Opcode_18();
		void Opcode_19();
		void Opcode_1A();
		void Opcode_1B();
		void Opcode_1C();
		void Opcode_1D();
		void Opcode_1E();
		void Opcode_1F();

		void Opcode_20();
		void Opcode_21();
		void Opcode_22();
		void Opcode_23();
		void Opcode_24();
		void Opcode_25();
		void Opcode_26();
		void Opcode_27();
		void Opcode_28();
		void Opcode_29();
		void Opcode_2A();
		void Opcode_2B();
		void Opcode_2C();
		void Opcode_2D();
		void Opcode_2E();
		void Opcode_2F();

		void Opcode_30();
		void Opcode_31();
		void Opcode_32();
		void Opcode_33();
		void Opcode_34();
		void Opcode_35();
		void Opcode_36();
		void Opcode_37();
		void Opcode_38();
		void Opcode_39();
		void Opcode_3A();
		void Opcode_3B();
		void Opcode_3C();
		void Opcode_3D();
		void Opcode_3E();
		void Opcode_3F();

		void Opcode_40();
		void Opcode_41();
		void Opcode_42();
		void Opcode_43();
		void Opcode_44();
		void Opcode_45();
		void Opcode_46();
		void Opcode_47();
		void Opcode_48();
		void Opcode_49();
		void Opcode_4A();
		void Opcode_4B();
		void Opcode_4C();
		void Opcode_4D();
		void Opcode_4E();
		void Opcode_4F();

		void Opcode_50();
		void Opcode_51();
		void Opcode_52();
		void Opcode_53();
		void Opcode_54();
		void Opcode_55();
		void Opcode_56();
		void Opcode_57();
		void Opcode_58();
		void Opcode_59();
		void Opcode_5A();
		void Opcode_5B();
		void Opcode_5C();
		void Opcode_5D();
		void Opcode_5E();
		void Opcode_5F();

		void Opcode_60();
		void Opcode_61();
		void Opcode_62();
		void Opcode_63();
		void Opcode_64();
		void Opcode_65();
		void Opcode_66();
		void Opcode_67();
		void Opcode_68();
		void Opcode_69();
		void Opcode_6A();
		void Opcode_6B();
		void Opcode_6C();
		void Opcode_6D();
		void Opcode_6E();
		void Opcode_6F();

		void Opcode_70();
		void Opcode_71();
		void Opcode_72();
		void Opcode_73();
		void Opcode_74();
		void Opcode_75();
		void Opcode_76();
		void Opcode_77();
		void Opcode_78();
		void Opcode_79();
		void Opcode_7A();
		void Opcode_7B();
		void Opcode_7C();
		void Opcode_7D();
		void Opcode_7E();
		void Opcode_7F();

		void Opcode_80();
		void Opcode_81();
		void Opcode_82();
		void Opcode_83();
		void Opcode_84();
		void Opcode_85();
		void Opcode_86();
		void Opcode_87();
		void Opcode_88();
		void Opcode_89();
		void Opcode_8A();
		void Opcode_8B();
		void Opcode_8C();
		void Opcode_8D();
		void Opcode_8E();
		void Opcode_8F();

		void Opcode_90();
		void Opcode_91();
		void Opcode_92();
		void Opcode_93();
		void Opcode_94();
		void Opcode_95();
		void Opcode_96();
		void Opcode_97();
		void Opcode_98();
		void Opcode_99();
		void Opcode_9A();
		void Opcode_9B();
		void Opcode_9C();
		void Opcode_9D();
		void Opcode_9E();
		void Opcode_9F();

		void Opcode_A0();
		void Opcode_A1();
		void Opcode_A2();
		void Opcode_A3();
		void Opcode_A4();
		void Opcode_A5();
		void Opcode_A6();
		void Opcode_A7();
		void Opcode_A8();
		void Opcode_A9();
		void Opcode_AA();
		void Opcode_AB();
		void Opcode_AC();
		void Opcode_AD();
		void Opcode_AE();
		void Opcode_AF();

		void Opcode_B0();
		void Opcode_B1();
		void Opcode_B2();
		void Opcode_B3();
		void Opcode_B4();
		void Opcode_B5();
		void Opcode_B6();
		void Opcode_B7();
		void Opcode_B8();
		void Opcode_B9();
		void Opcode_BA();
		void Opcode_BB();
		void Opcode_BC();
		void Opcode_BD();
		void Opcode_BE();
		void Opcode_BF();

		void Opcode_C0();
		void Opcode_C1();
		void Opcode_C2();
		void Opcode_C3();
		void Opcode_C4();
		void Opcode_C5();
		void Opcode_C6();
		void Opcode_C7();
		void Opcode_C8();
		void Opcode_C9();
		void Opcode_CA();
		void Opcode_CB();
		void Opcode_CC();
		void Opcode_CD();
		void Opcode_CE();
		void Opcode_CF();

		void Opcode_D0();
		void Opcode_D1();
		void Opcode_D2();
		void Opcode_D3();
		void Opcode_D4();
		void Opcode_D5();
		void Opcode_D6();
		void Opcode_D7();
		void Opcode_D8();
		void Opcode_D9();
		void Opcode_DA();
		void Opcode_DB();
		void Opcode_DC();
		void Opcode_DD();
		void Opcode_DE();
		void Opcode_DF();

		void Opcode_E0();
		void Opcode_E1();
		void Opcode_E2();
		void Opcode_E3();
		void Opcode_E4();
		void Opcode_E5();
		void Opcode_E6();
		void Opcode_E7();
		void Opcode_E8();
		void Opcode_E9();
		void Opcode_EA();
		void Opcode_EB();
		void Opcode_EC();
		void Opcode_ED();
		void Opcode_EE();
		void Opcode_EF();

		void Opcode_F0();
		void Opcode_F1();
		void Opcode_F2();
		void Opcode_F3();
		void Opcode_F4();
		void Opcode_F5();
		void Opcode_F6();
		void Opcode_F7();
		void Opcode_F8();
		void Opcode_F9();
		void Opcode_FA();
		void Opcode_FB();
		void Opcode_FC();
		void Opcode_FD();
		void Opcode_FE();
		void Opcode_FF();

		/**************************Opcode CB functions*****************************/
		void Opcode_CB_00();
		void Opcode_CB_01();
		void Opcode_CB_02();
		void Opcode_CB_03();
		void Opcode_CB_04();
		void Opcode_CB_05();
		void Opcode_CB_06();
		void Opcode_CB_07();
		void Opcode_CB_08();
		void Opcode_CB_09();
		void Opcode_CB_0A();
		void Opcode_CB_0B();
		void Opcode_CB_0C();
		void Opcode_CB_0D();
		void Opcode_CB_0E();
		void Opcode_CB_0F();

		void Opcode_CB_10();
		void Opcode_CB_11();
		void Opcode_CB_12();
		void Opcode_CB_13();
		void Opcode_CB_14();
		void Opcode_CB_15();
		void Opcode_CB_16();
		void Opcode_CB_17();
		void Opcode_CB_18();
		void Opcode_CB_19();
		void Opcode_CB_1A();
		void Opcode_CB_1B();
		void Opcode_CB_1C();
		void Opcode_CB_1D();
		void Opcode_CB_1E();
		void Opcode_CB_1F();

		void Opcode_CB_20();
		void Opcode_CB_21();
		void Opcode_CB_22();
		void Opcode_CB_23();
		void Opcode_CB_24();
		void Opcode_CB_25();
		void Opcode_CB_26();
		void Opcode_CB_27();
		void Opcode_CB_28();
		void Opcode_CB_29();
		void Opcode_CB_2A();
		void Opcode_CB_2B();
		void Opcode_CB_2C();
		void Opcode_CB_2D();
		void Opcode_CB_2E();
		void Opcode_CB_2F();

		void Opcode_CB_30();
		void Opcode_CB_31();
		void Opcode_CB_32();
		void Opcode_CB_33();
		void Opcode_CB_34();
		void Opcode_CB_35();
		void Opcode_CB_36();
		void Opcode_CB_37();
		void Opcode_CB_38();
		void Opcode_CB_39();
		void Opcode_CB_3A();
		void Opcode_CB_3B();
		void Opcode_CB_3C();
		void Opcode_CB_3D();
		void Opcode_CB_3E();
		void Opcode_CB_3F();

		void Opcode_CB_40();
		void Opcode_CB_41();
		void Opcode_CB_42();
		void Opcode_CB_43();
		void Opcode_CB_44();
		void Opcode_CB_45();
		void Opcode_CB_46();
		void Opcode_CB_47();
		void Opcode_CB_48();
		void Opcode_CB_49();
		void Opcode_CB_4A();
		void Opcode_CB_4B();
		void Opcode_CB_4C();
		void Opcode_CB_4D();
		void Opcode_CB_4E();
		void Opcode_CB_4F();

		void Opcode_CB_50();
		void Opcode_CB_51();
		void Opcode_CB_52();
		void Opcode_CB_53();
		void Opcode_CB_54();
		void Opcode_CB_55();
		void Opcode_CB_56();
		void Opcode_CB_57();
		void Opcode_CB_58();
		void Opcode_CB_59();
		void Opcode_CB_5A();
		void Opcode_CB_5B();
		void Opcode_CB_5C();
		void Opcode_CB_5D();
		void Opcode_CB_5E();
		void Opcode_CB_5F();

		void Opcode_CB_60();
		void Opcode_CB_61();
		void Opcode_CB_62();
		void Opcode_CB_63();
		void Opcode_CB_64();
		void Opcode_CB_65();
		void Opcode_CB_66();
		void Opcode_CB_67();
		void Opcode_CB_68();
		void Opcode_CB_69();
		void Opcode_CB_6A();
		void Opcode_CB_6B();
		void Opcode_CB_6C();
		void Opcode_CB_6D();
		void Opcode_CB_6E();
		void Opcode_CB_6F();

		void Opcode_CB_70();
		void Opcode_CB_71();
		void Opcode_CB_72();
		void Opcode_CB_73();
		void Opcode_CB_74();
		void Opcode_CB_75();
		void Opcode_CB_76();
		void Opcode_CB_77();
		void Opcode_CB_78();
		void Opcode_CB_79();
		void Opcode_CB_7A();
		void Opcode_CB_7B();
		void Opcode_CB_7C();
		void Opcode_CB_7D();
		void Opcode_CB_7E();
		void Opcode_CB_7F();

		void Opcode_CB_80();
		void Opcode_CB_81();
		void Opcode_CB_82();
		void Opcode_CB_83();
		void Opcode_CB_84();
		void Opcode_CB_85();
		void Opcode_CB_86();
		void Opcode_CB_87();
		void Opcode_CB_88();
		void Opcode_CB_89();
		void Opcode_CB_8A();
		void Opcode_CB_8B();
		void Opcode_CB_8C();
		void Opcode_CB_8D();
		void Opcode_CB_8E();
		void Opcode_CB_8F();

		void Opcode_CB_90();
		void Opcode_CB_91();
		void Opcode_CB_92();
		void Opcode_CB_93();
		void Opcode_CB_94();
		void Opcode_CB_95();
		void Opcode_CB_96();
		void Opcode_CB_97();
		void Opcode_CB_98();
		void Opcode_CB_99();
		void Opcode_CB_9A();
		void Opcode_CB_9B();
		void Opcode_CB_9C();
		void Opcode_CB_9D();
		void Opcode_CB_9E();
		void Opcode_CB_9F();

		void Opcode_CB_A0();
		void Opcode_CB_A1();
		void Opcode_CB_A2();
		void Opcode_CB_A3();
		void Opcode_CB_A4();
		void Opcode_CB_A5();
		void Opcode_CB_A6();
		void Opcode_CB_A7();
		void Opcode_CB_A8();
		void Opcode_CB_A9();
		void Opcode_CB_AA();
		void Opcode_CB_AB();
		void Opcode_CB_AC();
		void Opcode_CB_AD();
		void Opcode_CB_AE();
		void Opcode_CB_AF();

		void Opcode_CB_B0();
		void Opcode_CB_B1();
		void Opcode_CB_B2();
		void Opcode_CB_B3();
		void Opcode_CB_B4();
		void Opcode_CB_B5();
		void Opcode_CB_B6();
		void Opcode_CB_B7();
		void Opcode_CB_B8();
		void Opcode_CB_B9();
		void Opcode_CB_BA();
		void Opcode_CB_BB();
		void Opcode_CB_BC();
		void Opcode_CB_BD();
		void Opcode_CB_BE();
		void Opcode_CB_BF();

		void Opcode_CB_C0();
		void Opcode_CB_C1();
		void Opcode_CB_C2();
		void Opcode_CB_C3();
		void Opcode_CB_C4();
		void Opcode_CB_C5();
		void Opcode_CB_C6();
		void Opcode_CB_C7();
		void Opcode_CB_C8();
		void Opcode_CB_C9();
		void Opcode_CB_CA();
		void Opcode_CB_CB();
		void Opcode_CB_CC();
		void Opcode_CB_CD();
		void Opcode_CB_CE();
		void Opcode_CB_CF();

		void Opcode_CB_D0();
		void Opcode_CB_D1();
		void Opcode_CB_D2();
		void Opcode_CB_D3();
		void Opcode_CB_D4();
		void Opcode_CB_D5();
		void Opcode_CB_D6();
		void Opcode_CB_D7();
		void Opcode_CB_D8();
		void Opcode_CB_D9();
		void Opcode_CB_DA();
		void Opcode_CB_DB();
		void Opcode_CB_DC();
		void Opcode_CB_DD();
		void Opcode_CB_DE();
		void Opcode_CB_DF();

		void Opcode_CB_E0();
		void Opcode_CB_E1();
		void Opcode_CB_E2();
		void Opcode_CB_E3();
		void Opcode_CB_E4();
		void Opcode_CB_E5();
		void Opcode_CB_E6();
		void Opcode_CB_E7();
		void Opcode_CB_E8();
		void Opcode_CB_E9();
		void Opcode_CB_EA();
		void Opcode_CB_EB();
		void Opcode_CB_EC();
		void Opcode_CB_ED();
		void Opcode_CB_EE();
		void Opcode_CB_EF();

		void Opcode_CB_F0();
		void Opcode_CB_F1();
		void Opcode_CB_F2();
		void Opcode_CB_F3();
		void Opcode_CB_F4();
		void Opcode_CB_F5();
		void Opcode_CB_F6();
		void Opcode_CB_F7();
		void Opcode_CB_F8();
		void Opcode_CB_F9();
		void Opcode_CB_FA();
		void Opcode_CB_FB();
		void Opcode_CB_FC();
		void Opcode_CB_FD();
		void Opcode_CB_FE();
		void Opcode_CB_FF();
	};
}

#endif