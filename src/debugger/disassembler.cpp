#include "disassembler.h"
#include <iostream>
#include <vector>
#include <iomanip>
#include <exception>
#include <stdexcept>

namespace gasyboy
{
    Disassembler::Disassembler(Cartridge &cartridge)
    {

        opcodeTable = {
            {1, 0x00, "NOP"},
            {3, 0x01, "LD BC, d16"},
            {1, 0x02, "LD (BC), A"},
            {1, 0x03, "INC BC"},
            {1, 0x04, "INC B"},
            {1, 0x05, "DEC B"},
            {2, 0x06, "LD B, d8"},
            {1, 0x07, "RLCA"},
            {3, 0x08, "LD (a16), SP"},
            {1, 0x09, "ADD HL, BC"},
            {1, 0x0A, "LD A, (BC)"},
            {1, 0x0B, "DEC BC"},
            {1, 0x0C, "INC C"},
            {1, 0x0D, "DEC C"},
            {2, 0x0E, "LD C, d8"},
            {1, 0x0F, "RRCA"},
            {3, 0x10, "STOP"},
            {3, 0x11, "LD DE, d16"},
            {1, 0x12, "LD (DE), A"},
            {1, 0x13, "INC DE"},
            {1, 0x14, "INC D"},
            {1, 0x15, "DEC D"},
            {2, 0x16, "LD D, d8"},
            {1, 0x17, "RLA"},
            {2, 0x18, "JR r8"},
            {1, 0x19, "ADD HL, DE"},
            {1, 0x1A, "LD A, (DE)"},
            {1, 0x1B, "DEC DE"},
            {1, 0x1C, "INC E"},
            {1, 0x1D, "DEC E"},
            {2, 0x1E, "LD E, d8"},
            {1, 0x1F, "RRA"},
            {3, 0x20, "JR NZ, r8"},
            {3, 0x21, "LD HL, d16"},
            {1, 0x22, "LD (HL+), A"},
            {1, 0x23, "INC HL"},
            {1, 0x24, "INC H"},
            {1, 0x25, "DEC H"},
            {2, 0x26, "LD H, d8"},
            {1, 0x27, "DAA"},
            {2, 0x28, "JR Z, r8"},
            {1, 0x29, "ADD HL, HL"},
            {1, 0x2A, "LD A, (HL+)"},
            {1, 0x2B, "DEC HL"},
            {1, 0x2C, "INC L"},
            {1, 0x2D, "DEC L"},
            {2, 0x2E, "LD L, d8"},
            {1, 0x2F, "CPL"},
            {2, 0x30, "JR NC, r8"},
            {3, 0x31, "LD SP, d16"},
            {1, 0x32, "LD (HL-), A"},
            {1, 0x33, "INC SP"},
            {1, 0x34, "INC (HL)"},
            {1, 0x35, "DEC (HL)"},
            {2, 0x36, "LD (HL), d8"},
            {1, 0x37, "SCF"},
            {2, 0x38, "JR C, r8"},
            {1, 0x39, "ADD HL, SP"},
            {1, 0x3A, "LD A, (HL-)"},
            {1, 0x3B, "DEC SP"},
            {1, 0x3C, "INC A"},
            {1, 0x3D, "DEC A"},
            {2, 0x3E, "LD A, d8"},
            {1, 0x3F, "CCF"},
            {1, 0x40, "LD B, B"},
            {1, 0x41, "LD B, C"},
            {1, 0x42, "LD B, D"},
            {1, 0x43, "LD B, E"},
            {1, 0x44, "LD B, H"},
            {1, 0x45, "LD B, L"},
            {1, 0x46, "LD B, (HL)"},
            {1, 0x47, "LD B, A"},
            {1, 0x48, "LD C, B"},
            {1, 0x49, "LD C, C"},
            {1, 0x4A, "LD C, D"},
            {1, 0x4B, "LD C, E"},
            {1, 0x4C, "LD C, H"},
            {1, 0x4D, "LD C, L"},
            {1, 0x4E, "LD C, (HL)"},
            {1, 0x4F, "LD C, A"},
            {1, 0x50, "LD D, B"},
            {1, 0x51, "LD D, C"},
            {1, 0x52, "LD D, D"},
            {1, 0x53, "LD D, E"},
            {1, 0x54, "LD D, H"},
            {1, 0x55, "LD D, L"},
            {1, 0x56, "LD D, (HL)"},
            {1, 0x57, "LD D, A"},
            {1, 0x58, "LD E, B"},
            {1, 0x59, "LD E, C"},
            {1, 0x5A, "LD E, D"},
            {1, 0x5B, "LD E, E"},
            {1, 0x5C, "LD E, H"},
            {1, 0x5D, "LD E, L"},
            {1, 0x5E, "LD E, (HL)"},
            {1, 0x5F, "LD E, A"},
            {1, 0x60, "LD H, B"},
            {1, 0x61, "LD H, C"},
            {1, 0x62, "LD H, D"},
            {1, 0x63, "LD H, E"},
            {1, 0x64, "LD H, H"},
            {1, 0x65, "LD H, L"},
            {1, 0x66, "LD H, (HL)"},
            {1, 0x67, "LD H, A"},
            {1, 0x68, "LD L, B"},
            {1, 0x69, "LD L, C"},
            {1, 0x6A, "LD L, D"},
            {1, 0x6B, "LD L, E"},
            {1, 0x6C, "LD L, H"},
            {1, 0x6D, "LD L, L"},
            {1, 0x6E, "LD L, (HL)"},
            {1, 0x6F, "LD L, A"},
            {1, 0x70, "LD (HL), B"},
            {1, 0x71, "LD (HL), C"},
            {1, 0x72, "LD (HL), D"},
            {1, 0x73, "LD (HL), E"},
            {1, 0x74, "LD (HL), H"},
            {1, 0x75, "LD (HL), L"},
            {1, 0x76, "HALT"},
            {1, 0x77, "LD (HL), A"},
            {1, 0x78, "LD A, B"},
            {1, 0x79, "LD A, C"},
            {1, 0x7A, "LD A, D"},
            {1, 0x7B, "LD A, E"},
            {1, 0x7C, "LD A, H"},
            {1, 0x7D, "LD A, L"},
            {1, 0x7E, "LD A, (HL)"},
            {1, 0x7F, "LD A, A"},
            {1, 0x80, "ADD A, B"},
            {1, 0x81, "ADD A, C"},
            {1, 0x82, "ADD A, D"},
            {1, 0x83, "ADD A, E"},
            {1, 0x84, "ADD A, H"},
            {1, 0x85, "ADD A, L"},
            {1, 0x86, "ADD A, (HL)"},
            {1, 0x87, "ADD A, A"},
            {1, 0x88, "ADC A, B"},
            {1, 0x89, "ADC A, C"},
            {1, 0x8A, "ADC A, D"},
            {1, 0x8B, "ADC A, E"},
            {1, 0x8C, "ADC A, H"},
            {1, 0x8D, "ADC A, L"},
            {1, 0x8E, "ADC A, (HL)"},
            {1, 0x8F, "ADC A, A"},
            {1, 0x90, "SUB B"},
            {1, 0x91, "SUB C"},
            {1, 0x92, "SUB D"},
            {1, 0x93, "SUB E"},
            {1, 0x94, "SUB H"},
            {1, 0x95, "SUB L"},
            {1, 0x96, "SUB (HL)"},
            {1, 0x97, "SUB A"},
            {1, 0x98, "SBC A, B"},
            {1, 0x99, "SBC A, C"},
            {1, 0x9A, "SBC A, D"},
            {1, 0x9B, "SBC A, E"},
            {1, 0x9C, "SBC A, H"},
            {1, 0x9D, "SBC A, L"},
            {1, 0x9E, "SBC A, (HL)"},
            {1, 0x9F, "SBC A, A"},
            {1, 0xA0, "AND B"},
            {1, 0xA1, "AND C"},
            {1, 0xA2, "AND D"},
            {1, 0xA3, "AND E"},
            {1, 0xA4, "AND H"},
            {1, 0xA5, "AND L"},
            {1, 0xA6, "AND (HL)"},
            {1, 0xA7, "AND A"},
            {1, 0xA8, "XOR B"},
            {1, 0xA9, "XOR C"},
            {1, 0xAA, "XOR D"},
            {1, 0xAB, "XOR E"},
            {1, 0xAC, "XOR H"},
            {1, 0xAD, "XOR L"},
            {1, 0xAE, "XOR (HL)"},
            {1, 0xAF, "XOR A"},
            {1, 0xB0, "OR B"},
            {1, 0xB1, "OR C"},
            {1, 0xB2, "OR D"},
            {1, 0xB3, "OR E"},
            {1, 0xB4, "OR H"},
            {1, 0xB5, "OR L"},
            {1, 0xB6, "OR (HL)"},
            {1, 0xB7, "OR A"},
            {1, 0xB8, "CP B"},
            {1, 0xB9, "CP C"},
            {1, 0xBA, "CP D"},
            {1, 0xBB, "CP E"},
            {1, 0xBC, "CP H"},
            {1, 0xBD, "CP L"},
            {1, 0xBE, "CP (HL)"},
            {1, 0xBF, "CP A"},
            {1, 0xC0, "RET NZ"},
            {3, 0xC1, "POP BC"},
            {3, 0xC2, "JP NZ, a16"},
            {3, 0xC3, "JP a16"},
            {3, 0xC4, "CALL NZ, a16"},
            {3, 0xC5, "PUSH BC"},
            {2, 0xC6, "ADD A, d8"},
            {1, 0xC7, "RST 00H"},
            {1, 0xC8, "RET Z"},
            {1, 0xC9, "RET"},
            {3, 0xCA, "JP Z, a16"},
            {2, 0xCB, "PREFIX CB"},
            {3, 0xCC, "CALL Z, a16"},
            {3, 0xCD, "CALL a16"},
            {2, 0xCE, "ADC A, d8"},
            {1, 0xCF, "RST 08H"},
            {1, 0xD0, "RET NC"},
            {3, 0xD1, "POP DE"},
            {3, 0xD2, "JP NC, a16"},
            {1, 0xD3, "ILLEGAL"},
            {3, 0xD4, "CALL NC, a16"},
            {3, 0xD5, "PUSH DE"},
            {2, 0xD6, "SUB d8"},
            {1, 0xD7, "RST 10H"},
            {1, 0xD8, "RET C"},
            {1, 0xD9, "RETI"},
            {3, 0xDA, "JP C, a16"},
            {1, 0xDB, "ILLEGAL"},
            {3, 0xDC, "CALL C, a16"},
            {1, 0xDD, "ILLEGAL"},
            {2, 0xDE, "SBC A, d8"},
            {1, 0xDF, "RST 18H"},
            {2, 0xE0, "LDH (a8), A"},
            {3, 0xE1, "POP HL"},
            {2, 0xE2, "LD (C), A"},
            {1, 0xE3, "ILLEGAL"},
            {1, 0xE4, "ILLEGAL"},
            {3, 0xE5, "PUSH HL"},
            {2, 0xE6, "AND d8"},
            {1, 0xE7, "RST 20H"},
            {2, 0xE8, "ADD SP, r8"},
            {3, 0xE9, "JP (HL)"},
            {3, 0xEA, "LD (a16), A"},
            {1, 0xEB, "ILLEGAL"},
            {1, 0xEC, "ILLEGAL"},
            {1, 0xED, "ILLEGAL"},
            {2, 0xEE, "XOR d8"},
            {1, 0xEF, "RST 28H"},
            {2, 0xF0, "LDH A, (a8)"},
            {3, 0xF1, "POP AF"},
            {2, 0xF2, "LD A, (C)"},
            {1, 0xF3, "DI"},
            {1, 0xF4, "ILLEGAL"},
            {3, 0xF5, "PUSH AF"},
            {2, 0xF6, "OR d8"},
            {1, 0xF7, "RST 30H"},
            {2, 0xF8, "LD HL, SP+r8"},
            {3, 0xF9, "LD SP, HL"},
            {3, 0xFA, "LD A, (a16)"},
            {1, 0xFB, "EI"},
            {1, 0xFC, "ILLEGAL"},
            {1, 0xFD, "ILLEGAL"},
            {2, 0xFE, "CP d8"},
            {1, 0xFF, "RST 38H"},
        };

        // CB-prefixed opcodes
        cbOpcodeTable = {
            {1, 0x00, "RLC B"},
            {1, 0x01, "RLC C"},
            {1, 0x02, "RLC D"},
            {1, 0x03, "RLC E"},
            {1, 0x04, "RLC H"},
            {1, 0x05, "RLC L"},
            {1, 0x06, "RLC (HL)"},
            {1, 0x07, "RLC A"},
            {1, 0x08, "RRC B"},
            {1, 0x09, "RRC C"},
            {1, 0x0A, "RRC D"},
            {1, 0x0B, "RRC E"},
            {1, 0x0C, "RRC H"},
            {1, 0x0D, "RRC L"},
            {1, 0x0E, "RRC (HL)"},
            {1, 0x0F, "RRC A"},
            {2, 0x10, "RL B"},
            {2, 0x11, "RL C"},
            {2, 0x12, "RL D"},
            {2, 0x13, "RL E"},
            {2, 0x14, "RL H"},
            {2, 0x15, "RL L"},
            {2, 0x16, "RL (HL)"},
            {2, 0x17, "RL A"},
            {2, 0x18, "RR B"},
            {2, 0x19, "RR C"},
            {2, 0x1A, "RR D"},
            {2, 0x1B, "RR E"},
            {2, 0x1C, "RR H"},
            {2, 0x1D, "RR L"},
            {2, 0x1E, "RR (HL)"},
            {2, 0x1F, "RR A"},
            {2, 0x20, "SLA B"},
            {2, 0x21, "SLA C"},
            {2, 0x22, "SLA D"},
            {2, 0x23, "SLA E"},
            {2, 0x24, "SLA H"},
            {2, 0x25, "SLA L"},
            {2, 0x26, "SLA (HL)"},
            {2, 0x27, "SLA A"},
            {2, 0x28, "SRA B"},
            {2, 0x29, "SRA C"},
            {2, 0x2A, "SRA D"},
            {2, 0x2B, "SRA E"},
            {2, 0x2C, "SRA H"},
            {2, 0x2D, "SRA L"},
            {2, 0x2E, "SRA (HL)"},
            {2, 0x2F, "SRA A"},
            {2, 0x30, "SWAP B"},
            {2, 0x31, "SWAP C"},
            {2, 0x32, "SWAP D"},
            {2, 0x33, "SWAP E"},
            {2, 0x34, "SWAP H"},
            {2, 0x35, "SWAP L"},
            {2, 0x36, "SWAP (HL)"},
            {2, 0x37, "SWAP A"},
            {2, 0x38, "SRL B"},
            {2, 0x39, "SRL C"},
            {2, 0x3A, "SRL D"},
            {2, 0x3B, "SRL E"},
            {2, 0x3C, "SRL H"},
            {2, 0x3D, "SRL L"},
            {2, 0x3E, "SRL (HL)"},
            {2, 0x3F, "SRL A"},
            {2, 0x40, "BIT 0, B"},
            {2, 0x41, "BIT 0, C"},
            {2, 0x42, "BIT 0, D"},
            {2, 0x43, "BIT 0, E"},
            {2, 0x44, "BIT 0, H"},
            {2, 0x45, "BIT 0, L"},
            {2, 0x46, "BIT 0, (HL)"},
            {2, 0x47, "BIT 0, A"},
            {2, 0x48, "BIT 1, B"},
            {2, 0x49, "BIT 1, C"},
            {2, 0x4A, "BIT 1, D"},
            {2, 0x4B, "BIT 1, E"},
            {2, 0x4C, "BIT 1, H"},
            {2, 0x4D, "BIT 1, L"},
            {2, 0x4E, "BIT 1, (HL)"},
            {2, 0x4F, "BIT 1, A"},
            {2, 0x50, "BIT 2, B"},
            {2, 0x51, "BIT 2, C"},
            {2, 0x52, "BIT 2, D"},
            {2, 0x53, "BIT 2, E"},
            {2, 0x54, "BIT 2, H"},
            {2, 0x55, "BIT 2, L"},
            {2, 0x56, "BIT 2, (HL)"},
            {2, 0x57, "BIT 2, A"},
            {2, 0x58, "BIT 3, B"},
            {2, 0x59, "BIT 3, C"},
            {2, 0x5A, "BIT 3, D"},
            {2, 0x5B, "BIT 3, E"},
            {2, 0x5C, "BIT 3, H"},
            {2, 0x5D, "BIT 3, L"},
            {2, 0x5E, "BIT 3, (HL)"},
            {2, 0x5F, "BIT 3, A"},
            {2, 0x60, "BIT 4, B"},
            {2, 0x61, "BIT 4, C"},
            {2, 0x62, "BIT 4, D"},
            {2, 0x63, "BIT 4, E"},
            {2, 0x64, "BIT 4, H"},
            {2, 0x65, "BIT 4, L"},
            {2, 0x66, "BIT 4, (HL)"},
            {2, 0x67, "BIT 4, A"},
            {2, 0x68, "BIT 5, B"},
            {2, 0x69, "BIT 5, C"},
            {2, 0x6A, "BIT 5, D"},
            {2, 0x6B, "BIT 5, E"},
            {2, 0x6C, "BIT 5, H"},
            {2, 0x6D, "BIT 5, L"},
            {2, 0x6E, "BIT 5, (HL)"},
            {2, 0x6F, "BIT 5, A"},
            {2, 0x70, "BIT 6, B"},
            {2, 0x71, "BIT 6, C"},
            {2, 0x72, "BIT 6, D"},
            {2, 0x73, "BIT 6, E"},
            {2, 0x74, "BIT 6, H"},
            {2, 0x75, "BIT 6, L"},
            {2, 0x76, "BIT 6, (HL)"},
            {2, 0x77, "BIT 6, A"},
            {2, 0x78, "BIT 7, B"},
            {2, 0x79, "BIT 7, C"},
            {2, 0x7A, "BIT 7, D"},
            {2, 0x7B, "BIT 7, E"},
            {2, 0x7C, "BIT 7, H"},
            {2, 0x7D, "BIT 7, L"},
            {2, 0x7E, "BIT 7, (HL)"},
            {2, 0x7F, "BIT 7, A"},
            {2, 0x80, "RES 0, B"},
            {2, 0x81, "RES 0, C"},
            {2, 0x82, "RES 0, D"},
            {2, 0x83, "RES 0, E"},
            {2, 0x84, "RES 0, H"},
            {2, 0x85, "RES 0, L"},
            {2, 0x86, "RES 0, (HL)"},
            {2, 0x87, "RES 0, A"},
            {2, 0x88, "RES 1, B"},
            {2, 0x89, "RES 1, C"},
            {2, 0x8A, "RES 1, D"},
            {2, 0x8B, "RES 1, E"},
            {2, 0x8C, "RES 1, H"},
            {2, 0x8D, "RES 1, L"},
            {2, 0x8E, "RES 1, (HL)"},
            {2, 0x8F, "RES 1, A"},
            {2, 0x90, "RES 2, B"},
            {2, 0x91, "RES 2, C"},
            {2, 0x92, "RES 2, D"},
            {2, 0x93, "RES 2, E"},
            {2, 0x94, "RES 2, H"},
            {2, 0x95, "RES 2, L"},
            {2, 0x96, "RES 2, (HL)"},
            {2, 0x97, "RES 2, A"},
            {2, 0x98, "RES 3, B"},
            {2, 0x99, "RES 3, C"},
            {2, 0x9A, "RES 3, D"},
            {2, 0x9B, "RES 3, E"},
            {2, 0x9C, "RES 3, H"},
            {2, 0x9D, "RES 3, L"},
            {2, 0x9E, "RES 3, (HL)"},
            {2, 0x9F, "RES 3, A"},
            {2, 0xA0, "RES 4, B"},
            {2, 0xA1, "RES 4, C"},
            {2, 0xA2, "RES 4, D"},
            {2, 0xA3, "RES 4, E"},
            {2, 0xA4, "RES 4, H"},
            {2, 0xA5, "RES 4, L"},
            {2, 0xA6, "RES 4, (HL)"},
            {2, 0xA7, "RES 4, A"},
            {2, 0xA8, "RES 5, B"},
            {2, 0xA9, "RES 5, C"},
            {2, 0xAA, "RES 5, D"},
            {2, 0xAB, "RES 5, E"},
            {2, 0xAC, "RES 5, H"},
            {2, 0xAD, "RES 5, L"},
            {2, 0xAE, "RES 5, (HL)"},
            {2, 0xAF, "RES 5, A"},
            {2, 0xB0, "RES 6, B"},
            {2, 0xB1, "RES 6, C"},
            {2, 0xB2, "RES 6, D"},
            {2, 0xB3, "RES 6, E"},
            {2, 0xB4, "RES 6, H"},
            {2, 0xB5, "RES 6, L"},
            {2, 0xB6, "RES 6, (HL)"},
            {2, 0xB7, "RES 6, A"},
            {2, 0xB8, "RES 7, B"},
            {2, 0xB9, "RES 7, C"},
            {2, 0xBA, "RES 7, D"},
            {2, 0xBB, "RES 7, E"},
            {2, 0xBC, "RES 7, H"},
            {2, 0xBD, "RES 7, L"},
            {2, 0xBE, "RES 7, (HL)"},
            {2, 0xBF, "RES 7, A"},
            {2, 0xC0, "SET 0, B"},
            {2, 0xC1, "SET 0, C"},
            {2, 0xC2, "SET 0, D"},
            {2, 0xC3, "SET 0, E"},
            {2, 0xC4, "SET 0, H"},
            {2, 0xC5, "SET 0, L"},
            {2, 0xC6, "SET 0, (HL)"},
            {2, 0xC7, "SET 0, A"},
            {2, 0xC8, "SET 1, B"},
            {2, 0xC9, "SET 1, C"},
            {2, 0xCA, "SET 1, D"},
            {2, 0xCB, "SET 1, E"},
            {2, 0xCC, "SET 1, H"},
            {2, 0xCD, "SET 1, L"},
            {2, 0xCE, "SET 1, (HL)"},
            {2, 0xCF, "SET 1, A"},
            {2, 0xD0, "SET 2, B"},
            {2, 0xD1, "SET 2, C"},
            {2, 0xD2, "SET 2, D"},
            {2, 0xD3, "SET 2, E"},
            {2, 0xD4, "SET 2, H"},
            {2, 0xD5, "SET 2, L"},
            {2, 0xD6, "SET 2, (HL)"},
            {2, 0xD7, "SET 2, A"},
            {2, 0xD8, "SET 3, B"},
            {2, 0xD9, "SET 3, C"},
            {2, 0xDA, "SET 3, D"},
            {2, 0xDB, "SET 3, E"},
            {2, 0xDC, "SET 3, H"},
            {2, 0xDD, "SET 3, L"},
            {2, 0xDE, "SET 3, (HL)"},
            {2, 0xDF, "SET 3, A"},
            {2, 0xE0, "SET 4, B"},
            {2, 0xE1, "SET 4, C"},
            {2, 0xE2, "SET 4, D"},
            {2, 0xE3, "SET 4, E"},
            {2, 0xE4, "SET 4, H"},
            {2, 0xE5, "SET 4, L"},
            {2, 0xE6, "SET 4, (HL)"},
            {2, 0xE7, "SET 4, A"},
            {2, 0xE8, "SET 5, B"},
            {2, 0xE9, "SET 5, C"},
            {2, 0xEA, "SET 5, D"},
            {2, 0xEB, "SET 5, E"},
            {2, 0xEC, "SET 5, H"},
            {2, 0xED, "SET 5, L"},
            {2, 0xEE, "SET 5, (HL)"},
            {2, 0xEF, "SET 5, A"},
            {2, 0xF0, "SET 6, B"},
            {2, 0xF1, "SET 6, C"},
            {2, 0xF2, "SET 6, D"},
            {2, 0xF3, "SET 6, E"},
            {2, 0xF4, "SET 6, H"},
            {2, 0xF5, "SET 6, L"},
            {2, 0xF6, "SET 6, (HL)"},
            {2, 0xF7, "SET 6, A"},
            {2, 0xF8, "SET 7, B"},
            {2, 0xF9, "SET 7, C"},
            {2, 0xFA, "SET 7, D"},
            {2, 0xFB, "SET 7, E"},
            {2, 0xFC, "SET 7, H"},
            {2, 0xFD, "SET 7, L"},
            {2, 0xFE, "SET 7, (HL)"},
            {2, 0xFF, "SET 7, A"},
        };

        _romBanks = cartridge.getRomBanks();
    }

    void Disassembler::disassemble()
    {
        // Iterate over each bank
        for (size_t bankIndex = 0; bankIndex < _romBanks.size(); ++bankIndex)
        {
            // Iterate through each byte in the bank
            const auto &bank = _romBanks[bankIndex];
            size_t pc = 0;
            while (pc < bank.size())
            {
                uint8_t byte = bank[pc];
                if (byte == 0xCB)
                {
                    Opcode opcode = cbOpcodeTable[byte];
                    opcode.operands.emplace_back(bank[pc + 1]);
                    opcode.operands.emplace_back(bank[pc + 2]);
                    pc += 2;
                    disassembledRom.emplace_back(opcode);
                }
                else if (byte < opcodeTable.size())
                {
                    Opcode opcode = opcodeTable[byte];
                    for (int i = 1; i < opcode.numberOfBytes; i++)
                    {
                        opcode.operands.emplace_back(bank[pc + i]);
                    }
                    pc += opcode.numberOfBytes;
                    disassembledRom.emplace_back(opcode);
                }
                else
                {
                    Opcode opcode = {1, byte, "UNKNOWN", {}};
                    disassembledRom.emplace_back(opcode);
                    pc += 1; // Continue to the next byte
                }
            }
        }
    }
}
