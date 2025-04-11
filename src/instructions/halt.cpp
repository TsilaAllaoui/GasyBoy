#include "cpu.h"

namespace gasyboy
{
    void Cpu::HALT()
    {
        bool interruptPending = (_mmu->readRam(0xFF0F) & _mmu->readRam(0xFFFF) & 0x1F) > 0;
        if (!_interruptManager->isMasterInterruptEnabled() && interruptPending)
        {
            // HALT Bug: CPU does not halt, but skips the next opcode fetch
            _haltBug = true;
        }
        else
        {
            _registers->setHalted(true);
        }
    }
}