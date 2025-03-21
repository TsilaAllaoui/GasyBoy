#ifndef __INTERRUPT_MANAGER_PROVIDER_H__
#define __INTERRUPT_MANAGER_PROVIDER_H__

#include <memory>
#include "interruptManager.h"

namespace gasyboy
{
    namespace provider
    {
        class InterruptManagerProvider
        {
            static std::shared_ptr<InterruptManager> _interruptManagerInstance;

            InterruptManagerProvider() = default;

        public:
            ~InterruptManagerProvider() = default;

            InterruptManagerProvider(const InterruptManagerProvider &) = delete;
            InterruptManagerProvider &operator=(const InterruptManagerProvider &) = delete;

            static std::shared_ptr<InterruptManager> getInstance();

            static void deleteInstance();
        };
    }
}

#endif