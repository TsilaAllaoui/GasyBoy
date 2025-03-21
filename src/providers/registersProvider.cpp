#include "registersProvider.h"

namespace gasyboy
{
    namespace provider
    {
        std::shared_ptr<Registers> RegistersProvider::_registersInstance = nullptr;

        std::shared_ptr<Registers> RegistersProvider::getInstance()
        {
            if (!_registersInstance)
            {
                _registersInstance = std::make_shared<Registers>();
            }
            return _registersInstance;
        }

        void RegistersProvider::deleteInstance()
        {
            _registersInstance.reset();
        }
    }
}