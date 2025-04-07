#include "utilitiesProvider.h"

namespace gasyboy
{
    namespace provider
    {
        std::shared_ptr<Utilities> UtilitiesProvider::_utilitiesInstance = nullptr;

        std::shared_ptr<Utilities> UtilitiesProvider::getInstance()
        {
            if (!_utilitiesInstance)
            {
                Utilities utilities = {
                    true,
                    "",
                    "",
                    false,
                    false,
                    false,
                };
                _utilitiesInstance = std::make_shared<Utilities>(utilities);
            }
            return _utilitiesInstance;
        }

        void UtilitiesProvider::deleteInstance()
        {
            _utilitiesInstance.reset();
        }
    }
}
