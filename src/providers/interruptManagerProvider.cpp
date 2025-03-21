#include "interruptManagerProvider.h"

namespace gasyboy
{
    namespace provider
    {
        std::shared_ptr<InterruptManager> InterruptManagerProvider::_interruptManagerInstance = nullptr;

        std::shared_ptr<InterruptManager> InterruptManagerProvider::getInstance()
        {
            if (!_interruptManagerInstance)
            {
                _interruptManagerInstance = std::make_shared<InterruptManager>();
            }
            return _interruptManagerInstance;
        }

        void InterruptManagerProvider::deleteInstance()
        {
            _interruptManagerInstance.reset();
        }
    }
}