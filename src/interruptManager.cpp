#include "registersProvider.h"
#include "interruptManager.h"
#include "mmuProvider.h"
#include "mmuProvider.h"

namespace gasyboy
{
    InterruptManager::InterruptManager()
        : _mmu(provider::MmuProvider::getInstance()),
          _registers(provider::RegistersProvider::getInstance()),
          _masterInterrupt(false)
    {
        _interruptAddressMap = {
            {InterruptType::VBlank, InterruptAddress::VBlank},
            {InterruptType::LCDStat, InterruptAddress::LCDStat},
            {InterruptType::Timer, InterruptAddress::Timer},
            {InterruptType::Serial, InterruptAddress::Serial}};
    }

    InterruptManager &InterruptManager::operator=(const InterruptManager &other)
    {
        _mmu = provider::MmuProvider::getInstance();             // Reacquire MMU instance
        _registers = provider::RegistersProvider::getInstance(); // Reacquire Registers
        _masterInterrupt = false;                                // Reset interrupt state

        // Reinitialize the interrupt address map
        _interruptAddressMap = {
            {InterruptType::VBlank, InterruptAddress::VBlank},
            {InterruptType::LCDStat, InterruptAddress::LCDStat},
            {InterruptType::Timer, InterruptAddress::Timer},
            {InterruptType::Serial, InterruptAddress::Serial}};

        std::cout << "InterruptManager reset complete.\n";
        return *this;
    }

    void InterruptManager::handleInterrupts()
    {
        if (!_masterInterrupt)
            return; // Ensure master interrupt is enabled

        if (_registers->getInterruptEnabled())
        {
            uint8_t req = _mmu->readRam(0xFF0F);
            uint8_t enable = _mmu->readRam(0xFFFF);

            for (int i = 0; i < 5; i++)
            {
                if ((req & (1 << i)) && (enable & (1 << i))) // Check request & enable bit
                {
                    serviceInterrupt(static_cast<InterruptType>(i));
                    break; // The CPU only handles one interrupt at a time
                }
            }
        }
    }

    void InterruptManager::requestInterrupt(const InterruptType &interrupt)
    {
        uint8_t req = _mmu->readRam(0xFF0F);
        req |= (1 << static_cast<uint8_t>(interrupt));
        _mmu->writeRam(0xFF0F, req);
    }

    void InterruptManager::serviceInterrupt(const InterruptType &interrupt)
    {
        if (!_registers->getInterruptEnabled())
            return;

        _masterInterrupt = false;

        uint8_t req = _mmu->readRam(0xFF0F);
        req &= ~(1 << static_cast<uint8_t>(interrupt));
        _mmu->writeRam(0xFF0F, req);

        _registers->pushSP(_registers->PC); // Save current PC
        _registers->setHalted(false);       // Wake up CPU

        _registers->PC = static_cast<uint16_t>(_interruptAddressMap[interrupt]);
    }

    bool InterruptManager::isMasterInterruptEnabled()
    {
        return _masterInterrupt;
    }

    void InterruptManager::setMasterInterrupt(const bool &value)
    {
        _masterInterrupt = value;
    }

    void InterruptManager::reset()
    {
        _masterInterrupt = false;
    }
}