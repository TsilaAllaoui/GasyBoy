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
            static std::unique_ptr<InterruptManager> _interruptManagerInstance;

            InterruptManagerProvider() = default;

        public:
            ~InterruptManagerProvider() = default;

            InterruptManagerProvider(const InterruptManagerProvider &) = delete;
            InterruptManagerProvider &operator=(const InterruptManagerProvider &) = delete;

            static InterruptManager &getInstance();

            static void deleteInstance();
        };
    }
}

#endif