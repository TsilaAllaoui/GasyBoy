#include "mmuProvider.h"
#include "utilitiesProvider.h"
#include "gamepadProvider.h"

namespace gasyboy
{
    namespace provider
    {
        std::unique_ptr<Mmu> MmuProvider::_mmuInstance = nullptr;

        Mmu &MmuProvider::getInstance()
        {
            if (!_mmuInstance)
            {
                _mmuInstance = std::make_unique<Mmu>();
            }
            return *_mmuInstance;
        }

        Mmu &MmuProvider::create(const uint8_t *bytes, const size_t &romSize)
        {
            if (!_mmuInstance)
            {
                _mmuInstance = std::make_unique<Mmu>(bytes, romSize);
            }
            return *_mmuInstance;
        }

        void MmuProvider::create()
        {
            if (!_mmuInstance)
            {
                _mmuInstance = std::make_unique<Mmu>();
            }
        }

        void MmuProvider::deleteInstance()
        {
            _mmuInstance.reset(nullptr);
        }
    }
}