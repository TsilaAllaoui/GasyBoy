#include "utilitiesProvider.h"

namespace gasyboy
{
    namespace provider
    {
        std::unique_ptr<Utilities> UtilitiesProvider::_utilitiesInstance = nullptr;

        Utilities &UtilitiesProvider::getInstance()
        {
            if (!_utilitiesInstance)
            {
                Utilities utilities = {true, "", false};
                _utilitiesInstance = std::make_unique<Utilities>(utilities);
            }
            return *_utilitiesInstance;
        }

        void UtilitiesProvider::deleteInstance()
        {
            _utilitiesInstance.reset(nullptr);
        }
    }
}
