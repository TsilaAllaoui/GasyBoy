#ifndef _REGISTERS_H_
#define _REGISTERS_H_

#include "register.h"
#include "mmu.h"
#include <map>

namespace gasyboy
{
    class Registers
    {
        Mmu &_mmu;

        std::map<std::string, Register> _registersMap;

        // boolean to check interrupted state of the cpu
        bool _interruptEnabled;

        // boolean to check if cpu is halted
        bool _halted;

    public:
        // Constructors
        Registers(Mmu &mmu, const bool &bootBios = true);

        // Get the corresponding register
        Register getRegister(const std::string &reg);
        uint8_t getRegister(const char &reg);

        // Set corresponding register
        void setRegister(const std::string &reg, const uint16_t &value);
        void setRegister(const char &reg, const uint8_t &value);

        // Push PC to Sp and set PC to given address
        void pushSP(const uint16_t &address);

        // A special register for A && F(flags)
        SpecialRegister AF;

        // 3 16bits registers fromed of two 8bits registers each
        Register BC, DE, HL;

        // The Program Counter
        uint16_t PC;

        // The Stack Pointer
        uint16_t SP;

        // Set/Get _interruptEnabled
        void setInterruptEnabled(const bool &value);
        bool getInterruptEnabled();

        // Set/Get _halted
        void setHalted(const bool &value);
        bool getHalted();
    };
}

#endif