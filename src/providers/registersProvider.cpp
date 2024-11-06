#include "registersProvider.h"

namespace gasyboy
{
    namespace provider
    {
        std::unique_ptr<Registers> RegistersProvider::_registersInstance = nullptr;

        Registers &RegistersProvider::getInstance()
        {
            if (!_registersInstance)
            {
                _registersInstance = std::make_unique<Registers>();
            }
            return *_registersInstance;
        }

        void RegistersProvider::deleteInstance()
        {
            _registersInstance.reset(nullptr);
        }
    }
}