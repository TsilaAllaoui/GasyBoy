#ifndef _INTERRUPT_MANAGER_H_
#define _INTERRUPT_MANAGER_H_

#include "registers.h"
#include "mmu.h"
#include <map>

namespace gasyboy
{
    class InterruptManager
    {
        bool _masterInterrupt;
        Mmu &_mmu;
        Registers &_registers;

    public:
        InterruptManager();
        ~InterruptManager() = default;

        enum class InterruptType
        {
            VBlank = 0,  // V-Blank Type
            LCDStat = 1, // LCD Status Type
            Timer = 2,   // Timer Type
            Serial = 3,  // Serial Type
            Joypad = 4   // Joypad Type
        };

        enum class InterruptAddress
        {
            VBlank = 0x40,  // V-Blank Interrupt
            LCDStat = 0x48, // LCD Status Interrupt
            Timer = 0x50,   // Timer Interrupt
            Serial = 0x60   // Serial Interrupt
        };

        void handleInterrupts();

        void requestInterrupt(const InterruptType &interrupt);

        void serviceInterrupt(const InterruptType &interrupt);

        bool isMasterInterruptEnabled();

        void setMasterInterrupt(const bool &value);

        void reset();

    private:
        std::map<InterruptType, InterruptAddress> _interruptAddressMap;
    };
}

#endif