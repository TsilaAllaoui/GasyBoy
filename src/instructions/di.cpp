#include "cpu.h"

namespace gasyboy
{
    void Cpu::DI()
    {
        _registers->setInterruptEnabled(false);
        _interruptManager->setMasterInterrupt(false);
    }
}