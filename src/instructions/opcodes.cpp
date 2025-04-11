#ifndef OPCODES_H
#define OPCODES_H

#include "cpu.h"
#include "timer.h"

namespace gasyboy
{
    // Opcode 0x00
    void Cpu::Opcode_00()
    {
        NOP();
    }

    // Opcode 0x01
    void Cpu::Opcode_01()
    {
        LD_rr_16(_registers->PC + 1, Register::RegisterPairName::BC);
    }

    // Opcode 0x02
    void Cpu::Opcode_02()
    {
        LD_16_r(_registers->BC.get(), Register::RegisterName::A);
    }

    // Opcode 0x03
    void Cpu::Opcode_03()
    {
        INC_rr(Register::RegisterPairName::BC);
    }

    // Opcode 0x04
    void Cpu::Opcode_04()
    {
        INC_r(Register::RegisterName::B);
    }

    // Opcode 0x05
    void Cpu::Opcode_05()
    {
        DEC_r(Register::RegisterName::B);
    }

    // Opcode 0x06
    void Cpu::Opcode_06()
    {
        LD_r_n(_mmu->readRam(_registers->PC + 1), Register::RegisterName::B);
    }

    // Opcode 0x07
    void Cpu::Opcode_07()
    {
        RLCA();
    }

    // Opcode 0x08
    void Cpu::Opcode_08()
    {
        LD_16_rr(next2bytes(_registers->PC + 1), Register::RegisterPairName::SP);
    }

    // Opcode 0x09
    void Cpu::Opcode_09()
    {
        ADD_HL_rr(Register::RegisterPairName::BC);
    }

    // Opcode 0x0A
    void Cpu::Opcode_0A()
    {
        LD_r_16(_registers->BC.get(), Register::RegisterName::A);
    }

    // Opcode 0x0B
    void Cpu::Opcode_0B()
    {
        DEC_rr(Register::RegisterPairName::BC);
    }

    // Opcode 0x0C
    void Cpu::Opcode_0C()
    {
        INC_r(Register::RegisterName::C);
    }

    // Opcode 0x0D
    void Cpu::Opcode_0D()
    {
        DEC_r(Register::RegisterName::C);
    }

    // Opcode 0x0E
    void Cpu::Opcode_0E()
    {
        LD_r_n(_mmu->readRam(_registers->PC + 1), Register::RegisterName::C);
    }

    // Opcode 0x0F
    void Cpu::Opcode_0F()
    {
        RRCA();
    }

    // Opcode 0x10
    void Cpu::Opcode_10()
    {
        _registers->setStopMode(true); // Enter STOP mode
        Timer::resetDIV();
    }

    // Opcode 0x11
    void Cpu::Opcode_11()
    {
        LD_rr_16(_registers->PC + 1, Register::RegisterPairName::DE);
    }

    // Opcode 0x12
    void Cpu::Opcode_12()
    {
        LD_16_r(_registers->DE.get(), Register::RegisterName::A);
    }

    // Opcode 0x13
    void Cpu::Opcode_13()
    {
        INC_rr(Register::RegisterPairName::DE);
    }

    // Opcode 0x14
    void Cpu::Opcode_14()
    {
        INC_r(Register::RegisterName::D);
    }

    // Opcode 0x15
    void Cpu::Opcode_15()
    {
        DEC_r(Register::RegisterName::D);
    }

    // Opcode 0x16
    void Cpu::Opcode_16()
    {
        LD_r_n(_mmu->readRam(_registers->PC + 1), Register::RegisterName::D);
    }

    // Opcode 0x17
    void Cpu::Opcode_17()
    {
        RLA();
    }

    // Opcode 0x18
    void Cpu::Opcode_18()
    {
        JR_e(_mmu->readRam(_registers->PC + 1));
        _pcManuallySet = true;
    }

    // Opcode 0x19
    void Cpu::Opcode_19()
    {
        ADD_HL_rr(Register::RegisterPairName::DE);
    }

    // Opcode 0x1A
    void Cpu::Opcode_1A()
    {
        LD_r_16(_registers->DE.get(), Register::RegisterName::A);
    }

    // Opcode 0x1B
    void Cpu::Opcode_1B()
    {
        DEC_rr(Register::RegisterPairName::DE);
    }

    // Opcode 0x1C
    void Cpu::Opcode_1C()
    {
        INC_r(Register::RegisterName::E);
    }

    // Opcode 0x1D
    void Cpu::Opcode_1D()
    {
        DEC_r(Register::RegisterName::E);
    }

    // Opcode 0x1E
    void Cpu::Opcode_1E()
    {
        LD_r_n(_mmu->readRam(_registers->PC + 1), Register::RegisterName::E);
    }

    // Opcode 0x1F
    void Cpu::Opcode_1F()
    {
        RRA();
    }

    // Opcode 0x20
    void Cpu::Opcode_20()
    {
        // JR_NZ_e takes the immediate from memory and conditionally jumps.
        JR_NZ_e(_mmu->readRam(_registers->PC + 1));
        // Evaluate flag Z (as shown in your switch)
        _registers->AF.getFlag(Register::FlagName::Z);

        _pcManuallySet = true;
    }

    // Opcode 0x21
    void Cpu::Opcode_21()
    {
        LD_rr_nn(next2bytes(_registers->PC + 1), Register::RegisterPairName::HL);
    }

    // Opcode 0x22
    void Cpu::Opcode_22()
    {
        LD_16_r(_registers->HL.get(), Register::RegisterName::A);
        INC_rr(Register::RegisterPairName::HL);
    }

    // Opcode 0x23
    void Cpu::Opcode_23()
    {
        INC_rr(Register::RegisterPairName::HL);
    }

    // Opcode 0x24
    void Cpu::Opcode_24()
    {
        INC_r(Register::RegisterName::H);
    }

    // Opcode 0x25
    void Cpu::Opcode_25()
    {
        DEC_r(Register::RegisterName::H);
    }

    // Opcode 0x26
    void Cpu::Opcode_26()
    {
        LD_r_n(_mmu->readRam(_registers->PC + 1), Register::RegisterName::H);
    }

    // Opcode 0x27
    void Cpu::Opcode_27()
    {
        DAA();
    }

    // Opcode 0x28
    void Cpu::Opcode_28()
    {
        JR_Z_e(_mmu->readRam(_registers->PC + 1));
        _pcManuallySet = true;
    }

    // Opcode 0x29
    void Cpu::Opcode_29()
    {
        ADD_HL_rr(Register::RegisterPairName::HL);
    }

    // Opcode 0x2A
    void Cpu::Opcode_2A()
    {
        LD_r_n(_mmu->readRam(_registers->HL.get()), Register::RegisterName::A);
        INC_rr(Register::RegisterPairName::HL);
    }

    // Opcode 0x2B
    void Cpu::Opcode_2B()
    {
        DEC_rr(Register::RegisterPairName::HL);
    }

    // Opcode 0x2C
    void Cpu::Opcode_2C()
    {
        INC_r(Register::RegisterName::L);
    }

    // Opcode 0x2D
    void Cpu::Opcode_2D()
    {
        DEC_r(Register::RegisterName::L);
    }

    // Opcode 0x2E
    void Cpu::Opcode_2E()
    {
        LD_r_n(_mmu->readRam(_registers->PC + 1), Register::RegisterName::L);
    }

    // Opcode 0x2F
    void Cpu::Opcode_2F()
    {
        CPL();
    }

    // Opcode 0x30
    void Cpu::Opcode_30()
    {
        JR_NC_e(_mmu->readRam(_registers->PC + 1));
        _pcManuallySet = true;
    }

    // Opcode 0x31
    void Cpu::Opcode_31()
    {
        LD_rr_nn(next2bytes(_registers->PC + 1), Register::RegisterPairName::SP);
    }

    // Opcode 0x32
    void Cpu::Opcode_32()
    {
        LD_16_r(_registers->HL.get(), Register::RegisterName::A);
        DEC_rr(Register::RegisterPairName::HL);
    }

    // Opcode 0x33
    void Cpu::Opcode_33()
    {
        INC_rr(Register::RegisterPairName::SP);
    }

    // Opcode 0x34
    void Cpu::Opcode_34()
    {
        INC_16();
    }

    // Opcode 0x35
    void Cpu::Opcode_35()
    {
        DEC_16();
    }

    // Opcode 0x36
    void Cpu::Opcode_36()
    {
        LD_16_n(_registers->HL.get(), _mmu->readRam(_registers->PC + 1));
    }

    // Opcode 0x37
    void Cpu::Opcode_37()
    {
        SCF();
    }

    // Opcode 0x38
    void Cpu::Opcode_38()
    {
        JR_C_e(_mmu->readRam(_registers->PC + 1));
        _pcManuallySet = true;
    }

    // Opcode 0x39
    void Cpu::Opcode_39()
    {
        ADD_HL_rr(Register::RegisterPairName::SP);
    }

    // Opcode 0x3A
    void Cpu::Opcode_3A()
    {
        LD_r_16(_registers->HL.get(), Register::RegisterName::A);
        DEC_rr(Register::RegisterPairName::HL);
    }

    // Opcode 0x3B
    void Cpu::Opcode_3B()
    {
        DEC_rr(Register::RegisterPairName::SP);
    }

    // Opcode 0x3C
    void Cpu::Opcode_3C()
    {
        INC_r(Register::RegisterName::A);
    }

    // Opcode 0x3D
    void Cpu::Opcode_3D()
    {
        DEC_r(Register::RegisterName::A);
    }

    // Opcode 0x3E
    void Cpu::Opcode_3E()
    {
        LD_r_n(_mmu->readRam(_registers->PC + 1), Register::RegisterName::A);
    }

    // Opcode 0x3F
    void Cpu::Opcode_3F()
    {
        CCF();
    }

    // Opcode 0x40
    void Cpu::Opcode_40()
    {
        LD_r_r(Register::RegisterName::B, Register::RegisterName::B);
    }

    // Opcode 0x41
    void Cpu::Opcode_41()
    {
        LD_r_r(Register::RegisterName::C, Register::RegisterName::B);
    }

    // Opcode 0x42
    void Cpu::Opcode_42()
    {
        LD_r_r(Register::RegisterName::D, Register::RegisterName::B);
    }

    // Opcode 0x43
    void Cpu::Opcode_43()
    {
        LD_r_r(Register::RegisterName::E, Register::RegisterName::B);
    }

    // Opcode 0x44
    void Cpu::Opcode_44()
    {
        LD_r_r(Register::RegisterName::H, Register::RegisterName::B);
    }

    // Opcode 0x45
    void Cpu::Opcode_45()
    {
        LD_r_r(Register::RegisterName::L, Register::RegisterName::B);
    }

    // Opcode 0x46
    void Cpu::Opcode_46()
    {
        LD_r_16(_registers->HL.get(), Register::RegisterName::B);
    }

    // Opcode 0x47
    void Cpu::Opcode_47()
    {
        LD_r_r(Register::RegisterName::A, Register::RegisterName::B);
    }

    // Opcode 0x48
    void Cpu::Opcode_48()
    {
        LD_r_r(Register::RegisterName::B, Register::RegisterName::C);
    }

    // Opcode 0x49
    void Cpu::Opcode_49()
    {
        LD_r_r(Register::RegisterName::C, Register::RegisterName::C);
    }

    // Opcode 0x4A
    void Cpu::Opcode_4A()
    {
        LD_r_r(Register::RegisterName::D, Register::RegisterName::C);
    }

    // Opcode 0x4B
    void Cpu::Opcode_4B()
    {
        LD_r_r(Register::RegisterName::E, Register::RegisterName::C);
    }

    // Opcode 0x4C
    void Cpu::Opcode_4C()
    {
        LD_r_r(Register::RegisterName::H, Register::RegisterName::C);
    }

    // Opcode 0x4D
    void Cpu::Opcode_4D()
    {
        LD_r_r(Register::RegisterName::L, Register::RegisterName::C);
    }

    // Opcode 0x4E
    void Cpu::Opcode_4E()
    {
        LD_r_16(_registers->HL.get(), Register::RegisterName::C);
    }

    // Opcode 0x4F
    void Cpu::Opcode_4F()
    {
        LD_r_r(Register::RegisterName::A, Register::RegisterName::C);
    }

    // Opcode 0x50
    void Cpu::Opcode_50()
    {
        LD_r_r(Register::RegisterName::B, Register::RegisterName::D);
    }

    // Opcode 0x51
    void Cpu::Opcode_51()
    {
        LD_r_r(Register::RegisterName::C, Register::RegisterName::D);
    }

    // Opcode 0x52
    void Cpu::Opcode_52()
    {
        LD_r_r(Register::RegisterName::D, Register::RegisterName::D);
    }

    // Opcode 0x53
    void Cpu::Opcode_53()
    {
        LD_r_r(Register::RegisterName::E, Register::RegisterName::D);
    }

    // Opcode 0x54
    void Cpu::Opcode_54()
    {
        LD_r_r(Register::RegisterName::H, Register::RegisterName::D);
    }

    // Opcode 0x55
    void Cpu::Opcode_55()
    {
        LD_r_r(Register::RegisterName::L, Register::RegisterName::D);
    }

    // Opcode 0x56
    void Cpu::Opcode_56()
    {
        LD_r_16(_registers->HL.get(), Register::RegisterName::D);
    }

    // Opcode 0x57
    void Cpu::Opcode_57()
    {
        LD_r_r(Register::RegisterName::A, Register::RegisterName::D);
    }

    // Opcode 0x58
    void Cpu::Opcode_58()
    {
        LD_r_r(Register::RegisterName::B, Register::RegisterName::E);
    }

    // Opcode 0x59
    void Cpu::Opcode_59()
    {
        LD_r_r(Register::RegisterName::C, Register::RegisterName::E);
    }

    // Opcode 0x5A
    void Cpu::Opcode_5A()
    {
        LD_r_r(Register::RegisterName::D, Register::RegisterName::E);
    }

    // Opcode 0x5B
    void Cpu::Opcode_5B()
    {
        LD_r_r(Register::RegisterName::E, Register::RegisterName::E);
    }

    // Opcode 0x5C
    void Cpu::Opcode_5C()
    {
        LD_r_r(Register::RegisterName::H, Register::RegisterName::E);
    }

    // Opcode 0x5D
    void Cpu::Opcode_5D()
    {
        LD_r_r(Register::RegisterName::L, Register::RegisterName::E);
    }

    // Opcode 0x5E
    void Cpu::Opcode_5E()
    {
        LD_r_16(_registers->HL.get(), Register::RegisterName::E);
    }

    // Opcode 0x5F
    void Cpu::Opcode_5F()
    {
        LD_r_r(Register::RegisterName::A, Register::RegisterName::E);
    }

    // Opcode 0x60
    void Cpu::Opcode_60()
    {
        LD_r_r(Register::RegisterName::B, Register::RegisterName::H);
    }

    // 0x61
    void Cpu::Opcode_61()
    {
        LD_r_r(Register::RegisterName::C, Register::RegisterName::H);
    }
    // 0x62
    void Cpu::Opcode_62()
    {
        LD_r_r(Register::RegisterName::D, Register::RegisterName::H);
    }
    // 0x63
    void Cpu::Opcode_63()
    {
        LD_r_r(Register::RegisterName::E, Register::RegisterName::H);
    }
    // 0x64
    void Cpu::Opcode_64()
    {
        LD_r_r(Register::RegisterName::H, Register::RegisterName::H);
    }
    // 0x65
    void Cpu::Opcode_65()
    {
        LD_r_r(Register::RegisterName::L, Register::RegisterName::H);
    }
    // 0x66
    void Cpu::Opcode_66()
    {
        LD_r_16(_registers->HL.get(), Register::RegisterName::H);
    }
    // 0x67
    void Cpu::Opcode_67()
    {
        LD_r_r(Register::RegisterName::A, Register::RegisterName::H);
    }
    // 0x68
    void Cpu::Opcode_68()
    {
        LD_r_r(Register::RegisterName::B, Register::RegisterName::L);
    }
    // 0x69
    void Cpu::Opcode_69()
    {
        LD_r_r(Register::RegisterName::C, Register::RegisterName::L);
    }
    // 0x6A
    void Cpu::Opcode_6A()
    {
        LD_r_r(Register::RegisterName::D, Register::RegisterName::L);
    }
    // 0x6B
    void Cpu::Opcode_6B()
    {
        LD_r_r(Register::RegisterName::E, Register::RegisterName::L);
    }
    // 0x6C
    void Cpu::Opcode_6C()
    {
        LD_r_r(Register::RegisterName::H, Register::RegisterName::L);
    }
    // 0x6D
    void Cpu::Opcode_6D()
    {
        LD_r_r(Register::RegisterName::L, Register::RegisterName::L);
    }
    // 0x6E
    void Cpu::Opcode_6E()
    {
        LD_r_16(_registers->HL.get(), Register::RegisterName::L);
    }
    // 0x6F
    void Cpu::Opcode_6F()
    {
        LD_r_r(Register::RegisterName::A, Register::RegisterName::L);
    }
    // 0x70
    void Cpu::Opcode_70()
    {
        LD_16_r(_registers->HL.get(), Register::RegisterName::B);
    }
    // 0x71
    void Cpu::Opcode_71()
    {
        LD_16_r(_registers->HL.get(), Register::RegisterName::C);
    }
    // 0x72
    void Cpu::Opcode_72()
    {
        LD_16_r(_registers->HL.get(), Register::RegisterName::D);
    }
    // 0x73
    void Cpu::Opcode_73()
    {
        LD_16_r(_registers->HL.get(), Register::RegisterName::E);
    }
    // 0x74
    void Cpu::Opcode_74()
    {
        LD_16_r(_registers->HL.get(), Register::RegisterName::H);
    }
    // 0x75
    void Cpu::Opcode_75()
    {
        LD_16_r(_registers->HL.get(), Register::RegisterName::L);
    }
    // 0x76
    void Cpu::Opcode_76()
    {
        HALT();
    }
    // 0x77
    void Cpu::Opcode_77()
    {
        LD_16_r(_registers->HL.get(), Register::RegisterName::A);
    }
    // 0x78
    void Cpu::Opcode_78()
    {
        LD_r_r(Register::RegisterName::B, Register::RegisterName::A);
    }
    // 0x79
    void Cpu::Opcode_79()
    {
        LD_r_r(Register::RegisterName::C, Register::RegisterName::A);
    }
    // 0x7A
    void Cpu::Opcode_7A()
    {
        LD_r_r(Register::RegisterName::D, Register::RegisterName::A);
    }
    // 0x7B
    void Cpu::Opcode_7B()
    {
        LD_r_r(Register::RegisterName::E, Register::RegisterName::A);
    }
    // 0x7C
    void Cpu::Opcode_7C()
    {
        LD_r_r(Register::RegisterName::H, Register::RegisterName::A);
    }
    // 0x7D
    void Cpu::Opcode_7D()
    {
        LD_r_r(Register::RegisterName::L, Register::RegisterName::A);
    }
    // 0x7E
    void Cpu::Opcode_7E()
    {
        LD_r_16(_registers->HL.get(), Register::RegisterName::A);
    }
    // 0x7F
    void Cpu::Opcode_7F()
    {
        LD_r_r(Register::RegisterName::A, Register::RegisterName::A);
    }

    // 0x80
    void Cpu::Opcode_80()
    {
        ADD_A_r(Register::RegisterName::B);
    }
    // 0x81
    void Cpu::Opcode_81()
    {
        ADD_A_r(Register::RegisterName::C);
    }
    // 0x82
    void Cpu::Opcode_82()
    {
        ADD_A_r(Register::RegisterName::D);
    }
    // 0x83
    void Cpu::Opcode_83()
    {
        ADD_A_r(Register::RegisterName::E);
    }
    // 0x84
    void Cpu::Opcode_84()
    {
        ADD_A_r(Register::RegisterName::H);
    }
    // 0x85
    void Cpu::Opcode_85()
    {
        ADD_A_r(Register::RegisterName::L);
    }
    // 0x86
    void Cpu::Opcode_86()
    {
        ADD_A_16();
    }
    // 0x87
    void Cpu::Opcode_87()
    {
        ADD_A_r(Register::RegisterName::A);
    }
    // 0x88
    void Cpu::Opcode_88()
    {
        ADC_A_r(Register::RegisterName::B);
    }
    // 0x89
    void Cpu::Opcode_89()
    {
        ADC_A_r(Register::RegisterName::C);
    }
    // 0x8A
    void Cpu::Opcode_8A()
    {
        ADC_A_r(Register::RegisterName::D);
    }
    // 0x8B
    void Cpu::Opcode_8B()
    {
        ADC_A_r(Register::RegisterName::E);
    }
    // 0x8C
    void Cpu::Opcode_8C()
    {
        ADC_A_r(Register::RegisterName::H);
    }
    // 0x8D
    void Cpu::Opcode_8D()
    {
        ADC_A_r(Register::RegisterName::L);
    }
    // 0x8E
    void Cpu::Opcode_8E()
    {
        ADC_A_16();
    }
    // 0x8F
    void Cpu::Opcode_8F()
    {
        ADC_A_r(Register::RegisterName::A);
    }
    // 0x90
    void Cpu::Opcode_90()
    {
        SUB_r(Register::RegisterName::B);
    }
    // 0x91
    void Cpu::Opcode_91()
    {
        SUB_r(Register::RegisterName::C);
    }
    // 0x92
    void Cpu::Opcode_92()
    {
        SUB_r(Register::RegisterName::D);
    }
    // 0x93
    void Cpu::Opcode_93()
    {
        SUB_r(Register::RegisterName::E);
    }
    // 0x94
    void Cpu::Opcode_94()
    {
        SUB_r(Register::RegisterName::H);
    }
    // 0x95
    void Cpu::Opcode_95()
    {
        SUB_r(Register::RegisterName::L);
    }
    // 0x96
    void Cpu::Opcode_96()
    {
        SUB_16();
    }
    // 0x97
    void Cpu::Opcode_97()
    {
        SUB_r(Register::RegisterName::A);
    }
    // 0x98
    void Cpu::Opcode_98()
    {
        SBC_r(Register::RegisterName::B);
    }
    // 0x99
    void Cpu::Opcode_99()
    {
        SBC_r(Register::RegisterName::C);
    }
    // 0x9A
    void Cpu::Opcode_9A()
    {
        SBC_r(Register::RegisterName::D);
    }
    // 0x9B
    void Cpu::Opcode_9B()
    {
        SBC_r(Register::RegisterName::E);
    }
    // 0x9C
    void Cpu::Opcode_9C()
    {
        SBC_r(Register::RegisterName::H);
    }
    // 0x9D
    void Cpu::Opcode_9D()
    {
        SBC_r(Register::RegisterName::L);
    }
    // 0x9E
    void Cpu::Opcode_9E()
    {
        SBC_16();
    }
    // 0x9F
    void Cpu::Opcode_9F()
    {
        SBC_r(Register::RegisterName::A);
    }

    // 0xA0
    void Cpu::Opcode_A0()
    {
        AND_r(Register::RegisterName::B);
    }
    // 0xA1
    void Cpu::Opcode_A1()
    {
        AND_r(Register::RegisterName::C);
    }
    // 0xA2
    void Cpu::Opcode_A2()
    {
        AND_r(Register::RegisterName::D);
    }
    // 0xA3
    void Cpu::Opcode_A3()
    {
        AND_r(Register::RegisterName::E);
    }
    // 0xA4
    void Cpu::Opcode_A4()
    {
        AND_r(Register::RegisterName::H);
    }
    // 0xA5
    void Cpu::Opcode_A5()
    {
        AND_r(Register::RegisterName::L);
    }
    // 0xA6
    void Cpu::Opcode_A6()
    {
        AND_16();
    }
    // 0xA7
    void Cpu::Opcode_A7()
    {
        AND_r(Register::RegisterName::A);
    }
    // 0xA8
    void Cpu::Opcode_A8()
    {
        XOR_r(Register::RegisterName::B);
    }
    // 0xA9
    void Cpu::Opcode_A9()
    {
        XOR_r(Register::RegisterName::C);
    }
    // 0xAA
    void Cpu::Opcode_AA()
    {
        XOR_r(Register::RegisterName::D);
    }
    // 0xAB
    void Cpu::Opcode_AB()
    {
        XOR_r(Register::RegisterName::E);
    }
    // 0xAC
    void Cpu::Opcode_AC()
    {
        XOR_r(Register::RegisterName::H);
    }
    // 0xAD
    void Cpu::Opcode_AD()
    {
        XOR_r(Register::RegisterName::L);
    }
    // 0xAE
    void Cpu::Opcode_AE()
    {
        XOR_16();
    }
    // 0xAF
    void Cpu::Opcode_AF()
    {
        XOR_r(Register::RegisterName::A);
    }
    // 0xB0
    void Cpu::Opcode_B0()
    {
        OR_r(Register::RegisterName::B);
    }
    // 0xB1
    void Cpu::Opcode_B1()
    {
        OR_r(Register::RegisterName::C);
    }
    // 0xB2
    void Cpu::Opcode_B2()
    {
        OR_r(Register::RegisterName::D);
    }
    // 0xB3
    void Cpu::Opcode_B3()
    {
        OR_r(Register::RegisterName::E);
    }
    // 0xB4
    void Cpu::Opcode_B4()
    {
        OR_r(Register::RegisterName::H);
    }
    // 0xB5
    void Cpu::Opcode_B5()
    {
        OR_r(Register::RegisterName::L);
    }
    // 0xB6
    void Cpu::Opcode_B6()
    {
        OR_16();
    }
    // 0xB7
    void Cpu::Opcode_B7()
    {
        OR_r(Register::RegisterName::A);
    }
    // 0xB8
    void Cpu::Opcode_B8()
    {
        CP_r(Register::RegisterName::B);
    }
    // 0xB9
    void Cpu::Opcode_B9()
    {
        CP_r(Register::RegisterName::C);
    }
    // 0xBA
    void Cpu::Opcode_BA()
    {
        CP_r(Register::RegisterName::D);
    }
    // 0xBB
    void Cpu::Opcode_BB()
    {
        CP_r(Register::RegisterName::E);
    }
    // 0xBC
    void Cpu::Opcode_BC()
    {
        CP_r(Register::RegisterName::H);
    }
    // 0xBD
    void Cpu::Opcode_BD()
    {
        CP_r(Register::RegisterName::L);
    }
    // 0xBE
    void Cpu::Opcode_BE()
    {
        CP_16();
    }
    // 0xBF
    void Cpu::Opcode_BF()
    {
        CP_r(Register::RegisterName::A);
    }

    // 0xC0
    void Cpu::Opcode_C0()
    {
        RET_c(Register::FlagName::NZ);
        _pcManuallySet = true;
    }
    // 0xC1
    void Cpu::Opcode_C1()
    {
        POP(Register::RegisterPairName::BC);
    }
    // 0xC2
    void Cpu::Opcode_C2()
    {
        JP_c_16(Register::FlagName::NZ, next2bytes(_registers->PC + 1));
        _pcManuallySet = true;
    }
    // 0xC3
    void Cpu::Opcode_C3()
    {
        JP_16(_registers->PC + 1);
        _pcManuallySet = true;
    }
    // 0xC4
    void Cpu::Opcode_C4()
    {
        CALL_c(Register::FlagName::NZ);
        _pcManuallySet = true;
    }
    // 0xC5
    void Cpu::Opcode_C5()
    {
        PUSH(Register::RegisterPairName::BC);
    }
    // 0xC6
    void Cpu::Opcode_C6()
    {
        ADD_A_n(_mmu->readRam(_registers->PC + 1));
    }
    // 0xC7
    void Cpu::Opcode_C7()
    {
        RST_p(0x00);
        _pcManuallySet = true;
    }
    // 0xC8
    void Cpu::Opcode_C8()
    {
        RET_c(Register::FlagName::Z);
        _pcManuallySet = true;
    }
    // 0xC9
    void Cpu::Opcode_C9()
    {
        RET();
        _pcManuallySet = true;
    }
    // 0xCA
    void Cpu::Opcode_CA()
    {
        JP_c_16(Register::FlagName::Z, next2bytes(_registers->PC + 1));
        _pcManuallySet = true;
    }
    // 0xCB is the extended opcode prefix and handled separately.
    // 0xCC
    void Cpu::Opcode_CC()
    {
        CALL_c(Register::FlagName::Z);
        _pcManuallySet = true;
    }
    // 0xCD
    void Cpu::Opcode_CD()
    {
        CALL();
        _pcManuallySet = true;
    }
    // 0xCE
    void Cpu::Opcode_CE()
    {
        ADC_A_n(_mmu->readRam(_registers->PC + 1));
    }
    // 0xCF
    void Cpu::Opcode_CF()
    {
        RST_p(0x08);
        _pcManuallySet = true;
    }
    // 0xD0
    void Cpu::Opcode_D0()
    {
        RET_c(Register::FlagName::NC);
        _pcManuallySet = true;
    }
    // 0xD1
    void Cpu::Opcode_D1()
    {
        POP(Register::RegisterPairName::DE);
    }
    // 0xD2
    void Cpu::Opcode_D2()
    {
        JP_c_16(Register::FlagName::NC, next2bytes(_registers->PC + 1));
        _pcManuallySet = true;
    }
    // 0xD4
    void Cpu::Opcode_D4()
    {
        CALL_c(Register::FlagName::NC);
        _pcManuallySet = true;
    }
    // 0xD5
    void Cpu::Opcode_D5()
    {
        PUSH(Register::RegisterPairName::DE);
    }
    // 0xD6
    void Cpu::Opcode_D6()
    {
        SUB_n(_mmu->readRam(_registers->PC + 1));
    }
    // 0xD7
    void Cpu::Opcode_D7()
    {
        RST_p(0x10);
        _pcManuallySet = true;
    }
    // 0xD8
    void Cpu::Opcode_D8()
    {
        RET_c(Register::FlagName::C);
        _pcManuallySet = true;
    }
    // 0xD9
    void Cpu::Opcode_D9()
    {
        RETI();
        _pcManuallySet = true;
    }
    // 0xDA
    void Cpu::Opcode_DA()
    {
        JP_c_16(Register::FlagName::C, next2bytes(_registers->PC + 1));
        _pcManuallySet = true;
    }
    // 0xDC
    void Cpu::Opcode_DC()
    {
        CALL_c(Register::FlagName::C);
        _pcManuallySet = true;
    }
    // 0xDE
    void Cpu::Opcode_DE()
    {
        SBC_n(_mmu->readRam(_registers->PC + 1));
    }
    // 0xDF
    void Cpu::Opcode_DF()
    {
        RST_p(0x18);
        _pcManuallySet = true;
    }

    // 0xE0
    void Cpu::Opcode_E0()
    {
        _mmu->writeRam(_mmu->readRam(_registers->PC + 1) + 0xFF00, _registers->AF.getLeftRegister());
    }
    // 0xE1
    void Cpu::Opcode_E1()
    {
        POP(Register::RegisterPairName::HL);
    }
    // 0xE2
    void Cpu::Opcode_E2()
    {
        _mmu->writeRam(0xFF00 + _registers->BC.getRightRegister(), _registers->AF.getLeftRegister());
    }
    // 0xE5
    void Cpu::Opcode_E5()
    {
        PUSH(Register::RegisterPairName::HL);
    }
    // 0xE6
    void Cpu::Opcode_E6()
    {
        AND_n(_mmu->readRam(_registers->PC + 1));
    }
    // 0xE7
    void Cpu::Opcode_E7()
    {
        RST_p(0x20);
        _pcManuallySet = true;
    }
    // 0xE8
    void Cpu::Opcode_E8()
    {
        ADD_SP_n();
    }
    // 0xE9
    void Cpu::Opcode_E9()
    {
        JP_16();
        _pcManuallySet = true;
    }
    // 0xEA
    void Cpu::Opcode_EA()
    {
        LD_16_r(next2bytes(_registers->PC + 1), Register::RegisterName::A);
    }
    // 0xEE
    void Cpu::Opcode_EE()
    {
        XOR_n(_mmu->readRam(_registers->PC + 1));
    }
    // 0xEF
    void Cpu::Opcode_EF()
    {
        RST_p(0x28);
        _pcManuallySet = true;
    }
    // 0xF0
    void Cpu::Opcode_F0()
    {
        _registers->AF.setLeftRegister(_mmu->readRam(_mmu->readRam(_registers->PC + 1) + 0xFF00));
    }
    // 0xF1
    void Cpu::Opcode_F1()
    {
        POP(Register::RegisterPairName::AF);
    }
    // 0xF2
    void Cpu::Opcode_F2()
    {
        _registers->AF.setLeftRegister(_mmu->readRam(_registers->BC.getRightRegister() + 0xFF00));
    }
    // 0xF3
    void Cpu::Opcode_F3()
    {
        DI();
    }
    // 0xF5
    void Cpu::Opcode_F5()
    {
        PUSH(Register::RegisterPairName::AF);
    }
    // 0xF6
    void Cpu::Opcode_F6()
    {
        OR_n(_mmu->readRam(_registers->PC + 1));
    }
    // 0xF7
    void Cpu::Opcode_F7()
    {
        RST_p(0x30);
        _pcManuallySet = true;
    }
    // 0xF8
    void Cpu::Opcode_F8()
    {
        LD_HL_SP_n();
    }
    // 0xF9
    void Cpu::Opcode_F9()
    {
        _registers->SP = _registers->HL.get();
    }
    // 0xFA
    void Cpu::Opcode_FA()
    {
        LD_r_16(next2bytes(_registers->PC + 1), Register::RegisterName::A);
    }
    // 0xFB
    void Cpu::Opcode_FB()
    {
        EI();
    }
    // 0xFC
    void Cpu::Opcode_FC()
    {
        CALL_c(Register::FlagName::C);
        _pcManuallySet = true;
    }
    // 0xFD
    void Cpu::Opcode_FD()
    {
        // Not implemented – placeholder.
        NOP();
    }
    // 0xFE
    void Cpu::Opcode_FE()
    {
        CP_n(_mmu->readRam(_registers->PC + 1));
    }
    // 0xFF
    void Cpu::Opcode_FF()
    {
        RST_p(0x38);
        _pcManuallySet = true;
    }
}

#endif