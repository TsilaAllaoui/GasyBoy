#include "cpu.h"

namespace gasyboy
{ //-------------------------------------------------------------------
    // CB-Prefix Opcodes: Rotate and Shift Instructions
    //-------------------------------------------------------------------

    // 0x00: RLC B
    void Cpu::Opcode_CB_00() { RLC_r(Register::RegisterName::B); }
    // 0x01: RLC C
    void Cpu::Opcode_CB_01() { RLC_r(Register::RegisterName::C); }
    // 0x02: RLC D
    void Cpu::Opcode_CB_02() { RLC_r(Register::RegisterName::D); }
    // 0x03: RLC E
    void Cpu::Opcode_CB_03() { RLC_r(Register::RegisterName::E); }
    // 0x04: RLC H
    void Cpu::Opcode_CB_04() { RLC_r(Register::RegisterName::H); }
    // 0x05: RLC L
    void Cpu::Opcode_CB_05() { RLC_r(Register::RegisterName::L); }
    // 0x06: RLC (HL)
    void Cpu::Opcode_CB_06() { RLC_16(); }
    // 0x07: RLC A
    void Cpu::Opcode_CB_07() { RLC_r(Register::RegisterName::A); }

    // 0x08: RRC B
    void Cpu::Opcode_CB_08() { RRC_r(Register::RegisterName::B); }
    // 0x09: RRC C
    void Cpu::Opcode_CB_09() { RRC_r(Register::RegisterName::C); }
    // 0x0A: RRC D
    void Cpu::Opcode_CB_0A() { RRC_r(Register::RegisterName::D); }
    // 0x0B: RRC E
    void Cpu::Opcode_CB_0B() { RRC_r(Register::RegisterName::E); }
    // 0x0C: RRC H
    void Cpu::Opcode_CB_0C() { RRC_r(Register::RegisterName::H); }
    // 0x0D: RRC L
    void Cpu::Opcode_CB_0D() { RRC_r(Register::RegisterName::L); }
    // 0x0E: RRC (HL)
    void Cpu::Opcode_CB_0E() { RRC_16(); }
    // 0x0F: RRC A
    void Cpu::Opcode_CB_0F() { RRC_r(Register::RegisterName::A); }

    // 0x10: RL B
    void Cpu::Opcode_CB_10() { RL_r(Register::RegisterName::B); }
    // 0x11: RL C
    void Cpu::Opcode_CB_11() { RL_r(Register::RegisterName::C); }
    // 0x12: RL D
    void Cpu::Opcode_CB_12() { RL_r(Register::RegisterName::D); }
    // 0x13: RL E
    void Cpu::Opcode_CB_13() { RL_r(Register::RegisterName::E); }
    // 0x14: RL H
    void Cpu::Opcode_CB_14() { RL_r(Register::RegisterName::H); }
    // 0x15: RL L
    void Cpu::Opcode_CB_15() { RL_r(Register::RegisterName::L); }
    // 0x16: RL (HL)
    void Cpu::Opcode_CB_16() { RL_16(); }
    // 0x17: RL A
    void Cpu::Opcode_CB_17() { RL_r(Register::RegisterName::A); }

    // 0x18: RR B
    void Cpu::Opcode_CB_18() { RR_r(Register::RegisterName::B); }
    // 0x19: RR C
    void Cpu::Opcode_CB_19() { RR_r(Register::RegisterName::C); }
    // 0x1A: RR D
    void Cpu::Opcode_CB_1A() { RR_r(Register::RegisterName::D); }
    // 0x1B: RR E
    void Cpu::Opcode_CB_1B() { RR_r(Register::RegisterName::E); }
    // 0x1C: RR H
    void Cpu::Opcode_CB_1C() { RR_r(Register::RegisterName::H); }
    // 0x1D: RR L
    void Cpu::Opcode_CB_1D() { RR_r(Register::RegisterName::L); }
    // 0x1E: RR (HL)
    void Cpu::Opcode_CB_1E() { RR_16(); }
    // 0x1F: RR A
    void Cpu::Opcode_CB_1F() { RR_r(Register::RegisterName::A); }

    // 0x20: SLA B
    void Cpu::Opcode_CB_20() { SLA_r(Register::RegisterName::B); }
    // 0x21: SLA C
    void Cpu::Opcode_CB_21() { SLA_r(Register::RegisterName::C); }
    // 0x22: SLA D
    void Cpu::Opcode_CB_22() { SLA_r(Register::RegisterName::D); }
    // 0x23: SLA E
    void Cpu::Opcode_CB_23() { SLA_r(Register::RegisterName::E); }
    // 0x24: SLA H
    void Cpu::Opcode_CB_24() { SLA_r(Register::RegisterName::H); }
    // 0x25: SLA L
    void Cpu::Opcode_CB_25() { SLA_r(Register::RegisterName::L); }
    // 0x26: SLA (HL)
    void Cpu::Opcode_CB_26() { SLA_16(); }
    // 0x27: SLA A
    void Cpu::Opcode_CB_27() { SLA_r(Register::RegisterName::A); }

    // 0x28: SRA B
    void Cpu::Opcode_CB_28() { SRA_r(Register::RegisterName::B); }
    // 0x29: SRA C
    void Cpu::Opcode_CB_29() { SRA_r(Register::RegisterName::C); }
    // 0x2A: SRA D
    void Cpu::Opcode_CB_2A() { SRA_r(Register::RegisterName::D); }
    // 0x2B: SRA E
    void Cpu::Opcode_CB_2B() { SRA_r(Register::RegisterName::E); }
    // 0x2C: SRA H
    void Cpu::Opcode_CB_2C() { SRA_r(Register::RegisterName::H); }
    // 0x2D: SRA L
    void Cpu::Opcode_CB_2D() { SRA_r(Register::RegisterName::L); }
    // 0x2E: SRA (HL)
    void Cpu::Opcode_CB_2E() { SRA_16(); }
    // 0x2F: SRA A
    void Cpu::Opcode_CB_2F() { SRA_r(Register::RegisterName::A); }

    // 0x30: SWAP B
    void Cpu::Opcode_CB_30() { SWAP_r(Register::RegisterName::B); }
    // 0x31: SWAP C
    void Cpu::Opcode_CB_31() { SWAP_r(Register::RegisterName::C); }
    // 0x32: SWAP D
    void Cpu::Opcode_CB_32() { SWAP_r(Register::RegisterName::D); }
    // 0x33: SWAP E
    void Cpu::Opcode_CB_33() { SWAP_r(Register::RegisterName::E); }
    // 0x34: SWAP H
    void Cpu::Opcode_CB_34() { SWAP_r(Register::RegisterName::H); }
    // 0x35: SWAP L
    void Cpu::Opcode_CB_35() { SWAP_r(Register::RegisterName::L); }
    // 0x36: SWAP (HL)
    void Cpu::Opcode_CB_36() { SWAP_16(); }
    // 0x37: SWAP A
    void Cpu::Opcode_CB_37() { SWAP_r(Register::RegisterName::A); }

    // 0x38: SRL B
    void Cpu::Opcode_CB_38() { SRL_r(Register::RegisterName::B); }
    // 0x39: SRL C
    void Cpu::Opcode_CB_39() { SRL_r(Register::RegisterName::C); }
    // 0x3A: SRL D
    void Cpu::Opcode_CB_3A() { SRL_r(Register::RegisterName::D); }
    // 0x3B: SRL E
    void Cpu::Opcode_CB_3B() { SRL_r(Register::RegisterName::E); }
    // 0x3C: SRL H
    void Cpu::Opcode_CB_3C() { SRL_r(Register::RegisterName::H); }
    // 0x3D: SRL L
    void Cpu::Opcode_CB_3D() { SRL_r(Register::RegisterName::L); }
    // 0x3E: SRL (HL)
    void Cpu::Opcode_CB_3E() { SRL_16(); }
    // 0x3F: SRL A
    void Cpu::Opcode_CB_3F() { SRL_r(Register::RegisterName::A); }

    //-------------------------------------------------------------------
    // CB-Prefix Opcodes: BIT Instructions (Bit Test)
    //-------------------------------------------------------------------

    // 0x40: BIT 0, B
    void Cpu::Opcode_CB_40() { BIT_b_r(0, Register::RegisterName::B); }
    // 0x41: BIT 0, C
    void Cpu::Opcode_CB_41() { BIT_b_r(0, Register::RegisterName::C); }
    // 0x42: BIT 0, D
    void Cpu::Opcode_CB_42() { BIT_b_r(0, Register::RegisterName::D); }
    // 0x43: BIT 0, E
    void Cpu::Opcode_CB_43() { BIT_b_r(0, Register::RegisterName::E); }
    // 0x44: BIT 0, H
    void Cpu::Opcode_CB_44() { BIT_b_r(0, Register::RegisterName::H); }
    // 0x45: BIT 0, L
    void Cpu::Opcode_CB_45() { BIT_b_r(0, Register::RegisterName::L); }
    // 0x46: BIT 0, (HL)
    void Cpu::Opcode_CB_46() { BIT_b_16(0); }
    // 0x47: BIT 0, A
    void Cpu::Opcode_CB_47() { BIT_b_r(0, Register::RegisterName::A); }

    // 0x48: BIT 1, B
    void Cpu::Opcode_CB_48() { BIT_b_r(1, Register::RegisterName::B); }
    // 0x49: BIT 1, C
    void Cpu::Opcode_CB_49() { BIT_b_r(1, Register::RegisterName::C); }
    // 0x4A: BIT 1, D
    void Cpu::Opcode_CB_4A() { BIT_b_r(1, Register::RegisterName::D); }
    // 0x4B: BIT 1, E
    void Cpu::Opcode_CB_4B() { BIT_b_r(1, Register::RegisterName::E); }
    // 0x4C: BIT 1, H
    void Cpu::Opcode_CB_4C() { BIT_b_r(1, Register::RegisterName::H); }
    // 0x4D: BIT 1, L
    void Cpu::Opcode_CB_4D() { BIT_b_r(1, Register::RegisterName::L); }
    // 0x4E: BIT 1, (HL)
    void Cpu::Opcode_CB_4E() { BIT_b_16(1); }
    // 0x4F: BIT 1, A
    void Cpu::Opcode_CB_4F() { BIT_b_r(1, Register::RegisterName::A); }

    // 0x50: BIT 2, B
    void Cpu::Opcode_CB_50() { BIT_b_r(2, Register::RegisterName::B); }
    // 0x51: BIT 2, C
    void Cpu::Opcode_CB_51() { BIT_b_r(2, Register::RegisterName::C); }
    // 0x52: BIT 2, D
    void Cpu::Opcode_CB_52() { BIT_b_r(2, Register::RegisterName::D); }
    // 0x53: BIT 2, E
    void Cpu::Opcode_CB_53() { BIT_b_r(2, Register::RegisterName::E); }
    // 0x54: BIT 2, H
    void Cpu::Opcode_CB_54() { BIT_b_r(2, Register::RegisterName::H); }
    // 0x55: BIT 2, L
    void Cpu::Opcode_CB_55() { BIT_b_r(2, Register::RegisterName::L); }
    // 0x56: BIT 2, (HL)
    void Cpu::Opcode_CB_56() { BIT_b_16(2); }
    // 0x57: BIT 2, A
    void Cpu::Opcode_CB_57() { BIT_b_r(2, Register::RegisterName::A); }

    // 0x58: BIT 3, B
    void Cpu::Opcode_CB_58() { BIT_b_r(3, Register::RegisterName::B); }
    // 0x59: BIT 3, C
    void Cpu::Opcode_CB_59() { BIT_b_r(3, Register::RegisterName::C); }
    // 0x5A: BIT 3, D
    void Cpu::Opcode_CB_5A() { BIT_b_r(3, Register::RegisterName::D); }
    // 0x5B: BIT 3, E
    void Cpu::Opcode_CB_5B() { BIT_b_r(3, Register::RegisterName::E); }
    // 0x5C: BIT 3, H
    void Cpu::Opcode_CB_5C() { BIT_b_r(3, Register::RegisterName::H); }
    // 0x5D: BIT 3, L
    void Cpu::Opcode_CB_5D() { BIT_b_r(3, Register::RegisterName::L); }
    // 0x5E: BIT 3, (HL)
    void Cpu::Opcode_CB_5E() { BIT_b_16(3); }
    // 0x5F: BIT 3, A
    void Cpu::Opcode_CB_5F() { BIT_b_r(3, Register::RegisterName::A); }

    // 0x60: BIT 4, B
    void Cpu::Opcode_CB_60() { BIT_b_r(4, Register::RegisterName::B); }
    // 0x61: BIT 4, C
    void Cpu::Opcode_CB_61() { BIT_b_r(4, Register::RegisterName::C); }
    // 0x62: BIT 4, D
    void Cpu::Opcode_CB_62() { BIT_b_r(4, Register::RegisterName::D); }
    // 0x63: BIT 4, E
    void Cpu::Opcode_CB_63() { BIT_b_r(4, Register::RegisterName::E); }
    // 0x64: BIT 4, H
    void Cpu::Opcode_CB_64() { BIT_b_r(4, Register::RegisterName::H); }
    // 0x65: BIT 4, L
    void Cpu::Opcode_CB_65() { BIT_b_r(4, Register::RegisterName::L); }
    // 0x66: BIT 4, (HL)
    void Cpu::Opcode_CB_66() { BIT_b_16(4); }
    // 0x67: BIT 4, A
    void Cpu::Opcode_CB_67() { BIT_b_r(4, Register::RegisterName::A); }

    // 0x68: BIT 5, B
    void Cpu::Opcode_CB_68() { BIT_b_r(5, Register::RegisterName::B); }
    // 0x69: BIT 5, C
    void Cpu::Opcode_CB_69() { BIT_b_r(5, Register::RegisterName::C); }
    // 0x6A: BIT 5, D
    void Cpu::Opcode_CB_6A() { BIT_b_r(5, Register::RegisterName::D); }
    // 0x6B: BIT 5, E
    void Cpu::Opcode_CB_6B() { BIT_b_r(5, Register::RegisterName::E); }
    // 0x6C: BIT 5, H
    void Cpu::Opcode_CB_6C() { BIT_b_r(5, Register::RegisterName::H); }
    // 0x6D: BIT 5, L
    void Cpu::Opcode_CB_6D() { BIT_b_r(5, Register::RegisterName::L); }
    // 0x6E: BIT 5, (HL)
    void Cpu::Opcode_CB_6E() { BIT_b_16(5); }
    // 0x6F: BIT 5, A
    void Cpu::Opcode_CB_6F() { BIT_b_r(5, Register::RegisterName::A); }

    // 0x70: BIT 6, B
    void Cpu::Opcode_CB_70() { BIT_b_r(6, Register::RegisterName::B); }
    // 0x71: BIT 6, C
    void Cpu::Opcode_CB_71() { BIT_b_r(6, Register::RegisterName::C); }
    // 0x72: BIT 6, D
    void Cpu::Opcode_CB_72() { BIT_b_r(6, Register::RegisterName::D); }
    // 0x73: BIT 6, E
    void Cpu::Opcode_CB_73() { BIT_b_r(6, Register::RegisterName::E); }
    // 0x74: BIT 6, H
    void Cpu::Opcode_CB_74() { BIT_b_r(6, Register::RegisterName::H); }
    // 0x75: BIT 6, L
    void Cpu::Opcode_CB_75() { BIT_b_r(6, Register::RegisterName::L); }
    // 0x76: BIT 6, (HL)
    void Cpu::Opcode_CB_76() { BIT_b_16(6); }
    // 0x77: BIT 6, A
    void Cpu::Opcode_CB_77() { BIT_b_r(6, Register::RegisterName::A); }

    // 0x78: BIT 7, B
    void Cpu::Opcode_CB_78() { BIT_b_r(7, Register::RegisterName::B); }
    // 0x79: BIT 7, C
    void Cpu::Opcode_CB_79() { BIT_b_r(7, Register::RegisterName::C); }
    // 0x7A: BIT 7, D
    void Cpu::Opcode_CB_7A() { BIT_b_r(7, Register::RegisterName::D); }
    // 0x7B: BIT 7, E
    void Cpu::Opcode_CB_7B() { BIT_b_r(7, Register::RegisterName::E); }
    // 0x7C: BIT 7, H
    void Cpu::Opcode_CB_7C() { BIT_b_r(7, Register::RegisterName::H); }
    // 0x7D: BIT 7, L
    void Cpu::Opcode_CB_7D() { BIT_b_r(7, Register::RegisterName::L); }
    // 0x7E: BIT 7, (HL)
    void Cpu::Opcode_CB_7E() { BIT_b_16(7); }
    // 0x7F: BIT 7, A
    void Cpu::Opcode_CB_7F() { BIT_b_r(7, Register::RegisterName::A); }

    //-------------------------------------------------------------------
    // CB-Prefix Opcodes: RES (Reset Bit) Instructions
    //-------------------------------------------------------------------

    // 0x80: RES 0, B
    void Cpu::Opcode_CB_80() { RES_b_r(0, Register::RegisterName::B); }
    // 0x81: RES 0, C
    void Cpu::Opcode_CB_81() { RES_b_r(0, Register::RegisterName::C); }
    // 0x82: RES 0, D
    void Cpu::Opcode_CB_82() { RES_b_r(0, Register::RegisterName::D); }
    // 0x83: RES 0, E
    void Cpu::Opcode_CB_83() { RES_b_r(0, Register::RegisterName::E); }
    // 0x84: RES 0, H
    void Cpu::Opcode_CB_84() { RES_b_r(0, Register::RegisterName::H); }
    // 0x85: RES 0, L
    void Cpu::Opcode_CB_85() { RES_b_r(0, Register::RegisterName::L); }
    // 0x86: RES 0, (HL)
    void Cpu::Opcode_CB_86() { RES_b_16(0); }
    // 0x87: RES 0, A
    void Cpu::Opcode_CB_87() { RES_b_r(0, Register::RegisterName::A); }

    // 0x88: RES 1, B
    void Cpu::Opcode_CB_88() { RES_b_r(1, Register::RegisterName::B); }
    // 0x89: RES 1, C
    void Cpu::Opcode_CB_89() { RES_b_r(1, Register::RegisterName::C); }
    // 0x8A: RES 1, D
    void Cpu::Opcode_CB_8A() { RES_b_r(1, Register::RegisterName::D); }
    // 0x8B: RES 1, E
    void Cpu::Opcode_CB_8B() { RES_b_r(1, Register::RegisterName::E); }
    // 0x8C: RES 1, H
    void Cpu::Opcode_CB_8C() { RES_b_r(1, Register::RegisterName::H); }
    // 0x8D: RES 1, L
    void Cpu::Opcode_CB_8D() { RES_b_r(1, Register::RegisterName::L); }
    // 0x8E: RES 1, (HL)
    void Cpu::Opcode_CB_8E() { RES_b_16(1); }
    // 0x8F: RES 1, A
    void Cpu::Opcode_CB_8F() { RES_b_r(1, Register::RegisterName::A); }

    // 0x90: RES 2, B
    void Cpu::Opcode_CB_90() { RES_b_r(2, Register::RegisterName::B); }
    // 0x91: RES 2, C
    void Cpu::Opcode_CB_91() { RES_b_r(2, Register::RegisterName::C); }
    // 0x92: RES 2, D
    void Cpu::Opcode_CB_92() { RES_b_r(2, Register::RegisterName::D); }
    // 0x93: RES 2, E
    void Cpu::Opcode_CB_93() { RES_b_r(2, Register::RegisterName::E); }
    // 0x94: RES 2, H
    void Cpu::Opcode_CB_94() { RES_b_r(2, Register::RegisterName::H); }
    // 0x95: RES 2, L
    void Cpu::Opcode_CB_95() { RES_b_r(2, Register::RegisterName::L); }
    // 0x96: RES 2, (HL)
    void Cpu::Opcode_CB_96() { RES_b_16(2); }
    // 0x97: RES 2, A
    void Cpu::Opcode_CB_97() { RES_b_r(2, Register::RegisterName::A); }

    // 0x98: RES 3, B
    void Cpu::Opcode_CB_98() { RES_b_r(3, Register::RegisterName::B); }
    // 0x99: RES 3, C
    void Cpu::Opcode_CB_99() { RES_b_r(3, Register::RegisterName::C); }
    // 0x9A: RES 3, D
    void Cpu::Opcode_CB_9A() { RES_b_r(3, Register::RegisterName::D); }
    // 0x9B: RES 3, E
    void Cpu::Opcode_CB_9B() { RES_b_r(3, Register::RegisterName::E); }
    // 0x9C: RES 3, H
    void Cpu::Opcode_CB_9C() { RES_b_r(3, Register::RegisterName::H); }
    // 0x9D: RES 3, L
    void Cpu::Opcode_CB_9D() { RES_b_r(3, Register::RegisterName::L); }
    // 0x9E: RES 3, (HL)
    void Cpu::Opcode_CB_9E() { RES_b_16(3); }
    // 0x9F: RES 3, A
    void Cpu::Opcode_CB_9F() { RES_b_r(3, Register::RegisterName::A); }

    // 0xA0: RES 4, B
    void Cpu::Opcode_CB_A0() { RES_b_r(4, Register::RegisterName::B); }
    // 0xA1: RES 4, C
    void Cpu::Opcode_CB_A1() { RES_b_r(4, Register::RegisterName::C); }
    // 0xA2: RES 4, D
    void Cpu::Opcode_CB_A2() { RES_b_r(4, Register::RegisterName::D); }
    // 0xA3: RES 4, E
    void Cpu::Opcode_CB_A3() { RES_b_r(4, Register::RegisterName::E); }
    // 0xA4: RES 4, H
    void Cpu::Opcode_CB_A4() { RES_b_r(4, Register::RegisterName::H); }
    // 0xA5: RES 4, L
    void Cpu::Opcode_CB_A5() { RES_b_r(4, Register::RegisterName::L); }
    // 0xA6: RES 4, (HL)
    void Cpu::Opcode_CB_A6() { RES_b_16(4); }
    // 0xA7: RES 4, A
    void Cpu::Opcode_CB_A7() { RES_b_r(4, Register::RegisterName::A); }

    // 0xA8: RES 5, B
    void Cpu::Opcode_CB_A8() { RES_b_r(5, Register::RegisterName::B); }
    // 0xA9: RES 5, C
    void Cpu::Opcode_CB_A9() { RES_b_r(5, Register::RegisterName::C); }
    // 0xAA: RES 5, D
    void Cpu::Opcode_CB_AA() { RES_b_r(5, Register::RegisterName::D); }
    // 0xAB: RES 5, E
    void Cpu::Opcode_CB_AB() { RES_b_r(5, Register::RegisterName::E); }
    // 0xAC: RES 5, H
    void Cpu::Opcode_CB_AC() { RES_b_r(5, Register::RegisterName::H); }
    // 0xAD: RES 5, L
    void Cpu::Opcode_CB_AD() { RES_b_r(5, Register::RegisterName::L); }
    // 0xAE: RES 5, (HL)
    void Cpu::Opcode_CB_AE() { RES_b_16(5); }
    // 0xAF: RES 5, A
    void Cpu::Opcode_CB_AF() { RES_b_r(5, Register::RegisterName::A); }

    // 0xB0: RES 6, B
    void Cpu::Opcode_CB_B0() { RES_b_r(6, Register::RegisterName::B); }
    // 0xB1: RES 6, C
    void Cpu::Opcode_CB_B1() { RES_b_r(6, Register::RegisterName::C); }
    // 0xB2: RES 6, D
    void Cpu::Opcode_CB_B2() { RES_b_r(6, Register::RegisterName::D); }
    // 0xB3: RES 6, E
    void Cpu::Opcode_CB_B3() { RES_b_r(6, Register::RegisterName::E); }
    // 0xB4: RES 6, H
    void Cpu::Opcode_CB_B4() { RES_b_r(6, Register::RegisterName::H); }
    // 0xB5: RES 6, L
    void Cpu::Opcode_CB_B5() { RES_b_r(6, Register::RegisterName::L); }
    // 0xB6: RES 6, (HL)
    void Cpu::Opcode_CB_B6() { RES_b_16(6); }
    // 0xB7: RES 6, A
    void Cpu::Opcode_CB_B7() { RES_b_r(6, Register::RegisterName::A); }

    // 0xB8: RES 7, B
    void Cpu::Opcode_CB_B8() { RES_b_r(7, Register::RegisterName::B); }
    // 0xB9: RES 7, C
    void Cpu::Opcode_CB_B9() { RES_b_r(7, Register::RegisterName::C); }
    // 0xBA: RES 7, D
    void Cpu::Opcode_CB_BA() { RES_b_r(7, Register::RegisterName::D); }
    // 0xBB: RES 7, E
    void Cpu::Opcode_CB_BB() { RES_b_r(7, Register::RegisterName::E); }
    // 0xBC: RES 7, H
    void Cpu::Opcode_CB_BC() { RES_b_r(7, Register::RegisterName::H); }
    // 0xBD: RES 7, L
    void Cpu::Opcode_CB_BD() { RES_b_r(7, Register::RegisterName::L); }
    // 0xBE: RES 7, (HL)
    void Cpu::Opcode_CB_BE() { RES_b_16(7); }
    // 0xBF: RES 7, A
    void Cpu::Opcode_CB_BF() { RES_b_r(7, Register::RegisterName::A); }

    //-------------------------------------------------------------------
    // CB-Prefix Opcodes: SET (Set Bit) Instructions
    //-------------------------------------------------------------------

    // 0xC0: SET 0, B
    void Cpu::Opcode_CB_C0() { SET_b_r(0, Register::RegisterName::B); }
    // 0xC1: SET 0, C
    void Cpu::Opcode_CB_C1() { SET_b_r(0, Register::RegisterName::C); }
    // 0xC2: SET 0, D
    void Cpu::Opcode_CB_C2() { SET_b_r(0, Register::RegisterName::D); }
    // 0xC3: SET 0, E
    void Cpu::Opcode_CB_C3() { SET_b_r(0, Register::RegisterName::E); }
    // 0xC4: SET 0, H
    void Cpu::Opcode_CB_C4() { SET_b_r(0, Register::RegisterName::H); }
    // 0xC5: SET 0, L
    void Cpu::Opcode_CB_C5() { SET_b_r(0, Register::RegisterName::L); }
    // 0xC6: SET 0, (HL)
    void Cpu::Opcode_CB_C6() { SET_b_16(0); }
    // 0xC7: SET 0, A
    void Cpu::Opcode_CB_C7() { SET_b_r(0, Register::RegisterName::A); }

    // 0xC8: SET 1, B
    void Cpu::Opcode_CB_C8() { SET_b_r(1, Register::RegisterName::B); }
    // 0xC9: SET 1, C
    void Cpu::Opcode_CB_C9() { SET_b_r(1, Register::RegisterName::C); }
    // 0xCA: SET 1, D
    void Cpu::Opcode_CB_CA() { SET_b_r(1, Register::RegisterName::D); }
    // 0xCB: SET 1, E
    void Cpu::Opcode_CB_CB() { SET_b_r(1, Register::RegisterName::E); }
    // 0xCC: SET 1, H
    void Cpu::Opcode_CB_CC() { SET_b_r(1, Register::RegisterName::H); }
    // 0xCD: SET 1, L
    void Cpu::Opcode_CB_CD() { SET_b_r(1, Register::RegisterName::L); }
    // 0xCE: SET 1, (HL)
    void Cpu::Opcode_CB_CE() { SET_b_16(1); }
    // 0xCF: SET 1, A
    void Cpu::Opcode_CB_CF() { SET_b_r(1, Register::RegisterName::A); }

    // 0xD0: SET 2, B
    void Cpu::Opcode_CB_D0() { SET_b_r(2, Register::RegisterName::B); }
    // 0xD1: SET 2, C
    void Cpu::Opcode_CB_D1() { SET_b_r(2, Register::RegisterName::C); }
    // 0xD2: SET 2, D
    void Cpu::Opcode_CB_D2() { SET_b_r(2, Register::RegisterName::D); }
    // 0xD3: SET 2, E
    void Cpu::Opcode_CB_D3() { SET_b_r(2, Register::RegisterName::E); }
    // 0xD4: SET 2, H
    void Cpu::Opcode_CB_D4() { SET_b_r(2, Register::RegisterName::H); }
    // 0xD5: SET 2, L
    void Cpu::Opcode_CB_D5() { SET_b_r(2, Register::RegisterName::L); }
    // 0xD6: SET 2, (HL)
    void Cpu::Opcode_CB_D6() { SET_b_16(2); }
    // 0xD7: SET 2, A
    void Cpu::Opcode_CB_D7() { SET_b_r(2, Register::RegisterName::A); }

    // 0xD8: SET 3, B
    void Cpu::Opcode_CB_D8() { SET_b_r(3, Register::RegisterName::B); }
    // 0xD9: SET 3, C
    void Cpu::Opcode_CB_D9() { SET_b_r(3, Register::RegisterName::C); }
    // 0xDA: SET 3, D
    void Cpu::Opcode_CB_DA() { SET_b_r(3, Register::RegisterName::D); }
    // 0xDB: SET 3, E
    void Cpu::Opcode_CB_DB() { SET_b_r(3, Register::RegisterName::E); }
    // 0xDC: SET 3, H
    void Cpu::Opcode_CB_DC() { SET_b_r(3, Register::RegisterName::H); }
    // 0xDD: SET 3, L
    void Cpu::Opcode_CB_DD() { SET_b_r(3, Register::RegisterName::L); }
    // 0xDE: SET 3, (HL)
    void Cpu::Opcode_CB_DE() { SET_b_16(3); }
    // 0xDF: SET 3, A
    void Cpu::Opcode_CB_DF() { SET_b_r(3, Register::RegisterName::A); }

    // 0xE0: SET 4, B
    void Cpu::Opcode_CB_E0() { SET_b_r(4, Register::RegisterName::B); }
    // 0xE1: SET 4, C
    void Cpu::Opcode_CB_E1() { SET_b_r(4, Register::RegisterName::C); }
    // 0xE2: SET 4, D
    void Cpu::Opcode_CB_E2() { SET_b_r(4, Register::RegisterName::D); }
    // 0xE3: SET 4, E
    void Cpu::Opcode_CB_E3() { SET_b_r(4, Register::RegisterName::E); }
    // 0xE4: SET 4, H
    void Cpu::Opcode_CB_E4() { SET_b_r(4, Register::RegisterName::H); }
    // 0xE5: SET 4, L
    void Cpu::Opcode_CB_E5() { SET_b_r(4, Register::RegisterName::L); }
    // 0xE6: SET 4, (HL)
    void Cpu::Opcode_CB_E6() { SET_b_16(4); }
    // 0xE7: SET 4, A
    void Cpu::Opcode_CB_E7() { SET_b_r(4, Register::RegisterName::A); }

    // 0xE8: SET 5, B
    void Cpu::Opcode_CB_E8() { SET_b_r(5, Register::RegisterName::B); }
    // 0xE9: SET 5, C
    void Cpu::Opcode_CB_E9() { SET_b_r(5, Register::RegisterName::C); }
    // 0xEA: SET 5, D
    void Cpu::Opcode_CB_EA() { SET_b_r(5, Register::RegisterName::D); }
    // 0xEB: SET 5, E
    void Cpu::Opcode_CB_EB() { SET_b_r(5, Register::RegisterName::E); }
    // 0xEC: SET 5, H
    void Cpu::Opcode_CB_EC() { SET_b_r(5, Register::RegisterName::H); }
    // 0xED: SET 5, L
    void Cpu::Opcode_CB_ED() { SET_b_r(5, Register::RegisterName::L); }
    // 0xEE: SET 5, (HL)
    void Cpu::Opcode_CB_EE() { SET_b_16(5); }
    // 0xEF: SET 5, A
    void Cpu::Opcode_CB_EF() { SET_b_r(5, Register::RegisterName::A); }

    // 0xF0: SET 6, B
    void Cpu::Opcode_CB_F0() { SET_b_r(6, Register::RegisterName::B); }
    // 0xF1: SET 6, C
    void Cpu::Opcode_CB_F1() { SET_b_r(6, Register::RegisterName::C); }
    // 0xF2: SET 6, D
    void Cpu::Opcode_CB_F2() { SET_b_r(6, Register::RegisterName::D); }
    // 0xF3: SET 6, E
    void Cpu::Opcode_CB_F3() { SET_b_r(6, Register::RegisterName::E); }
    // 0xF4: SET 6, H
    void Cpu::Opcode_CB_F4() { SET_b_r(6, Register::RegisterName::H); }
    // 0xF5: SET 6, L
    void Cpu::Opcode_CB_F5() { SET_b_r(6, Register::RegisterName::L); }
    // 0xF6: SET 6, (HL)
    void Cpu::Opcode_CB_F6() { SET_b_16(6); }
    // 0xF7: SET 6, A
    void Cpu::Opcode_CB_F7() { SET_b_r(6, Register::RegisterName::A); }

    // 0xF8: SET 7, B
    void Cpu::Opcode_CB_F8() { SET_b_r(7, Register::RegisterName::B); }
    // 0xF9: SET 7, C
    void Cpu::Opcode_CB_F9() { SET_b_r(7, Register::RegisterName::C); }
    // 0xFA: SET 7, D
    void Cpu::Opcode_CB_FA() { SET_b_r(7, Register::RegisterName::D); }
    // 0xFB: SET 7, E
    void Cpu::Opcode_CB_FB() { SET_b_r(7, Register::RegisterName::E); }
    // 0xFC: SET 7, H
    void Cpu::Opcode_CB_FC() { SET_b_r(7, Register::RegisterName::H); }
    // 0xFD: SET 7, L
    void Cpu::Opcode_CB_FD() { SET_b_r(7, Register::RegisterName::L); }
    // 0xFE: SET 7, (HL)
    void Cpu::Opcode_CB_FE() { SET_b_16(7); }
    // 0xFF: SET 7, A
    void Cpu::Opcode_CB_FF() { SET_b_r(7, Register::RegisterName::A); }
}