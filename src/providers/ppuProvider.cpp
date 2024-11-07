#include "ppuProvider.h"
#include "utilitiesProvider.h"
#include "gamepadProvider.h"

namespace gasyboy
{
    namespace provider
    {
        std::unique_ptr<Ppu> PpuProvider::_ppuInstance = nullptr;

        Ppu &PpuProvider::getInstance()
        {
            if (!_ppuInstance)
            {
                _ppuInstance = std::make_unique<Ppu>();
            }
            return *_ppuInstance;
        }

        void PpuProvider::deleteInstance()
        {
            _ppuInstance.reset(nullptr);
        }
    }
}