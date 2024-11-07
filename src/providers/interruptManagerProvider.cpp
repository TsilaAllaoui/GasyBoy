#include "interruptManagerProvider.h"

namespace gasyboy
{
    namespace provider
    {
        std::unique_ptr<InterruptManager> InterruptManagerProvider::_interruptManagerInstance = nullptr;

        InterruptManager &InterruptManagerProvider::getInstance()
        {
            if (!_interruptManagerInstance)
            {
                _interruptManagerInstance = std::make_unique<InterruptManager>();
            }
            return *_interruptManagerInstance;
        }

        void InterruptManagerProvider::deleteInstance()
        {
            _interruptManagerInstance.reset(nullptr);
        }
    }
}