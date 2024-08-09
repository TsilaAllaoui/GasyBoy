#include "interruptManager.h"

namespace gasyboy
{
    InterruptManager::InterruptManager(Mmu &mmu, Registers &registers)
        : _mmu(mmu),
          _registers(registers),
          _masterInterrupt(false)
    {
        _interruptAddressMap = {
            {InterruptType::VBlank, InterruptAddress::VBlank},
            {InterruptType::LCDStat, InterruptAddress::LCDStat},
            {InterruptType::Timer, InterruptAddress::Timer},
            {InterruptType::Serial, InterruptAddress::Serial}};
    }

    void InterruptManager::handleInterrupts()
    {
        if (_registers.getInterruptEnabled())
        {
            uint8_t req = _mmu.readRam(0xFF0F);
            uint8_t enable = _mmu.readRam(0xFFFF);

            if (req > 0)
            {
                for (int i = 0; i < 5; i++)
                {
                    if (req & (1 << i))
                        if (enable & (1 << i))
                            serviceInterrupt(static_cast<InterruptType>(i));
                }
            }
        }
    }

    void InterruptManager::requestInterrupt(const InterruptType &interrupt)
    {
        uint8_t req = _mmu.readRam(0xFF0F);
        req |= (1 << static_cast<uint8_t>(interrupt));
        _mmu.writeRam(0xFF0F, req);
    }

    void InterruptManager::serviceInterrupt(const InterruptType &interrupt)
    {
        _masterInterrupt = false;
        uint8_t req = _mmu.readRam(0xFF0F);
        req &= ~(1 << static_cast<uint8_t>(interrupt));

        _mmu.writeRam(0xFF0F, req);

        _registers.pushSP(_registers.PC);

        _registers.setHalted(false);

        _registers.PC = static_cast<uint16_t>(_interruptAddressMap[interrupt]);
    }

    bool InterruptManager::isMasterInterruptEnabled()
    {
        return _masterInterrupt;
    }

    void InterruptManager::setMasterInterrupt(const bool &value)
    {
        _masterInterrupt = value;
    }
}