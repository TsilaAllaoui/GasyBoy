#include "ppuProvider.h"
#include "utilitiesProvider.h"
#include "gamepadProvider.h"

namespace gasyboy
{
    namespace provider
    {
        std::shared_ptr<Ppu> PpuProvider::_ppuInstance = nullptr;

        std::shared_ptr<Ppu> PpuProvider::getInstance()
        {
            if (!_ppuInstance)
            {
                _ppuInstance = std::make_shared<Ppu>();
            }
            return _ppuInstance;
        }

        void PpuProvider::deleteInstance()
        {
            _ppuInstance.reset();
        }
    }
}