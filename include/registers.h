#ifndef _REGISTERS_H_
#define _REGISTERS_H_

#include "register.h"
#include <map>

namespace gasyboy
{
    class Registers
    {
        std::map<std::string, Register> _registersMap;

    public:
        // Constructors
        Registers();

        // Get the corresponding register
        Register getRegister(const std::string &reg);
        uint8_t getRegister(const char &reg);

        // Set corresponding register
        void setRegister(const std::string &reg, const uint16_t &value);
        void setRegister(const char &reg, const uint8_t &value);

        // A special register for A && F(flags)
        SpecialRegister AF;

        // 3 16bits registers fromed of two 8bits registers each
        Register BC, DE, HL;

        // The Program Counter
        uint16_t PC;

        // The Stack Pointer
        uint16_t SP;
    };
}

#endif