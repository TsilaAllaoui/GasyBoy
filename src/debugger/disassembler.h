#ifndef __DISASSEMBLER_H__
#define __DISASSEMBLER_H__

#include "cartridge.h"
#include <string>
#include <unordered_map>
#include <mutex>

namespace gasyboy
{
    // structure for an opcode
    struct Opcode
    {
        int numberOfBytes;
        uint8_t byte;
        std::string mnemonic;
        std::vector<uint8_t> operands;
    };

    class Disassembler
    {
        std::vector<std::vector<uint8_t>> _romBanks;
        // Opcodes tables
        std::vector<Opcode> cbOpcodeTable;
        std::vector<Opcode> opcodeTable;

    public:
        Disassembler(Cartridge &cartridge);

        std::vector<Opcode> disassembledRom;

        std::mutex mutex;

        // Disassemble current cartridge rom
        void disassemble();
    };
}

#endif