#include "mmuProvider.h"
#include "utilitiesProvider.h"
#include "gamepadProvider.h"

namespace gasyboy
{
    namespace provider
    {
        std::shared_ptr<Mmu> MmuProvider::_mmuInstance = nullptr;

        std::shared_ptr<Mmu> MmuProvider::getInstance()
        {
            if (!_mmuInstance)
            {
                _mmuInstance = std::make_shared<Mmu>();
            }
            return _mmuInstance;
        }

        std::shared_ptr<Mmu> MmuProvider::create(const uint8_t *bytes, const size_t &romSize)
        {
            _mmuInstance.reset();
            _mmuInstance = std::make_shared<Mmu>(bytes, romSize);
            return _mmuInstance;
        }

        void MmuProvider::deleteInstance()
        {
            _mmuInstance.reset();
        }
    }
}