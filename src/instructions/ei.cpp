#include "cpu.h"

namespace gasyboy
{
    void Cpu::EI()
    {
        _registers->setInterruptEnabled(true);
        _interruptManager->setMasterInterrupt(true);
    }
}